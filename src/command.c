#include <command.h>
#include <string.h>
#include <wordexp.h>
#include "input.h"
#include "shell.h"

char* regex_match(char* string, int num);
//char* tokenize(char *string);
size_t argc_count(char* string);
char **parse_space(const struct dc_posix_env *env, struct dc_error *err, const char *path_str);
char* wrd_process(char* string);



void parse_command(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command) {

    char* after_err_reg = strdup(regex_match(command->line, 2));
    if (strstr(after_err_reg, ">>") != NULL) {
        command->stderr_overwrite = true;
    }

//    printf("1st regex %s\n", after_err_reg);
    char* after_err_reg_2 = strdup(regex_match(after_err_reg, 3));
    if (strcmp(after_err_reg_2, "") == 0) {
        command->stderr_file = NULL;
    }
//    command->stderr_file = after_err_reg_2;
//    printf("2nd regex %s\n", after_err_reg_2);
    command->stderr_file = wrd_process(after_err_reg_2);
    free(after_err_reg);
    free(after_err_reg_2);



    char* after_out_reg = strdup(regex_match(command->line, 1));
    if (strstr(after_out_reg, ">>") != NULL) {
        command->stdout_overwrite = true;
    }
        printf("1st regex :%s\n", after_out_reg);

    char* after_out_reg_2 = strdup(regex_match(after_out_reg, 4));
    if (strcmp(after_out_reg_2, "") == 0) {
        command->stdout_file = NULL;
    }
    command->stdout_file = after_out_reg_2;
    printf("2nd regex :%s\n", after_out_reg_2);
    command->stdout_file = wrd_process(after_out_reg_2);
    free(after_out_reg);
    free(after_out_reg_2);

    /**
     * FREE <tokenize_command>
     */
//    char *user_input = strdup(command->line); // FREE
//    command->command = tokenize(user_input);
//    command->argc =
    command->argv = calloc(command->argc, 0); // FREE

}


char* regex_match(char* string, int num) {
    regex_t regex;
    regmatch_t match;
    int status;
    int matched;

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
        case 3:
            status = regcomp(&regex, "[~]?[\\/]?[a-zA-Z].*", REG_EXTENDED);
            break;
        case 4:
            status = regcomp(&regex, "[ ]?[~]?[\\/]?[a-zA-Z].*[ ]", REG_EXTENDED);
            break;
    }

    matched = regexec(&regex, string,  1, &match, 0);

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
    regfree(&regex);

    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;

        str = malloc((unsigned long) (length + 1));
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
//        printf("%s\n", str);
//        free(str);
        return str;
    }
}


char* wrd_process(char* string) {
    wordexp_t exp;
    int status;

    status = wordexp(string, &exp, 0);

    char* ptr = malloc(sizeof(char) * strlen(exp.we_wordv[0]));
    if (status == 0) {
        strcpy(ptr, exp.we_wordv[0]);
        wordfree(&exp);
    }
    return ptr;
}








