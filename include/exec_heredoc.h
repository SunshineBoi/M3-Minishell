#ifndef EXEC_HEREDOC_H
# define EXEC_HEREDOC_H

void	handlesig_heredoc(int sig);
char	*parse_heredoc_delimiter(const char *target, int *is_quoted);
char	*expand_heredoc_line(const char *line, char **envp, int last_status);
int		write_expanded(t_app *app, int fd, char *line, int is_quoted);
int		read_heredoc_loop(t_app *app, int fd, const char *delim,
			int is_quoted);
int		open_heredoc_write_file(char **path);
int		reopen_heredoc_read_file(char *path);

#endif
