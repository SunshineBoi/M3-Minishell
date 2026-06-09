#include "minishell.h"

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
		return (printerr_syntax(tokens->val), 0);
	// pipe cannot followed by pipe
	else if (ispipe(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);
	return (1);
}

int	is_valid_redir(t_tokensll *tokens)
{
	// redirect cannot end with null
	if (!tokens->next)
		return (printerr_syntax("newline"), 0);

	// redir cannot followed by pipe
	else if (ispipe(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);

	// redir cannot followed by redir
	else if (isredir(tokens->next->type))
		return (printerr_syntax(tokens->next->val), 0);

	// only valid if redir is followed by normal string
	return (1);
}

int	validate_tokensll(t_app *app)
{
	t_tokensll	*tokensll;

	tokensll = app->tokensll;
	if (!tokensll)
		return (1);
	// first token cannot be pipe
	if (ispipe(tokensll->type))
	{
		app->exitcode = EX_SYNTAX;
		printerr_syntax(tokensll->val);
		return (0);
	}
	while (tokensll)
	{
		if (ispipe(tokensll->type) && !is_valid_pipe(tokensll))
			app->exitcode = EX_SYNTAX;
		else if (isredir(tokensll->type) && !is_valid_redir(tokensll))
			app->exitcode = EX_SYNTAX;
		if (app->exitcode == EX_SYNTAX)
			return (0);
		tokensll = tokensll->next;
	}
	return (1);
}
