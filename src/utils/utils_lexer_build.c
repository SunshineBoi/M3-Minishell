#include "minishell.h"


/**
 * @brief Ensure token buffer can hold at least new_size bytes.
 * @param token Token with a dynamically sized buffer.
 * @param old_size Current used size.
 * @param new_size Desired size.
 * @return 1 on success, ERR_MALLOC on failure.
 */
static int	_resizebuffer(t_tokensll *token, int old_size, int new_size)
{
	if (new_size >= token->val_size)
	{
		token->val_size = (new_size) * 2;
		token->val = ft_realloc(token->val, old_size, token->val_size);
		if (!token->val)
			return (ERR_MALLOC);
	}
	return (1);
}

static int	_quotes_build_helper(char *str, char quote)
{
	int	i;

	i = 1;
	while (str[i])
	{
		if (str[i] == '\\' && str[i + 1] && quote == '"')
		{
			i += 2;
			continue ;
		}
		if (str[i] == quote)
			break ;
		i++;
	}
	if (str[i] == '\0')
		return (printerr_quotes(), ERR_QUOTE);
	i += 1;
	return (i);
}

/**
 * @brief Append a quoted segment to the token buffer.
 * @param str Input buffer starting at the quote.
 * @param token Token node to append to.
 * @param quote Quote character to match.
 * @return Characters consumed, or ERR_QUOTE/ERR_MALLOC on failure.
 */
int	quotes_build(char *str, t_tokensll *token, char quote)
{
	int	start_i;
	int	quote_len;

	start_i = 0;
	if (token && token->val == NULL)
	{
		token->val = malloc(token->val_size);
		if (!token->val)
			return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	}
	else
		start_i = ft_strlen(token->val);
	quote_len = _quotes_build_helper(str, quote);
	if (quote_len < 0)
		return (quote_len);
	// adjust buffer size if needed
	if (_resizebuffer(token, start_i, start_i + quote_len) == ERR_MALLOC)
		return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	// starts appending
	ft_memcpy((token->val) + start_i, str, quote_len);
	token->val[quote_len + start_i] = '\0';
	return (quote_len);
}

/**
 * @brief Append a single character to the token buffer.
 * @param ch Character to append.
 * @param token Token node to append to.
 * @return 1 on success, ERR_MALLOC on failure.
 */
int	char_build(char ch, t_tokensll *token)
{
	int	dest_i;

	dest_i = 0;
	if (token && token->val == NULL)
	{
		token->val = malloc(token->val_size);
		if (!token->val)
			return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	}
	else
		dest_i = ft_strlen(token->val);
	// starts appending
	if (dest_i + 1 >= token->val_size)
	{
		token->val_size = (dest_i + 1) * 2;
		token->val = ft_realloc(token->val, dest_i, token->val_size);
		if (!token->val)
			return (printerr_syscall(ERR_MALLOC), ERR_MALLOC);
	}
	token->val[dest_i] = ch;
	token->val[dest_i + 1] = '\0';
	return (1);
}

int backslash_build(char *str, t_tokensll *token)
{
	int	built_skipped;

	built_skipped = 0;
	if (*str)
	{
		built_skipped = char_build(*str, token);
		if (built_skipped == ERR_MALLOC)
			return (ERR_MALLOC);
		built_skipped = 2;
	}
	else
	{
		// ! to confirm if trailing \ throws an error or as literal?
		built_skipped = char_build('\\', token);
		if (built_skipped == ERR_MALLOC)
			return (ERR_MALLOC);
	}
	return (built_skipped);
}
