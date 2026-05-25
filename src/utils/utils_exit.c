
#include "minishell.h"

/**
 * @brief Exit the program on fatal error codes.
 * @param code Internal error code to evaluate.
 * @return EXIT_FAILURE for error cases.
 */
int	errexit(t_errcode code)
{
	if (code == EXIT_FAILURE)
		return (exit(EXIT_FAILURE), EXIT_FAILURE);
	return (EXIT_FAILURE);
}

// ! for further review later
/**
 * @brief Terminate immediately with a nonzero exit code.
 */
void	hardexit()
{
	exit(EX_ERR);
}

/*
rl_clear_history -> need to clear history before shell exits
rl_on_new_line -> when ctrl+c, tell lib we have move to new line
*/