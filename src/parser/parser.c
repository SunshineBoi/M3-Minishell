#include "minishell.h"

static char	*dup_str(const char *s)
{
	size_t	len;
	char	*dup;
	size_t	i;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	dup = malloc(len + 1);
	if (!dup)
		return (NULL);
	i = 0;
	while (i < len)
	{
		dup[i] = s[i];
		i++;
	}
	dup[i] = '\0';
	return (dup);
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

static t_redir	*redir_new(t_redir_type type, const char *target)
{
	t_redir	*redir;

	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->type = type;
	redir->target = dup_str(target);
	if (!redir->target)
	{
		free(redir);
		return (NULL);
	}
	redir->fd = -1;
	redir->next = NULL;
	return (redir);
}

static void	redir_free(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
		free(redir->target);
		free(redir);
		redir = next;
	}
}

static t_redir	*redir_append(t_redir *head, t_redir *node)
{
	t_redir	*tail;

	if (!head)
		return (node);
	tail = head;
	while (tail->next)
		tail = tail->next;
	tail->next = node;
	return (head);
}

static void	advance(t_parser *p)
{
	if (p->cur)
	{
		p->cur = p->cur->next;
		p->index++;
	}
}

static int	is_redir_token(t_token_type type)
{
	return (type == TOK_DIRIN || type == TOK_DIROUT
		|| type == TOK_DIRAPPND || type == TOK_HEREDOC);
}

static t_redir_type	tok_to_redir(t_token_type type)
{
	if (type == TOK_DIRIN)
		return (REDIR_IN);
	if (type == TOK_DIROUT)
		return (REDIR_OUT);
	if (type == TOK_DIRAPPND)
		return (REDIR_APPEND);
	return (REDIR_HEREDOC);
}

static t_ast_node	*parse_simple_command(t_parser *p)
{
	t_argv_builder	ab;
	t_redir		*redirs;
	t_tokensll		*tok;
	t_ast_node		*node;
	t_span			span;

	if (argv_builder_init(&ab) != 0)
		return (NULL);
	redirs = NULL;
	span.start = p->index;
	span.end = p->index;
	// loop all string until a pipe operator
	while (p->cur && (p->cur->type == TOK_STR || is_redir_token(p->cur->type)))
	{
		tok = p->cur;
		// if is a normal string
		if (tok->type == TOK_STR)
		{
			char	*word = dup_str(tok->val);
			if (!word || argv_builder_push(&ab, word) != 0)
			{
				free(word);
				argv_builder_free(&ab);
				redir_free(redirs);
				return (NULL);
			}
			advance(p);
			span.end = p->index;
			continue ;
		}
		// if is a redirection operator
		if (is_redir_token(tok->type))
		{
			t_redir	*new_redir;
			advance(p);
			if (!p->cur || p->cur->type != TOK_STR)
			{
				argv_builder_free(&ab);
				redir_free(redirs);
				return (NULL);
			}
			new_redir = redir_new(tok_to_redir(tok->type), p->cur->val);
			if (!new_redir)
			{
				argv_builder_free(&ab);
				redir_free(redirs);
				return (NULL);
			}
			redirs = redir_append(redirs, new_redir);
			advance(p);
			span.end = p->index;
			continue ;
		}
	}
	if (ab.count == 0 && !redirs)
	{
		argv_builder_free(&ab);
		return (NULL);
	}
	node = ast_new_cmd(ab.argv, redirs, span);
	if (!node)
	{
		argv_builder_free(&ab);
		redir_free(redirs);
		return (NULL);
	}
	return (node);
}

static t_ast_node	*parse_pipeline(t_parser *p)
{
	t_ast_node	*left;
	t_ast_node	*right;
	t_span		span;

	left = parse_simple_command(p);
	if (!left)
		return (NULL);
	while (p->cur && p->cur->type == TOK_PIPE)
	{
		t_ast_node	*old_left;

		advance(p);
		right = parse_simple_command(p);
		if (!right)
			return (ast_free(left), NULL);
		span.start = left->span.start;
		span.end = right->span.end;
		old_left = left;
		// if pipe exist, combine both left and right node
		left = ast_new_binop(BIN_PIPE, old_left, right, span);
		if (!left)
		{
			ast_free(old_left);
			ast_free(right);
			return (NULL);
		}
	}
	return (left);
}

t_ast_node	*parse_tokens(t_tokensll *tokens)
{
	t_parser	p;
	t_ast_node	*root;

	p.cur = tokens;
	p.index = 0;
	root = parse_pipeline(&p);
	if (!root)
		return (NULL);
	if (p.cur)
	{
		ast_free(root);
		return (NULL);
	}
	return (root);
}
