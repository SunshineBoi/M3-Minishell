/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd_path_build.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 16:30:00 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/08 16:30:00 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*join_cd_path(const char *old_pwd, const char *target)
{
	char	*tmp;
	char	*path;

	if (target[0] == '/')
		return (ft_strdup(target));
	if (!old_pwd)
		return (NULL);
	tmp = ft_strjoin(old_pwd, "/");
	if (!tmp)
		return (NULL);
	path = ft_strjoin(tmp, target);
	free(tmp);
	return (path);
}

char	*build_logical_pwd(const char *old_pwd, const char *target)
{
	char	*path;
	char	*new_pwd;

	path = join_cd_path(old_pwd, target);
	if (!path)
		return (NULL);
	new_pwd = normalize_cd_path(path);
	free(path);
	return (new_pwd);
}
