/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 21:00:17 by kong              #+#    #+#             */
/*   Updated: 2026/07/02 21:00:37 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*read_prompt(void)
{
	char	*prompt;

	signals_at_prompt();
	g_signal = 0;
	if (isatty(STDIN_FILENO))
		prompt = readline("minishell$ ");
	else
		prompt = read_line_fd(STDIN_FILENO);
	return (prompt);
}

int	cleanup_app(t_app *app)
{
	int	exit_code;

	if (!app)
		return (EX_ERR);
	exit_code = app->exitcode;
	if (app->tokensll)
		freetokensll(app->tokensll);
	if (app->ast)
		ast_free(app->ast);
	if (app->prompt)
		free(app->prompt);
	env_free(app->env_list);
	if (app->envp)
		freelst(app->envp);
	clear_history();
	free(app);
	return (exit_code);
}

static int	run_prompt_cycle(t_app *app)
{
	char	*prompt;

	prompt = read_prompt();
	if (!prompt)
	{
		if (isatty(STDIN_FILENO))
			ft_putstr_fd("exit\n", 1);
		return (0);
	}
	if (g_signal == SIGINT)
		handle_sigint_in_main(app);
	app->prompt = prompt;
	process_prompt(app, prompt);
	if (*prompt && !app->should_exit && isatty(STDIN_FILENO))
		add_history(prompt);
	free(prompt);
	app->prompt = NULL;
	if (app->should_exit)
		return (0);
	return (1);
}

int	minishell(char **envp)
{
	t_app	*app;

	app = init_app(envp);
	if (!app)
		hardexit();
	while (1)
	{
		if (!run_prompt_cycle(app))
			break ;
	}
	return (cleanup_app(app));
}
