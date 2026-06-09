#ifndef PARSER_H
# define PARSER_H

typedef enum e_token_type
{
	TOK_STR,
	TOK_PIPE,
	TOK_DIRIN,
	TOK_DIROUT,
	TOK_DIRAPPND,
	TOK_HEREDOC,
	TOK_EOF,
}	t_token_type;

typedef struct s_tokensll
{
	char				*val;
	long long			val_size;
	t_token_type		type;
	struct s_tokensll	*next;
}	t_tokensll;

typedef struct s_sll_ops
{
	t_tokensll	*curr;
	t_tokensll	*prev;
	t_tokensll	*head;
}	t_sll_ops;

typedef struct s_strbuf
{
	char	*buf;
	size_t	len;
	size_t	cap;
}	t_strbuf;

typedef struct s_wordlist
{
	char	**items;
	size_t	count;
	size_t	cap;
}	t_wordlist;

typedef struct s_argv_builder
{
	char	**argv;
	size_t	count;
	size_t	cap;
}	t_argv_builder;

typedef struct s_parser
{
	t_tokensll	*cur;
	int			index;
}	t_parser;

typedef struct s_expand_ctx
{
	const char	*input;
	char		**envp;
	int			last_status;
	t_qstate	state;
	t_strbuf	sb;
	t_wordlist	wl;
	size_t		i;
	int			word_in_progress;
}	t_expand_ctx;

/* === parser.c === */
void		advance(t_parser *p);
t_ast_node	*parse_tokens(t_tokensll *tokens);

/* === parser_helpers.c === */
int			parse_word(t_parser *p, t_argv_builder *ab);
int			parse_redir(t_parser *p, t_redir **redirs);
void		free_ab_redirs(t_argv_builder *ab, t_redir *redirs);
int			is_redir_token(t_token_type type);
t_redir_type	tok_to_redir(t_token_type type);

/* === expand_helpers === */
int			init_ctx(t_expand_ctx *ctx, const char *input, char **envp,
				int last_status);
int			flush_word(t_wordlist *wl, t_strbuf *sb, int force_empty);
int			append_unquoted_text(t_strbuf *sb, t_wordlist *wl,
				const char *text, int *word_in_progress);
int			handle_whitespace(t_expand_ctx *ctx);
int			handle_quotes(t_expand_ctx *ctx);
int			expand_step(t_expand_ctx *ctx);

#endif