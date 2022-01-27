#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "shell_impl.h"
#include "state.h"
#include "util.h"


regex_t regex;
regex_t * regex_setting(int num);
char* prompt_state(const struct dc_posix_env *env, struct dc_error *err);



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


    states->stdin = stdin;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->stdout = stdout;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->stderr = stderr;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }


    states->command = NULL;
    if (dc_error_has_error(err)) {
        states->fatal_error = true;
        return ERROR;
    }

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
        states->stdin = stdin;
        states->stdout = stdout;
        states->stderr = stderr;
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
        states->stdin = stdin;
        states->stdout = stdout;
        states->stderr = stderr;
        states->in_redirect_regex = regex_setting(0);
        states->out_redirect_regex = regex_setting(1);
        states->err_redirect_regex = regex_setting(2);
        states->prompt = get_prompt(env, err);
        states->path = parse_path(env, err, get_path(env, err));
        states->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
        states->current_line = NULL;
        states->current_line_length = 0;
        states->command = NULL;
    }

    return READ_COMMANDS;
}