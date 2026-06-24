#include <criterion/criterion.h>
#include <string.h>
#include "minishell.h"

Test(env, init_get_set_unset)
{
	char envp0[] = "A=1";
	char envp1[] = "B=2";
	char envp2[] = "NOVAL";
	char envp3[] = "C=";
	char *envp[] = {envp0, envp1, envp2, envp3, NULL};
	t_env *list;

	list = env_init(envp);
	cr_assert_not_null(list);
	cr_assert_str_eq(env_get(list, "A"), "1");
	cr_assert_str_eq(env_get(list, "B"), "2");
	cr_assert_null(env_get(list, "NOVAL"));
	cr_assert_str_eq(env_get(list, "C"), "");

	cr_assert_eq(env_set(&list, "A", "9"), 0);
	cr_assert_str_eq(env_get(list, "A"), "9");
	cr_assert_eq(env_set(&list, "D", NULL), 0);
	cr_assert_null(env_get(list, "D"));

	cr_assert_eq(env_unset(&list, "B"), 0);
	cr_assert_null(env_get(list, "B"));

	env_free(list);
}

Test(env, to_array_skips_null_values)
{
	t_env *list;
	char **arr;

	list = NULL;
	env_set(&list, "A", "1");
	env_set(&list, "B", NULL);
	env_set(&list, "C", "3");

	arr = env_to_array(list);
	cr_assert_not_null(arr);
	cr_assert_str_eq(arr[0], "C=3");
	cr_assert_str_eq(arr[1], "A=1");
	cr_assert_null(arr[2]);

	freelst(arr);
	env_free(list);
}

Test(env, set_null_on_existing_value_preserves_value)
{
	t_env *list;

	list = NULL;
	cr_assert_eq(env_set(&list, "A", "1"), 0);
	cr_assert_eq(env_set(&list, "A", NULL), 0);
	cr_assert_str_eq(env_get(list, "A"), "1");
	env_free(list);
}

Test(env, unset_head_middle_and_missing)
{
	t_env *list;

	list = NULL;
	env_set(&list, "A", "1");
	env_set(&list, "B", "2");
	env_set(&list, "C", "3");
	cr_assert_eq(env_unset(&list, "C"), 0);
	cr_assert_null(env_get(list, "C"));
	cr_assert_str_eq(env_get(list, "B"), "2");
	cr_assert_eq(env_unset(&list, "A"), 0);
	cr_assert_null(env_get(list, "A"));
	cr_assert_str_eq(env_get(list, "B"), "2");
	cr_assert_eq(env_unset(&list, "NOPE"), 0);
	env_free(list);
}
