/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_heredoc_io.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 20:02:32 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/04 20:02:35 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include "exec_heredoc.h"

static int	write_all(int fd, const char *buf, size_t len)
{
	ssize_t	written;
	size_t	total;

	total = 0;
	while (total < len)
	{
		written = write(fd, buf + total, len - total);
		if (written == -1)
			return (-1);
		if (written == 0)
			return (-1);
		total += written;
	}
	return (0);
}

int	write_expanded(t_app *app, int fd, char *line, int is_quoted)
{
	char	*expanded;

	if (!is_quoted)
	{
		expanded = expand_heredoc_line(line, app);
		free(line);
		if (!expanded)
			return (-1);
		line = expanded;
	}
	if (write_all(fd, line, ft_strlen(line)) == -1
		|| write_all(fd, "\n", 1) == -1)
		return (free(line), -1);
	free(line);
	return (0);
}

static char	*get_next_line_non_interactive(void)
{
	char	*line;
	size_t	len;
	ssize_t	read;

	line = NULL;
	len = 0;
	read = getline(&line, &len, stdin);
	if (read == -1)
	{
		free(line);
		return (NULL);
	}
	if (read > 0 && line[read - 1] == '\n')
		line[read - 1] = '\0';
	return (line);
}

int	read_heredoc_loop(t_app *app, int fd, const char *delim, int is_quoted)
{
	char	*line;

	while (1)
	{
		if (isatty(STDIN_FILENO))
		{
			write(1, "> ", 2);
			line = get_next_line_non_interactive();
		}
		else
			line = get_next_line_non_interactive();
		if (g_signal == SIGINT)
			return (free(line), -1);
		if (!line)
		{
			ft_putstr_fd("minishell: warning: here-document ", 2);
			ft_putstr_fd("delimited by end-of-file\n", 2);
			return (0);
		}
		if (ft_strcmp(line, delim) == 0)
			return (free(line), 0);
		if (write_expanded(app, fd, line, is_quoted) == -1)
			return (-1);
	}
	return (0);
}
