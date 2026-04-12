#include "../include/error.h"
#include "../include/subprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define TEST_PROCESS_OPTS                                                      \
  (subprocess_option_inherit_environment | subprocess_option_search_user_path)
#define CLR_RUNTIME_EXEC "mono"
#define FACILE_COMPILER_PATH "../build/facile"

// assembler exec used in Makefile, done before test running.

int run_test(const char *name, int is_err, const char **sends,
             const char **gets, const char *expected_err) {
  char out[256] = {0};
  char line[256], arg_file[256];
  int fail = 0, i = 0, status = 0;

  // faciel compiler, facile_file
  const char *command_line[4] = {0};

  if (is_err) {
    snprintf(arg_file, sizeof(arg_file), "%s.facile", name);
    command_line[0] = FACILE_COMPILER_PATH;
  } else {
    snprintf(arg_file, sizeof(arg_file), "build/%s.exe", name);
    command_line[0] = CLR_RUNTIME_EXEC;
  }
  command_line[1] = arg_file;

  int options = TEST_PROCESS_OPTS;
  if (is_err)
    options |= subprocess_option_combined_stdout_stderr;

  struct subprocess_s process;
  if (subprocess_create(command_line, options, &process) != 0) {
    printf("\033[0;31m[FAIL]\033[0m %s (Failed to launch process)\n", name);
    return 1;
  }

  if (!is_err && sends) {
    FILE *p_stdin = subprocess_stdin(&process);
    if (p_stdin) {
      for (int j = 0; sends[j]; j++)
        fprintf(p_stdin, "%s\n", sends[j]);
      fclose(p_stdin);
      process.stdin_file = NULL;
    }
  }

  FILE *p_stdout = subprocess_stdout(&process);
  if (p_stdout) {
    while (fgets(line, sizeof(line), p_stdout)) {
      line[strcspn(line, "\r\n")] = 0;
      if (!line[0])
        continue;

      if (is_err) {
        if (strlen(out) + strlen(line) < sizeof(out) - 1)
          strcat(out, line);
      } else {
        if (!gets || !gets[i] || strcmp(line, gets[i++]))
          fail = 1;
      }
    }
  }

  subprocess_join(&process, &status);
  subprocess_destroy(&process);

  if (is_err)
    fail = (status == 0) || !strstr(out, expected_err);
  else if (status != 0 || (gets && gets[i]))
    fail = 1;

  printf("\033[0;3%dm[%s]\033[0m %s\n", fail ? 1 : 2, fail ? "FAIL" : "PASS",
         name);
  return fail;
}

#define TEST_NON_INTERACTIVE(name, ...)                                        \
  int name() {                                                                 \
    const char *g[] = {__VA_ARGS__, NULL};                                     \
    return run_test(#name, 0, NULL, g, NULL);                                  \
  }

#define TEST_INTERACTIVE(name, s_arr, g_arr)                                   \
  int name() {                                                                 \
    const char *s[] = s_arr, *g[] = g_arr;                                     \
    return run_test(#name, 0, s, g, NULL);                                     \
  }

#define TEST_COMPILATION_FAIL(name, err)                                       \
  int name() { return run_test(#name, 1, NULL, NULL, err); }

#define LIST_OF_TESTS(X)                                                       \
  X(NON, test_booleans, ("1", "2"))                                            \
  X(NON, test_math, ("25"))                                                    \
  X(NON, test_types, ("The answer is", "42"))                                  \
  X(INT, test_read_empty, (SENDS("0")), (GETS("0")))                           \
  X(INT, test_read_math, (SENDS("20")), (GETS("30")))                          \
  X(NON, test_read_expr_precedence, ("50", "60", "10"))                        \
  X(NON, test_if_condition, ("5"))                                             \
  X(NON, test_if_else, ("5", "3"))                                             \
  X(NON, test_while_loop, ("0", "1", "2", "3", "4", "5", "6", "7", "8", "9"))  \
  X(NON, test_while_loop_nested,                                               \
    ("5", "5", "4", "3", "2", "1", "4", "3", "2", "1"))                        \
  X(INT, test_largest_common_denominator, (SENDS("387", "129")),               \
    (GETS("129")))                                                             \
  X(NON, test_elseif, ("3"))                                                   \
  X(NON, test_break_continue, ("1", "3"))                                      \
  X(INT, test_types_intake, (SENDS("1", "2", "bonjur")),                       \
    (GETS("3", "bonjur")))                                                     \
  X(NON, test_spec_relational, ("1", "1", "1", "1", "1", "0"))                 \
  X(NON, test_spec_logic, ("0", "1", "0", "1"))                                \
  X(NON, test_hello_world, ("Hello, World too"))                               \
  X(COMP, test_type_mismatch, FACILE_ERR(NON_NUMERIC_MATH))                    \
  X(COMP, test_redeclaration, FACILE_ERR(REDECLARATION))

#define SENDS(...) {__VA_ARGS__, NULL}
#define GETS(...) {__VA_ARGS__, NULL}

#define STRIP_PARENS(...) __VA_ARGS__
#define DEFINE_TEST(type, name, ...) X_##type(name, __VA_ARGS__)

#define X_NON(name, args) TEST_NON_INTERACTIVE(name, STRIP_PARENS args)
#define X_INT(name, sends, gets)                                               \
  TEST_INTERACTIVE(name, STRIP_PARENS sends, STRIP_PARENS gets)
#define X_COMP(name, err) TEST_COMPILATION_FAIL(name, err)

LIST_OF_TESTS(DEFINE_TEST)

#undef X_NON
#undef X_INT

typedef int (*test_func)(void);

int main() {
#define REGISTER_TEST(type, name, ...) name,
  test_func tests[] = {LIST_OF_TESTS(REGISTER_TEST)};

  int test_count = sizeof(tests) / sizeof(tests[0]);
  int *scoreboard =
      (int *)mmap(NULL, test_count * sizeof(*scoreboard),
                  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  for (int i = 0; i < test_count; i++)
    scoreboard[i] = -1;

  for (int i = 0; i < test_count; i++)
    if (fork() == 0)
      scoreboard[i] = tests[i](), exit(0);

  while (wait(NULL) > 0)
    ;

  int passed = 0, failed = 0;
  for (int i = 0; i < test_count; i++)
    scoreboard[i] != 1 ? passed++ : failed++;

  printf("Results: %d Passed, %d Failed\n", passed, failed);
  munmap(scoreboard, test_count * sizeof(*scoreboard));
  return failed > 0 ? 1 : 0;
}
