#ifndef BUILTIN_H
# define BUILTIN_H

int	is_builtin(const char *cmd);
int	exec_builtin(char **argv, t_app *app);
int	builtin_echo(char **argv);
int	builtin_cd(char **argv, t_app *app);
int	builtin_pwd(void);
int	builtin_export(char **argv, t_app *app);
int	builtin_unset(char **argv, t_app *app);
int	builtin_env(t_app *app);
int	builtin_exit(char **argv, t_app *app);

#endif
