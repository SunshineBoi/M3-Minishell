#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "ast.h"

# define BUFFER_SIZE 4096



typedef enum s_errcode
{
	ERR_MALLOC = -1,
	ERR_QUOTE = -2,
	ERR_CMDNEXEC = -3,
	ERR_CMDNFOUND = -4,
	ERR_PIPE = -5,
	ERR_FORK = -6,
}	t_errcode;

/**
 * @brief Represents standard POSIX exit status codes for a process.
 *
 * These codes are returned to the operating system upon process termination
 * to indicate the outcome of the execution.
 */
typedef enum e_exit_code
{
    EX_OK          = 0,   /**< Program executed successfully. */
    EX_ERR         = 1,   /**< General catchall for minor/generic errors. */
    EX_CMD_NEXEC   = 126, /**< Command invoked but is not executable. */
    EX_CMD_NOTFOUND = 127, /**< Command cannot be found in PATH. */
    EX_SIG_BASE    = 128  /**< Base offset for fatal signal terminations (EX_SIG_BASE + signum). */
} exit_code_t;

typedef enum s_token_type
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

typedef struct s_app
{
	t_sll_ops	*llops;
	t_tokensll	*tokensll;
	char		*envp;
	int			exitcode;
}	t_app;

// utils_exit.c
/**
 * @brief Terminate the program immediately.
 *
 * Intended for unrecoverable error paths where cleanup is either not
 * possible or not required.
 */
void	hardexit();

// utils_lexer_token.c
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

// utils_lexer.c
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
/**
 * @brief Print a human-readable error message for an error code.
 * @param code Internal error code.
 */
void	printerr(t_errcode code);

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

#endif