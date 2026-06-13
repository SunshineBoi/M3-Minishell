#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

int g_malloc_fail_countdown = -1;

void * __real_malloc(size_t size);
void * __wrap_malloc(size_t size)
{
	if (g_malloc_fail_countdown == 0)
	{
		g_malloc_fail_countdown = -1;
		return NULL;
	}
	if (g_malloc_fail_countdown > 0)
		g_malloc_fail_countdown--;
	return __real_malloc(size);
}

static char **make_argv1(const char *a)
{
	char **argv;

	argv = malloc(sizeof(char *) * 2);
	argv[0] = strdup(a);
	argv[1] = NULL;
	return argv;
}

static char **make_argv2(const char *a, const char *b)
{
	char **argv;

	argv = malloc(sizeof(char *) * 3);
	argv[0] = strdup(a);
	argv[1] = strdup(b);
	argv[2] = NULL;
	return argv;
}

static t_redir *make_redir(const char *target, t_redir_type type)
{
	t_redir *redir;

	redir = malloc(sizeof(t_redir));
	redir->type = type;
	redir->target = strdup(target);
	redir->fd = -1;
	redir->next = NULL;
	return redir;
}

static void free_argv_test(char **argv)
{
	size_t i;

	if (!argv)
		return;
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

static void free_redir_test(t_redir *redir)
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

Test(ast, ast_new_cmd_sets_fields)
{
	t_span span;
	char **argv;
	t_redir *redir;
	t_ast_node *node;

	span.start = 1;
	span.end = 5;
	argv = make_argv2("echo", "hi");
	redir = make_redir("out.txt", REDIR_OUT);

	node = ast_new_cmd(argv, redir, span);
	cr_assert_not_null(node);
	cr_assert_eq(node->type, NODE_CMD);
	cr_assert_eq(node->span.start, 1);
	cr_assert_eq(node->span.end, 5);
	cr_assert_eq(node->content.cmd.argv, argv);
	cr_assert_eq(node->content.cmd.redirs, redir);

	ast_free(node);
}

Test(ast, ast_new_binop_sets_fields)
{
	t_ast_node *left;
	t_ast_node *right;
	t_ast_node *node;
	t_span span;

	left = ast_new_cmd(make_argv1("ls"), NULL, (t_span){0, 1});
	right = ast_new_cmd(make_argv1("wc"), NULL, (t_span){3, 4});
	span.start = 0;
	span.end = 4;

	node = ast_new_binop(BIN_PIPE, left, right, span);
	cr_assert_not_null(node);
	cr_assert_eq(node->type, NODE_BINOP);
	cr_assert_eq(node->content.binop.op, BIN_PIPE);
	cr_assert_eq(node->content.binop.left, left);
	cr_assert_eq(node->content.binop.right, right);
	cr_assert_eq(node->span.start, 0);
	cr_assert_eq(node->span.end, 4);

	ast_free(node);
}

Test(ast, ast_free_allows_null)
{
	ast_free(NULL);
	cr_assert(true);
}

Test(ast, ast_new_cmd_malloc_fail_returns_null)
{
	t_ast_node *node;
	char **argv;
	t_redir *redir;

	argv = make_argv2("echo", "hi");
	redir = make_redir("out.txt", REDIR_OUT);

	g_malloc_fail_countdown = 0;
	node = ast_new_cmd(argv, redir, (t_span){0, 0});
	g_malloc_fail_countdown = -1;
	cr_assert_null(node);

	free_argv_test(argv);
	free_redir_test(redir);
}
