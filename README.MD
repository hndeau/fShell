# CS620 Custom UNIX Shell Program

> This is a CLI that implements internal calls such as exit, cd, and pwd. 
> The program is inspired by the Kali linux terminal and provides detailed error
> reporting for permission and incorrect usages. As well as general error reporting
> for incorrect usage.

## Compilation

> To compile, use Make with the provided Makefile:
> ````
> make myshell
> 
> make myhistory
> ````
> Example run usage:
> ```
> ./myshell [BIN_DIR]
> 
> ./myhistory [BIN_DIR]
> ```
> * Note: The binary directory path(BIN_DIR) is optional. And defaults to /usr/bin

## Usage

> To run the program, simply run the myShell executable file in the command line: `./myShell`
> Once inside the program, you can enter commands as you would in a regular shell. For example:
>```
> ┌──(Jett@unix)-《~》
> └─$$ pwd
> /home/Jett
>```
> If the user does not have permission to perform an operation, they will receive an error message:
> ```
> ┌──(Jett@unix)-《~》
> └─$ ls /root
> ls: cannot open directory '/root': Permission denied
> ```

This program was developed by Henry Nadeau for the CS620 class at UNH.