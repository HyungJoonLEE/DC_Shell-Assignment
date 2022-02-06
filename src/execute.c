#include <unistd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_stdio.h>
#include <dc_posix/dc_unistd.h>
#include <dc_error/error.h>
#include "execute.h"


void execute(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path) {

    pid_t pid;
    pid = fork();

    if (pid == 0) {
        int status;

        redirect(env, err, command);
        if (dc_error_has_error(err)) {
            exit(126);
        }
        run(env, err, command, path);
        status = handle_run_error(err, command);
        exit(status);
    }

    else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            command->exit_code = exit_code;
        }
    }
}


void redirect(const struct dc_posix_env *env, struct dc_error *err, struct command *command) {
//    printf("This is stdin :%s:\n", command->stdin_file);
//    printf("This is stdout :%s:\n", command->stdout_file);
//    printf("This is stderr :%s:\n", command->stderr_file);

    if (command->stdin_file != NULL) {
        int fd;
        fd = dc_open(env, err, command->stdin_file, DC_O_RDWR, S_IRWXO | S_IRWXG | S_IRWXU);

        if (dc_error_has_no_error(err)) {
            dc_dup2(env, err, fd, STDIN_FILENO);
        }

        dc_close(env, err, fd);
    }


    if (command->stdout_file != NULL) {
        int fd;

        if (command->stdout_overwrite == true) {
            fd = dc_open(env, err, command->stdout_file,  DC_O_CREAT | DC_O_RDWR | DC_O_APPEND, S_IRWXU);
        }
        else {
            fd = dc_open(env, err, command->stdout_file, DC_O_CREAT | DC_O_RDWR | DC_O_TRUNC, S_IRWXU);
        }

        if (dc_error_has_no_error(err)) {
            dc_dup2(env, err, fd, STDOUT_FILENO);
        }

        dc_close(env, err, fd);
    }


    if (command->stderr_file != NULL) {
        int fd;

        if (command->stderr_overwrite == true) {
            fd = dc_open(env, err, command->stderr_file ,DC_O_CREAT | DC_O_RDWR | DC_O_APPEND, S_IRWXU);
        }
        else {
            fd = dc_open(env, err, command->stderr_file ,DC_O_CREAT | DC_O_RDWR | DC_O_TRUNC, S_IRWXU);
        }

        if (dc_error_has_no_error(err)) {
            dc_dup2(env, err, fd, STDERR_FILENO);
        }

        dc_close(env, err, fd);
    }
}


int run(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char** path) {
    if (strstr(command->command, "/") != NULL) {
        command->argv[0] = strdup(command->command);
        dc_execv(env, err, command->command, command->argv);
    }
    else {
        if (!path[0]) {
            DC_ERROR_RAISE_ERRNO(err, ENOENT);
        }
        else {
            for (char *c = *path; c; c = *++path) {
                strcat(c, "/");
                strcat(c, command->command);
                command->argv[0] = c;
                dc_execv(env, err, c, command->argv);
                if(dc_error_has_error(err)) {
                    if (!dc_error_is_errno(err, ENOENT)) {
                        break;
                    }
                }
            }
        }
    }
    return EXIT_FAILURE;
}


int handle_run_error(struct dc_error *err, struct command *command) {
    if (dc_error_is_errno(err, E2BIG)) return 1;
    else if (dc_error_is_errno(err, EACCES)) return 2;
    else if (dc_error_is_errno(err, EINVAL)) return 3;
    else if (dc_error_is_errno(err, ELOOP)) return 4;
    else if (dc_error_is_errno(err, ENAMETOOLONG)) return 5;
    else if (dc_error_is_errno(err, ENOENT)) return 127;
    else if (dc_error_is_errno(err, ENOTDIR)) return 6;
    else if (dc_error_is_errno(err, ENOEXEC)) return 7;
    else if (dc_error_is_errno(err, ENOMEM)) return 8;
    else if (dc_error_is_errno(err, ETXTBSY)) return 9;
    else return 125;
}
