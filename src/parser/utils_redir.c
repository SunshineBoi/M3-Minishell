/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_redir.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 17:21:02 by kong              #+#    #+#             */
/*   Updated: 2026/07/08 14:00:17 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_redir_token(t_token_type type)
{
	return (type == TOK_DIRIN || type == TOK_DIROUT
		|| type == TOK_DIRAPPND || type == TOK_HEREDOC);
}

t_redir	*redir_new(t_redir_type type, const char *target, int src_fd)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->type = type;
	redir->target = ft_strndup((char *)target, ft_strlen(target));
	if (!redir->target)
	{
		free(redir);
		return (NULL);
	}
	redir->fd = -1;
	if (src_fd != -1)
		redir->src_fd = src_fd;
	else if (type == REDIR_IN || type == REDIR_HEREDOC)
		redir->src_fd = STDIN_FILENO;
	else
		redir->src_fd = STDOUT_FILENO;
	redir->next = NULL;
	return (redir);
}

void	redir_free(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
		if (redir->fd != -1)
			close(redir->fd);
		free(redir->target);
		free(redir);
		redir = next;
	}
}

t_redir	*redir_append(t_redir *head, t_redir *node)
{
	t_redir	*tail;

	if (!head)
		return (node);
	tail = head;
	while (tail->next)
		tail = tail->next;
	tail->next = node;
	return (head);
}
