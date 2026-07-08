/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_dispatch.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/08 00:00:00 by kong              #+#    #+#             */
/*   Updated: 2026/07/08 00:00:00 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	parse_command_token(t_parser *p, t_argv_builder *ab, t_redir **redirs)
{
	if (p->cur->type == TOK_STR)
		return (parse_word(p, ab));
	if (p->cur->type == TOK_IONUM)
		return (parse_redir_ionum(p, redirs));
	return (parse_redir(p, redirs, -1));
}
