#include "minishell.h"

static void	free_argv(char **argv)
{
	size_t	i;

	if (!argv)
		return ;
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

static void	free_redirs(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
		free(redir->target);
		free(redir);
		redir = next;
	}
}

t_ast_node	*ast_new_cmd(char **argv, t_redir *redirs, t_span span)
{
	t_ast_node	*node;

	node = malloc(sizeof(t_ast_node));
	if (!node)
		return (printerr_syscall(ERR_MALLOC), NULL);
	node->type = NODE_CMD;
	node->span = span;
	node->content.cmd.argv = argv;
	node->content.cmd.redirs = redirs;
	return (node);
}

t_ast_node	*ast_new_binop(t_binop_type op, t_ast_node *left,
			t_ast_node *right, t_span span)
{
	t_ast_node	*node;

	node = malloc(sizeof(t_ast_node));
	if (!node)
		return (printerr_syscall(ERR_MALLOC), NULL);
	node->type = NODE_BINOP;
	node->span = span;
	node->content.binop.op = op;
	node->content.binop.left = left;
	node->content.binop.right = right;
	return (node);
}

void	ast_free(t_ast_node *node)
{
	if (!node)
		return ;
	if (node->type == NODE_CMD)
	{
		free_argv(node->content.cmd.argv);
		free_redirs(node->content.cmd.redirs);
	}
	else if (node->type == NODE_BINOP)
	{
		ast_free(node->content.binop.left);
		ast_free(node->content.binop.right);
	}
	free(node);
}
