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

static int	is_valid_assignment(const char *arg, char **name, char **value)
{
	int	i;
	int	len;

	i = 0;
	if (!arg || (!is_name_start(arg[0])))
		return (0);
	while (arg[i] && arg[i] != '=')
	{
		if (arg[i] != '_' && (arg[i] < 'A' || arg[i] > 'Z')
			&& (arg[i] < 'a' || arg[i] > 'z') && (arg[i] < '0' || arg[i] > '9'))
			return (0);
		i++;
	}
	if (arg[i] != '=')
		return (0);
	*name = ft_strndup((char *)arg, i);
	if (!*name)
		return (0);
	len = ft_strlen(arg + i + 1);
	if (len > 0 && arg[i + 1 + len - 1] == ';')
		*value = ft_strndup((char *)arg + i + 1, len - 1);
	else
		*value = ft_strdup(arg + i + 1);
	return (*value != NULL || (free(*name), 0));
}

static int	apply_assignment(t_app *app, char *name, char *raw, int last_status)
{
	char	*val;
	char	**w;
	size_t	c;

	if (expand_word(raw, app->envp, last_status, &w, &c) == 0)
	{
		if (c > 0 && w[0])
			val = ft_strdup(w[0]);
		else
			val = ft_strdup("");
		freelst(w);
	}
	else
		val = ft_strdup("");
	free(raw);
	if (env_set(&app->env_list, name, val) != 0)
	{
		free(name);
		free(val);
		return (ERR_MALLOC);
	}
	free(name);
	free(val);
	return (update_env_array(app));
}

static int	process_assignments(t_app *app, t_cmd_node *cmd, int last_status)
{
	char	*name;
	char	*raw_val;
	int		j;

	while (cmd->argv && cmd->argv[0])
	{
		if (!is_valid_assignment(cmd->argv[0], &name, &raw_val))
			break ;
		if (apply_assignment(app, name, raw_val, last_status) != 0)
			return (ERR_MALLOC);
		free(cmd->argv[0]);
		j = 0;
		while (cmd->argv[j])
		{
			cmd->argv[j] = cmd->argv[j + 1];
			j++;
		}
	}
	return (0);
}

static int	expand_cmd_node(t_app *app, t_cmd_node *cmd)
{
	char		**expanded;
	int			res;
	char		**envp;
	int			last_status;

	last_status = app->exitcode;
	if (process_assignments(app, cmd, last_status) != 0)
		return (ERR_MALLOC);
	envp = app->envp;
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
