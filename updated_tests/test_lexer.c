#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

static void assert_token(t_tokensll *tok, t_token_type type, const char *val)
{
	cr_assert_not_null(tok);
	cr_assert_eq(tok->type, type);
	cr_assert_not_null(tok->val);
	cr_assert_str_eq(tok->val, val);
}

Test(lexer, tokens_simple_words)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo hi");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);

	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "hi");

	freetokensll(tokens);
}

Test(lexer, tokens_with_pipe_and_redirs)
{
	t_tokensll *tokens;
	t_tokensll *tok;

	tokens = build_tokensll("cat < in | grep x >> out");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 8);

	tok = tokens;
	assert_token(tok, TOK_STR, "cat");
	assert_token(tok->next, TOK_DIRIN, "<");
	assert_token(tok->next->next, TOK_STR, "in");
	assert_token(tok->next->next->next, TOK_PIPE, "|");
	assert_token(tok->next->next->next->next, TOK_STR, "grep");
	assert_token(tok->next->next->next->next->next, TOK_STR, "x");
	assert_token(tok->next->next->next->next->next->next, TOK_DIRAPPND, ">>");
	assert_token(tok->next->next->next->next->next->next->next, TOK_STR, "out");

	freetokensll(tokens);
}

Test(lexer, tokens_no_spaces)
{
	t_tokensll *tokens;
	t_tokensll *tok;

	tokens = build_tokensll("a|b>>c<<d>e<f");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 11);

	tok = tokens;
	assert_token(tok, TOK_STR, "a");
	assert_token(tok->next, TOK_PIPE, "|");
	assert_token(tok->next->next, TOK_STR, "b");
	assert_token(tok->next->next->next, TOK_DIRAPPND, ">>");
	assert_token(tok->next->next->next->next, TOK_STR, "c");
	assert_token(tok->next->next->next->next->next, TOK_HEREDOC, "<<");
	assert_token(tok->next->next->next->next->next->next, TOK_STR, "d");
	assert_token(tok->next->next->next->next->next->next->next, TOK_DIROUT, ">" );
	assert_token(tok->next->next->next->next->next->next->next->next, TOK_STR, "e");
	assert_token(tok->next->next->next->next->next->next->next->next->next, TOK_DIRIN, "<");
	assert_token(tok->next->next->next->next->next->next->next->next->next->next, TOK_STR, "f");

	freetokensll(tokens);
}

Test(lexer, unterminated_quote_returns_null)
{
	t_tokensll *tokens;

	tokens = build_tokensll("'abc");
	cr_assert_null(tokens);
}

Test(lexer, single_quotes_preserved_for_expansion)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo 'a b'");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "'a b'");

	freetokensll(tokens);
}

Test(lexer, double_quotes_preserved_for_expansion)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo \"a\\\"b\"");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "\"a\\\"b\"");

	freetokensll(tokens);
}

Test(lexer, backslash_escapes_space_outside_quotes)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo a\\ b");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "a b");

	freetokensll(tokens);
}

Test(lexer, variables_are_not_expanded)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo $HOME");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "$HOME");

	freetokensll(tokens);
}

Test(lexer, consecutive_whitespace_is_ignored)
{
	t_tokensll *tokens;

	tokens = build_tokensll("ls    -l\t\t-a");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 3);
	assert_token(tokens, TOK_STR, "ls");
	assert_token(tokens->next, TOK_STR, "-l");
	assert_token(tokens->next->next, TOK_STR, "-a");
	freetokensll(tokens);
}

Test(lexer, escaped_quotes_outside_quotes)
{
	t_tokensll *tokens;

	tokens = build_tokensll("echo \\\"a\\\"");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "\"a\"");
	freetokensll(tokens);
}

Test(lexer, invalid_operators_tokenize_separately)
{
	t_tokensll *tokens;

	tokens = build_tokensll(">>>");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens, TOK_DIRAPPND, ">>");
	assert_token(tokens->next, TOK_DIROUT, ">");
	freetokensll(tokens);

	tokens = build_tokensll("|||");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 3);
	assert_token(tokens, TOK_PIPE, "|");
	assert_token(tokens->next, TOK_PIPE, "|");
	assert_token(tokens->next->next, TOK_PIPE, "|");
	freetokensll(tokens);
}

Test(lexer, quoted_strings_stay_single_tokens)
{
	t_tokensll	*tokens;

	tokens = build_tokensll("echo 'hello world' \"good bye\"");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 3);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "'hello world'");
	assert_token(tokens->next->next, TOK_STR, "\"good bye\"");
	freetokensll(tokens);
}

Test(lexer, empty_quoted_strings_are_tokens)
{
	t_tokensll	*tokens;

	tokens = build_tokensll("echo \"\" ''");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 3);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "\"\"");
	assert_token(tokens->next->next, TOK_STR, "''");
	freetokensll(tokens);
}

Test(lexer, variables_are_not_expanded_by_lexer)
{
	t_tokensll	*tokens;

	tokens = build_tokensll("echo $HOME \"$USER\" '$PATH' $?");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 5);
	assert_token(tokens, TOK_STR, "echo");
	assert_token(tokens->next, TOK_STR, "$HOME");
	assert_token(tokens->next->next, TOK_STR, "\"$USER\"");
	assert_token(tokens->next->next->next, TOK_STR, "'$PATH'");
	assert_token(tokens->next->next->next->next, TOK_STR, "$?");
	freetokensll(tokens);
}

Test(lexer, unterminated_quotes_return_null)
{
	cr_assert_null(build_tokensll("'abc"));
	cr_assert_null(build_tokensll("\"abc"));
}

Test(lexer, empty_or_space_only_returns_null)
{
	cr_assert_null(build_tokensll(""));
	cr_assert_null(build_tokensll("    \t\n  "));
}

Test(lexer, invalid_pipe_sequences_are_visible_to_syntax_validator)
{
	t_tokensll	*tokens;

	tokens = build_tokensll("echo hello || cat");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 5);
	assert_token(tokens->next->next, TOK_PIPE, "|");
	assert_token(tokens->next->next->next, TOK_PIPE, "|");
	freetokensll(tokens);

	tokens = build_tokensll("echo hello | | cat");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 5);
	assert_token(tokens->next->next, TOK_PIPE, "|");
	assert_token(tokens->next->next->next, TOK_PIPE, "|");
	freetokensll(tokens);
}

Test(lexer, redirection_operator_sequences_are_visible_to_syntax_validator)
{
	t_tokensll	*tokens;

	tokens = build_tokensll("echo hello > > out");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 5);
	assert_token(tokens->next->next, TOK_DIROUT, ">");
	assert_token(tokens->next->next->next, TOK_DIROUT, ">");
	freetokensll(tokens);

	tokens = build_tokensll("cat <<");
	cr_assert_not_null(tokens);
	cr_assert_eq(ft_sllsize(tokens), 2);
	assert_token(tokens->next, TOK_HEREDOC, "<<");
	freetokensll(tokens);
}

