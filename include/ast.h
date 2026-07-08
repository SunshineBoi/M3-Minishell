/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ast.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:56:09 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/08 12:52:03 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AST_H
# define AST_H

# include <stddef.h>

typedef enum e_node_type
{
	NODE_CMD,
	NODE_BINOP
}								t_node_type;

typedef enum e_binop_type
{
	BIN_PIPE,
	BIN_AND,
	BIN_OR,
	BIN_SEQ
}								t_binop_type;

typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}								t_redir_type;

typedef struct s_span
{
	int							start;
	int							end;
}								t_span;

typedef struct s_redir
{
	t_redir_type				type;
	char						*target;
	int							fd;
	int							src_fd;
	struct s_redir				*next;
}								t_redir;

typedef struct s_cmd_node
{
	char						**argv;
	char						**envp;
	t_redir						*redirs;
}								t_cmd_node;

typedef struct s_ast_node
{
	t_node_type					type;
	t_span						span;
	union u_ast_content
	{
		t_cmd_node				cmd;
		struct s_binop_node
		{
			t_binop_type		op;
			struct s_ast_node	*left;
			struct s_ast_node	*right;
		}						binop;
	}							content;
}								t_ast_node;

t_ast_node						*ast_new_cmd(char **argv, t_redir *redirs,
									t_span span);
t_ast_node						*ast_new_binop(t_binop_type op,
									t_ast_node *left, t_ast_node *right,
									t_span span);
void							ast_free(t_ast_node *node);

#endif
