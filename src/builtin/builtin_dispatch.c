/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_dispatch.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 11:48:15 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 11:48:16 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Check if a command name is a shell builtin.
 * @param cmd Command name (argv[0]).
 * @return 1 if builtin, 0 otherwise.
 */
int	is_builtin(const char *cmd)
{
	if (!cmd)
		return (0);
	if (ft_strcmp(cmd, "echo") == 0)
		return (1);
	if (ft_strcmp(cmd, "cd") == 0)
		return (1);
	if (ft_strcmp(cmd, "pwd") == 0)
		return (1);
	if (ft_strcmp(cmd, "export") == 0)
		return (1);
	if (ft_strcmp(cmd, "unset") == 0)
		return (1);
	if (ft_strcmp(cmd, "env") == 0)
		return (1);
	if (ft_strcmp(cmd, "exit") == 0)
		return (1);
	return (0);
}

/**
 * @brief Dispatch to the appropriate builtin implementation.
 * @param argv NULL-terminated argument array.
 * @param app Shell context.
 * @return Exit status of the builtin.
 */
int	exec_builtin(char **argv, t_app *app)
{
	if (ft_strcmp(argv[0], "echo") == 0)
		return (builtin_echo(argv));
	if (ft_strcmp(argv[0], "cd") == 0)
		return (builtin_cd(argv, app));
	if (ft_strcmp(argv[0], "pwd") == 0)
		return (builtin_pwd(app));
	if (ft_strcmp(argv[0], "export") == 0)
		return (builtin_export(argv, app));
	if (ft_strcmp(argv[0], "unset") == 0)
		return (builtin_unset(argv, app));
	if (ft_strcmp(argv[0], "env") == 0)
		return (builtin_env(app));
	if (ft_strcmp(argv[0], "exit") == 0)
		return (builtin_exit(argv, app));
	return (1);
}
