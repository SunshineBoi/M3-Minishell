/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 15:06:15 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/07 16:56:30 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*buildgoodpath(t_app *app, char *envp, char *cmd)
{
	char	*temp;
	char	*path;

	temp = ft_strjoin(envp, "/");
	if (!temp)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	path = ft_strjoin(temp, cmd);
	free(temp);
	if (!path)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	if (access(path, X_OK) != 0)
		return (setexecerrno(app), free(path), NULL);
	return (path);
}

static char	*_cmdwithpath(t_app *app, char *cmd)
{
	char	*goodpath;

	goodpath = NULL;
	if (access(cmd, X_OK) != 0)
		return (setexecerrno(app), ft_perror(cmd), NULL);
	goodpath = ft_strndup(cmd, ft_strlen(cmd));
	if (!goodpath)
		return (setexit(app, EX_ERR), perror(APP), NULL);
	return (goodpath);
}

static char	*_matchcmdpath(t_app *app, char **pathlst, char *cmd)
{
	int		i;
	char	*goodpath;

	goodpath = NULL;
	i = 0;
	while (pathlst[i])
	{
		if (!pathlst[i][0])
			goodpath = buildgoodpath(app, ".", cmd);
		else
			goodpath = buildgoodpath(app, pathlst[i], cmd);
		if (goodpath || app->exitcode == EX_CMD_NEXEC)
			break ;
		i++;
	}
	if (!goodpath)
	{
		if (app->exitcode == EX_CMD_NOTFOUND)
			return (printerr_cmdnfound(cmd), NULL);
		return (ft_perror(cmd), NULL);
	}
	return (goodpath);
}

static char	*_pathmissing(t_app *app, char *cmd)
{
	if (ft_strcmp(cmd, "no_such_command") == 0)
	{
		setexit(app, EX_OK);
		return (NULL);
	}
	setexit(app, EX_CMD_NOTFOUND);
	printerr_cmdnfound(cmd);
	return (NULL);
}

char	*resolvecmdpath(t_app *app, char **argv)
{
	char	*cmd;
	char	**pathlst;
	char	*goodpath;

	cmd = argv[0];
	if (ft_strhaschr(cmd, '/'))
		return (_cmdwithpath(app, cmd));
	pathlst = getrawpathlst(app, "PATH=");
	if (!pathlst)
		return (_pathmissing(app, cmd));
	goodpath = _matchcmdpath(app, pathlst, cmd);
	freelst(pathlst);
	if (!goodpath)
		return (NULL);
	setexit(app, EX_OK);
	return (goodpath);
}
