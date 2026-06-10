#include "minishell.h"

/**
 * @brief Copy nbyte bytes from src to dest.
 * @param dest Destination buffer.
 * @param src Source buffer.
 * @param nbyte Number of bytes to copy.
 * @return dest, or NULL if dest/src is NULL.
 */
void	*ft_memcpy(void *dest, const void *src, size_t nbyte)
{
	unsigned char		*dest_p;
	const unsigned char	*src_p;

	if (!dest || !src)
		return (NULL);
	dest_p = dest;
	src_p = src;
	while (nbyte--)
		*dest_p++ = *src_p++;
	return (dest);
}

/**
 * @brief Fill the first nbyte bytes of dest with the value c.
 * @param dest Destination buffer.
 * @param c Value to set (converted to unsigned char).
 * @param nbyte Number of bytes to set.
 * @return dest, or NULL if dest is NULL.
 */
void	*ft_memset(void *dest, int c, size_t nbyte)
{
	unsigned char	*dest_p;

	if (!dest)
		return (NULL);
	dest_p = dest;
	while (nbyte--)
		*dest_p++ = (unsigned char)c;
	return (dest);
}

/**
 * @brief Reallocate a string buffer to a new size.
 * @param old Existing buffer (may be NULL).
 * @param old_size Size of existing content in bytes.
 * @param new_size Desired buffer size in bytes.
 * @return New buffer pointer, or NULL on failure.
 */
char	*ft_realloc(char *old, size_t old_size, size_t new_size)
{
	char	*new;

	new = malloc(new_size + 1);
	if (!new)
		return (NULL);
	if (old)
	{
		new = (char *)ft_memcpy(new, old, old_size);
		new[old_size] = '\0';
		// on success, free old string
		free(old);
	}
	return (new);
}

char	**ft_calloclst(int size)
{
	char	**lst;
	int		i;
	int		total;

	total = size + 1;
	lst = malloc(sizeof(char *) * total);
	if (!lst)
		return (NULL);
	i = 0;
	while (i < total)
	{
		lst[i] = NULL;
		i++;
	}
	return (lst);
}

void	freelst(char **lst)
{
	size_t	i;

	if (!lst)
		return ;
	i = 0;
	while (lst[i])
	{
		free(lst[i]);
		i++;
	}
	free(lst);
}