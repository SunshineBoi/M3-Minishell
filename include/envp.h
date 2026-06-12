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
/**
 * @brief Convert the environment list to a NULL-terminated char** array.
 *
 * Only variables with non-NULL values are included (matching `env` behavior).
 * Caller must free with freelst().
 */
char	**env_to_array(t_env *list);
/**
 * @brief Set or create an environment variable.
 * @param list Pointer to head pointer (may insert at head).
 * @param key Variable name.
 * @param value Variable value (NULL to mark export-only).
 * @return 0 on success, ERR_MALLOC on failure.
 */
int		env_set(t_env **list, const char *key, const char *value);
/**
 * @brief Remove a variable by key.
 * @param list Pointer to head pointer.
 * @param key Variable name to remove.
 * @return 0 always (silent success even if not found).
 */
int		env_unset(t_env **list, const char *key);
// void	env_free_array(char **arr);


/* === envp_utils.c === */
/**
 * @brief Count nodes in the environment list.
 */
size_t	env_count(t_env *list);
/**
 * @brief Free the entire environment list.
 * @param list Head of the list.
 */
void	env_free(t_env *list);
/**
 * @brief Create a new environment node.
 * @param key Variable name (duplicated).
 * @param value Variable value (duplicated, may be NULL).
 * @return New node, or NULL on malloc failure.
 */
t_env	*env_new_node(const char *key, const char *value);
/**
 * @brief Build the environment linked list from the envp array.
 * @param envp NULL-terminated array of "KEY=VALUE" strings.
 * @return Head of the new list, or NULL on failure.
 */
t_env	*env_init(char **envp);


#endif
