/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_pipeline_child.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 00:00:00 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/05 00:00:00 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	do_childproc(t_app *app, t_cmd_node *cmdnode, int fdin, int fdout)
{
	signals_default();
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
	if (open_redirs(app, cmdnode) == -1)
		return (close_redirsfd(cmdnode), -1);
	if (do_dup2redirs(app, cmdnode) == -1)
		return (close_redirsfd(cmdnode), -1);
	close_redirsfd(cmdnode);
	if (do_exec(app, cmdnode) == -1)
		return (-1);
	return (0);
}
