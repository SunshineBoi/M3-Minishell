/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:55:32 by kong              #+#    #+#             */
/*   Updated: 2026/07/08 14:21:59 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	advance(t_parser *p)
{
	if (p->cur)
	{
		p->cur = p->cur->next;
		p->index++;
	}
}

static void	free_left_right(t_ast_node *left, t_ast_node *right)
{
	ast_free(left);
	ast_free(right);
}

static t_ast_node	*parse_simple_command(t_parser *p)
{
	t_argv_builder	ab;
	t_redir			*redirs;
	t_ast_node		*node;
	int				start;

	if (argv_builder_init(&ab) != 0)
		return (NULL);
	redirs = NULL;
	start = p->index;
	while (p->cur && (p->cur->type == TOK_STR || p->cur->type == TOK_IONUM
			|| is_redir_token(p->cur->type)))
		if (parse_command_token(p, &ab, &redirs) == -1)
			return (free_ab_redirs(&ab, redirs), NULL);
	if (ab.count == 0 && !redirs)
		return (free_ab_redirs(&ab, redirs), NULL);
	node = ast_new_cmd(ab.argv, redirs, (t_span){start, p->index});
	if (!node)
		free_ab_redirs(&ab, redirs);
	return (node);
}

static t_ast_node	*parse_pipeline(t_parser *p)
{
	t_ast_node	*left;
	t_ast_node	*right;
	t_ast_node	*old_left;

	left = parse_simple_command(p);
	if (!left)
		return (NULL);
	while (p->cur && p->cur->type == TOK_PIPE)
	{
		advance(p);
		right = parse_simple_command(p);
		if (!right)
			return (ast_free(left), NULL);
		old_left = left;
		left = ast_new_binop(BIN_PIPE, old_left, right,
				(t_span){old_left->span.start, right->span.end});
		if (!left)
			return (free_left_right(old_left, right), NULL);
	}
	return (left);
}

t_ast_node	*parse_tokens(t_tokensll *tokens)
{
	t_parser	p;
	t_ast_node	*root;

	p.cur = tokens;
	p.index = 0;
	root = parse_pipeline(&p);
	if (!root)
		return (NULL);
	if (p.cur)
	{
		ast_free(root);
		return (NULL);
	}
	return (root);
}
