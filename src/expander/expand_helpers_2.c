#include "minishell.h"

static int	handle_backslash(t_expand_ctx *ctx)
{
	if (ctx->state == Q_DQUOTE && ctx->input[ctx->i] == '\\'
		&& (ctx->input[ctx->i + 1] == '\\'
			|| ctx->input[ctx->i + 1] == '"'
			|| ctx->input[ctx->i + 1] == '$'))
	{
		ctx->word_in_progress = 1;
		if (sb_push_char(&ctx->sb, ctx->input[ctx->i + 1]) != 0)
			return (ERR_MALLOC);
		ctx->i += 2;
		return (1);
	}
	return (0);
}

static int	handle_status(t_expand_ctx *ctx)
{
	char	*status;

	status = itoa_status(ctx->last_status);
	if (!status)
		return (ERR_MALLOC);
	if (ctx->state == Q_NONE)
	{
		if (append_unquoted_text(&ctx->sb, &ctx->wl, status,
				&ctx->word_in_progress) != 0)
			return (free(status), ERR_MALLOC);
	}
	else
	{
		ctx->word_in_progress = 1;
		if (sb_push_str(&ctx->sb, status) != 0)
			return (free(status), ERR_MALLOC);
	}
	free(status);
	ctx->i += 2;
	return (1);
}

static int	handle_env_var(t_expand_ctx *ctx)
{
	size_t		len;
	char		*key;
	const char	*val;

	len = var_name_len(ctx->input + ctx->i + 1);
	key = ft_strndup((char *)ctx->input + ctx->i + 1, (int)len);
	if (!key)
		return (ERR_MALLOC);
	val = env_lookup(ctx->envp, key, len);
	free(key);
	if (ctx->state == Q_NONE && append_unquoted_text(&ctx->sb, &ctx->wl, val,
			&ctx->word_in_progress) != 0)
		return (ERR_MALLOC);
	if (ctx->state != Q_NONE)
	{
		ctx->word_in_progress = 1;
		if (sb_push_str(&ctx->sb, val) != 0)
			return (ERR_MALLOC);
	}
	ctx->i += 1 + len;
	return (1);
}

static int	handle_variable(t_expand_ctx *ctx)
{
	if (ctx->input[ctx->i] == '$' && ctx->state != Q_SQUOTE)
	{
		if (ctx->input[ctx->i + 1] == '?')
			return (handle_status(ctx));
		if (ctx->input[ctx->i + 1] >= '0' && ctx->input[ctx->i + 1] <= '9')
		{
			ctx->i += 2;
			return (1);
		}
		if (is_name_start(ctx->input[ctx->i + 1]))
			return (handle_env_var(ctx));
		ctx->word_in_progress = 1;
		if (sb_push_char(&ctx->sb, '$') != 0)
			return (ERR_MALLOC);
		ctx->i++;
		return (1);
	}
	return (0);
}

int	expand_step(t_expand_ctx *ctx)
{
	int	res;

	res = handle_whitespace(ctx);
	if (res == 0)
		res = handle_quotes(ctx);
	if (res == 0)
		res = handle_backslash(ctx);
	if (res == 0)
		res = handle_variable(ctx);
	if (res != 0)
		return (res);
	ctx->word_in_progress = 1;
	if (sb_push_char(&ctx->sb, ctx->input[ctx->i++]) != 0)
		return (ERR_MALLOC);
	return (0);
}
