#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <sys/types.h>
# include <signal.h>
# include <errno.h>
# include <sys/wait.h>
# include <limits.h>

typedef enum e_qstate
{
	Q_NONE,
	Q_SQUOTE,
	Q_DQUOTE
}	t_qstate;

# include "ast.h"
# include "parser.h"
# include "exec.h"

# define BUFFER_SIZE 4096
# define APP "minishell"

extern volatile sig_atomic_t g_signal;

void	signals_at_prompt(void);

typedef enum e_errcode
{
	ERR_MALLOC = -1, // malloc/realloc failed
	ERR_QUOTE = -2, // unclosed quote in lexer
	ERR_SYNTAX = -3, // grammartically syntax error
	ERR_CMDNEXEC = -4, // permission denied / is a directory / ambiguous redir
	ERR_CMDNFOUND = -5, // command not found in PATH
	ERR_PIPE = -6, // pipe() syscall failed
	ERR_FORK = -7, // fork() syscall failed
	ERR_CD = -8,
	ERR_REDIR = -9,
}	t_errcode;

/**
 * @brief Represents standard POSIX exit status codes for a process.
 *
 * These codes are returned to the operating system upon process termination
 * to indicate the outcome of the execution.
 */
typedef enum e_exitcode
{
    EX_OK = 0,   /**< Program executed successfully. */
    EX_ERR = 1,   /**< General catchall for minor/generic errors. */
	EX_SYNTAX = 2,
    EX_CMD_NEXEC   = 126, /**< Command invoked but is not executable. */
    EX_CMD_NOTFOUND = 127, /**< Command cannot be found in PATH. */
    EX_SIG_BASE    = 128  /**< Base offset for fatal signal terminations (EX_SIG_BASE + signum). */
}	t_exitcode;



typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef struct s_app
{
	t_tokensll	*tokensll;
	t_ast_node	*ast;
	char		**envp;
	t_env		*env_list;
	int			exitcode;
}	t_app;

/* === utils_exit.c === */
/**
 * @brief Terminate the program immediately.
 *
 * Intended for unrecoverable error paths where cleanup is either not
 * possible or not required.
 */
void	hardexit();
void	setexit(t_app *app, t_exitcode code);

/* === env.c === */
t_env	*env_init(char **envp);
void	env_free(t_env *list);
char	*env_get(t_env *list, const char *key);
int		env_set(t_env **list, const char *key, const char *value);
int		env_unset(t_env **list, const char *key);
char	**env_to_array(t_env *list);
void	env_free_array(char **arr);

/* === utils_lexer_build.c === */
int	quotes_build(char *str, t_tokensll *token, char quote);
int	char_build(char ch, t_tokensll *token);
int backslash_build(char *str, t_tokensll *token);

// * utils_lexer_token.c
/**
 * @brief Initialize a pipe operator token.
 * @param token Token node to populate.
 * @return 0 on success, nonzero on failure.
 */
int	build_ops_pipe(t_tokensll *token);

/**
 * @brief Initialize a redirection output (>) token.
 * @param token Token node to populate.
 * @return 0 on success, nonzero on failure.
 */
int	build_ops_dirout(t_tokensll *token);

/**
 * @brief Initialize a redirection append (>>) token.
 * @param token Token node to populate.
 * @return 0 on success, nonzero on failure.
 */
int	build_ops_dirappnd(t_tokensll *token);

/**
 * @brief Initialize a redirection input (<) token.
 * @param token Token node to populate.
 * @return 0 on success, nonzero on failure.
 */
int	build_ops_dirin(t_tokensll *token);

/**
 * @brief Initialize a heredoc (<<) token.
 * @param token Token node to populate.
 * @return 0 on success, nonzero on failure.
 */
int	build_ops_heredoc(t_tokensll *token);

// * utils_lexer.c
/**
 * @brief Build a string token from the given buffer.
 * @param token Token node to populate.
 * @param str Source string buffer.
 * @return 0 on success, nonzero on failure.
 */
int	string_build(t_tokensll *token, char *str);

/**
 * @brief Build a special token (operator) from the input.
 * @param str Input buffer starting at the operator.
 * @param token Token node to populate.
 * @return Number of characters consumed, or negative on failure.
 */
int	special_build(char *str, t_tokensll *token);

// utils_mem.c
/**
 * @brief Copy memory from src to dest.
 * @param dest Destination buffer.
 * @param src Source buffer.
 * @param nbyte Number of bytes to copy.
 * @return Destination pointer.
 */
void	*ft_memcpy(void *dest, const void *src, size_t nbyte);

/**
 * @brief Reallocate a string buffer to a new size.
 * @param old Existing buffer (may be NULL).
 * @param old_size Size of the existing buffer in bytes.
 * @param new_size Desired buffer size in bytes.
 * @return Newly allocated buffer, or NULL on failure.
 */
char	*ft_realloc(char *old, size_t old_size, size_t new_size);

// utils_print.c
/**
 * @brief Write a string to a file descriptor.
 * @param s Null-terminated string to write.
 * @param fd File descriptor to write to.
 */
void	ft_putstr_fd(char *s, int fd);
void	print_tokensll(t_tokensll *tokensll);
void	errmsg(const char *prefix, const char *arg, const char *msg);

// utils_printerr.c
void	printerr_syscall(t_errcode code);
void	printerr_syntax(char *tokval);
void	printerr_quotes();
void	printerr_redir(char *filename);
void	printerr_cmdnfound(char *cmd);
void	ft_perror(char *msg);

// utils_printerr_builtin.c
void	printerr_cd(char *filename);

// utils_sll.c
/**
 * @brief Allocate and initialize a new token node.
 * @return Pointer to a new token node, or NULL on failure.
 */
t_tokensll	*init_token();

/**
 * @brief Allocate and initialize sll iterator helpers.
 * @return Pointer to ops structure, or NULL on failure.
 */
t_sll_ops	*init_sll_ops();

/**
 * @brief Count the number of nodes in a token list.
 * @param sll Head of the list.
 * @return Number of nodes.
 */
int	ft_sllsize(t_tokensll *sll);

/**
 * @brief Free a single token node and its contents.
 * @param token Token node to free.
 */
void	freetoken(t_tokensll *token);

/**
 * @brief Free a token list and all its nodes.
 * @param sll Head of the list.
 */
void	freetokensll(t_tokensll *sll);

// utils_argv.c
int		argv_builder_init(t_argv_builder *ab);
int		argv_builder_push(t_argv_builder *ab, char *word);
void	argv_builder_free(t_argv_builder *ab);
void	free_argv(char **argv);
int		push_words_to_builder(t_argv_builder *ab, char **words, size_t count);

// utils_redir.c
t_redir	*redir_new(t_redir_type type, const char *target);
void	redir_free(t_redir *redir);
t_redir	*redir_append(t_redir *head, t_redir *node);

// utils_strbuf.c
int		sb_init(t_strbuf *sb);
int		sb_reserve(t_strbuf *sb, size_t needed);
int		sb_push_char(t_strbuf *sb, char c);
int		sb_push_str(t_strbuf *sb, const char *s);
void	sb_free(t_strbuf *sb);

// utils_wordlist.c
int		wl_init(t_wordlist *wl);
int		wl_push(t_wordlist *wl, char *word);
void	wl_free(t_wordlist *wl);
void	free_words(char **words);
void	free_words_from(char **words, size_t start);

// utils_env.c
char	*itoa_status(int status);
int		is_name_start(char c);
size_t	var_name_len(const char *s);
const char	*env_lookup(char **envp, const char *key, size_t key_len);

// utils_str.c
/**
 * @brief Compute the length of a null-terminated string.
 * @param str Input string.
 * @return Length excluding the null terminator.
 */
size_t	ft_strlen(const char *str);

/**
 * @brief Duplicate up to len characters into a new string.
 * @param str Source buffer.
 * @param len Maximum number of characters to copy.
 * @return Newly allocated string, or NULL on failure.
 */
char	*ft_strndup(char *str, int len);

/**
 * @brief Compare two null-terminated strings.
 * @param s1 First string.
 * @param s2 Second string.
 * @return Difference between the first differing characters.
 */
int	ft_strcmp(const char *s1, const char *s2);
char	*ft_strdup(const char *s);
char	*ft_strjoin(const char *s1, const char *s2);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strchr(const char *s, int c);
char	*ft_itoa(int n);

// utils_validator_tokens.c
int	ispipe(t_token_type type);
int	isredir(t_token_type type);
int	is_valid_pipe(t_tokensll *tokens);
int	is_valid_redir(t_tokensll *tokens);
int	validate_tokensll(t_app *app);

// utils_validator.c
/**
 * @brief Check if a character is considered whitespace.
 * @param ch Character code to test.
 * @return Nonzero if whitespace, zero otherwise.
 */
int	iswhitespace(int ch);

/**
 * @brief Check if a character is a special shell symbol.
 * @param ch Character code to test.
 * @return Nonzero if special, zero otherwise.
 */
int	isspecialsym(int ch);

// lexer.c
/**
 * @brief Tokenize the input string into a linked list.
 * @param str Input command line string.
 * @return Head of the token list, or NULL on failure.
 */
t_tokensll	*build_tokensll(char *str);

// expand.c
/**
 * @brief Expand variables, quotes, and field-splitting for a single word.
 * @param input Raw word with quotes preserved from the lexer.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @param out_words Output NULL-terminated array of expanded words.
 * @param out_count Output count of words.
 * @return 0 on success, ERR_MALLOC on failure.
 */
int	expand_word(const char *input, char **envp, int last_status,
		char ***out_words, size_t *out_count);

/**
 * @brief Expand a command argv list into a new argv list.
 * @param argv Original argv list.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @param out_argv Output NULL-terminated argv list.
 * @return 0 on success, ERR_MALLOC on failure.
 */
int	expand_argv(char **argv, char **envp, int last_status, char ***out_argv);

/**
 * @brief Expand redirection targets in-place.
 * @param redir Redirection list.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @return 0 on success, ERR_* on failure.
 */
int	expand_redirs(t_redir *redir, char **envp, int last_status);

/**
 * @brief Expand all command nodes in an AST recursively.
 * @param node Root node.
 * @param envp Environment array (KEY=VALUE).
 * @param last_status Last command exit status.
 * @return 0 on success, ERR_* on failure.
 */
int	expand_ast(t_ast_node *node, char **envp, int last_status);

// * === exec.c === *
int	execute_ast(t_app *app, t_ast_node *ast);
int	is_builtin(const char *cmd);
int	exec_builtin(char **argv, t_app *app);

// * === builtins === *
int	builtin_echo(char **argv);
int	builtin_cd(char **argv, t_app *app);
int	builtin_pwd(void);
int	builtin_export(char **argv, t_app *app);
int	builtin_unset(char **argv, t_app *app);
int	builtin_env(t_app *app);
int	builtin_exit(char **argv, t_app *app);

#endif
