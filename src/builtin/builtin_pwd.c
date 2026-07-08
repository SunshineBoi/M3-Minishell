/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_pwd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 12:42:56 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 12:42:57 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief pwd builtin — print shell's logical working directory.
 */
int	builtin_pwd(t_app *app)
{
	char	cwd[PATH_MAX];
	char	*pwd;

	pwd = env_get(app->env_list, "PWD");
	if (pwd && *pwd)
	{
		ft_putstr_fd(pwd, 1);
		write(1, "\n", 1);
		return (EX_OK);
	}
	if (getcwd(cwd, sizeof(cwd)))
	{
		ft_putstr_fd(cwd, 1);
		write(1, "\n", 1);
		return (EX_OK);
	}
	errmsg("pwd", NULL, strerror(errno));
	return (EX_ERR);
}
