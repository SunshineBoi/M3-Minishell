
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