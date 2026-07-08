/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:54:43 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/08 13:40:46 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include <stddef.h>
# include "ast.h"

typedef enum e_token_type
{
	TOK_STR,
	TOK_PIPE,
	TOK_DIRIN,
	TOK_DIROUT,
	TOK_DIRAPPND,
	TOK_HEREDOC,
	TOK_EOF,
	TOK_IONUM
}						t_token_type;

typedef enum e_qstate
{
	Q_NONE,
	Q_SQUOTE,
	Q_DQUOTE
}						t_qstate;

typedef struct s_tokensll
{
	char				*val;
	long long			val_size;
	t_token_type		type;
	struct s_tokensll	*next;
}						t_tokensll;

typedef struct s_sll_ops
{
	t_tokensll			*curr;
	t_tokensll			*prev;
	t_tokensll			*head;
}						t_sll_ops;

typedef struct s_strbuf
{
	char				*buf;
	size_t				len;
	size_t				cap;
}						t_strbuf;

typedef struct s_wordlist
{
	char				**items;
	size_t				count;
	size_t				cap;
}						t_wordlist;

typedef struct s_argv_builder
{
	char				**argv;
	size_t				count;
	size_t				cap;
}						t_argv_builder;

typedef struct s_parser
{
	t_tokensll			*cur;
	int					index;
}						t_parser;

typedef struct s_expand_ctx
{
	const char			*input;
	char				**envp;
	int					last_status;
	t_qstate			state;
	t_strbuf			sb;
	t_wordlist			wl;
	size_t				i;
	int					word_in_progress;
}						t_expand_ctx;

/* === parser.c === */
void					advance(t_parser *p);
t_ast_node				*parse_tokens(t_tokensll *tokens);

/* === parser_helpers.c === */
int						parse_word(t_parser *p, t_argv_builder *ab);
int						parse_redir(t_parser *p, t_redir **redirs, int src_fd);
int						parse_redir_ionum(t_parser *p, t_redir **redirs);
void					free_ab_redirs(t_argv_builder *ab, t_redir *redirs);
t_redir_type			tok_to_redir(t_token_type type);

/* === parser_dispatch.c === */
int						parse_command_token(t_parser *p, t_argv_builder *ab,
							t_redir **redirs);

/* === utils_argv.c === */
int						argv_builder_init(t_argv_builder *ab);
int						argv_builder_push(t_argv_builder *ab, char *word);
void					argv_builder_free(t_argv_builder *ab);
int						push_words_to_builder(t_argv_builder *ab, char **words,
							size_t count);

/* === utils_redir.c === */
t_redir					*redir_new(t_redir_type type, const char *target,
							int src_fd);
void					redir_free(t_redir *redir);
t_redir					*redir_append(t_redir *head, t_redir *node);
int						is_redir_token(t_token_type type);

#endif
