#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX_PATH_LENGTH 1024
#define DEBUG 0

int main(int argc, char *argv[]) {
    char input_buffer[80];
    char *binary_path = "/usr/bin";
    char *user = getenv("USER"); // Get the current user's name
    if (user == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL)
            user = pw->pw_name;
    }
    char *user_home = getenv("HOME");
    int user_home_len = (int) strlen(user_home);
    char cwd[PATH_MAX]; // Create a buffer to store the current working directory
    struct utsname device_buffer;
    int device_return = uname(&device_buffer);

    if (argc == 2) {
        binary_path = argv[1];
    }

    if (user == NULL) {
        fprintf(stderr, "Unable to determine user!\n");
        exit(EXIT_FAILURE);
    }

    if (device_return == -1) {
        fprintf(stderr, "ERROR: Unable to determine device name!\n");
        exit(EXIT_FAILURE);
    }

    if (DEBUG)
        printf("Enter a command (or type \"quit\" to quit)\n");

    // Pipe to redirect stderr to stdout and synchronize the stream
    if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
        perror("dup2 failed!");
    }

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get the current working directory
            fprintf(stderr, "Unable to determine current working directory.\n");
            exit(EXIT_FAILURE);
        }

        // Took inspiration from Kali's terminal because it looks nice
        if (strncmp(cwd, user_home, user_home_len) == 0) {
            printf("┌──(%s@%s)-《~%s》\n└─$ ", user, device_buffer.nodename, &cwd[user_home_len]);
        } else {
            printf("┌──(%s@%s)-《%s》\n└─$ ", user, device_buffer.nodename, cwd);
        }
        fgets(input_buffer, 80, stdin);

        // Remove the newline character at the end of the input string
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        if (strcmp(input_buffer, "quit") == 0 || strcmp(input_buffer, "exit") == 0)
            exit(EXIT_SUCCESS);

        // Extract the first token from the input string
        char *token = strtok(input_buffer, " ");

        if (token == NULL) {
            fprintf(stderr, "ERROR: no command specified\n");
            continue;
        }

        // Build the full path
        char command_path[MAX_PATH_LENGTH];
        int result = snprintf(command_path, MAX_PATH_LENGTH, "%s/%s", binary_path, token);

        if (result >= MAX_PATH_LENGTH) {
            fprintf(stderr, "ERROR: command path too long\n");
            continue;
        }

        // Construct the argument list for the command executable
        char *args[11] = {NULL};
        int i = 0;

        args[i++] = token;
        char valid = 0;
        while ((token = strtok(NULL, " ")) != NULL) {
            if (i >= 10) {
                fprintf(stderr, "ERROR: Too many arguments\n");
                break;
            }

            char *index = token;
            while (*index != '\0') {
                if (*index == '*' || *index == '&' || *index == '|' || *index == '>') {
                    valid = *index;
                }
                index++;
            }
            if (valid) {
                fprintf(stderr, "ERROR: Illegal character \'%c\'\n", valid);
                break;
            }
            args[i++] = token;
        }

        if (valid) // No other way I could see to break out of the inner while loop then continue to the next iteration
            continue;

        if (strcmp(args[0], "cd") == 0) { // Check and handle if cd internal call
            int perm;
            if (!args[1]) {
                perm = chdir(user_home);  // Change to home directory
            } else {
                perm = chdir(args[1]); // Change to provided directory
            }

            if (perm == -1) {
                perror("chdir");
            }
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) {
            if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get the current working directory
                fprintf(stderr, "Unable to determine current working directory.\n");
                exit(EXIT_FAILURE);
            }
            printf("%s\n", cwd);
            continue;
        }

        // Fork to execute the command since it's not a basic syscall
        pid_t pid = fork();

        if (pid == -1) {
            fprintf(stderr, "Failed to execute fork command\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            int status = execv(command_path, args);

            if (status == -1) {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(EXIT_FAILURE);
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (DEBUG) {
                if (WIFEXITED(status)) {
                    printf("Child process exited with status %d\n", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("Child process terminated by signal %d\n", WTERMSIG(status));
                }
            }
        }
    }
}