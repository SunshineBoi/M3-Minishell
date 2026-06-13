#include "minishell.h"

/**
 * @brief Update PWD and OLDPWD environment variables after chdir.
 */
static void	update_pwd(t_app *app, const char *old_pwd)
{
	char	cwd[PATH_MAX];

	if (old_pwd)
		env_set(&app->env_list, "OLDPWD", old_pwd);
	if (getcwd(cwd, sizeof(cwd)))
		env_set(&app->env_list, "PWD", cwd);
	update_env_array(app);
}

/**
 * @brief cd builtin — change directory.
 *
 * - No args → cd to $HOME
 * - "-" → cd to $OLDPWD, print new path
 * - Otherwise → cd to given path
 */
int	builtin_cd(char **argv, t_app *app)
{
	char	*target;
	char	*old_pwd;
	char	cwd[PATH_MAX];

	old_pwd = NULL;
	if (getcwd(cwd, sizeof(cwd)))
		old_pwd = cwd;
	if (!argv[1])
	{
		target = env_get(app->env_list, "HOME");
		if (!target)
			return (errmsg("cd", NULL, "HOME not set"), 1);
	}
	else if (ft_strcmp(argv[1], "-") == 0)
	{
		target = env_get(app->env_list, "OLDPWD");
		if (!target)
			return (errmsg("cd", NULL, "OLDPWD not set"), 1);
	}
	else
		target = argv[1];
	if (chdir(target) != 0)
	{
		errmsg("cd", target, strerror(errno));
		return (1);
	}
	if (argv[1] && ft_strcmp(argv[1], "-") == 0)
	{
		ft_putstr_fd(target, 1);
		write(1, "\n", 1);
	}
	update_pwd(app, old_pwd);
	return (0);
}
