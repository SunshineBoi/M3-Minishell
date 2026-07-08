/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkai-yua <lkai-yua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:53:49 by lkai-yua          #+#    #+#             */
/*   Updated: 2026/07/05 18:53:51 by lkai-yua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTIN_H
# define BUILTIN_H

typedef struct s_app	t_app;

int			is_builtin(const char *cmd);
int			exec_builtin(char **argv, t_app *app);
int			builtin_echo(char **argv);
int			builtin_cd(char **argv, t_app *app);
const char	*resolve_cd_target(char **argv, t_app *app);
char		*build_logical_pwd(const char *old_pwd, const char *target);
char		*normalize_cd_path(char *path);
int			builtin_pwd(t_app *app);
int			is_valid_identifier(const char *name);
int			builtin_export(char **argv, t_app *app);
int			builtin_unset(char **argv, t_app *app);
int			builtin_env(t_app *app);
int			builtin_exit(char **argv, t_app *app);

#endif
