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

# include <stdio.h>
# include <stdlib.h>
# include <readline/readline.h>
# include <readline/history.h>

typedef enum s_errcode
{
	ERR_CMDNEXEC,
	ERR_CMDNFOUND,
	ERR_MALLOC,
	ERR_PIPE,
	ERR_FORK,
}	t_errcode;

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

typedef struct s_token
{
	char			*val;
	t_token_type	type;
	struct s_token	*next;
}	t_token;

#endif