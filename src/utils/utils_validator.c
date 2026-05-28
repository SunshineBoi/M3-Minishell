#include "minishell.h"

/**
 * @brief Check if a character is whitespace.
 * @param ch Character code to test.
 * @return Nonzero if whitespace, zero otherwise.
 */
int	iswhitespace(int ch)
{
	return (ch == ' ' || ch == '\n' || ch == '\t'
		|| ch == '\f' || ch == '\v' || ch == '\r');
}

/**
 * @brief Check if a character is a special shell symbol.
 * @param ch Character code to test.
 * @return Nonzero if special, zero otherwise.
 */
int	isspecialsym(int ch)
{
	return (ch == '|' || ch == '>' || ch == '<');
}

int	ispipe(t_token_type type)
{
	return (type == TOK_PIPE);
}

int	isredir(t_token_type type)
{
	return (type == TOK_DIROUT || type == TOK_DIRAPPND
		|| type == TOK_DIRIN || type == TOK_HEREDOC);
}

int	is_valid_pipe(t_tokensll *tokens)
{
	// pipe cannot end with null
	// bash behavior is to prompt next line
	if (!tokens->next)
		return (printsynerr(tokens->val), 0);
	// pipe cannot followed by pipe
	else if (ispipe(tokens->next->type))
		return (printsynerr(tokens->val), 0);
	return (1);
}

int	is_valid_redir(t_tokensll *tokens)
{
	// redirect cannot end with null
	if (!tokens->next)
		return (printsynerr("newline"), 0);
	
	// redir cannot followed by pipe
	else if (ispipe(tokens->next->type))
		return (printsynerr(tokens->next->val), 0);

	// redir cannot followed by redir
	else if (isredir(tokens->next->type))
		return (printsynerr(tokens->next->val), 0);

	// only valid if redir is followed by normal string
	return (1);
}

int	validate_tokensll(t_tokensll *tokens)
{
	if (!tokens)
		return (1);
	// first token cannot be pipe
	if (ispipe(tokens->type))
		return (printsynerr(tokens->val), 0);
	while (tokens)
	{
		if (ispipe(tokens->type) && !is_valid_pipe(tokens))
			return (0);			
		else if (isredir(tokens->type) && !is_valid_redir(tokens))
			return (0);
		tokens = tokens->next;
	}
	return (1);
}
