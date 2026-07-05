/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:58:07 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/05 18:58:08 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPANDER_H
# define EXPANDER_H

# include <stddef.h>
# include "ast.h"
# include "envp.h"
# include "parser.h"

typedef struct s_app	t_app;

/* === expand.c === */
/**
 * @brief Expand variables, quotes, and field-splitting for a single word.
 * @param input Raw word with quotes preserved from the lexer.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @param out Output word list.
 * @return 0 on success, ERR_MALLOC on failure.
 */
int			expand_word(const char *input, char **envp, int last_status,
				t_wordlist *out);

/**
 * @brief Expand a command argv list into a new argv list.
 * @param argv Original argv list.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @param out_argv Output NULL-terminated argv list.
 * @return 0 on success, ERR_MALLOC on failure.
 */
int			expand_argv(char **argv, char **envp, int last_status,
				char ***out_argv);

/**
 * @brief Expand redirection targets in-place.
 * @param redir Redirection list.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @return 0 on success, ERR_* on failure.
 */
int			expand_redirs(t_redir *redir, char **envp, int last_status);

/**
 * @brief Expand all command nodes in an AST recursively.
 * @param app Application context.
 * @param node Root node.
 * @return 0 on success, ERR_* on failure.
 */
int			expand_ast(t_app *app, t_ast_node *node);

/* === expand_assignment.c === */
typedef struct s_assign_ctx
{
	t_app	*app;
	t_env	*tmp_env;
	int		last_status;
	int		count;
	int		has_cmd;
}			t_assign_ctx;

int			process_assignments(t_app *app, t_cmd_node *cmd, int last_status);

/* === expand_assignment_utils.c === */
int			is_valid_assignment(const char *arg, char **name, char **value);
int			assignment_prefix_count(char **argv);
void		shift_argv_left(char **argv);

/* === expand_helpers_1.c / expand_helpers_2.c === */
int			init_ctx(t_expand_ctx *ctx, const char *input, char **envp,
				int last_status);
int			flush_word(t_wordlist *wl, t_strbuf *sb, int force_empty);
int			append_unquoted_text(t_strbuf *sb, t_wordlist *wl, const char *text,
				int *word_in_progress);
int			handle_whitespace(t_expand_ctx *ctx);
int			handle_quotes(t_expand_ctx *ctx);
int			expand_step(t_expand_ctx *ctx);

/* === utils_strbuf.c === */
int			sb_init(t_strbuf *sb);
int			sb_reserve(t_strbuf *sb, size_t needed);
int			sb_push_char(t_strbuf *sb, char c);
int			sb_push_str(t_strbuf *sb, const char *s);
void		sb_free(t_strbuf *sb);

/* === utils_wordlist.c === */
int			wl_init(t_wordlist *wl);
int			wl_push(t_wordlist *wl, char *word);
void		wl_free(t_wordlist *wl);
void		free_words_from(char **words, size_t start);

/* === utils_expand_env.c === */
char		*itoa_status(int status);
int			is_name_start(char c);
size_t		var_name_len(const char *s);
const char	*env_lookup(char **envp, const char *key, size_t key_len);

#endif
