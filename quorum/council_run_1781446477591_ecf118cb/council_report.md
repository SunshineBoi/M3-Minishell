# Minishell Execution Engine Critique Report

**Run ID:** `council_run_1781446477591_ecf118cb`
**Status:** COMPLETED
**Date:** 2026-06-14

---

## 1. Executive Summary

This report consolidates the technical critique of the Minishell execution module (`src/exec/`) compiled from a multi-model LLM council audit. The audit evaluated the codebase against standard shell behaviors (POSIX/Bash), focusing on:
1. **Stability & Deadlocks**: Eliminating blocks during heredoc input pre-collection.
2. **Resource Hygiene**: Identifying leaks of file descriptors (pipes, heredocs, redirections) and memory buffers in parent and child processes.
3. **Robustness & Correctness**: Correcting AST command counting, checking syscall returns, and handling PATH resolution edge cases.
4. **Signal & Process State**: Restoring parent signal handlers and proper child termination semantics.

The findings are structured below by functional execution area.

---

## 2. Structured Findings

### 2.1 Heredocs & File Descriptor Management

#### [Confirmed Defect] Heredoc Pipe Deadlock on Large Input
* **Severity:** Critical
* **Confidence:** High
* **Evidence:** [exec_fd.c:L243-244](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L243-L244) and [exec_fd.c:L261-273](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L261-L273); collection occurs before any child execution in [exec_dispatch.c:L54-55](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_dispatch.c#L54-L55).
* **Reasoning:** `collect_one_heredoc()` creates a pipe and synchronously writes the entire heredoc into the write end. Because no child process has been spawned to read and drain the pipe, writing a heredoc that exceeds the platform's kernel pipe capacity (typically 64 KiB on Linux) blocks the parent process permanently.
* **Fix:** Store heredoc data in temporary files (e.g., using `O_TMPFILE` or unique naming in `/tmp`), or fork a short-lived writer process to concurrently write to the pipe.

#### [Confirmed Defect] Heredoc FDs Leak into All Pipeline Children
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_fd.c:L273](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L273), [exec_pipeline.c:L87-95](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline.c#L87-L95), and [exec_fd.c:L68-72](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L68-L72).
* **Reasoning:** Collected heredoc read descriptors are stored without the `FD_CLOEXEC` flag. When pipeline commands are forked, children inherit all open heredoc descriptors. Because each child only closes its own redirection descriptors, other commands' heredoc read descriptors remain open across `execve()`, causing descriptor leaks and potential pipeline hangs if a command waits on EOF.
* **Fix:** Apply `FD_CLOEXEC` to all heredoc descriptors. Explicitly close all unrelated heredoc read descriptors in the child process prior to calling `execve()`.

#### [Confirmed Defect] Parent Retains Heredoc Descriptors
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_fd.c:L270-274](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L270-L274) and [exec_dispatch.c:L30-47](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_dispatch.c#L30-L47).
* **Reasoning:** The parent shell never closes the read ends of heredocs after completing execution. Descriptors accumulate in the parent shell process with each executed command, leading to `EMFILE` descriptor exhaustion.
* **Fix:** Implement a parent-side post-execution traversal to close all collected heredoc read descriptors.

#### [Confirmed Defect] Heredocs Leak on Collection Failure
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_fd.c:L281-290](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L281-L290) and [exec_fd.c:L303-307](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L303-L307).
* **Reasoning:** If a multi-heredoc command or pipeline fails or is interrupted during the collection of a subsequent heredoc, `collect_heredocs()` returns `-1` immediately. Any heredoc read descriptors already opened during that call are left open, leaking FDs.
* **Fix:** Make heredoc collection transactional: on failure, walk the AST and close all heredoc descriptors collected in the current run before returning.

#### [Confirmed Defect] Heredoc Write Failures Ignored
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_fd.c:L184-199](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_fd.c#L184-L199) (especially lines 196-197).
* **Reasoning:** Return values from `write()` are ignored. Interrupted, partial, or failed writes (e.g., due to ENOSPC or EPIPE) will silently truncate or corrupt the heredoc without reporting an error.
* **Fix:** Implement a checked `write_all()` loop that retries on `EINTR`, handles short writes, and propagates errors to abort execution.

---

### 2.2 Pipeline Management & AST Traversal

#### [Confirmed Defect] `count_cmds` Undercounts Right-Nested Pipelines
* **Severity:** Critical
* **Confidence:** High
* **Evidence:** [exec_pipeline_utils.c:L3-16](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline_utils.c#L3-L16) vs [exec_pipeline.c:L3-18](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline.c#L3-L18).
* **Reasoning:** `count_cmds()` counts commands by traversing only the left child of binary nodes. However, `_flatten_cmds()` traverses both left and right branches recursively. For right-nested or balanced pipeline structures (e.g., `a | (b | c)`), `count_cmds()` undercounts, causing `setup_cmds()` to under-allocate the `cmds` array and leading to out-of-bounds heap writes in `_flatten_cmds()`.
* **Fix:** Refactor `count_cmds()` to use the exact same recursive traversal logic as `_flatten_cmds()`.

#### [Confirmed Defect] Pipeline `dup2` Failures Ignored
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_pipeline.c:L58-67](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline.c#L58-L67).
* **Reasoning:** Return values of `dup2()` for pipe input/output redirection are ignored. If duplication fails, the child process runs with incorrect streams (or the shell's own standard streams) and can execute destructive commands.
* **Fix:** Assertively check all `dup2()` calls; on failure, output a diagnostic error and terminate the child via `_exit()`.

#### [Confirmed Defect] Resource Leak on Pipeline Setup Failure
* **Severity:** Medium
* **Confidence:** High
* **Evidence:** [exec_dispatch.c:L42-43](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_dispatch.c#L42-L43) and [exec_pipeline.c:L85-90](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline.c#L85-L90).
* **Reasoning:** If `start_pipeline()` fails (e.g., due to fork or pipe failures), `exec_allcmds()` returns immediately after freeing the command array. The allocated `t_pipeops` and any pipe descriptors opened in previous loop iterations are leaked.
* **Fix:** Call `free_pipeops(pipeops, i-1)` on the setup failure path to close open pipes and deallocate memory.

#### [Likely Defect] Child Process Uses `exit` Instead of `_exit`
* **Severity:** Medium
* **Confidence:** High
* **Evidence:** [exec_pipeline_run.c:L50](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline_run.c#L50) and [exec_pipeline.c:L98](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline.c#L98).
* **Reasoning:** Using `exit()` in forked children runs parent-registered `atexit` handlers and flushes stdio buffers, causing potential duplicate output or incorrect cleanup side-effects in the parent context.
* **Fix:** Replace child terminations with `_exit()`.

---

### 2.3 Command Dispatch, PATH Resolution & Signal Handling

#### [Confirmed Defect] PATH Split Delimiter Errors and Infinite Loop
* **Severity:** Critical
* **Confidence:** High
* **Evidence:** [exec_path_utils.c:L23-40](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path_utils.c#L23-L40) and [exec_path_utils.c:L56-63](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path_utils.c#L56-L63).
* **Reasoning:** When splitting `PATH` components, empty fields (e.g. leading delimiters like `:...` or repeated `::`) result in `_splithelper()` returning `0` or `1` without advancing, causing infinite loops or generating null slots in the path array. This prematurely terminates path search since `_matchcmdpath()` stops traversing at the first null.
* **Fix:** Normalize splitter logic to safely parse and represent empty components as the current directory (`"."`).

#### [Confirmed Defect] Empty PATH Discards Current Directory Lookup
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_path_utils.c:L49-64](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path_utils.c#L49-L64) and [exec_path.c:L40-55](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path.c#L40-L55).
* **Reasoning:** POSIX specifies that an empty `PATH` variable means the current directory is searched. However, if `PATH` is empty, the custom splitter returns an empty list, bypassing current directory lookup.
* **Fix:** Emit one empty string (representing `"."`) when `PATH` is completely empty.

#### [Confirmed Defect] PATH Search Stops at First Non-Executable Candidate
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_path.c:L40-49](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path.c#L40-L49) (specifically lines 46-47).
* **Reasoning:** If a matching filename is found in an early `PATH` directory but it lacks execute permissions, the search aborts immediately upon encountering `EACCES`. A valid executable with the same name in a later `PATH` directory will never be reached.
* **Fix:** Track permission-denied occurrences and continue searching. Only return `EACCES` if no executable match is found in subsequent directories.

#### [Confirmed Defect] Special-Cased Command Name Success Override
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_path.c:L74-80](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_path.c#L74-L80) (lines 77-78).
* **Reasoning:** The resolver contains a hardcoded exception for the command name `"no_such_command"`, treating it as successful when `PATH` is unset. This causes nonexistent commands to report an exit status of `0`.
* **Fix:** Remove the command-specific exception and treat all commands uniformly.

#### [Likely Defect] Persistent Ignored Signals in Parent
* **Severity:** High
* **Confidence:** Medium-High
* **Evidence:** [exec_dispatch.c:L41](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_dispatch.c#L41).
* **Reasoning:** `signals_ignore()` is invoked in the parent shell prior to launching a pipeline, but there is no corresponding handler restoration on the success or error return paths of `exec_allcmds()`. This leaves the parent shell permanently ignoring interactive signals (SIGINT/SIGQUIT).
* **Fix:** Implement a signal save/restore scope to ensure the parent's interactive signal dispositions are restored immediately after pipeline execution completes or fails.

#### [Confirmed Defect] Non-EINTR waitpid Failures Masked as Success
* **Severity:** High
* **Confidence:** High
* **Evidence:** [exec_pipeline_run.c:L5-15](file:///home/harry/Documents/42-Projects/Minishell/minishell-github/src/exec/exec_pipeline_run.c#L5-L15).
* **Reasoning:** If `waitpid()` fails with errors other than `EINTR` (e.g., `ECHILD`), the loop breaks and the unacquired `status` (initialized to `0`) is evaluated by `WIFEXITED`, reporting a false successful exit code.
* **Fix:** Check the return value of `waitpid()`. Only evaluate `status` macros if the return value matches the expected child PID.

---

## 3. High-Priority Repair Strategy

1. **Heredoc Engine Refactor**: Replace pipe-backed heredoc collection with temporary file storage to prevent deadlocks. Apply `FD_CLOEXEC` and implement parent-side descriptor cleanup.
2. **Recursive Command Counting**: Align `count_cmds()` with the traversal logic of `_flatten_cmds()` to prevent buffer overflows.
3. **Sycall Hardening**: Enforce return value checks for `dup2()`, `write()`, and `waitpid()`. Replace `exit()` with `_exit()` in child paths.
4. **Signal and Descriptor Scopes**: Ensure parent signal states and file descriptors are restored on all exit branches of the dispatch execution module.
