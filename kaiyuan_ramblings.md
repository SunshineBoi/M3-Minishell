## 1. Why do we need `envp` and `env_list`

You can defend it like this:

We keep `env_list` because the shell environment is mutable. Builtins like `export`, `unset`, and `cd` need to add, remove, and update variables while the shell is running. A linked list of `t_env` nodes is much easier and cleaner for those operations than constantly editing a `char **`.

`envp` is still needed because some parts of the shell need the standard Unix environment format:

```c
char **envp
```

Especially:

```c
execve(path, argv, envp);
```

So the design is:

```text
env_list = internal editable source of truth
envp     = generated array view for execve / expansion
```

A good evaluator answer:

> We could store only `envp`, but then every `export` or `unset` would require manually resizing a NULL-terminated array, shifting entries, rebuilding `"KEY=VALUE"` strings, and carefully freeing old memory. That is error-prone. With `env_list`, updates are natural: find, insert, delete, or modify one node. Then when we need a `char **`, we regenerate `envp` from the list.

You can also mention ownership:

> `env_list` gives us structured ownership of `key`, `value`, and `next`. `envp` is a flat compatibility format. It is convenient for `execve`, but not ideal as the shell’s main data structure.

Short version:

```text
Only envp is possible, but harder to maintain.
env_list is better for shell logic.
envp is better for Unix APIs.
So we keep both and sync envp from env_list.
```

---

## 2. 