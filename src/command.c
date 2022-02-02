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
size_t wrd_count(char* string);
char* ltrim(char *s);


void parse_command(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command) {

    // STDERR
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



    // STDOUT
    char* after_out_reg = strdup(regex_match(command->line, 1));
    if (strstr(after_out_reg, ">>") != NULL) {
        command->stdout_overwrite = true;
    }
//    printf("1st regex :%s\n", after_out_reg);

    size_t len = strlen(after_out_reg) - strlen(err_ref) + 1;
    char out_err[len];
    strncpy(out_err, after_out_reg, len);
    out_err[len - 1] = '\0';
//    printf("out_err : %s\n", out_err);

    char* after_out_reg_2 = strdup(regex_match(out_err, 4));
    if (strcmp(after_out_reg_2, "") == 0) {
        command->stdout_file = NULL;
    }
    else {
//        command->stdout_file = after_out_reg_2;
//        printf("2nd regex :%s\n", after_out_reg_2);
        command->stdout_file = wrd_process(after_out_reg_2);
        free(after_out_reg_2);
    }
    char* out_ref = strdup(after_out_reg);
    free(after_out_reg);
    free(err_ref);


    // STDIN
    char* after_in_reg = strdup(regex_match(command->line, 0));
//    printf("stdin 1st regex: %s\n", after_in_reg);
//    printf("out_ref = %s\n", out_ref);

    char *trim_air = strdup(ltrim(after_in_reg));  //
    char *trim_or = strdup(ltrim(out_ref)); //
//    printf("after in reg len = %lu\n ", strlen(trim_air));
//    printf("out ref len = %lu\n ", strlen(trim_or));
    int len2 = (int)(strlen(trim_air) - strlen(trim_or));
    if (len2 <= 0) {
        command->stdin_file = NULL;
    }
    else {
        char in_out_err[len2];
        strncpy(in_out_err, trim_air, len2);
        in_out_err[len2] = '\0';
//        printf("in_out_err : %s\n", in_out_err);
        char* after_in_reg_2 = strdup(regex_match(in_out_err, 5));
        if (strcmp(after_in_reg_2, "") == 0) {
            command->stdin_file = NULL;
        }
        else {
//            command->stdin_file = after_in_reg_2;
//            printf("2nd regex :%s\n", after_in_reg_2);
            command->stdin_file = wrd_process(after_in_reg_2);
            free(after_in_reg_2);
        }
    }

    free(trim_air);
    free(trim_or);
    free(after_in_reg);


    // command->command
    char* after_in = strdup(regex_match(command->line, 0));
    size_t len3 = strlen(command->line) - strlen(after_in) + 1;
    char command_arr[len3];
    strncpy(command_arr, command->line, len3);
    command_arr[len3] = '\0';
//    printf("command_arr = %s\n", command_arr);

    // command->argc ????????????????????
    size_t argc_count = wrd_count(command_arr);
    if (argc_count == 0) {
        argc_count++;
    }
    command->argc = argc_count;



    strtok(command_arr, " ");
//    printf("command_arr = %s\n", command_arr);
//    printf("len : %d\n", strlen(command_arr));
    command->command = strdup(command_arr);
//    printf("command :%s\n", command->command);
    free(after_in);


    // NEED TO WORK FROM HERE
    char** argv_arr[command->argc + 2];
    command->argv = (char **) argv_arr;
}


char* regex_match(char* string, int num) {
    regex_t regex;
    regmatch_t match;
    int status;
    int matched;

    switch(num) {
        case 0:
            status = regcomp(&regex, "[ \\t\\f\\v]<.*", REG_EXTENDED);//in
            break;
        case 1:
            status = regcomp(&regex, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);//out
            break;
        case 2:
            status = regcomp(&regex, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);//err
            break;
        case 3:
            status = regcomp(&regex, "[^> *]*$", REG_EXTENDED);//err2
            break;
        case 4:
            status = regcomp(&regex, "[^< *]*$", REG_EXTENDED);//ou2
            break;
        case 5:
            status = regcomp(&regex, "[^<*]*.$", REG_EXTENDED);//in2
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


size_t wrd_count(char* string) {
    wordexp_t exp;
    int status;
    size_t count = 0;

    status = wordexp(string, &exp, 0);
    if (status == 0) {
        count = exp.we_wordc;
        wordfree(&exp);
    }
    return count;
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