#include <command.h>
#include <string.h>
#include <wordexp.h>
#include <ctype.h>
#include "input.h"
#include "shell.h"

char* regex_match(char* string, int num);
//char* tokenize(char *string);
size_t argc_count(char* string);
char **parse_space(const struct dc_posix_env *env, struct dc_error *err, const char *path_str);
char* wrd_process(char* string);
char* ltrim(char *s);



void parse_command(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command) {

    char* after_err_reg = strdup(regex_match(command->line, 2));
    if (strstr(after_err_reg, ">>") != NULL) {
        command->stderr_overwrite = true;
    }
//    printf("1st regex :%s\n", after_err_reg);

    if (strcmp(after_err_reg, "") == 0) {
        command->stderr_file = NULL;
    }
    else {
        char* after_err_reg_2 = strdup(regex_match(after_err_reg, 3));
        if (strcmp(after_err_reg_2, "") == 0) {
            command->stderr_file = NULL;
        }
//        command->stderr_file = after_err_reg_2;
//        printf("2nd regex :%s\n", after_err_reg_2);
        command->stderr_file = wrd_process(after_err_reg_2);
        free(after_err_reg_2);
    }
    char *err_ref = strdup(after_err_reg);
    free(after_err_reg);

    char* after_out_reg = strdup(regex_match(command->line, 1));
    if (strstr(after_out_reg, ">>") != NULL) {
        command->stdout_overwrite = true;
    }
    printf("1st regex :%s\n", after_out_reg);

    size_t len = strlen(after_out_reg) - strlen(err_ref) + 1;
    char out_err[len];
    strncpy(out_err, after_out_reg, strlen(after_out_reg) - strlen(err_ref) + 1);
    out_err[len - 1] = '\0';
    printf("out_err : %s\n", out_err);

    char* after_out_reg_2 = strdup(regex_match(out_err, 4));
    if (strcmp(after_out_reg_2, "") == 0) {
        command->stdout_file = NULL;
    }
    else {
        command->stdout_file = after_out_reg_2;
        printf("2nd regex :%s\n", after_out_reg_2);
        command->stdout_file = wrd_process(after_out_reg_2);
        free(after_out_reg_2);
    }
    free(after_out_reg);

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
            status = regcomp(&regex, "[^> *]*$", REG_EXTENDED);
            break;
        case 4:
            status = regcomp(&regex, "[^< *]*$", REG_EXTENDED);
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
    else {
        return strdup("");
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


char* ltrim(char *s) {
    char* begin;
    begin = s;

    while (*begin != '\0') {
        if (isspace(*begin))
            begin++;
        else {
            s = begin;
            break;
        }
    }
    return s;
}






