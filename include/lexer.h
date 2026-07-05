/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:59:52 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/05 18:59:53 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

# include "parser.h"

typedef struct s_app	t_app;

/* === lexer.c === */
/**
 * @brief Tokenize the input string into a linked list.
 * @param str Input command line string.
 * @return Head of the token list, or NULL on failure.
 */
t_tokensll	*build_tokensll(t_app *app, char *str);

/* === utils_lexer_build.c === */
int			quotes_build(char *str, t_tokensll *token, char quote);
int			char_build(char ch, t_tokensll *token);
int			backslash_build(char *str, t_tokensll *token);

/* === utils_lexer_token.c === */
/**
 * @brief Initialize a pipe operator token.
 */
int			build_ops_pipe(t_tokensll *token);

/**
 * @brief Initialize a redirection output (>) token.
 */
int			build_ops_dirout(t_tokensll *token);

/**
 * @brief Initialize a redirection append (>>) token.
 */
int			build_ops_dirappnd(t_tokensll *token);

/**
 * @brief Initialize a redirection input (<) token.
 */
int			build_ops_dirin(t_tokensll *token);

/**
 * @brief Initialize a heredoc (<<) token.
 */
int			build_ops_heredoc(t_tokensll *token);

/* === utils_lexer.c === */
/**
 * @brief Build a string token from the given buffer.
 */
int			string_build(t_tokensll *token, char *str);

/**
 * @brief Build a special token (operator) from the input.
 * @return Number of characters consumed, or negative on failure.
 */
int			special_build(char *str, t_tokensll *token);

/* === utils_sll.c === */
/**
 * @brief Allocate and initialize a new token node.
 */
t_tokensll	*init_token(void);

/**
 * @brief Allocate and initialize sll iterator helpers.
 */
t_sll_ops	*init_sll_ops(void);

/**
 * @brief Count the number of nodes in a token list.
 */
int			ft_sllsize(t_tokensll *sll);

/**
 * @brief Free a single token node and its contents.
 */
void		freetoken(t_tokensll *token);

/**
 * @brief Free a token list and all its nodes.
 */
void		freetokensll(t_tokensll *sll);

/* === utils_validator_tokens.c === */
int			ispipe(t_token_type type);
int			isredir(t_token_type type);
int			is_valid_pipe(t_tokensll *tokens);
int			is_valid_redir(t_tokensll *tokens);

/* === validator_tokens.c === */
int			validate_tokensll(t_app *app);

#endif
