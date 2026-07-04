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

static int	is_assignment_word(const char *arg)
{
	int	i;

	i = 0;
	if (!arg || !is_name_start(arg[0]))
		return (0);
	while (arg[i] && arg[i] != '=')
	{
		if (arg[i] != '_' && (arg[i] < 'A' || arg[i] > 'Z')
			&& (arg[i] < 'a' || arg[i] > 'z') && (arg[i] < '0' || arg[i] > '9'))
			return (0);
		i++;
	}
	return (arg[i] == '=');
}

static int	assignment_prefix_count(char **argv)
{
	int	count;

	count = 0;
	while (argv && argv[count] && is_assignment_word(argv[count]))
		count++;
	return (count);
}

static void	shift_argv_left(char **argv)
{
	int	j;

	j = 0;
	while (argv[j])
	{
		argv[j] = argv[j + 1];
		j++;
	}
}

static int	expand_assignment_value(t_app *app, char *raw, int last_status,
		char **out)
{
	t_wordlist	wl;

	*out = NULL;
	if (expand_word(raw, app->envp, last_status, &wl.items, &wl.count) == 0)
	{
		if (wl.count > 0 && wl.items[0])
			*out = ft_strdup(wl.items[0]);
		else
			*out = ft_strdup("");
		freelst(wl.items);
	}
	else
		*out = ft_strdup("");
	free(raw);
	if (!*out)
		return (ERR_MALLOC);
	return (0);
}

static int	apply_assignment(t_app *app, t_env **env, char *name, char *raw,
		int last_status)
{
	char	*val;

	if (expand_assignment_value(app, raw, last_status, &val) != 0)
		return (free(name), ERR_MALLOC);
	if (env_set(env, name, val) != 0)
	{
		free(name);
		free(val);
		return (ERR_MALLOC);
	}
	free(name);
	free(val);
	return (0);
}

int	process_assignments(t_app *app, t_cmd_node *cmd, int last_status)
{
	char	*name;
	char	*raw_val;
	t_env	*tmp_env;
	int		count;
	int		has_cmd;
	int		i;

	count = assignment_prefix_count(cmd->argv);
	if (count == 0)
		return (0);
	has_cmd = (cmd->argv[count] != NULL);
	tmp_env = NULL;
	if (has_cmd)
		tmp_env = env_init(app->envp);
	i = 0;
	while (i < count)
	{
		if (!is_valid_assignment(cmd->argv[0], &name, &raw_val))
			break ;
		if (has_cmd && apply_assignment(app, &tmp_env, name, raw_val,
				last_status) != 0)
			return (env_free(tmp_env), ERR_MALLOC);
		if (!has_cmd && apply_assignment(app, &app->env_list, name, raw_val,
				last_status) != 0)
			return (ERR_MALLOC);
		free(cmd->argv[0]);
		shift_argv_left(cmd->argv);
		i++;
	}
	if (!has_cmd)
		return (update_env_array(app));
	cmd->envp = env_to_array(tmp_env);
	env_free(tmp_env);
	if (!cmd->envp)
		return (ERR_MALLOC);
	return (0);
}
