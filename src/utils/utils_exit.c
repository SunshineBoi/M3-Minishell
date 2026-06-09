#include "minishell.h"

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