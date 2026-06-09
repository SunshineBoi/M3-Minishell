#include "minishell.h"

int	wl_init(t_wordlist *wl)
{
	wl->cap = 4;
	wl->count = 0;
	wl->items = malloc(sizeof(char *) * wl->cap);
	if (!wl->items)
		return (ERR_MALLOC);
	wl->items[0] = NULL;
	return (0);
}

int	wl_push(t_wordlist *wl, char *word)
{
	char	**newitems;

	if (wl->count + 1 >= wl->cap)
	{
		size_t	old_cap;

		old_cap = wl->cap;
		wl->cap *= 2;
		newitems = (char **)ft_realloc((char *)wl->items,
					sizeof(char *) * old_cap,
					sizeof(char *) * wl->cap);
		if (!newitems)
			return (ERR_MALLOC);
		wl->items = newitems;
	}
	wl->items[wl->count++] = word;
	wl->items[wl->count] = NULL;
	return (0);
}

void	wl_free(t_wordlist *wl)
{
	size_t	i;

	if (!wl->items)
		return ;
	i = 0;
	while (i < wl->count)
	{
		free(wl->items[i]);
		i++;
	}
	free(wl->items);
	wl->items = NULL;
	wl->count = 0;
	wl->cap = 0;
}

void	free_words(char **words)
{
	size_t	i;

	if (!words)
		return ;
	i = 0;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

void	free_words_from(char **words, size_t start)
{
	size_t	i;

	if (!words)
		return ;
	i = start;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}
