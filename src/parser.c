#include "minishell.h"



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
	redir->target = ft_strndup(target, ft_strlen(target));
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

static void	free_ab_redirs(t_argv_builder *ab, t_redir *redirs)
{
	argv_builder_free(ab);
	redir_free(redirs);
}

static int	parse_word(t_parser *p, t_argv_builder *ab)
{
	char	*word;

	word = ft_strndup(p->cur->val, ft_strlen(p->cur->val));
	if (!word)
		return (-1);
	if (argv_builder_push(ab, word) != 0)
	{
		free(word);
		return (-1);
	}
	advance(p);
	return (0);
}

static int	parse_redir(t_parser *p, t_redir **redirs)
{
	t_token_type	type;
	t_redir			*new_redir;

	type = p->cur->type;
	advance(p);
	if (!p->cur || p->cur->type != TOK_STR)
		return (-1);
	new_redir = redir_new(tok_to_redir(type), p->cur->val);
	if (!new_redir)
		return (-1);
	*redirs = redir_append(*redirs, new_redir);
	advance(p);
	return (0);
}

static t_ast_node	*parse_simple_command(t_parser *p)
{
	t_argv_builder	ab;
	t_redir		*redirs;
	t_ast_node	*node;
	int			start;

	if (argv_builder_init(&ab) != 0)
		return (NULL);
	redirs = NULL;
	start = p->index;
	while (p->cur && (p->cur->type == TOK_STR || is_redir_token(p->cur->type)))
	{
		if (p->cur->type == TOK_STR && parse_word(p, &ab) == 0)
			continue ;
		if (is_redir_token(p->cur->type) && parse_redir(p, &redirs) == 0)
			continue ;
		return (free_ab_redirs(&ab, redirs), NULL);
	}
	if (ab.count == 0 && !redirs)
		return (free_ab_redirs(&ab, redirs), NULL);
	node = ast_new_cmd(ab.argv, redirs, (t_span){start, p->index});
	if (!node)
		free_ab_redirs(&ab, redirs);
	return (node);
}

static void	free_left_right(t_ast_node *left, t_ast_node *right)
{
	ast_free(left);
	ast_free(right);
}

static t_ast_node	*parse_pipeline(t_parser *p)
{
	t_ast_node	*left;
	t_ast_node	*right;

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
		old_left = left;
		left = ast_new_binop(BIN_PIPE, old_left, right,
				(t_span){old_left->span.start, right->span.end});
		if (!left)
			return (free_left_right(old_left, right), NULL);
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
