/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_assignment.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 19:30:09 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 19:31:10 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	init_assign_ctx(t_assign_ctx *ctx, t_app *app, t_cmd_node *cmd,
		int last_status)
{
	ctx->app = app;
	ctx->last_status = last_status;
	ctx->count = assignment_prefix_count(cmd->argv);
	ctx->has_cmd = (cmd->argv[ctx->count] != NULL);
	ctx->tmp_env = NULL;
	if (ctx->count == 0)
		return (0);
	if (ctx->has_cmd)
		ctx->tmp_env = env_init(app->envp);
	return (0);
}

static int	expand_assignment_value(t_assign_ctx *ctx, char *raw, char **out)
{
	t_wordlist	wl;

	*out = NULL;
	if (expand_word(raw, ctx->app->envp, ctx->last_status, &wl) == 0)
	{
		if (wl.count > 0 && wl.items[0])
			*out = ft_strdup(wl.items[0]);
		else
			*out = ft_strdup("");
		freelst(wl.items);
	}
	else
		*out = ft_strdup("");
	free(raw);
	if (!*out)
		return (ERR_MALLOC);
	return (0);
}

static int	apply_assignment(t_assign_ctx *ctx, t_env **env, char *name,
		char *raw)
{
	char	*val;

	if (expand_assignment_value(ctx, raw, &val) != 0)
		return (free(name), ERR_MALLOC);
	if (env_set(env, name, val) != 0)
	{
		free(name);
		free(val);
		return (ERR_MALLOC);
	}
	free(name);
	free(val);
	return (0);
}

static int	consume_assignment(t_assign_ctx *ctx, t_cmd_node *cmd)
{
	char	*name;
	char	*raw_val;
	t_env	**env;

	if (!is_valid_assignment(cmd->argv[0], &name, &raw_val))
		return (1);
	env = &ctx->app->env_list;
	if (ctx->has_cmd)
		env = &ctx->tmp_env;
	if (apply_assignment(ctx, env, name, raw_val) != 0)
	{
		if (ctx->has_cmd)
			env_free(ctx->tmp_env);
		return (ERR_MALLOC);
	}
	free(cmd->argv[0]);
	shift_argv_left(cmd->argv);
	return (0);
}

int	process_assignments(t_app *app, t_cmd_node *cmd, int last_status)
{
	t_assign_ctx	ctx;
	int				i;
	int				res;

	if (init_assign_ctx(&ctx, app, cmd, last_status) != 0 || ctx.count == 0)
		return (0);
	i = 0;
	while (i++ < ctx.count)
	{
		res = consume_assignment(&ctx, cmd);
		if (res == 1)
			break ;
		if (res < 0)
			return (res);
	}
	if (!ctx.has_cmd)
		return (update_env_array(app));
	cmd->envp = env_to_array(ctx.tmp_env);
	env_free(ctx.tmp_env);
	if (!cmd->envp)
		return (ERR_MALLOC);
	return (0);
}
