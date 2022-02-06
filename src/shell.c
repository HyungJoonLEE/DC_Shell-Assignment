#include "shell.h"
#include "shell_impl.h"

#include <string.h>
#include <stdlib.h>

static void error_reporter(const struct dc_error *err);

int run_shell(const struct dc_posix_env *env, struct dc_error *error, FILE *in, FILE *out, FILE *err) {

    int ret_val;
    struct dc_fsm_info *fsm_info;
    static struct dc_fsm_transition transitions[] = {
        {DC_FSM_INIT,       INIT_STATE,         init_state},
        {INIT_STATE,        READ_COMMANDS,      read_commands},
        {INIT_STATE,        ERROR,              handle_error},
        {READ_COMMANDS,     RESET_STATE,        reset_state},
        {READ_COMMANDS,     SEPARATE_COMMANDS,  separate_commands},
        {READ_COMMANDS,     ERROR,              handle_error},
        {SEPARATE_COMMANDS, PARSE_COMMANDS,     parse_commands},
        {SEPARATE_COMMANDS, ERROR,              handle_error},
        {PARSE_COMMANDS,    EXECUTE_COMMANDS,   execute_commands},
        {PARSE_COMMANDS,    ERROR,              handle_error},
        {EXECUTE_COMMANDS,  RESET_STATE,        reset_state},
        {EXECUTE_COMMANDS,  EXIT,               do_exit},
        {EXECUTE_COMMANDS,  ERROR,              handle_error},
        {RESET_STATE,       READ_COMMANDS,      read_commands},
        {EXIT,              DESTROY_STATE,      destroy_state},
        {ERROR,             RESET_STATE,        reset_state},
        {ERROR,             DESTROY_STATE,      destroy_state},
        {DESTROY_STATE,     DC_FSM_EXIT,        NULL}
    };

//    dc_error_init(error, error_reporter);
    dc_posix_env_init(env, NULL /*trace_reporter*/);
    ret_val = EXIT_SUCCESS;
    fsm_info = dc_fsm_info_create(env, error, "dc_shell");
//    dc_fsm_info_set_will_change_state(fsm_info, will_change_state);
//    dc_fsm_info_set_did_change_state(fsm_info, did_change_state);
//    dc_fsm_info_set_bad_change_state(fsm_info, bad_change_state);

    if(dc_error_has_no_error(error))
    {

        struct state states;
        int from_state;
        int to_state;

        states.stdin = in;
        states.stdout = out;
        states.stderr = err;

        dc_fsm_run(env, error, fsm_info, &from_state, &to_state, &states, transitions);
        dc_fsm_info_destroy(env, &fsm_info);
    }

    return ret_val;
}




