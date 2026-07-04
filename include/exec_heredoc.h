/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_heredoc.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 20:02:32 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/04 20:02:35 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXEC_HEREDOC_H
# define EXEC_HEREDOC_H

void	handlesig_heredoc(int sig);
char	*parse_heredoc_delimiter(const char *target, int *is_quoted);
char	*expand_heredoc_line(const char *line, t_app *app);
int		write_expanded(t_app *app, int fd, char *line, int is_quoted);
int		read_heredoc_loop(t_app *app, int fd, const char *delim,
			int is_quoted);
int		open_heredoc_write_file(char **path);
int		reopen_heredoc_read_file(char *path);
int		finish_heredoc_file(char *path, int write_fd, int res);

#endif
