/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_env.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 12:09:14 by kong              #+#    #+#             */
/*   Updated: 2026/07/03 12:09:15 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief env builtin — print all environment variables with values.
 *
 * Variables with NULL values (export-only) are excluded.
 */
int	builtin_env(t_app *app)
{
	int	i;

	i = 0;
	while (app->envp && app->envp[i])
	{
		ft_putstr_fd(app->envp[i], 1);
		write(1, "\n", 1);
		i++;
	}
	return (EX_OK);
}
