#include <stdio.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/path.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "builtins.h"


void builtin_cd(const struct dc_posix_env *env, struct dc_error *err, struct command *command, FILE *errstream) {
    if (command->argv[1] == NULL) {
        char* home_expand;
        dc_expand_path(env, err, &home_expand, "~/");
        dc_chdir(env, err, home_expand);

    }

    else {
        dc_chdir(env, err, command->argv[1]);
    }

    command->exit_code = 0;

    char* work_path;
    work_path = strdup(command->argv[1]);
    if (dc_error_has_error(err)) {
        int err_code = err->errno_code;
        if (dc_error_is_errno(err, err_code)) {

            switch (err_code) {
                case EACCES:
                    printf("EACCES: %s: Permission denied\n", work_path);
                    break;
                case ELOOP:
                    printf("ELOOP: %s: Too many levels of symbolic links\n", work_path);
                    break;
                case ENAMETOOLONG:
                    printf("ENAMETOOLONG: %s: File name too long\n", work_path);
                    break;
                case ENOENT:
                    printf("ENOENT: %s: No such file or directory\n", work_path);
                    break;
                case ENOTDIR:
                    printf("ENOTDIR: %s: Not a directory\n", work_path);
                    break;
            }
            command->exit_code = 1;
        }
    }
}






