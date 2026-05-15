/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_exit.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 20:49:23 by kong              #+#    #+#             */
/*   Updated: 2026/05/15 20:49:23 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	errexit(t_errcode code)
{
	if (code == ERR_MALLOC)
		perror("minishell");

	exit(EXIT_FAILURE);
}
