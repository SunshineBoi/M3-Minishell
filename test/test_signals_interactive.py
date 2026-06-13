#!/usr/bin/env python3
import pexpect
import sys

def test_signals():
    # Start minishell
    print("Spawning minishell...")
    child = pexpect.spawn('./minishell', timeout=2, encoding='utf-8')
    child.logfile = sys.stdout
    
    # 1. Expect initial prompt
    child.expect('minishell\\$ ')
    print("\n[OK] Initial prompt found.")
    
    # 2. Test Ctrl+C (SIGINT) at empty prompt
    child.sendcontrol('c')
    # It should print a newline and a new prompt
    child.expect('\r\nminishell\\$ ')
    print("\n[OK] Ctrl+C at empty prompt redrew the prompt.")
    
    # 3. Test Ctrl+C (SIGINT) with text typed
    child.send("some partial command")
    import time
    time.sleep(0.2)
    child.sendcontrol('c')
    child.expect('\r\nminishell\\$ ')
    print("\n[OK] Ctrl+C with typed text cancelled input and redrew the prompt.")
    
    # 4. Test Ctrl+\ (SIGQUIT) - should be ignored
    child.sendcontrol('\\')
    # Check shell is still active by running a command
    time.sleep(0.2)
    child.sendline("echo alive")
    child.expect("alive")
    child.expect('minishell\\$ ')
    print("\n[OK] Ctrl+\\ was ignored and shell remained active.")
    
    # 5. Test Ctrl+D (EOF) on empty line - should exit
    time.sleep(0.2)
    child.sendcontrol('d')
    child.expect(pexpect.EOF)
    print("\n[OK] Ctrl+D exited the shell.")
    
    # Check exit status (should be 0)
    child.close()
    if child.exitstatus != 0:
        print(f"[FAIL] Expected exit status 0, but got {child.exitstatus}")
        sys.exit(1)
    print("[SUCCESS] All interactive signal prompt tests passed!")

if __name__ == '__main__':
    test_signals()
