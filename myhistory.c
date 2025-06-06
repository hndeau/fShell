#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <limits.h>

#include <sys/wait.h>

#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>

#define MAX_PATH_LENGTH 1024

char input_buffer[80];
char history_buffer[10][80];
int history_index = 0;
char *binary_path;
char *user;
char *user_home;
int user_home_len;
char cwd[PATH_MAX]; // Create a buffer to store the current working directory

void print_history() {
    for (int i = history_index - 1;
         (i >= history_index - 10) && (i >= 0); i--) {
        printf("%d:%s\n", i + 1, history_buffer[i % 10]);
    }
}

int execCD(char *path) {
    if (path)
        return chdir(path); // Change to provided directory
    return chdir(user_home); // Change to home directory
}

int main(int argc, char *argv[]) {
    struct utsname device_buffer;
    int device_return = uname(&device_buffer);
    binary_path = "/usr/bin";
    user = getenv("USER"); // Get the current user's name
    if (user == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw != NULL)
            user = pw->pw_name;
    }
    user_home = getenv("HOME");
    user_home_len = (int) strlen(user_home);

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

    // Pipe to redirect stderr to stdout and synchronize the stream
    if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
        perror("dup2 failed!");

    if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get the current working directory
        fprintf(stderr, "Unable to determine current working directory!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
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

        if (strcmp(input_buffer, "history") == 0) {
            print_history();
            continue;
        }
        strcpy(history_buffer[history_index++ % 10], input_buffer);

        // Extract the first token from the input string
        char *token = strtok(input_buffer, " ");

        if (token == NULL) {
            fprintf(stderr, "Error: No command specified\n");
            continue;
        }

        // Build the full path
        char command_path[MAX_PATH_LENGTH];
        int result = snprintf(command_path, MAX_PATH_LENGTH, "%s/%s", binary_path, token);

        if (result >= MAX_PATH_LENGTH) {
            fprintf(stderr, "Error: Command path too long\n");
            continue;
        }

        // Construct the argument list for the command executable
        char *args[11] = { NULL };
        int i = 0;

        args[i++] = token;
        char valid = 0;
        while ((token = strtok(NULL, " ")) != NULL) {
            if (i >= 10) {
                fprintf(stderr, "Error: Too many arguments\n");
                break;
            }

            char *index = token;
            while (*index != '\0') { // TODO Implement symbols
                if (*index == '*' || *index == '&' || *index == '|' || *index == '>') {
                    valid = *index;
                }
                index++;
            }
            if (valid) {
                fprintf(stderr, "Error: Illegal character \'%c\'\n", valid);
                break;
            }
            args[i++] = token;
        }

        if (valid) // No other way I could see to break out of the inner while loop then continue to the next iteration
            continue;

        if (strcmp(args[0], "cd") == 0) { // Check if cd internal call
            int perm = execCD(args[1]);
            if (perm == -1) {
                perror("cd");
            } else {
                if (!args[1]) {
                    strcpy(cwd, "~");
                } else {
                    strcpy(cwd, args[1]);
                }
            }
            continue;
        }

        if (strcmp(args[0], "pwd") == 0) {
            if (getcwd(cwd, sizeof(cwd)) == NULL) { // Get the current working directory
                fprintf(stderr, "Unable to determine current working directory!\n");
                exit(EXIT_FAILURE);
            }
            printf("%s\n", cwd);
            continue;
        }

        // Fork to execute the command since it's not a basic syscall
        pid_t pid = fork();

        if (pid == -1) {
            fprintf(stderr, "Failed to execute fork command!\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            int status = execv(command_path, args);

            if (status == -1) {
                fprintf(stderr, "%s: Command not found\n", args[0]);
                exit(EXIT_FAILURE);
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}