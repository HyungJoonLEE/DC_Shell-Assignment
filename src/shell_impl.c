#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <dc_posix/dc_stdio.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/filesystem.h>
#include <dc_posix/dc_string.h>
#include "shell_impl.h"
#include "state.h"
#include "util.h"
#include "shell.h"
#include "input.h"
#include "builtins.h"


regex_t regex;
regex_t * regex_setting(int num);



int init_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;


    states->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->in_redirect_regex = regex_setting(0);
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->out_redirect_regex = regex_setting(1);
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->err_redirect_regex = regex_setting(2);
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->path = parse_path(env, err, get_path(env, err));
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->prompt = get_prompt(env, err);
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->current_line = NULL;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->current_line_length = 0;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->command = NULL;

    return READ_COMMANDS;
}

regex_t * regex_setting(int num) {
    int status;

    switch(num) {
        case 0:
            status = regcomp(&regex, "[ \\t\\f\\v]<.*", REG_EXTENDED);
            break;
        case 1:
            status = regcomp(&regex, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);
            break;
        case 2:
            status = regcomp(&regex, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);
            break;
    }

    if (status != 0) {
        size_t size;
        char* msg;

        size = regerror(status, &regex, NULL, 0);
        msg = malloc(size + 1);
        regerror(status, &regex, msg, size + 1);
        fprintf(stderr, "%s", msg);
        free(msg);
        exit(1);
    }

    else return &regex;
}


int destroy_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    states->fatal_error = false;
    states->in_redirect_regex = NULL;
    states->out_redirect_regex = NULL;
    states->err_redirect_regex = NULL;
    states->prompt = NULL;
    states->path = NULL;
    states->max_line_length = 0;
    states->current_line = NULL;
    destroy_command(env, states->command);

    return DC_FSM_EXIT;
}



int reset_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;
    do_reset_state(env, err, states);

    return READ_COMMANDS;
}


int read_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    size_t len = 0;

    if (dc_error_has_no_error(err)) {

        states->fatal_error = false;

        char *pwd = dc_get_working_dir(env, err);
        if (dc_error_has_error(err)) {
            states->fatal_error = true;
            return ERROR;
        }

        fprintf(states->stdout, "[%s] %s", pwd, states->prompt);
        if (dc_error_has_error(err)) {
            states->fatal_error = true;
            return ERROR;
        }
        char *rcl = read_command_line(env, err, states->stdin, &len);

        if (dc_error_has_error(err)) {
            states->fatal_error = true;
            free(rcl);
            free(pwd);
            return ERROR;
        }

        states->current_line = strdup(rcl);
        if (dc_error_has_error(err)) {
            free(rcl);
            free(pwd);
            states->fatal_error = true;
            return ERROR;
        }

        states->current_line_length = dc_strlen(env, rcl);
        if (dc_error_has_error(err)) {
            free(rcl);
            free(pwd);
            states->fatal_error = true;
            return ERROR;
        }

        free(rcl);
        free(pwd);

        if (len == 0) {
            return RESET_STATE;
        }
    }
    return SEPARATE_COMMANDS; // 4
}


int separate_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    states->command = dc_calloc(env, err, 1, sizeof(*states->command));
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->line = dc_strdup(env, err, states->current_line);
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->command = NULL;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->exit_code = 0;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    for (size_t i = 0; i < states->command->argc; i++) {
        states->command->argv[i] = NULL;
        if (dc_error_has_error(err)){
            states->fatal_error = true;
            return ERROR;
        }
    }

    states->command->argv = NULL;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->argc = 0;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->stdin_file = NULL;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->stdout_file = NULL;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->stderr_file = NULL;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->stdout_overwrite = false;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    states->command->stderr_overwrite = false;
    if (dc_error_has_error(err)){
        states->fatal_error = true;
        return ERROR;
    }

    return PARSE_COMMANDS;
}


int parse_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    parse_command(env, err, states, states->command);
    if (dc_error_has_error(err)) {
        return ERROR;
    }

    return EXECUTE_COMMANDS;
}


int execute_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    if (strcmp(states->command->command, "cd") == 0) {
        builtin_cd(env, err, states->command, states->stderr);
    }
    else if(strcmp(states->command->command, "exit") == 0) {
        return EXIT;
    }
    else {
        execute(env, err, states->command, states->path);
        if (dc_error_has_error(err)) {
            states->fatal_error = true;
        }
    }

    fprintf(states->stdout, "%d\n", states->command->exit_code);
    if (states->fatal_error == true) {
        return ERROR;
    }
    return RESET_STATE;
}


int do_exit(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    do_reset_state(env, err, states);
    return DESTROY_STATE;
}


int handle_error(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    if(states->current_line == NULL) {
        fprintf(states->stderr, "internal error (%d) %s\n", err->err_code, err->message);
    }
    else {
        fprintf(states->stderr, "internal error (%d) %s: \"%s\"\n", err->err_code, err->message, states->current_line);
    }
    if (states->fatal_error == true) {
        return DESTROY_STATE;
    }
    return RESET_STATE;
}




