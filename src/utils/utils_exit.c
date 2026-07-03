/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_exit.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 15:38:18 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 15:38:18 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Terminate immediately with a nonzero exit code.
 */
void	hardexit(void)
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
