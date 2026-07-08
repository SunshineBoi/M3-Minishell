/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 21:11:42 by kong              #+#    #+#             */
/*   Updated: 2026/07/02 21:11:43 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*get_old_pwd(t_app *app, char *cwd)
{
	char	*old_pwd;

	old_pwd = env_get(app->env_list, "PWD");
	if (old_pwd && old_pwd[0] == '/')
		return (old_pwd);
	if (getcwd(cwd, PATH_MAX))
		return (cwd);
	return (NULL);
}

/**
 * @brief Update PWD and OLDPWD environment variables after chdir.
 */
static int	update_pwd(t_app *app, const char *old_pwd, const char *new_pwd)
{
	if (old_pwd && env_set(&app->env_list, "OLDPWD", old_pwd) != EX_OK)
		return (printerr_syscall(ERR_MALLOC), EX_ERR);
	if (env_set(&app->env_list, "PWD", new_pwd) != EX_OK)
		return (printerr_syscall(ERR_MALLOC), EX_ERR);
	return (EX_OK);
}

static int	finish_cd(char **argv, t_app *app,
	const char *old_pwd, char *new_pwd)
{
	int	status;

	if (argv[1] && ft_strcmp(argv[1], "-") == 0)
	{
		ft_putstr_fd(new_pwd, 1);
		write(1, "\n", 1);
	}
	status = update_pwd(app, old_pwd, new_pwd);
	free(new_pwd);
	return (status);
}

/**
 * @brief cd builtin — change directory.
 *
 * - No args → cd to $HOME
 * - "-" → cd to $OLDPWD, print new path
 * - Otherwise → cd to given path
 */
int	builtin_cd(char **argv, t_app *app)
{
	const char	*target;
	char		*old_pwd;
	char		*new_pwd;
	char		cwd[PATH_MAX];

	if (argv[1] && argv[2])
		return (errmsg("cd", NULL, "too many arguments"), EX_ERR);
	old_pwd = get_old_pwd(app, cwd);
	target = resolve_cd_target(argv, app);
	if (!target)
		return (EX_ERR);
	if (!*target)
		return (errmsg("cd", target, strerror(ENOENT)), EX_ERR);
	new_pwd = build_logical_pwd(old_pwd, target);
	if (!new_pwd)
		return (printerr_syscall(ERR_MALLOC), EX_ERR);
	if (chdir(new_pwd) != 0)
	{
		free(new_pwd);
		return (errmsg("cd", target, strerror(errno)), EX_ERR);
	}
	return (finish_cd(argv, app, old_pwd, new_pwd));
}
