/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_printerr_builtin.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:39:25 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 15:39:25 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	printerr_syscall(t_errcode code)
{
	if (code == ERR_MALLOC || code == ERR_CMDNEXEC
		|| code == ERR_PIPE || code == ERR_FORK)
		perror("minishell");
}

void	printerr_cd(char *filename)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(filename, 2);
	ft_putstr_fd(": no such file or directory\n", 2);
}

/**
 * @brief Print a formatted error: minishell: prefix: msg
 *        or minishell: prefix: arg: msg
 * @param prefix Usually the command or builtin name.
 * @param arg Optional extra context (filename, variable). May be NULL.
 * @param msg The error description.
 */
void	errmsg(const char *prefix, const char *arg, const char *msg)
{
	ft_putstr_fd("minishell: ", 2);
	if (prefix)
	{
		ft_putstr_fd((char *)prefix, 2);
		ft_putstr_fd(": ", 2);
	}
	if (arg)
	{
		ft_putstr_fd((char *)arg, 2);
		ft_putstr_fd(": ", 2);
	}
	ft_putstr_fd((char *)msg, 2);
	ft_putstr_fd("\n", 2);
}
