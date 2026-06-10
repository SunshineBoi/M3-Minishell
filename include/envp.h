#ifndef ENVP_H
# define ENVP_H

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

/* === envp.c === */

/**
 * @brief Look up a variable by key.
 * @param list Head of the environment list.
 * @param key Variable name to find.
 * @return The value string (may be NULL for export-only), or NULL if not found.
 */
char	*env_get(t_env *list, const char *key);
char	**env_to_array(t_env *list);
int		env_set(t_env **list, const char *key, const char *value);
int		env_unset(t_env **list, const char *key);
// void	env_free_array(char **arr);

/* === envp_utils.c === */

size_t	env_count(t_env *list);
void	env_free(t_env *list);
t_env	*env_init(char **envp);


#endif
