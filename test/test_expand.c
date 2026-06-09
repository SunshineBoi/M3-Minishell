#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

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
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};
	t_shell shell = {envp, 42};

	cr_assert_eq(expand_word("$?", &shell, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "42");
	free_words(words);
}

Test(expand, unset_variable_is_removed_unquoted)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=ok", NULL};
	t_shell shell = {envp, 0};

	cr_assert_eq(expand_word("$NOPE", &shell, &words, &count), 0);
	cr_assert_eq(count, 0);
	cr_assert_null(words[0]);
	free_words(words);
}

Test(expand, quote_rules)
{
	char **words;
	size_t count;
	char *envp[] = {"VAR=hi", NULL};
	t_shell shell = {envp, 0};

	cr_assert_eq(expand_word("\"$VAR\"", &shell, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "hi");
	free_words(words);

	cr_assert_eq(expand_word("'$VAR'", &shell, &words, &count), 0);
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
	t_shell shell = {envp, 0};

	cr_assert_eq(expand_word("$VAR", &shell, &words, &count), 0);
	cr_assert_eq(count, 3);
	assert_words(words, 3, expected);
	free_words(words);

	cr_assert_eq(expand_word("\"$VAR\"", &shell, &words, &count), 0);
	cr_assert_eq(count, 1);
	cr_assert_str_eq(words[0], "a b c");
	free_words(words);
}

Test(expand, argv_expansion)
{
	char *argv[] = {"echo", "$VAR", NULL};
	char **expanded;
	char *envp[] = {"VAR=a b", NULL};
	t_shell shell = {envp, 0};

	cr_assert_eq(expand_argv(argv, &shell, &expanded), 0);
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
	t_shell shell = {envp1, 0};

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, &shell), 0);
	cr_assert_str_eq(r1->target, "out.txt");
	redir_free(r1);

	r2 = redir_new(REDIR_HEREDOC, "$FILE");
	cr_assert_not_null(r2);
	cr_assert_eq(expand_redirs(r2, &shell), 0);
	cr_assert_str_eq(r2->target, "$FILE");
	redir_free(r2);
}

Test(expand, redirection_ambiguous_error)
{
	t_redir *r1;
	char *envp2[] = {"FILE=a b", NULL};
	char *envp3[] = {"FILE=", NULL};
	t_shell shell2 = {envp2, 0};
	t_shell shell3 = {envp3, 0};

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, &shell2), ERR_CMDNEXEC);
	redir_free(r1);

	r1 = redir_new(REDIR_OUT, "$FILE");
	cr_assert_not_null(r1);
	cr_assert_eq(expand_redirs(r1, &shell3), ERR_CMDNEXEC);
	redir_free(r1);
}
