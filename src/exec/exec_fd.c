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
