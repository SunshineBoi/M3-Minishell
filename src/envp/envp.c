/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 13:11:10 by kong              #+#    #+#             */
/*   Updated: 2026/07/04 13:11:14 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Count nodes in the environment list.
 */
static size_t	env_count(t_env *list)
{
	size_t	n;

	n = 0;
	while (list)
	{
		if (list->value)
			n++;
		list = list->next;
	}
	return (n);
}

/**
 * @brief Convert the environment list to a NULL-terminated char** array.
 *
 * Only variables with non-NULL values are included (matching `env` behavior).
 * Caller must free with freelst().
 */
char	**env_to_array(t_env *list)
{
	char	**arr;
	size_t	i;
	char	*tmp;

	arr = ft_calloclst(env_count(list));
	if (!arr)
		return (NULL);
	i = 0;
	while (list)
	{
		if (list->value)
		{
			tmp = ft_strjoin(list->key, "=");
			if (!tmp)
				return (freelst(arr), NULL);
			arr[i] = ft_strjoin(tmp, list->value);
			free(tmp);
			if (!arr[i])
				return (freelst(arr), NULL);
			i++;
		}
		list = list->next;
	}
	arr[i] = NULL;
	return (arr);
}

int	update_env_array(t_app *app)
{
	char	**new_envp;

	new_envp = env_to_array(app->env_list);
	if (!new_envp)
		return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	if (app->envp)
		freelst(app->envp);
	app->envp = new_envp;
	return (EX_OK);
}
