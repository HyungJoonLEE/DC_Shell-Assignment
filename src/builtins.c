#include <stdio.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/path.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "builtins.h"


void builtin_cd(const struct dc_posix_env *env, struct dc_error *err, struct command *command, FILE *errstream) {
    char* work_path;

    if (command->argv[1] == NULL) {
        char* home_expand;
        dc_expand_path(env, err, &home_expand, "~/");
        dc_chdir(env, err, home_expand);
        work_path = strdup(home_expand);
    }

    else {
        dc_chdir(env, err, command->argv[1]);
        work_path = strdup(command->argv[1]);
    }
    command->exit_code = 0;


    if (dc_error_has_error(err)) {
        if (dc_error_is_errno(err, EACCES)) {
            fprintf(errstream, "%s: Permission denied\n", work_path);
        }
        else if (dc_error_is_errno(err, ELOOP)) {
            fprintf(errstream, "%s: Too many levels of symbolic links\n", work_path);
        }
        else if (dc_error_is_errno(err, ENAMETOOLONG)) {
            fprintf(errstream, "%s: File name too long\n", work_path);
        }
        else if (dc_error_is_errno(err, ENOENT)) {
            fprintf(errstream, "%s: does not exist\n", work_path);
        }
        else if (dc_error_is_errno(err, ENOTDIR)) {
            fprintf(errstream, "%s: is not a directory\n", work_path);
        }
        command->exit_code = 1;
    }

    free(work_path);
    work_path = NULL;
}






