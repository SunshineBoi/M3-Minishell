/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_print.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 20:26:35 by kong              #+#    #+#             */
/*   Updated: 2026/05/20 20:26:35 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	ft_putstr_fd(char *s, int fd)
{
	if (!s || fd < 0)
		return ;
	write(fd, s, ft_strlen(s));
}

// ! for further review later
void	printerr(t_errcode code)
{
	if (code == ERR_MALLOC)
		perror("minishell");
	else if (code == ERR_QUOTE)
		ft_putstr_fd("minishell: Invalid quotes\n", 2);
}
