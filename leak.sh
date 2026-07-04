#!/bin/sh

set -u

MINISHELL=${1:-./minishell}
VALGRIND=${VALGRIND:-valgrind}
LOGDIR=${LEAK_LOG_DIR:-/tmp/minishell_leaks_$$}
PASS=0
FAIL=0

if [ ! -x "$MINISHELL" ]; then
	printf 'error: minishell not executable: %s\n' "$MINISHELL" >&2
	exit 1
fi

if ! command -v "$VALGRIND" >/dev/null 2>&1; then
	printf 'error: valgrind not found\n' >&2
	exit 1
fi

mkdir -p "$LOGDIR" || exit 1

has_leak_or_error()
{
	log=$1

	grep -Eq 'in use at exit: [1-9][0-9,]* bytes' "$log" \
		|| grep -Eq 'definitely lost: [1-9][0-9,]* bytes' "$log" \
		|| grep -Eq 'indirectly lost: [1-9][0-9,]* bytes' "$log" \
		|| grep -Eq 'possibly lost: [1-9][0-9,]* bytes' "$log" \
		|| grep -Eq 'ERROR SUMMARY: [1-9][0-9,]* errors' "$log"
}

run_case()
{
	name=$1
	trace_children=$2
	input=$3
	log="$LOGDIR/$(printf '%s' "$name" | tr ' /' '__').log"

	if [ "$trace_children" = "yes" ]; then
		printf '%s' "$input" | "$VALGRIND" \
			--trace-children=yes \
			--leak-check=full \
			--show-leak-kinds=all \
			"$MINISHELL" >"$log" 2>&1
	else
		printf '%s' "$input" | "$VALGRIND" \
			--leak-check=full \
			--show-leak-kinds=all \
			"$MINISHELL" >"$log" 2>&1
	fi

	if has_leak_or_error "$log"; then
		FAIL=$((FAIL + 1))
		printf 'not ok - %s\n' "$name"
		printf '  log: %s\n' "$log"
	else
		PASS=$((PASS + 1))
		printf 'ok - %s\n' "$name"
	fi
}

run_case 'eof' no ''
run_case 'builtin exit' no 'exit
'
run_case 'exit non numeric' no 'exit abc
'
run_case 'exit too many args' no 'exit 1 2
exit
'
run_case 'invalid command child' yes 'idontexist
exit
'
run_case 'valid external command child' yes '/bin/echo hi
exit
'
run_case 'pipeline children' yes 'echo hi | cat | wc -c
exit
'
run_case 'redirection failure child' yes 'cat < does_not_exist
exit
'
run_case 'builtin redirection' no 'echo hi > /tmp/minishell_leak_builtin_redir
exit
'
run_case 'heredoc normal' yes 'cat << EOF
hello
EOF
exit
'
run_case 'heredoc eof' no 'cat << EOF
'
run_case 'multiple heredoc eof' no 'cat << hi << test << again
'
run_case 'syntax errors' no 'echo >
|
cat <<
exit
'
run_case 'env mutation' no 'export A=hello
unset A
export B=world
exit
'

printf '\nsummary: %d passed, %d failed\n' "$PASS" "$FAIL"
if [ "$FAIL" -eq 0 ]; then
	rm -rf "$LOGDIR"
	exit 0
fi
printf 'logs kept in: %s\n' "$LOGDIR"
exit 1
