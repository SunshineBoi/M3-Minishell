/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_assignment.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 19:30:09 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 19:31:10 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_valid_assignment(const char *arg, char **name, char **value)
{
	int	i;
	int	len;

	i = 0;
	if (!arg || (!is_name_start(arg[0])))
		return (0);
	while (arg[i] && arg[i] != '=')
	{
		if (arg[i] != '_' && (arg[i] < 'A' || arg[i] > 'Z')
			&& (arg[i] < 'a' || arg[i] > 'z') && (arg[i] < '0' || arg[i] > '9'))
			return (0);
		i++;
	}
	if (arg[i] != '=')
		return (0);
	*name = ft_strndup((char *)arg, i);
	if (!*name)
		return (0);
	len = ft_strlen(arg + i + 1);
	if (len > 0 && arg[i + 1 + len - 1] == ';')
		*value = ft_strndup((char *)arg + i + 1, len - 1);
	else
		*value = ft_strdup(arg + i + 1);
	return (*value != NULL || (free(*name), 0));
}

static int	apply_assignment(t_app *app, char *name, char *raw, int last_status)
{
	char		*val;
	t_wordlist	wl;

	if (expand_word(raw, app->envp, last_status, &wl) == 0)
	{
		if (wl.count > 0 && wl.items[0])
			val = ft_strdup(wl.items[0]);
		else
			val = ft_strdup("");
		freelst(wl.items);
	}
	else
		val = ft_strdup("");
	free(raw);
	if (env_set(&app->env_list, name, val) != 0)
	{
		free(name);
		free(val);
		return (ERR_MALLOC);
	}
	free(name);
	free(val);
	return (update_env_array(app));
}

int	process_assignments(t_app *app, t_cmd_node *cmd, int last_status)
{
	char	*name;
	char	*raw_val;
	int		j;

	while (cmd->argv && cmd->argv[0])
	{
		if (!is_valid_assignment(cmd->argv[0], &name, &raw_val))
			break ;
		if (apply_assignment(app, name, raw_val, last_status) != 0)
			return (ERR_MALLOC);
		free(cmd->argv[0]);
		j = 0;
		while (cmd->argv[j])
		{
			cmd->argv[j] = cmd->argv[j + 1];
			j++;
		}
	}
	return (0);
}
