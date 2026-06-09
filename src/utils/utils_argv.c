#include "minishell.h"

int	argv_builder_init(t_argv_builder *ab)
{
	ab->cap = 4;
	ab->count = 0;
	ab->argv = malloc(sizeof(char *) * ab->cap);
	if (!ab->argv)
		return (ERR_MALLOC);
	ab->argv[0] = NULL;
	return (0);
}

int	argv_builder_push(t_argv_builder *ab, char *word)
{
	char	**newargv;

	if (ab->count + 1 >= ab->cap)
	{
		size_t	old_cap;

		old_cap = ab->cap;
		ab->cap *= 2;
		newargv = (char **)ft_realloc((char *)ab->argv,
					sizeof(char *) * old_cap,
					sizeof(char *) * ab->cap);
		if (!newargv)
			return (ERR_MALLOC);
		ab->argv = newargv;
	}
	ab->argv[ab->count++] = word;
	ab->argv[ab->count] = NULL;
	return (0);
}

void	argv_builder_free(t_argv_builder *ab)
{
	size_t	i;

	if (!ab->argv)
		return ;
	i = 0;
	while (i < ab->count)
	{
		free(ab->argv[i]);
		i++;
	}
	free(ab->argv);
	ab->argv = NULL;
	ab->count = 0;
	ab->cap = 0;
}
