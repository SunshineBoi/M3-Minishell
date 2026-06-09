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

#endif