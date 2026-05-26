#include "minishell.h"



static int	is_name_start(char c)
{
	return ((c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| c == '_');
}

static int	is_name_char(char c)
{
	return (is_name_start(c) || (c >= '0' && c <= '9'));
}

static size_t	var_name_len(const char *s)
{
	size_t	len;

	if (!s || !is_name_start(*s))
		return (0);
	len = 0;
	while (s[len] && is_name_char(s[len]))
		len++;
	return (len);
}

static const char	*env_lookup(char **envp, const char *key, size_t key_len)
{
	size_t	i;

	if (!envp || !key)
		return ("");
	i = 0;
	while (envp[i])
	{
		if (ft_strlen(envp[i]) > key_len
			&& envp[i][key_len] == '=')
		{
			size_t	j = 0;
			while (j < key_len && envp[i][j] == key[j])
				j++;
			if (j == key_len)
				return (envp[i] + key_len + 1);
		}
		i++;
	}
	return ("");
}

static char	*itoa_status(int status)
{
	char	buf[12];
	int		neg;
	int		i;
	long	n;
	char	*out;
	int		j;

	n = status;
	neg = (n < 0);
	if (neg)
		n = -n;
	i = 0;
	if (n == 0)
		buf[i++] = '0';
	while (n > 0)
	{
		buf[i++] = (char)('0' + (n % 10));
		n /= 10;
	}
	if (neg)
		buf[i++] = '-';
	out = malloc((size_t)i + 1);
	if (!out)
		return (NULL);
	out[i] = '\0';
	j = 0;
	while (i-- > 0)
		out[j++] = buf[i];
	return (out);
}

static int	sb_init(t_strbuf *sb)
{
	sb->cap = 16;
	sb->len = 0;
	sb->buf = malloc(sb->cap + 1);
	if (!sb->buf)
		return (ERR_MALLOC);
	sb->buf[0] = '\0';
	return (0);
}

static int	sb_reserve(t_strbuf *sb, size_t needed)
{
	char	*newbuf;
	size_t	newcap;

	if (needed + 1 <= sb->cap)
		return (0);
	newcap = (needed + 1) * 2;
	newbuf = ft_realloc(sb->buf, sb->len, newcap);
	if (!newbuf)
		return (ERR_MALLOC);
	sb->buf = newbuf;
	sb->cap = newcap;
	return (0);
}

static int	sb_push_char(t_strbuf *sb, char c)
{
	if (sb_reserve(sb, sb->len + 1) != 0)
		return (ERR_MALLOC);
	sb->buf[sb->len++] = c;
	sb->buf[sb->len] = '\0';
	return (0);
}

static int	sb_push_str(t_strbuf *sb, const char *s)
{
	size_t	add;

	if (!s)
		return (0);
	add = ft_strlen(s);
	if (add == 0)
		return (0);
	if (sb_reserve(sb, sb->len + add) != 0)
		return (ERR_MALLOC);
	ft_memcpy(sb->buf + sb->len, s, add);
	sb->len += add;
	sb->buf[sb->len] = '\0';
	return (0);
}

static void	sb_clear(t_strbuf *sb)
{
	sb->len = 0;
	if (sb->buf)
		sb->buf[0] = '\0';
}

static void	sb_free(t_strbuf *sb)
{
	free(sb->buf);
	sb->buf = NULL;
	sb->len = 0;
	sb->cap = 0;
}

static int	wl_init(t_wordlist *wl)
{
	wl->cap = 4;
	wl->count = 0;
	wl->items = malloc(sizeof(char *) * wl->cap);
	if (!wl->items)
		return (ERR_MALLOC);
	wl->items[0] = NULL;
	return (0);
}

static int	wl_push(t_wordlist *wl, char *word)
{
	char	**newitems;

	if (wl->count + 1 >= wl->cap)
	{
		size_t	old_cap;

		old_cap = wl->cap;
		wl->cap *= 2;
		newitems = (char **)ft_realloc((char *)wl->items,
					sizeof(char *) * old_cap,
					sizeof(char *) * wl->cap);
		if (!newitems)
			return (ERR_MALLOC);
		wl->items = newitems;
	}
	wl->items[wl->count++] = word;
	wl->items[wl->count] = NULL;
	return (0);
}

static void	wl_free(t_wordlist *wl)
{
	size_t	i;

	if (!wl->items)
		return ;
	i = 0;
	while (i < wl->count)
	{
		free(wl->items[i]);
		i++;
	}
	free(wl->items);
	wl->items = NULL;
	wl->count = 0;
	wl->cap = 0;
}

static int	flush_word(t_wordlist *wl, t_strbuf *sb, int force_empty)
{
	char	*word;

	if (sb->len == 0 && !force_empty)
		return (0);
	if (sb->len == 0)
		word = ft_strndup("", 0);
	else
		word = ft_strndup(sb->buf, (int)sb->len);
	if (!word)
		return (ERR_MALLOC);
	if (wl_push(wl, word) != 0)
	{
		free(word);
		return (ERR_MALLOC);
	}
	sb_clear(sb);
	return (0);
}

static int	append_unquoted_text(t_strbuf *sb, t_wordlist *wl,
		const char *text, int *word_in_progress)
{
	size_t	i;

	if (!text)
		return (0);
	i = 0;
	while (text[i])
	{
		if (iswhitespace(text[i]))
		{
			if (*word_in_progress)
			{
				if (flush_word(wl, sb, 1) != 0)
					return (ERR_MALLOC);
				*word_in_progress = 0;
			}
			i++;
			continue ;
		}
		*word_in_progress = 1;
		if (sb_push_char(sb, text[i]) != 0)
			return (ERR_MALLOC);
		i++;
	}
	return (0);
}

static void	free_words(char **words)
{
	size_t	i;

	if (!words)
		return ;
	i = 0;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

static void	free_words_from(char **words, size_t start)
{
	size_t	i;

	if (!words)
		return ;
	i = start;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

int	expand_word(const char *input, char **envp, int last_status,
		char ***out_words, size_t *out_count)
{
	t_qstate	state;
	t_strbuf	sb;
	t_wordlist	wl;
	size_t		i;
	int			word_in_progress;

	if (!out_words || !out_count)
		return (ERR_MALLOC);
	if (sb_init(&sb) != 0)
		return (ERR_MALLOC);
	if (wl_init(&wl) != 0)
		return (sb_free(&sb), ERR_MALLOC);
	state = Q_NONE;
	word_in_progress = 0;
	i = 0;
	while (input && input[i])
	{
		if (state == Q_NONE && iswhitespace(input[i]))
		{
			if (word_in_progress)
			{
				if (flush_word(&wl, &sb, 1) != 0)
					return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				word_in_progress = 0;
			}
			i++;
			continue ;
		}
		if (input[i] == '\'' && state != Q_DQUOTE)
		{
			state = (state == Q_SQUOTE) ? Q_NONE : Q_SQUOTE;
			word_in_progress = 1;
			i++;
			continue ;
		}
		if (input[i] == '"' && state != Q_SQUOTE)
		{
			state = (state == Q_DQUOTE) ? Q_NONE : Q_DQUOTE;
			word_in_progress = 1;
			i++;
			continue ;
		}
		if (state == Q_NONE && input[i] == '\\')
		{
			if (input[i + 1] == '\n')
			{
				i += 2;
				continue ;
			}
			if (input[i + 1])
			{
				word_in_progress = 1;
				if (sb_push_char(&sb, input[i + 1]) != 0)
					return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				i += 2;
				continue ;
			}
			word_in_progress = 1;
			if (sb_push_char(&sb, '\\') != 0)
				return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
			i++;
			continue ;
		}
		if (state == Q_DQUOTE && input[i] == '\\' && input[i + 1] == '\n')
		{
			i += 2;
			continue ;
		}
		if (state == Q_DQUOTE && input[i] == '\\'
			&& (input[i + 1] == '\\' || input[i + 1] == '"'
				|| input[i + 1] == '$'))
		{
			word_in_progress = 1;
			if (sb_push_char(&sb, input[i + 1]) != 0)
				return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
			i += 2;
			continue ;
		}
		if (input[i] == '$' && state != Q_SQUOTE)
		{
			if (input[i + 1] == '?')
			{
				char	*status = itoa_status(last_status);
				if (!status)
					return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				if (state == Q_NONE)
				{
					if (append_unquoted_text(&sb, &wl, status,
								&word_in_progress) != 0)
						return (free(status), wl_free(&wl), sb_free(&sb),
								ERR_MALLOC);
				}
				else
				{
					word_in_progress = 1;
					if (sb_push_str(&sb, status) != 0)
						return (free(status), wl_free(&wl), sb_free(&sb),
								ERR_MALLOC);
				}
				free(status);
				i += 2;
				continue ;
			}
			if (input[i + 1] >= '0' && input[i + 1] <= '9')
			{
				i += 2;
				continue ;
			}
			if (is_name_start(input[i + 1]))
			{
				size_t	len = var_name_len(input + i + 1);
				char	*key = ft_strndup((char *)input + i + 1, (int)len);
				const char	*val;
				if (!key)
					return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				val = env_lookup(envp, key, len);
				free(key);
				if (state == Q_NONE)
				{
					if (append_unquoted_text(&sb, &wl, val,
								&word_in_progress) != 0)
						return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				}
				else
				{
					word_in_progress = 1;
					if (sb_push_str(&sb, val) != 0)
						return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
				}
				i += 1 + len;
				continue ;
			}
			word_in_progress = 1;
			if (sb_push_char(&sb, '$') != 0)
				return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
			i++;
			continue ;
		}
		word_in_progress = 1;
		if (sb_push_char(&sb, input[i]) != 0)
			return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
		i++;
	}
	if (word_in_progress)
	{
		if (flush_word(&wl, &sb, 1) != 0)
			return (wl_free(&wl), sb_free(&sb), ERR_MALLOC);
	}
	*out_words = wl.items;
	*out_count = wl.count;
	sb_free(&sb);
	return (0);
}

static int	argv_builder_init(t_argv_builder *ab)
{
	ab->cap = 4;
	ab->count = 0;
	ab->argv = malloc(sizeof(char *) * ab->cap);
	if (!ab->argv)
		return (ERR_MALLOC);
	ab->argv[0] = NULL;
	return (0);
}

static int	argv_builder_push(t_argv_builder *ab, char *word)
{
	char	**newargv;

	if (ab->count + 1 >= ab->cap)
	{
		size_t	old_cap;

		old_cap = ab->cap;
		ab->cap *= 2;
		newargv = (char **)ft_realloc((char *)ab->argv,
					sizeof(char *) * old_cap,
					sizeof(char *) * ab->cap);
		if (!newargv)
			return (ERR_MALLOC);
		ab->argv = newargv;
	}
	ab->argv[ab->count++] = word;
	ab->argv[ab->count] = NULL;
	return (0);
}

static void	argv_builder_free(t_argv_builder *ab)
{
	size_t	i;

	if (!ab->argv)
		return ;
	i = 0;
	while (i < ab->count)
	{
		free(ab->argv[i]);
		i++;
	}
	free(ab->argv);
	ab->argv = NULL;
	ab->count = 0;
	ab->cap = 0;
}

static void	free_argv(char **argv)
{
	size_t	i;

	if (!argv)
		return ;
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}

int	expand_argv(char **argv, char **envp, int last_status, char ***out_argv)
{
	t_argv_builder	ab;
	size_t		i;

	if (!out_argv)
		return (ERR_MALLOC);
	if (argv_builder_init(&ab) != 0)
		return (ERR_MALLOC);
	i = 0;
	while (argv && argv[i])
	{
		char	**words;
		size_t	count;
		size_t	j;

		if (expand_word(argv[i], envp, last_status, &words, &count) != 0)
			return (argv_builder_free(&ab), ERR_MALLOC);
		j = 0;
		while (j < count)
		{
			if (argv_builder_push(&ab, words[j]) != 0)
			{
				free_words_from(words, j);
				return (argv_builder_free(&ab), ERR_MALLOC);
			}
			j++;
		}
		free(words);
		i++;
	}
	*out_argv = ab.argv;
	return (0);
}

int	expand_redirs(t_redir *redir, char **envp, int last_status)
{
	char	**words;
	size_t	count;

	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			redir = redir->next;
			continue ;
		}
		if (expand_word(redir->target, envp, last_status, &words, &count) != 0)
			return (ERR_MALLOC);
		if (count != 1 || words[0][0] == '\0')
		{
			free_words(words);
			return (ERR_CMDNEXEC);
		}
		free(redir->target);
		redir->target = words[0];
		free(words);
		redir = redir->next;
	}
	return (0);
}

int	expand_ast(t_ast_node *node, char **envp, int last_status)
{
	char	**expanded;
	int		res;

	if (!node)
		return (0);
	if (node->type == NODE_CMD)
	{
		if (expand_argv(node->content.cmd.argv, envp,
				last_status, &expanded) != 0)
			return (ERR_MALLOC);
		free_argv(node->content.cmd.argv);
		node->content.cmd.argv = expanded;
		res = expand_redirs(node->content.cmd.redirs, envp, last_status);
		if (res != 0)
			return (res);
		return (0);
	}
	if (node->type == NODE_BINOP)
	{
		res = expand_ast(node->content.binop.left, envp, last_status);
		if (res != 0)
			return (res);
		res = expand_ast(node->content.binop.right, envp, last_status);
		if (res != 0)
			return (res);
	}
	return (0);
}
