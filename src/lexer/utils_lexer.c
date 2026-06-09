#include "minishell.h"

/**
 * @brief Build a string token by consuming a word segment.
 * @param token Token node to populate.
 * @param str Input buffer at the start of a word.
 * @return Characters consumed, or ERR_QUOTE/ERR_MALLOC on failure.
 */
int	string_build(t_tokensll *token, char *str)
{
	int	len;
	int	built_skipped;

	len = 0;
	while (str[len] && !iswhitespace(str[len]) && !isspecialsym(str[len]))
	{
		built_skipped = 0;
		// ! to confirm if need to implement backslash
		if (str[len] == '\\')
			built_skipped = backslash_build(str + len + 1, token);
		else if (str[len] == '\'' || str[len] == '"')
			built_skipped = quotes_build(str + len, token, str[len]);
		else
			built_skipped = char_build(str[len], token);
		if (built_skipped == ERR_QUOTE)
			return (ERR_QUOTE);
		else if (built_skipped == ERR_MALLOC)
			return (ERR_MALLOC);
		len += built_skipped;
	}
	return (len);
}

/**
 * @brief Build a special operator token from the input.
 * @param str Input buffer at a special character.
 * @param token Token node to populate.
 * @return Characters consumed, or ERR_MALLOC on failure.
 */
int	special_build(char *str, t_tokensll *token)
{
	int	build_flag;

	build_flag = 0;
	if (*str == '|')
		build_flag = build_ops_pipe(token);
	else if (*str == '>')
	{	
		if (*(str + 1) && *(str + 1) == '>')
			build_flag = build_ops_dirappnd(token);
		else
			build_flag = build_ops_dirout(token);
	}
	else if (*str == '<')
	{
		if (*(str + 1) && *(str + 1) == '<')
			build_flag = build_ops_heredoc(token);
		else
			build_flag = build_ops_dirin(token);
	}
	if (build_flag == ERR_MALLOC)
		return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	return (build_flag);
}
