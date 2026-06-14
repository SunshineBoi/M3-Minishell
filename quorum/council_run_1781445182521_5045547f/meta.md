# Council Member Report — meta

Run ID: council_run_1781445182521_5045547f

Show thinking

Minishell `src/exec/` review — dispatch, pipeline, path, and fd handling


Based only on the seven files provided, here are the concrete problems that will bite you in practice.



### 1. Confirmed defect — `count_cmds` undercounts right-heavy pipelines


* Severity: high — heap overflow
* Confidence: high
* Evidence: `src/exec/exec_pipeline_utils.c:3-16` counts only `root = root->content.binop.left`; `src/exec/exec_pipeline.c:9-10` `_flatten_cmds` recurses left *and* right
* Reasoning: For `a | b | c` built as `BINOP(a, BINOP(b,c))`, `count_cmds` returns 1, `setup_cmds` mallocs 2 pointers, `_flatten_cmds` writes 3 + NULL → out-of-bounds write. This matches your `n_cmd > 256` guard but not the shape bug.
* Missing context: AST builder guarantees left-associativity — not shown.
* Validation test: Build AST manually with right recursion, run under ASAN: `ASAN_OPTIONS=detect_leaks=0 ./minishell -c "echo a | cat | wc"` → heap-buffer-overflow in `_flatten_cmds`.
* Fix: replace `count_cmds` with same traversal as `_flatten_cmds`, or compute count during flatten.



### 2. Confirmed defect — PATH split drops trailing empty entries


* Severity: medium — POSIX compliance
* Confidence: high
* Evidence: `src/exec/exec_path_utils.c:43-64` loop `while (*str)`; `countbydelim` at `:3-20` returns 2 for `"a:"` but loop stops after first token
* Reasoning: Empty components mean “current directory”. Your code loses trailing `:` so `PATH="bin:"` will not search `.`
* Missing context: `ft_calloclst` zero-initialization — assumed.
* Validation test: `export PATH="/bin:"; touch ./mycmd; chmod +x ./mycmd; ./minishell -c "mycmd"` → should run, currently prints “command not found”.
* Fix: iterate `size` times, not `while(*str)`, and always emit empty string for consecutive/trailing delimiters.



### 3. Likely defect — `restore_fd` can permanently break parent stdio


* Severity: high
* Confidence: medium
* Evidence: `src/exec/exec_fd.c:88-100` calls `dup2` without checking return; `src/exec/exec_dispatch.c:25` calls it unconditionally after builtin
* Reasoning: If `dup(temp_in)` succeeded but later `dup2(temp_in,0)` fails (EBADF, EINTR), you close the backup and leave STDIN redirected to the builtin's file.
* Missing context: signal race during restore not shown.
* Validation test: builtin with redirection: `(ulimit -n 3; echo hi > /tmp/x)` then try to read — parent stdin closed.
* Fix: check `dup2` return, and on failure `dup2` back from saved copy or abort shell.



### 4. Confirmed defect — pipe fd aliasing → double close


* Severity: medium — fd hygiene, hides real errors
* Confidence: high
* Evidence: `src/exec/exec_pipeline_utils.c:47-48` `prevfdin = pipebuf[0]`; `src/exec/exec_pipeline_utils.c:63-66` `free_pipeops` closes both `prevfdin` and `pipebuf[0]`
* Reasoning: After `update_pipeops`, both variables hold same fd. Next `free_pipeops` closes it twice (second returns EBADF).
* Missing context: none
* Validation test: `strace -e close ./minishell -c "ls | cat"` → observe same fd closed twice.
* Fix: after assignment set `pipebuf[0] = -1`.



### 5. Likely defect — child leaks pipe ends on redirection error


* Severity: medium
* Confidence: high
* Evidence: `src/exec/exec_pipeline.c:68-71` on `open_redirs` or `do_dup2redirs` failure, returns -1 without closing `fdin`/`fdout` passed from parent
* Reasoning: Child inherits read end of previous pipe and write end of next pipe. If redirection fails, those stay open until `exit`, preventing EOF and causing pipeline hang.
* Missing context: whether parent closes its copies before wait — it does in `update_pipeops`.
* Validation test: `cat /etc/passwd | grep root > /no_perm | wc -l` → `wc` hangs because write end of first pipe never closed in middle child.
* Fix: in `do_childproc` error paths, `close(fdin); close(fdout);` before return.



### 6. Confirmed defect — `open_redirs` leaves partial state to caller


* Severity: medium
* Confidence: high
* Evidence: `src/exec/exec_fd.c:23-25` returns -1 immediately; does not reset `redir->fd` for failed entry
* Reasoning: Callers (`exec_builtinproc`, `do_childproc`) rely on `close_redirsfd` to clean, but failed entry retains previous value (maybe -1, maybe stale). For HEREDOC, loop `continue` at `:20-21` skips setting fd, leaving old fd from prior command.
* Missing context: initialization of `t_redir.fd` in parser.
* Validation test: run builtin twice: `echo hi > a; echo hi > /no_perm` then `lsof -p $$` — observe fd to `a` still open.
* Fix: initialize all `redir->fd = -1` before loop, and on error iterate backwards closing opened fds inside `open_redirs`.



### 7. Likely defect — `wait_allpids` can wait on -1


* Severity: medium
* Confidence: medium
* Evidence: `src/exec/exec_pipeline_utils.c:31` `ft_memset(pids,-1)`; `src/exec/exec_pipeline_run.c:27-40` loops `i <= pid_id` without checking `pids[i] != -1`
* Reasoning: If `fork` fails early, `start_pipeline` calls `free_pipeops(pipeops, i-1)`. `wait_allpids` then calls `waitpid(-1, ...)` which waits for *any* child, potentially reaping unrelated children.
* Missing context: max size of `pids` array.
* Validation test: `ulimit -u 10; ./minishell -c "sleep 1 | sleep 1 | sleep 1"` → observe waitpid(-1) in strace.
* Fix: skip `pids[i] == -1`.



### 8. Hardening recommendation — path resolution does not reject directories


* Severity: low
* Confidence: high
* Evidence: `src/exec/exec_path.c:15-16` and `:25-26` use only `access(X_OK)`
* Reasoning: `access` succeeds on executable directories; `execve` later fails with EISDIR. Bash reports “is a directory” and exit 126.
* Missing context: `setexecerrno` mapping.
* Validation test: `mkdir d; chmod 755 d; ./minishell -c "./d"` → should print “is a directory”, currently prints “Permission denied”.
* Fix: after `access`, `stat` and reject `S_ISDIR`.



### 9. Architectural risk — heredoc fd overwritten without close


* Severity: medium
* Confidence: high
* Evidence: `src/exec/exec_fd.c:273` `redir->fd = pipe_fds[0]`; no prior close
* Reasoning: Multiple heredocs on same command (`cat <<A <<B`) leak first pipe read end.
* Missing context: parser reuses `t_redir` structs.
* Validation test: `cat <<A <<B` then `ls -l /proc/$$/fd` → extra pipe.
* Fix: if `redir->fd != -1` close before assignment.



### 10. Architectural risk — magic test string in resolver


* Severity: low
* Confidence: high
* Evidence: `src/exec/exec_path.c:77-78` special-case `"no_such_command"`
* Reasoning: Hard-coded behavior diverges from real shell, makes tests pass artificially, hides PATH-unset bug.
* Missing context: test harness intent.
* Validation test: unset PATH, run `no_such_command` → exits 0 instead of 127.
* Fix: remove; treat missing PATH uniformly.




## Cross-cutting issues


* Builtin vs pipeline: `exec_builtinproc` dups both stdin and stdout even if only one redirection exists (`exec_dispatch.c:14-15`). Wastes fds and doubles failure points.
* Signal handling: `signals_ignore` in `exec_allcmds:41` is never restored in shown code; parent may stay ignoring SIGINT after pipeline.
* Error propagation: `do_exec` calls `exit(app->exitcode)` after builtin in child (`exec_pipeline_run.c:50`) but parent also sets exit via `setexit`. Inconsistent exit codes for builtins in pipelines.



## Practical improvements


1. Centralize fd lifecycle: create `fd_saver` struct with save/restore that checks returns.
2. Make `open_redirs` atomic — on any failure, close all opened in same call.
3. Replace `count_cmds` with flatten-then-count.
4. Normalize PATH splitting to preserve empty fields.
5. In `start_pipeline`, after fork, child must close *all* pipe fds except its own ends — iterate over `pipeops->pids` array or use `closefrom`.



## Minimal test suite you need


* `>a >b` where `b` unwritable — check no fd leak
* `cat <<A <<B` — check fd count
* `PATH=":/bin"` with executable in `.`
* `a | b | c | d | e` built right-associative
* pipeline with middle command redirection failure — ensure no hang
* builtin `cd /tmp > out` then `pwd` — ensure stdout restored



Missing context that would change findings: definition of `t_pipeops.pids` size, initialization of `t_redir.fd`, and full signal-restore logic outside `src/exec/`.
