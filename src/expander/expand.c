#include "minishell.h"

int	expand_word(const char *input, char **envp, int last_status,
		char ***out_words, size_t *out_count)
{
	t_expand_ctx	ctx;
	int				res;

	if (!out_words || !out_count || init_ctx(&ctx, input, envp, last_status) != 0)
		return (ERR_MALLOC);
	while (input && input[ctx.i])
	{
		res = expand_step(&ctx);
		if (res < 0)
			return (wl_free(&ctx.wl), sb_free(&ctx.sb), res);
	}
	if (ctx.word_in_progress && flush_word(&ctx.wl, &ctx.sb, 1) != 0)
		return (wl_free(&ctx.wl), sb_free(&ctx.sb), ERR_MALLOC);
	*out_words = ctx.wl.items;
	*out_count = ctx.wl.count;
	sb_free(&ctx.sb);
	return (0);
}

int	expand_argv(char **argv, char **envp, int last_status, char ***out_argv)
{
	t_argv_builder	ab;
	char			**words;
	size_t			count;
	size_t			i;

	if (!out_argv || argv_builder_init(&ab) != 0)
		return (ERR_MALLOC);
	i = 0;
	while (argv && argv[i])
	{
		if (expand_word(argv[i], envp, last_status, &words, &count) != 0)
			return (argv_builder_free(&ab), ERR_MALLOC);
		if (push_words_to_builder(&ab, words, count) != 0)
			return (argv_builder_free(&ab), ERR_MALLOC);
		i++;
	}
	*out_argv = ab.argv;
	return (0);
}

int	expand_redirs(t_redir *redir, char **envp, int last_status)
{
	char	**words;
	size_t	count;

	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			redir = redir->next;
			continue ;
		}
		if (expand_word(redir->target, envp, last_status, &words, &count) != 0)
			return (ERR_MALLOC);
		if (count != 1 || words[0][0] == '\0')
		{
			freelst(words);
			return (errmsg(NULL, NULL, "ambiguous redirect"), ERR_CMDNEXEC);
		}
		free(redir->target);
		redir->target = words[0];
		free(words);
		redir = redir->next;
	}
	return (0);
}

// t_cmd_node *cmd, char **envp, int last_status
static int	expand_cmd_node(t_app *app, t_cmd_node *cmd)
{
	char		**expanded;
	int			res;
	char		**envp;
	int			last_status;

	envp = app->envp;
	last_status = app->exitcode;
	if (expand_argv(cmd->argv, envp, last_status, &expanded) != 0)
		return (ERR_MALLOC);
	freelst(cmd->argv);
	cmd->argv = expanded;
	if (!cmd->argv[0])
	{
		freelst(cmd->argv);
		cmd->argv = NULL;
	}
	res = expand_redirs(cmd->redirs, envp, last_status);
	if (res != 0)
		setexit(app, EX_ERR);
	return (res);
}

int	expand_ast(t_app *app, t_ast_node *node)
{
	int	res;

	if (!node)
		return (0);
	if (node->type == NODE_CMD)
		return (expand_cmd_node(app, &node->content.cmd));
	if (node->type == NODE_BINOP)
	{
		res = expand_ast(app, node->content.binop.left);
		if (res != 0)
			return (res);
		return (expand_ast(app, node->content.binop.right));
	}
	return (0);
}
