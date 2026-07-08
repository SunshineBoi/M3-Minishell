/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_readline_fd.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 00:00:00 by lkai-yua         #+#    #+#             */
/*   Updated: 2026/07/08 00:00:00 by lkai-yua        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*grow_line(char *line, size_t len, size_t *capacity)
{
	char	*grown;

	*capacity *= 2;
	grown = malloc(*capacity);
	if (!grown)
		return (free(line), NULL);
	ft_memcpy(grown, line, len);
	free(line);
	return (grown);
}

static int	append_char(char **line, char ch, size_t *len, size_t *capacity)
{
	if (*len + 1 >= *capacity)
	{
		*line = grow_line(*line, *len, capacity);
		if (!*line)
			return (-1);
	}
	(*line)[(*len)++] = ch;
	return (0);
}

char	*read_line_fd(int fd)
{
	char		*line;
	char		ch;
	size_t		len;
	size_t		capacity;
	ssize_t		bytes;

	capacity = 64;
	line = malloc(capacity);
	if (!line)
		return (NULL);
	len = 0;
	bytes = read(fd, &ch, 1);
	while (bytes > 0 && ch != '\n')
	{
		if (append_char(&line, ch, &len, &capacity) == -1)
			return (NULL);
		bytes = read(fd, &ch, 1);
	}
	if (bytes == -1 || (bytes == 0 && len == 0))
		return (free(line), NULL);
	line[len] = '\0';
	return (line);
}
