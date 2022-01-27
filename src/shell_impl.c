#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "shell_impl.h"
#include "state.h"



/**
* @param env the posix environment.
* @param err the error object
* @param arg the current struct state
* @return READ_COMMANDS or INIT_ERROR
*/
int init_state(const struct dc_posix_env *env, struct dc_error *err, struct state *arg) {

    if (arg->fatal_error == false ) {
        return READ_COMMANDS;
    }


    return ERROR;
}

/***
 * If any errors occur
 *
    set state.fatal_error to true and return ERROR
    set state.max_line_length to _SC_ARG_MAX via sysconf()
    set state.in_redirect_regex to [ \t\f\v]<.*
    set state.out_redirect_regex to [ \t\f\v][1^2]?>[>]?.*
    set state.err_redirect_regex to [ \t\f\v]2>[>]?.*
    get the PATH environment variables
    split PATH into an array, separate by :
    set state.path to the array
    get the PS1 environment variables
    if PS1 is NULL set state.prompt to “$”
    Otherwise, set state.prompt to the PS1 value
    set all other variables to 0, NULL, or false
    return READ_COMMANDS
*/


