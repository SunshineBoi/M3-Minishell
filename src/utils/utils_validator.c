/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_validator.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kong <kong@student.42singapore.sg>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 17:11:26 by kong              #+#    #+#             */
/*   Updated: 2026/05/22 17:11:26 by kong             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @brief Check if a character is whitespace.
 * @param ch Character code to test.
 * @return Nonzero if whitespace, zero otherwise.
 */
int	iswhitespace(int ch)
{
	return (ch == ' ' || ch == '\n' || ch == '\t'
		|| ch == '\f' || ch == '\v' || ch == '\r');
}

/**
 * @brief Check if a character is a special shell symbol.
 * @param ch Character code to test.
 * @return Nonzero if special, zero otherwise.
 */
int	isspecialsym(int ch)
{
	return (ch == '|' || ch == '>' || ch == '<');
}
