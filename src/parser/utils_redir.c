#include "minishell.h"

t_redir	*redir_new(t_redir_type type, const char *target)
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
	redir->next = NULL;
	return (redir);
}

void	redir_free(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
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
