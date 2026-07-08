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

static int	parse_shlvl_number(const char *value, long *level)
{
	int		sign;

	if (!value || !*value)
		return (0);
	sign = 1;
	if (*value == '-' || *value == '+')
	{
		if (*value++ == '-')
			sign = -1;
	}
	if (!*value)
		return (0);
	*level = 0;
	while (*value >= '0' && *value <= '9')
	{
		if (*level > 1000)
			return (0);
		*level = *level * 10 + (*value++ - '0');
	}
	if (*value)
		return (0);
	*level *= sign;
	return (1);
}

static int	parse_shlvl(const char *value)
{
	long	level;

	if (!parse_shlvl_number(value, &level))
		return (1);
	level++;
	if (level < 0)
		return (0);
	if (level > 999)
		return (1);
	return ((int)level);
}

static int	update_shlvl(t_env **env_list)
{
	char	*value;
	int		status;

	value = ft_itoa(parse_shlvl(env_get(*env_list, "SHLVL")));
	if (!value)
		return (ERR_MALLOC);
	status = env_set(env_list, "SHLVL", value);
	free(value);
	return (status);
}

t_app	*init_app(char **envp)
{
	t_app	*app;

	app = malloc(sizeof(t_app));
	if (!app)
		return (printerr_syscall(ERR_MALLOC), NULL);
	app->env_list = env_init(envp);
	if (update_shlvl(&app->env_list) != EX_OK)
		return (env_free(app->env_list), free(app),
			printerr_syscall(ERR_MALLOC), NULL);
	app->envp = env_to_array(app->env_list);
	if (!app->envp)
		return (env_free(app->env_list), free(app),
			printerr_syscall(ERR_MALLOC), NULL);
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
