#include <command.h>
#include <string.h>
#include <wordexp.h>
#include <ctype.h>
#include "input.h"
#include "shell.h"

char* regex_match(char* string, int num);
char* err_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* string);
char* out_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* string);
char* in_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* out_ref);
char* cmdcmd_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command);
size_t argc_count(char* string);
char **parse_space(const struct dc_posix_env *env, struct dc_error *err, const char *path_str);
char* wrd_process(char* string);
size_t wrd_count(char* string);
char* ltrim(char *s);


void parse_command(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command) {
    char* after_err_regex = strdup(err_func(env, err, state, command, command->line));
//    printf("after err regex :%s\n", after_err_regex);
    char* after_out_regex = strdup(out_func(env, err, state, command, after_err_regex));
//    printf("after out regex :%s\n", after_out_regex);
    char* after_in_regex = strdup(in_func(env, err, state, command, after_out_regex));
//    printf("after in regex :%s\n", after_in_regex);
    free(after_in_regex);
    free(after_out_regex);
    free(after_err_regex);




}


char* err_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* string) {
    regex_t regex;
    regmatch_t match;
    int status;
    int matched;

    status = regcomp(&regex, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);//err
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


    char *str = NULL;
    if (matched == 0) {
        regoff_t length = match.rm_eo - match.rm_so;

        str = malloc((unsigned long) (length + 1));
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
//        printf("Position: %lld\n", match.rm_so);
        printf("str :%s\n", str);

        int offset = 3;
        if (strstr(str, ">>") != NULL) {
            command->stderr_overwrite = true;
            offset++;
        }

        unsigned long long int cut_len = ((unsigned long long int) (match.rm_eo - match.rm_so)) -
                                         (unsigned long long int) offset;
        char cut_err_part[sizeof(char) * cut_len + 1];

        strncpy(cut_err_part, ltrim(str + offset), cut_len);
        cut_err_part[cut_len] = '\0';
//        printf("cut err part :%s\n", cut_err_part);
        command->stderr_file = wrd_process(strdup(cut_err_part));

        char *changed_str = malloc(sizeof(char) * (unsigned long long int) match.rm_so + 1);
        strncpy(changed_str, string, match.rm_so);
        changed_str[match.rm_so] = '\0';
        return changed_str;
    }
    return string;
}


char* out_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* string) {
    regex_t regex;
    regmatch_t match;
    int status;
    int matched;

    status = regcomp(&regex, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);//out
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


    char *str = NULL;
    if (matched == 0) {
        regoff_t length = match.rm_eo - match.rm_so;

        str = malloc((unsigned long) (length + 1));
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
//        printf("Position: %lld\n", match.rm_so);

        int offset = 3;
        if (strstr(str, ">>") != NULL) {
            command->stdout_overwrite = true;
            offset++;
        }


        unsigned long long int cut_len = ((unsigned long long int) (match.rm_eo - match.rm_so)) -
                                         (unsigned long long int) offset;
        char cut_out_part[sizeof(char) * cut_len + 1];

        strncpy(cut_out_part, ltrim(str + offset), cut_len);
        cut_out_part[cut_len] = '\0';
//        printf("cut out part :%s\n", cut_out_part);
        command->stdout_file = wrd_process(strdup(cut_out_part));


        char *changed_str = malloc(sizeof(char) * (unsigned long long int) match.rm_so + 1);
        strncpy(changed_str, string, match.rm_so);
        changed_str[match.rm_so] = '\0';
        return changed_str;
    }
    printf("overwrite in source: %d\n", command->stdout_overwrite);

    return string;
}


char* in_func(const struct dc_posix_env *env, struct dc_error *err,struct state *state, struct command *command, char* string) {
    regex_t regex;
    regmatch_t match;
    int status;
    int matched;

    status = regcomp(&regex, "[ \\t\\f\\v]<.*", REG_EXTENDED);//err
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


    char *str = NULL;
    if (matched == 0) {
        regoff_t length = match.rm_eo - match.rm_so;

        str = malloc((unsigned long) (length + 1));
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
//        printf("Position: %lld\n", match.rm_so);
//        printf("str :%s\n", str);

        int offset = 2;
        if (strstr(str, "<") != NULL) {
            command->stdout_overwrite = true;
            offset++;
        }


        unsigned long long int cut_len = ((unsigned long long int) (match.rm_eo - match.rm_so) -
                                          (unsigned long long int) offset) ;
        char cut_in_part[sizeof(char) * cut_len + 1];

        strncpy(cut_in_part, ltrim(str + offset), cut_len);
        cut_in_part[cut_len] = '\0';
//        printf("cut in part :%s\n", cut_in_part);
        command->stdin_file = wrd_process(strdup(cut_in_part));


        char *changed_str = malloc(sizeof(char) * (unsigned long long int) match.rm_so + 1);
        strncpy(changed_str, string, match.rm_so);
        changed_str[match.rm_so] = '\0';
        return changed_str;
    }
    return string;
}
//
//
//char* cmdcmd_func(const struct dc_posix_env *env, struct dc_error *err, struct state *state, struct command *command) {
//    char* after_in = strdup(regex_match(command->line, 0));
//    size_t len3 = strlen(command->line) - strlen(after_in) + 1;
//    char command_arr[len3];
//    strncpy(command_arr, command->line, len3);
//    command_arr[len3] = '\0';
//    strtok(command_arr, " ");
////    printf("command_arr = %s\n", command_arr);
////    printf("len : %d\n", strlen(command_arr));
//    command->command = strdup(command_arr);
////    printf("command :%s\n", command->command);
//    free(after_in);
//    return strdup(command_arr);
//}
//
//
//size_t argc_count(char* cmdcmd) {
//    size_t count = wrd_count(cmdcmd);
//    printf("cmdcmd :%s\n", cmdcmd );
//    if (count == 0) {
//        count++;
//    }
//    return count;
//}
//
//
//
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
//
//
//size_t wrd_count(char* string) {
//    wordexp_t exp;
//    int status;
//    size_t count = 0;
//
//    status = wordexp(string, &exp, 0);
//    if (status == 0) {
//        count = exp.we_wordc;
//        wordfree(&exp);
//    }
//    return count;
//}
//
//
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
