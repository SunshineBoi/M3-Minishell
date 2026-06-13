#include "minishell.h"

/**
 * @brief Check if an argument is a valid -n flag.
 *
 * A valid -n flag starts with '-' followed by one or more 'n' characters
 * exclusively (e.g. -n, -nn, -nnn). Anything else like -na is not valid.
 */
static int	is_n_flag(const char *arg)
{
	int	i;

	if (!arg || arg[0] != '-' || arg[1] != 'n')
		return (0);
	i = 1;
	while (arg[i])
	{
		if (arg[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief echo builtin — print arguments to stdout.
 *
 * Accepts any number of consecutive -n flags to suppress trailing newline.
 */
int	builtin_echo(char **argv)
{
	int	i;
	int	newline;

	newline = 1;
	i = 1;
	while (argv[i] && is_n_flag(argv[i]))
	{
		newline = 0;
		i++;
	}
	while (argv[i])
	{
		ft_putstr_fd(argv[i], 1);
		if (argv[i + 1])
			write(1, " ", 1);
		i++;
	}
	if (newline)
		write(1, "\n", 1);
	return (0);
}
