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

    if (!dc_error_has_error(err)) {
        states->fatal_error = false;
        states->in_redirect_regex = NULL;
        states->out_redirect_regex = NULL;
        states->err_redirect_regex = NULL;
        states->prompt = NULL;
        states->path = NULL;
        states->max_line_length = 0;
        states->current_line = NULL;
    }

    return DC_FSM_EXIT;
}



int reset_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    if (!dc_error_has_error(err)) {
        states->fatal_error = false;
        states->in_redirect_regex = regex_setting(0);
        states->out_redirect_regex = regex_setting(1);
        states->err_redirect_regex = regex_setting(2);
        states->prompt = get_prompt(env, err);
        states->path = parse_path(env, err, get_path(env, err));
        states->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
        states->current_line = NULL;
        states->current_line_length = 0;

        free(states->command);
        states->command = NULL;
    }

    return READ_COMMANDS;
}


/**
 * Prompt the user and read the command line (see read_command_line).
 * Sets the state->current_line and current_line_length.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return SEPARATE_COMMANDS
 */
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
            return ERROR;
        }

        states->current_line = strdup(rcl);
        states->current_line_length = dc_strlen(env, rcl);
        free(rcl);

        if (len == 0) {
            return RESET_STATE;
        }
    }
    return SEPARATE_COMMANDS; // 4
}


int separate_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;
    struct state new_states;

    if (dc_error_has_no_error(err)) {
        states->fatal_error = false;

        new_states.command = dc_calloc(env, err, 1, sizeof(*states->command));
        states->command = new_states.command;


        if (dc_error_has_error(err)) {
            free(new_states.command);
            states->fatal_error = true;
            return ERROR;
        }

        states->command->line = strdup(states->current_line);

        if (dc_error_has_error(err)) {
            free(new_states.command);
            free(states->current_line);
            states->fatal_error = true;
            return ERROR;
        }

//        new_states.command->line = NULL;
        new_states.command->command = NULL;
        new_states.command->argc = 0;
        new_states.command->argv = NULL;
        new_states.command->stdin_file = NULL;
        new_states.command->stdout_file = NULL;
        new_states.command->stdout_overwrite = false;
        new_states.command->stderr_file = NULL;
        new_states.command->stderr_overwrite = false;
        new_states.command->exit_code = 0;

        if (dc_error_has_error(err)) {
            free(new_states.command);
            states->fatal_error = true;
            return ERROR;
        }
    }

    return PARSE_COMMANDS;
}


int parse_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *states;
    states = (struct state*) arg;

    if (dc_error_has_no_error(err)) {
        states->fatal_error = false;

        if (dc_error_has_error(err)) {
            states->fatal_error = true;
            return ERROR;
        }


    }
}