#include "minishell.h"
#include "exec_heredoc.h"

char	*parse_heredoc_delimiter(const char *target, int *is_quoted)
{
	t_strbuf	sb;
	size_t		i;

	*is_quoted = 0;
	if (sb_init(&sb) != 0)
		return (NULL);
	i = 0;
	while (target[i])
	{
		if (target[i] == '\'' || target[i] == '"')
		{
			*is_quoted = 1;
			i++;
		}
		else
		{
			if (sb_push_char(&sb, target[i]) != 0)
			{
				sb_free(&sb);
				return (NULL);
			}
			i++;
		}
	}
	return (sb.buf);
}

static void	append_expanded_var(t_strbuf *sb, const char *line, size_t *i,
		char **envp, int status)
{
	char	*status_str;
	size_t	var_len;

	if (line[*i + 1] == '?')
	{
		status_str = itoa_status(status);
		if (status_str)
			sb_push_str(sb, status_str);
		free(status_str);
		*i += 2;
	}
	else if (is_name_start(line[*i + 1]))
	{
		var_len = var_name_len(line + *i + 1);
		sb_push_str(sb, env_lookup(envp, line + *i + 1, var_len));
		*i += 1 + var_len;
	}
	else
	{
		sb_push_char(sb, '$');
		(*i)++;
	}
}

char	*expand_heredoc_line(const char *line, char **envp, int last_status)
{
	t_strbuf	sb;
	size_t		i;

	if (sb_init(&sb) != 0)
		return (NULL);
	i = 0;
	while (line[i])
	{
		if (line[i] == '$' && line[i + 1])
			append_expanded_var(&sb, line, &i, envp, last_status);
		else
		{
			sb_push_char(&sb, line[i]);
			i++;
		}
	}
	return (sb.buf);
}
