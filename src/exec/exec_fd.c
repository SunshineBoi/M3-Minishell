#include "minishell.h"

int	open_redirs(t_app *app, t_cmd_node *node)
{
	t_redir	*redir;
	int		flag;

	redir = node->redirs;
	flag = 0;
	while (redir)
	{
		if (redir->type == REDIR_IN)
			flag = O_RDONLY;
		else if (redir->type == REDIR_OUT)
			flag = O_WRONLY | O_CREAT | O_TRUNC;
		else if (redir->type == REDIR_APPEND)
			flag = O_WRONLY | O_CREAT | O_APPEND;
		else if (redir->type == REDIR_HEREDOC)
		{
			redir = redir->next;
			continue ;
		}
		redir->fd = open(redir->target, flag, 0644);
		if (redir->fd == -1)
			return (setexit(app, EX_ERR), ft_perror(redir->target), -1);
		redir = redir->next;
	}
	return (0);
}

int	do_dup2redirs(t_app *app, t_cmd_node *node)
{
	t_redir	*redir;

	redir = node->redirs;
	while (redir)
	{
		if (redir->fd != -1)
		{
			if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
			{
				if (dup2(redir->fd, STDIN_FILENO) == -1)
					return (setexit(app, EX_ERR), perror(APP), -1);
			}
			else if (redir->type == REDIR_APPEND || redir->type == REDIR_OUT)
			{
				if (dup2(redir->fd, STDOUT_FILENO) == -1)
					return (setexit(app, EX_ERR), perror(APP), -1);
			}
		}
		redir = redir->next;
	}
	return (0);
}

void	close_redirsfd(t_cmd_node *node)
{
	t_redir	*redir;

	redir = node->redirs;
	while (redir)
	{
		if (redir->fd != -1)
		{
			close(redir->fd);
			redir->fd = -1;
		}
		redir = redir->next;
	}
}

void	close_pipefd(int *pipebuf)
{
	if (!pipebuf)
		return ;
	if (pipebuf[0] != -1)
	{
		close(pipebuf[0]);
		pipebuf[0] = -1;
	}
	if (pipebuf[1] != -1)
	{
		close(pipebuf[1]);
		pipebuf[1] = -1;
	}
}

void	restore_fd(int fdin, int fdout)
{
	if (fdin != -1)
	{
		dup2(fdin, STDIN_FILENO);
		close(fdin);
	}
	if (fdout != -1)
	{
		dup2(fdout, STDOUT_FILENO);
		close(fdout);
	}
}

static void	handlesig_heredoc(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
}

static char	*parse_heredoc_delimiter(const char *target, int *is_quoted)
{
	t_strbuf	sb;
	size_t		i;

	*is_quoted = 0;
	if (sb_init(&sb) != 0)
		return (NULL);
	i = 0;
	while (target[i])
	{
		if (target[i] == '\'' || target[i] == '"')
		{
			*is_quoted = 1;
			i++;
		}
		else
		{
			if (sb_push_char(&sb, target[i]) != 0)
			{
				sb_free(&sb);
				return (NULL);
			}
			i++;
		}
	}
	return (sb.buf);
}

static void	append_expanded_var(t_strbuf *sb, const char *line, size_t *i, char **envp, int status)
{
	char	*status_str;
	size_t	var_len;

	if (line[*i + 1] == '?')
	{
		status_str = itoa_status(status);
		if (status_str)
			sb_push_str(sb, status_str);
		free(status_str);
		*i += 2;
	}
	else if (is_name_start(line[*i + 1]))
	{
		var_len = var_name_len(line + *i + 1);
		sb_push_str(sb, env_lookup(envp, line + *i + 1, var_len));
		*i += 1 + var_len;
	}
	else
	{
		sb_push_char(sb, '$');
		(*i)++;
	}
}

static char	*expand_heredoc_line(const char *line, char **envp, int last_status)
{
	t_strbuf	sb;
	size_t		i;

	if (sb_init(&sb) != 0)
		return (NULL);
	i = 0;
	while (line[i])
	{
		if (line[i] == '$' && line[i + 1])
			append_expanded_var(&sb, line, &i, envp, last_status);
		else
		{
			sb_push_char(&sb, line[i]);
			i++;
		}
	}
	return (sb.buf);
}

static int	write_expanded(t_app *app, int fd, char *line, int is_quoted)
{
	char	*expanded;

	if (!is_quoted)
	{
		expanded = expand_heredoc_line(line, app->envp, app->exitcode);
		free(line);
		if (!expanded)
			return (-1);
		line = expanded;
	}
	write(fd, line, ft_strlen(line));
	write(fd, "\n", 1);
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

static int	read_heredoc_loop(t_app *app, int *pipe_fds, const char *delim, int is_quoted)
{
	char	*line;

	while (1)
	{
		if (isatty(STDIN_FILENO))
			line = readline("> ");
		else
			line = get_next_line_non_interactive();
		if (g_signal == SIGINT)
			return (free(line), -1);
		if (!line)
		{
			ft_putstr_fd("minishell: warning: here-document delimited by end-of-file\n", 2);
			return (0);
		}
		if (ft_strcmp(line, delim) == 0)
			return (free(line), 0);
		if (write_expanded(app, pipe_fds[1], line, is_quoted) == -1)
			return (-1);
	}
	return (0);
}

static int	collect_one_heredoc(t_app *app, t_redir *redir)
{
	int					pipe_fds[2];
	char				*delim;
	int					is_quoted;
	struct sigaction	sa;
	struct sigaction	old_sa;
	int					res;

	delim = parse_heredoc_delimiter(redir->target, &is_quoted);
	if (!delim)
		return (-1);
	if (pipe(pipe_fds) == -1)
		return (free(delim), -1);
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handlesig_heredoc;
	sigaction(SIGINT, &sa, &old_sa);
	res = read_heredoc_loop(app, pipe_fds, delim, is_quoted);
	sigaction(SIGINT, &old_sa, NULL);
	free(delim);
	close(pipe_fds[1]);
	if (res == -1)
		return (close(pipe_fds[0]), -1);
	redir->fd = pipe_fds[0];
	return (0);
}

static int	collect_cmd_heredocs(t_app *app, t_cmd_node *cmd)
{
	t_redir	*redir;

	redir = cmd->redirs;
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			if (collect_one_heredoc(app, redir) == -1)
			{
				setexit(app, EX_SIG_BASE + SIGINT);
				return (-1);
			}
		}
		redir = redir->next;
	}
	return (0);
}

int	collect_heredocs(t_app *app, t_ast_node *ast)
{
	if (!ast)
		return (0);
	if (ast->type == NODE_CMD)
		return (collect_cmd_heredocs(app, &ast->content.cmd));
	if (ast->type == NODE_BINOP)
	{
		if (collect_heredocs(app, ast->content.binop.left) == -1)
			return (-1);
		return (collect_heredocs(app, ast->content.binop.right));
	}
	return (0);
}
