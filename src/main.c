/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 21:00:05 by kong              #+#    #+#             */
/*   Updated: 2026/07/02 21:00:05 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->env_list = env_init(envp);
	app->envp = env_to_array(app->env_list);
	app->exitcode = EX_OK;
	app->should_exit = 0;
	app->tokensll = NULL;
	app->ast = NULL;
	app->prompt = NULL;
	return (app);
}

int	main(int ac, char **av, char **envp)
{
	(void)av;
	if (ac != 1)
	{
		ft_putstr_fd("minishell: invalid number of arguments\n", 2);
		return (EXIT_FAILURE);
	}
	return (minishell(envp));
}
