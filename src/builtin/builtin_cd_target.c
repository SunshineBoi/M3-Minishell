/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd_target.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 16:30:00 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/08 16:30:00 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static const char	*get_runtime_env(t_app *app, const char *key)
{
	const char	*value;

	value = env_lookup(app->envp, key, ft_strlen(key));
	if (value)
		return (value);
	return (env_get(app->env_list, key));
}

const char	*resolve_cd_target(char **argv, t_app *app)
{
	const char	*target;

	if (!argv[1])
	{
		target = get_runtime_env(app, "HOME");
		if (!target)
			errmsg("cd", NULL, "HOME not set");
		return (target);
	}
	if (ft_strcmp(argv[1], "-") == 0)
	{
		target = get_runtime_env(app, "OLDPWD");
		if (!target)
			errmsg("cd", NULL, "OLDPWD not set");
		return (target);
	}
	return (argv[1]);
}
