#include "shell.h"
#include "shell_impl.h"

#include <string.h>
int run_shell(const struct dc_posix_env *env, struct dc_error *err){

    printf("This is INIT_STATE = %d\n", INIT_STATE);
    printf("This is READ_COMMANDS = %d\n", READ_COMMANDS);
    printf("This is SEPARATE_COMMANDS = %d\n", SEPARATE_COMMANDS);
    printf("This is PARSE_COMMANDS = %d\n", PARSE_COMMANDS );
    printf("This is EXECUTE_COMMANDS = %d\n", EXECUTE_COMMANDS);


    return 0;
}





