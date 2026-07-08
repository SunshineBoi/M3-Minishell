/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd_path.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 16:30:00 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/08 16:30:00 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	pop_segment(char *out, size_t *pos)
{
	while (*pos > 1 && out[*pos - 1] != '/')
		(*pos)--;
	if (*pos > 1)
		(*pos)--;
	out[*pos] = '\0';
}

static void	append_segment(char *out, size_t *pos,
	const char *path, size_t start)
{
	if (*pos > 1)
		out[(*pos)++] = '/';
	while (path[start] && path[start] != '/')
		out[(*pos)++] = path[start++];
	out[*pos] = '\0';
}

static void	process_segment(char *out, size_t *pos,
	const char *path, size_t *i)
{
	if (path[*i] == '.' && (path[*i + 1] == '/' || !path[*i + 1]))
		(*i)++;
	else if (path[*i] == '.' && path[*i + 1] == '.'
		&& (path[*i + 2] == '/' || !path[*i + 2]))
	{
		pop_segment(out, pos);
		*i += 2;
	}
	else
		append_segment(out, pos, path, *i);
	while (path[*i] && path[*i] != '/')
		(*i)++;
}

char	*normalize_cd_path(char *path)
{
	char	*out;
	size_t	i;
	size_t	pos;

	out = malloc(ft_strlen(path) + 1);
	if (!out)
		return (NULL);
	out[0] = '/';
	out[1] = '\0';
	i = 0;
	pos = 1;
	while (path[i])
	{
		while (path[i] == '/')
			i++;
		if (path[i])
			process_segment(out, &pos, path, &i);
	}
	return (out);
}
