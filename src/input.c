#include <stdio.h>
#include <string.h>
#include <dc_posix/dc_stdio.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_util/strings.h>
#include "input.h"

char *read_command_line(const struct dc_posix_env *env, struct dc_error *err, FILE *stream, size_t *line_size) {
    unsigned long size = (unsigned long) *line_size + 1;
//    char *cmd_line = dc_malloc(env, err, size * sizeof(char));
//
//    dc_fgets(env, err, cmd_line, (int)size, stream);
//    dc_str_trim(env, cmd_line);
//    *line_size = dc_strlen(env, cmd_line);
//    return cmd_line;

    char *line;
    line = dc_malloc(env, err, size * sizeof(char));
    dc_getline(env, err, &line, line_size, stream);
    dc_str_trim(env, line);
    *line_size = dc_strlen(env, line);
    return line;
}
