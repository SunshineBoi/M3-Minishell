#include "minishell.h"

// ! for further review later
/**
 * @brief Terminate immediately with a nonzero exit code.
 */
void	hardexit()
{
	exit(EX_ERR);
}

void	setexit(t_app *app, t_exitcode code)
{
	app->exitcode = code;
}

void	setexecerrno(t_app *app)
{
	if (errno == EACCES || errno == EISDIR || errno == ENOEXEC)
		setexit(app, EX_CMD_NEXEC);
	else
		setexit(app, EX_CMD_NOTFOUND);
}
