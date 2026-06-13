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

static t_redir *redir_new(t_redir_type type, const char *target)
{
	t_redir *redir;

	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->type = type;
	redir->target = strdup(target);
	redir->fd = -1;
	redir->next = NULL;
	return (redir);
}

static void redir_free(t_redir *redir)
{
	t_redir *next;

	while (redir)
	{
		next = redir->next;
		free(redir->target);
		free(redir);
		redir = next;
	}
}

Test(expand, expands_exit_status)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$?", envp, 42, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "42");
	free_words(words);
}

Test(expand, unset_variable_is_removed_unquoted)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$NOPE", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 0);
	cr_assert_null(words[0]);
	free_words(words);
}

Test(expand, quote_rules)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=hi", NULL};

	cr_assert_eq(expand_word("\"$VAR\"", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "hi");
	free_words(words);

	cr_assert_eq(expand_word("'$VAR'", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "$VAR");
	free_words(words);
}

Test(expand, field_splitting)
{
	char **words;
	size_t count;
	const char *expected[] = {"a", "b", "c"};
	char *envp[] = {"VAR=a b c", NULL};

	cr_assert_eq(expand_word("$VAR", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 3);
	assert_words(words, 3, expected);
	free_words(words);

	cr_assert_eq(expand_word("\"$VAR\"", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "a b c");
	free_words(words);
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
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("$-", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "$-");
	free_words(words);

	cr_assert_eq(expand_word("$1", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 0);
	cr_assert_null(words[0]);
	free_words(words);
}

Test(expand, double_quote_backslash_rules)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};

	cr_assert_eq(expand_word("\"a\\\"b\"", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "a\"b");
	free_words(words);

	cr_assert_eq(expand_word("\"\\$VAR\"", envp, 0, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "$VAR");
	free_words(words);
}
