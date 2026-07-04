#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

static void free_words(char **words)
{
	size_t i;

	if (!words)
		return;
	i = 0;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

static void assert_words(char **words, size_t count, const char **expected)
{
	size_t i;

	cr_assert_not_null(words);
	for (i = 0; i < count; i++)
	{
		cr_assert_not_null(words[i]);
		cr_assert_str_eq(words[i], expected[i]);
	}
	cr_assert_null(words[count]);
}



Test(expand, expands_exit_status)
{
	t_wordlist wl;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$?", envp, 42, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "42");
	free_words(wl.items);
}

Test(expand, unset_variable_is_removed_unquoted)
{
	t_wordlist wl;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$NOPE", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 0);
	cr_assert_null(wl.items[0]);
	free_words(wl.items);
}

Test(expand, quote_rules)
{
	t_wordlist wl;
	char *envp[] = {"VAR=hi", NULL};

	cr_assert_eq(expand_word("\"$VAR\"", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "hi");
	free_words(wl.items);

	cr_assert_eq(expand_word("'$VAR'", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "$VAR");
	free_words(wl.items);
}

Test(expand, field_splitting)
{
	t_wordlist wl;
	const char *expected[] = {"a", "b", "c"};
	char *envp[] = {"VAR=a b c", NULL};

	cr_assert_eq(expand_word("$VAR", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 3);
	assert_words(wl.items, 3, expected);
	free_words(wl.items);

	cr_assert_eq(expand_word("\"$VAR\"", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "a b c");
	free_words(wl.items);
}

Test(expand, argv_expansion)
{
	char *argv[] = {"echo", "$VAR", NULL};
	char **expanded;
	char *envp[] = {"VAR=a b", NULL};

	cr_assert_eq(expand_argv(argv, envp, 0, &expanded), 0);
	cr_assert_str_eq(expanded[0], "echo");
	cr_assert_str_eq(expanded[1], "a");
	cr_assert_str_eq(expanded[2], "b");
	cr_assert_null(expanded[3]);

	free_words(expanded);
}

Test(expand, redirection_target_expansion)
{
	t_redir *r1;
	t_redir *r2;
	char *envp1[] = {"FILE=out.txt", NULL};

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, envp1, 0), 0);
	cr_assert_str_eq(r1->target, "out.txt");
	redir_free(r1);

	r2 = redir_new(REDIR_HEREDOC, "$FILE");
	cr_assert_not_null(r2);
	cr_assert_eq(expand_redirs(r2, envp1, 0), 0);
	cr_assert_str_eq(r2->target, "$FILE");
	redir_free(r2);
}

Test(expand, redirection_ambiguous_error)
{
	t_redir *r1;
	char *envp2[] = {"FILE=a b", NULL};
	char *envp3[] = {"FILE=", NULL};

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, envp2, 0), ERR_CMDNEXEC);
	redir_free(r1);

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, envp3, 0), ERR_CMDNEXEC);
	redir_free(r1);
}

Test(expand, literal_dollar_and_positional_are_handled)
{
	t_wordlist wl;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$-", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "$-");
	free_words(wl.items);

	cr_assert_eq(expand_word("$1", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 0);
	cr_assert_null(wl.items[0]);
	free_words(wl.items);
}

Test(expand, double_quote_backslash_rules)
{
	t_wordlist wl;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("\"a\\\"b\"", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "a\"b");
	free_words(wl.items);

	cr_assert_eq(expand_word("\"\\$VAR\"", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "$VAR");
	free_words(wl.items);
}

Test(expand, quoted_and_unquoted_segments_join_one_word)
{
	t_wordlist wl;
	char *envp[] = {"USER=marvin", "EMPTY=", NULL};

	cr_assert_eq(expand_word("pre\"$USER\"'!'post", envp, 0,
			&wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "premarvin!post");
	free_words(wl.items);

	cr_assert_eq(expand_word("a\"\"b''c$EMPTY", envp, 0,
			&wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "abc");
	free_words(wl.items);
}

Test(expand, variable_name_boundaries)
{
	t_wordlist wl;
	char *envp[] = {"A=one", "A1=two", "_X=under", NULL};

	cr_assert_eq(expand_word("$A1:$A-$_X", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "two:one-under");
	free_words(wl.items);

	cr_assert_eq(expand_word("${A}", envp, 0, &wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "${A}");
	free_words(wl.items);
}

Test(expand, unquoted_empty_middle_variable_does_not_split_word)
{
	t_wordlist wl;
	char *envp[] = {"EMPTY=", "WORD=mid", NULL};

	cr_assert_eq(expand_word("pre$EMPTY$WORD-post", envp, 0,
			&wl), 0);
	cr_assert_eq(wl.count, 1);
	cr_assert_str_eq(wl.items[0], "premid-post");
	free_words(wl.items);
}
