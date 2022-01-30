//#include <dc_util/strings.h>
//#include <dc_posix/dc_stdio.h>
//#include "tests.h"
//#include "execute.h"
//
//static void test_execute(const char *cmd, char **argv, char **path, int expected_exit_code);
//
//Describe(execute);
//
//static struct dc_posix_env environ;
//static struct dc_error error;
//
//BeforeEach(execute)
//{
//    dc_posix_env_init(&environ, NULL);
//    dc_error_init(&error, NULL);
//}
//
//AfterEach(execute)
//{
//    dc_error_reset(&error);
//}
//
//Ensure(execute, execute)
//{
//    char **path;
//    char **argv;
//    char out[1024];
//    char err[1024];
//
//    path = dc_strs_to_array(&environ, &error, 3, "/bin", "/usr/bin", NULL);
//
//    argv = dc_strs_to_array(&environ, &error, 2, NULL, NULL);
//    test_execute("pwd", argv, path, 0);
//
//    argv = dc_strs_to_array(&environ, &error, 2, NULL, NULL);
//    test_execute("ls", argv, path, 0);
//
//    path = dc_strs_to_array(&environ, &error, 1, NULL);
//    argv = dc_strs_to_array(&environ, &error, 2, NULL, NULL);
//    test_execute("ls", argv, path, 127);
//
//    path = dc_strs_to_array(&environ, &error, 2, "/");
//    argv = dc_strs_to_array(&environ, &error, 2, NULL, NULL);
//    test_execute("ls", argv, path, 127);
//}
//
//static void test_execute(const char *cmd, char **argv, char **path, int expected_exit_code)
//{
//    struct command command;
//
//    memset(&command, 0, sizeof(struct command));
//    command.command = strdup(cmd);
//    command.argv = argv;
//    execute(&environ, &error, &command, path);
//    assert_that(command.exit_code, is_equal_to(expected_exit_code));
//    free(command.command);
//}
//
//TestSuite *execute_tests(void)
//{
//    TestSuite *suite;
//
//    suite = create_test_suite();
//    add_test_with_context(suite, execute, execute);
//
//    return suite;
//}
