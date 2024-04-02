# Shell Part-A
## Write a basic shell program
The shell should have following features:
- Read commands in a loop and run commands without complete path names 
  and handle arguments+options for commands
- Show prompt with current working directory in the prompt and allow user
  to change the prompt to a particular string and revert back to CWD prompt
- This should be done with following two commands, with specified syntax:
```
PS1="whatever string you want "
PS1="\w$"

[ This also mandates that the "cd" command should work. 
Note that "cd" is always an internal command of shell, and not an executable.
It affects the cwd of the shell itself ]
```

- Handle all possible user input errors
- exit gracefully on typing "exit" or ctrl-D
- allow users to set a variable called PATH which is used for searching 
  the list of executables (just like in the case of 'bash' shell)
```
This should be done with a command that has following syntax
PATH=/usr/bin:/bin:/sbin
This combined with first point means that you will NOT be using execvp(), 
but rather implement a version of execvp() on your own.
```
- Use the keys : up and down array key and run a particular command from 
  history
> no need to handle the editing of the current command, just executing a
> command from history as it is will do.
> The history need not be persistent.

