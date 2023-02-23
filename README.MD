# CS620 Custom Shell Program

> Welcome to the CS620 Custom Shell Program! This is a command-line interface program that implements internal 
> calls such as exit, cd, and pwd. The program is inspired by the Kali linux terminal and provides detailed error 
> reporting for permission and incorrect usages.

## Usage

>To run the program, simply run the myShell executable file in the command line: `./myShell`
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