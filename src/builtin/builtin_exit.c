#include "minishell.h"

/**
 * @brief Count the number of arguments in argv.
 */
static int	argv_count(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
		i++;
	return (i);
}

/**
 * @brief Check if a string is a valid numeric argument.
 *
 * Allows optional leading +/- and digits only.
 * Also checks for 64-bit integer overflow/underflow.
 */
static int	is_numeric(const char *s, long long *val)
{
	int			i;
	int			sign;
	long long	result;

	i = 0;
	sign = 1;
	result = 0;
	while (s[i] == ' ' || (s[i] >= '\t' && s[i] <= '\r'))
		i++;
	if (s[i] == '+' || s[i] == '-')
	{
		if (s[i] == '-')
			sign = -1;
		i++;
	}
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		if (sign == 1 && (result > (LLONG_MAX - (s[i] - '0')) / 10))
			return (0);
		if (sign == -1 && (-result < (LLONG_MIN + (s[i] - '0')) / 10))
			return (0);
		result = result * 10 + (s[i] - '0');
		i++;
	}
	*val = result * sign;
	return (1);
}

/**
 * @brief exit builtin — exit the shell.
 *
 * Rules:
 *   - "exit" alone → exit with last status
 *   - "exit 42" → exit with status 42 (mod 256)
 *   - "exit abc" → "numeric argument required", exit with 2
 *   - "exit 1 2" → "too many arguments", do not exit, status 1
 */
int	builtin_exit(char **argv, t_app *app)
{
	long long	val;
	int			argc;

	ft_putstr_fd("exit\n", 2);
	argc = argv_count(argv);
	if (argc == 1)
	{
		env_free(app->env_list);
		exit(app->exitcode);
	}
	if (!is_numeric(argv[1], &val))
	{
		errmsg("exit", argv[1], "numeric argument required");
		env_free(app->env_list);
		exit(2);
	}
	if (argc > 2)
	{
		errmsg("exit", NULL, "too many arguments");
		return (1);
	}
	env_free(app->env_list);
	exit((unsigned char)val);
	return (0);
}
