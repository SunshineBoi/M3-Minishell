#include "minishell.h"
#include "exec_heredoc.h"

void	handlesig_heredoc(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
}

static int	collect_one_heredoc(t_app *app, t_redir *redir)
{
	int					write_fd;
	int					read_fd;
	char				*delim;
	char				*path;
	int					is_quoted;
	struct sigaction	sa;
	struct sigaction	old_sa;
	int					res;

	path = NULL;
	delim = parse_heredoc_delimiter(redir->target, &is_quoted);
	if (!delim)
		return (-1);
	write_fd = open_heredoc_write_file(&path);
	if (write_fd == -1)
		return (free(delim), -1);
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handlesig_heredoc;
	sigaction(SIGINT, &sa, &old_sa);
	res = read_heredoc_loop(app, write_fd, delim, is_quoted);
	sigaction(SIGINT, &old_sa, NULL);
	free(delim);
	if (res == -1)
		return (close(write_fd), unlink(path), free(path), -1);
	if (close(write_fd) == -1)
		return (unlink(path), free(path), -1);
	read_fd = reopen_heredoc_read_file(path);
	free(path);
	if (read_fd == -1)
		return (-1);
	redir->fd = read_fd;
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
