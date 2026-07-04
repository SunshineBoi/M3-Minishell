/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_heredoc.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 20:02:32 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/04 20:02:35 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#include "exec_heredoc.h"

void	handlesig_heredoc(int sig)
{
	g_signal = sig;
	write(1, "\n", 1);
}

static void	set_heredoc_sigint(struct sigaction *old_sa)
{
	struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = handlesig_heredoc;
	sigaction(SIGINT, &sa, old_sa);
}

static int	collect_one_heredoc(t_app *app, t_redir *redir)
{
	char				*delim;
	char				*path;
	int					is_quoted;
	int					write_fd;
	struct sigaction	old_sa;

	path = NULL;
	delim = parse_heredoc_delimiter(redir->target, &is_quoted);
	if (!delim)
		return (-1);
	write_fd = open_heredoc_write_file(&path);
	if (write_fd == -1)
		return (free(delim), -1);
	set_heredoc_sigint(&old_sa);
	redir->fd = read_heredoc_loop(app, write_fd, delim, is_quoted);
	sigaction(SIGINT, &old_sa, NULL);
	free(delim);
	redir->fd = finish_heredoc_file(path, write_fd, redir->fd);
	if (redir->fd == -1)
		return (-1);
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
