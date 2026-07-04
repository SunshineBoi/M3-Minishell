/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_assignment_utils.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 00:00:00 by kong              #+#    #+#             */
/*   Updated: 2026/07/04 00:00:00 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_valid_assignment(const char *arg, char **name, char **value)
{
	int	i;
	int	len;

	i = 0;
	if (!arg || (!is_name_start(arg[0])))
		return (0);
	while (arg[i] && arg[i] != '=')
	{
		if (arg[i] != '_' && (arg[i] < 'A' || arg[i] > 'Z')
			&& (arg[i] < 'a' || arg[i] > 'z')
			&& (arg[i] < '0' || arg[i] > '9'))
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
			&& (arg[i] < 'a' || arg[i] > 'z')
			&& (arg[i] < '0' || arg[i] > '9'))
			return (0);
		i++;
	}
	return (arg[i] == '=');
}

int	assignment_prefix_count(char **argv)
{
	int	count;

	count = 0;
	while (argv && argv[count] && is_assignment_word(argv[count]))
		count++;
	return (count);
}

void	shift_argv_left(char **argv)
{
	int	j;

	j = 0;
	while (argv[j])
	{
		argv[j] = argv[j + 1];
		j++;
	}
}
