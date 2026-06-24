#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

static t_tokensll *test_build_tokensll(char *str)
{
	t_app	app;

	app = (t_app){0};
	return (build_tokensll(&app, str));
}

static void assert_argv(char **argv, const char **expected, size_t count)
{
	size_t i;

	cr_assert_not_null(argv);
	i = 0;
	while (i < count)
	{
		cr_assert_not_null(argv[i]);
		cr_assert_str_eq(argv[i], expected[i]);
		i++;
	}
	cr_assert_null(argv[i]);
}

static void assert_redir(t_redir *redir, t_redir_type type, const char *target)
{
	cr_assert_not_null(redir);
	cr_assert_eq(redir->type, type);
	cr_assert_not_null(redir->target);
	cr_assert_str_eq(redir->target, target);
}

Test(parser, parse_simple_command)
{
	t_tokensll *tokens;
	t_ast_node *root;
	const char *argv_expected[] = {"echo", "hi", NULL};

	tokens = test_build_tokensll("echo hi");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	assert_argv(root->content.cmd.argv, argv_expected, 2);
	cr_assert_null(root->content.cmd.redirs);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_pipeline)
{
	t_tokensll *tokens;
	t_ast_node *root;
	const char *left_argv[] = {"echo", "hi", NULL};
	const char *right_argv[] = {"wc", "-l", NULL};

	tokens = test_build_tokensll("echo hi | wc -l");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_BINOP);
	cr_assert_eq(root->content.binop.op, BIN_PIPE);
	cr_assert_eq(root->content.binop.left->type, NODE_CMD);
	cr_assert_eq(root->content.binop.right->type, NODE_CMD);
	assert_argv(root->content.binop.left->content.cmd.argv, left_argv, 2);
	assert_argv(root->content.binop.right->content.cmd.argv, right_argv, 2);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_redirections_interleaved)
{
	t_tokensll *tokens;
	t_ast_node *root;
	t_redir *r;
	const char *argv_expected[] = {"cat", "out", NULL};

	tokens = test_build_tokensll("cat < in out > final");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	assert_argv(root->content.cmd.argv, argv_expected, 2);

	r = root->content.cmd.redirs;
	assert_redir(r, REDIR_IN, "in");
	assert_redir(r->next, REDIR_OUT, "final");
	cr_assert_null(r->next->next);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_multiple_redirections)
{
	t_tokensll *tokens;
	t_ast_node *root;
	t_redir *r;
	const char *argv_expected[] = {"cmd", NULL};

	tokens = test_build_tokensll("cmd < in > out >> append");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	assert_argv(root->content.cmd.argv, argv_expected, 1);

	r = root->content.cmd.redirs;
	assert_redir(r, REDIR_IN, "in");
	assert_redir(r->next, REDIR_OUT, "out");
	assert_redir(r->next->next, REDIR_APPEND, "append");
	cr_assert_null(r->next->next->next);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_invalid_syntax)
{
	t_tokensll *tokens;
	t_ast_node *root;

	tokens = test_build_tokensll("ls |");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_null(root);
	freetokensll(tokens);

	tokens = test_build_tokensll("| ls");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_null(root);
	freetokensll(tokens);

	tokens = test_build_tokensll("echo >");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_null(root);
	freetokensll(tokens);

	tokens = test_build_tokensll("cat < > out");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_null(root);
	freetokensll(tokens);
}

Test(parser, parse_redirection_only_command)
{
	t_tokensll *tokens;
	t_ast_node *root;
	t_redir *r;

	tokens = test_build_tokensll("> out");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	cr_assert_not_null(root->content.cmd.argv);
	cr_assert_null(root->content.cmd.argv[0]);
	r = root->content.cmd.redirs;
	cr_assert_not_null(r);
	cr_assert_eq(r->type, REDIR_OUT);
	cr_assert_str_eq(r->target, "out");
	cr_assert_null(r->next);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_empty_quoted_command)
{
	t_tokensll *tokens;
	t_ast_node *root;

	tokens = test_build_tokensll("\"\"");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	cr_assert_str_eq(root->content.cmd.argv[0], "\"\"");
	cr_assert_null(root->content.cmd.argv[1]);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_heredoc_redirection)
{
	t_tokensll *tokens;
	t_ast_node *root;
	t_redir *r;
	const char *argv_expected[] = {"cat", NULL};

	tokens = test_build_tokensll("cat << EOF");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_CMD);
	assert_argv(root->content.cmd.argv, argv_expected, 1);
	r = root->content.cmd.redirs;
	assert_redir(r, REDIR_HEREDOC, "EOF");
	cr_assert_null(r->next);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_redirection_only_pipeline_command)
{
	t_tokensll *tokens;
	t_ast_node *root;
	t_ast_node *right;
	t_redir *r;
	const char *left_argv[] = {"echo", "hi", NULL};

	tokens = test_build_tokensll("echo hi | > out");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_not_null(root);
	cr_assert_eq(root->type, NODE_BINOP);
	cr_assert_eq(root->content.binop.op, BIN_PIPE);
	assert_argv(root->content.binop.left->content.cmd.argv, left_argv, 2);
	right = root->content.binop.right;
	cr_assert_eq(right->type, NODE_CMD);
	cr_assert_not_null(right->content.cmd.argv);
	cr_assert_null(right->content.cmd.argv[0]);
	r = right->content.cmd.redirs;
	assert_redir(r, REDIR_OUT, "out");
	cr_assert_null(r->next);

	ast_free(root);
	freetokensll(tokens);
}

Test(parser, parse_rejects_double_pipe)
{
	t_tokensll *tokens;
	t_ast_node *root;

	tokens = test_build_tokensll("echo hi || wc");
	cr_assert_not_null(tokens);
	root = parse_tokens(tokens);
	cr_assert_null(root);
	freetokensll(tokens);
}
