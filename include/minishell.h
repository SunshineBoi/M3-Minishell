/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:07:29 by kong              #+#    #+#             */
/*   Updated: 2026/05/15 20:07:29 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <readline/readline.h>
# include <readline/history.h>

# define BUFFER_SIZE 4096

typedef enum s_errcode
{
	ERR_MALLOC = -1,
	ERR_QUOTE = -2,
	ERR_CMDNEXEC = -3,
	ERR_CMDNFOUND = -4,
	ERR_PIPE = -5,
	ERR_FORK = -6,
}	t_errcode;

typedef enum s_exitcode
{
	EXT_OK = 0,
	EXT_NOK = 1,
	EXT_CMDNEXEC = 126,
	EXT_CMDNFOUND = 127,
	EXT_SIGBASE = 128,
}	t_exitcode;

typedef enum s_token_type
{
	TOK_STR,
	TOK_PIPE,
	TOK_DIRIN,
	TOK_DIROUT,
	TOK_DIRAPPND,
	TOK_HEREDOC,
	TOK_EOF,
}	t_token_type;

typedef struct s_tokensll
{
	char				*val;
	long long			val_size;
	t_token_type		type;
	struct s_tokensll	*next;
}	t_tokensll;

typedef struct s_sll_ops
{
	t_tokensll	*curr;
	t_tokensll	*prev;
	t_tokensll	*head;
}	t_sll_ops;

typedef struct s_app
{
	t_sll_ops	*llops;
	t_tokensll	*tokensll;
	char		*envp;
	int			exitcode;
}	t_app;

// utils_exit.c
void	hardexit();

// utils_lexer_token.c
int	build_ops_pipe(t_tokensll *token);
int	build_ops_dirout(t_tokensll *token);
int	build_ops_dirappnd(t_tokensll *token);
int	build_ops_dirin(t_tokensll *token);
int	build_ops_heredoc(t_tokensll *token);

// utils_lexer.c
int	string_build(t_tokensll *token, char *str);
int	special_build(char *str, t_tokensll *token);

// utils_mem.c
void	*ft_memcpy(void *dest, const void *src, size_t nbyte);
char	*ft_realloc(char *old, size_t old_size, size_t new_size);

// utils_print.c
void	ft_putstr_fd(char *s, int fd);
void	printerr(t_errcode code);

// utils_sll.c
t_tokensll	*init_token();
t_sll_ops	*init_sll_ops();
int	ft_sllsize(t_tokensll *sll);
void	freetoken(t_tokensll *token);
void	freetokensll(t_tokensll *sll);

// utils_str.c
size_t	ft_strlen(const char *str);
char	*ft_strndup(char *str, int len);

// utils_validator.c
int	iswhitespace(int ch);
int	isspecialsym(int ch);

// lexer.c
t_tokensll	*build_tokensll(char *str);

#endif