/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 17:20:16 by kong              #+#    #+#             */
/*   Updated: 2026/07/08 14:22:02 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	parse_word(t_parser *p, t_argv_builder *ab)
{
	char	*word;

	word = ft_strndup(p->cur->val, ft_strlen(p->cur->val));
	if (!word)
		return (-1);
	if (argv_builder_push(ab, word) != 0)
	{
		free(word);
		return (-1);
	}
	advance(p);
	return (0);
}

int	parse_redir(t_parser *p, t_redir **redirs, int src_fd)
{
	t_token_type	type;
	t_redir			*new_redir;

	type = p->cur->type;
	advance(p);
	if (!p->cur || p->cur->type != TOK_STR)
		return (-1);
	new_redir = redir_new(tok_to_redir(type), p->cur->val, src_fd);
	if (!new_redir)
		return (-1);
	*redirs = redir_append(*redirs, new_redir);
	advance(p);
	return (0);
}

int	parse_redir_ionum(t_parser *p, t_redir **redirs)
{
	int	src_fd;

	src_fd = ft_atoi(p->cur->val);
	advance(p);
	return (parse_redir(p, redirs, src_fd));
}

void	free_ab_redirs(t_argv_builder *ab, t_redir *redirs)
{
	argv_builder_free(ab);
	redir_free(redirs);
}

t_redir_type	tok_to_redir(t_token_type type)
{
	if (type == TOK_DIRIN)
		return (REDIR_IN);
	if (type == TOK_DIROUT)
		return (REDIR_OUT);
	if (type == TOK_DIRAPPND)
		return (REDIR_APPEND);
	return (REDIR_HEREDOC);
}
