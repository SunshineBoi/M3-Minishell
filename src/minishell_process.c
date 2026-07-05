/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_process.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 00:00:00 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/05 00:00:00 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	build_prompt_ast(t_app *app, char *str)
{
	app->tokensll = build_tokensll(app, str);
	if (validate_tokensll(app) == -1)
	{
		freetokensll(app->tokensll);
		app->tokensll = NULL;
		return (-1);
	}
	app->ast = parse_tokens(app->tokensll);
	freetokensll(app->tokensll);
	app->tokensll = NULL;
	if (!app->ast)
		return (-1);
	return (0);
}

static int	prepare_prompt_ast(t_app *app)
{
	if (update_env_array(app) != 0)
	{
		setexit(app, EX_ERR);
		ast_free(app->ast);
		app->ast = NULL;
		return (-1);
	}
	if (expand_ast(app, app->ast) != 0)
	{
		ast_free(app->ast);
		app->ast = NULL;
		return (-1);
	}
	return (0);
}

void	process_prompt(t_app *app, char *str)
{
	if (build_prompt_ast(app, str) != 0)
		return ;
	if (prepare_prompt_ast(app) != 0)
		return ;
	execute_ast(app, app->ast);
	if (!app->should_exit && update_env_array(app) != 0)
		setexit(app, EX_ERR);
	ast_free(app->ast);
	app->ast = NULL;
}
