#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <state.h>
#include <command.h>
#include <dc_posix/dc_stdlib.h>


char *get_prompt(const struct dc_posix_env *env, struct dc_error *err) {
    const char *dollar = "$ ";
    char *prompt = dc_getenv(env, "PS1");
    char *dup_dollar = NULL;
    char *dup_prompt = NULL;

    if (!prompt) {
        dup_dollar = strdup(dollar);
        return dup_dollar;
    }

    dup_prompt = strdup(prompt);
    return dup_prompt;
}


char *get_path(const struct dc_posix_env *env, struct dc_error *err) {
    char *prompt = dc_getenv(env, "PATH");

    if (!prompt) {
        return NULL;
    }

    char *dup_prompt;
    dup_prompt = strdup(prompt);
    return dup_prompt;
}


char **parse_path(const struct dc_posix_env *env, struct dc_error *err, const char *path_str) {
    if (dc_error_has_no_error(err)) {
        char *path_copy = malloc(strlen(path_str) + 1);
        strcpy(path_copy, path_str);

        unsigned string_rows = 0;
        char ** arr = NULL;

        const char *delims = ":";
        char* ptr = strtok(path_copy, delims);
        while (ptr) {
            arr = realloc(arr, (string_rows + 1) * sizeof(arr));

            arr[string_rows] = malloc(strlen(ptr) + 1);
            strcpy(arr[string_rows], ptr);
            string_rows++;
            ptr = strtok(NULL, delims);
        }

        arr = realloc(arr, (string_rows + 1) * sizeof(arr));
        arr[string_rows] = NULL;
        free(path_copy);
        return arr;
    }
}


char *state_to_string(const struct dc_posix_env *env,  struct dc_error *err, const struct state *state) {
    size_t len;
    char *line;

    if(state->current_line == NULL) {
        len = strlen("current line = NULL");
    }
    else {
        len = strlen("current line = \"\"");
        len += state->current_line_length;
    }

    len += strlen(", fatal error = ");
    line = malloc(len + 1 + 100);

    if (state->current_line == NULL) {
        sprintf(line, "current_line = NULL, fatal_error = %d", state->fatal_error);
    }
    else {
        sprintf(line, "current_line = \"%s\", fatal_error = %d", state->current_line, state->fatal_error);
    }

    return line;
}


void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream) {
    char *str;
    struct dc_error err;
    str = state_to_string(env, &err, state);
    fprintf(stream, "%s\n", str);
    free(str);
}


void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state) {
    state->fatal_error = false;
    free(state->current_line);
    state->current_line = NULL;
    state->current_line_length = 0;
    destroy_command(env, state->command);
    state->command = NULL;
    dc_error_reset(err);
}

