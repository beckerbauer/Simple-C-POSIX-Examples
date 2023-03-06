# C-POSIX

The project "Simple C-POSIX examples" shows some simple use cases and comprehensible examples of the C-POSIX Library interacting with the **LINUX-Operating** system.
It can be helpful to look at those to understand the method of operation when working with the library and the POSIX-API.

## Interactive Shell

The interactive shell is a command-line shell program that can be used to execute other programs or commands.
It can execute programs in the foreground or background, and can also stop and continue running background processes.

### How to use

After starting the program, **`myshell>`** prompt will appear.
You can then enter the command you want to execute, followed by any necessary arguments. Press Enter to execute the command.

### Supported commands
- **logout**: Exits the program. If there are background processes running, the user will be prompted to confirm the exit. 
- **stop <pid>**: Stops the process with the specified PID.
- **cont <pid>**: Continues the process with the specified PID.
- **&**: Executes the command in the background.


### Example
```
$ ./myshell
myshell> ls
file1.txt file2.txt file3.txt
myshell> firefox
[Background: 1234]
myshell> stop 1234
[SIGSTP: pid 1234]
myshell> cont 1234
continuation of: 18
myshell> logout
Do you really want to logout? [y/n]: y
```

### How it works
When a command is entered, the interactive shell will fork a child process and execute the command in the child process using the `execvp` function.
If the `&` symbol is included in the command, the child process will run in the background.

**The interactive shell** also handles three different signals:

- **`SIGTSTP`**: Stops the current process and prints a message with the current process ID.
- **`SIGCONT`**: Continues a stopped process and prints a message with the process ID.
- **`SIGCHLD`**: Checks for any child processes that have terminated and removes them from the PID list.
If a process is running in the foreground, the shell will wait for the process to complete before prompting for the next command. If a process is running in the background, the shell will continue to accept commands while the background process runs.
