#include "../include/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int run_test(const char *name, int is_err, const char **sends,
             const char **gets, const char *expected_err) {
  char cmd[1024] = {0}, out[2048] = {0}, line[256];
  int fail = 0, i = 0;

  if (is_err)
    snprintf(cmd, sizeof(cmd), "../build/facile %s.facile 2>&1", name);
  else {
    strcpy(cmd, "printf '");
    for (int j = 0; sends && sends[j]; j++)
      snprintf(cmd + strlen(cmd), 1024 - strlen(cmd), "%s\\n", sends[j]);
    snprintf(cmd + strlen(cmd), 1024 - strlen(cmd), "' | mono build/%s.exe",
             name);
  }

  FILE *p = popen(cmd, "r");
  while (fgets(line, sizeof(line), p)) {
    line[strcspn(line, "\r\n")] = 0;
    if (!line[0])
      continue;

    if (is_err)
      strcat(out, line);
    else if (!gets || !gets[i] || strcmp(line, gets[i++]))
      fail = 1;
  }
  int status = pclose(p);

  if (is_err)
    fail = status == 0 || !strstr(out, expected_err);
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
  X(ERR, test_type_mismatch, FACILE_ERR(NON_NUMERIC_MATH))                     \
  X(ERR, test_redeclaration, FACILE_ERR(REDECLARATION))

#define SENDS(...) {__VA_ARGS__, NULL}
#define GETS(...) {__VA_ARGS__, NULL}

#define STRIP_PARENS(...) __VA_ARGS__
#define DEFINE_TEST(type, name, ...) X_##type(name, __VA_ARGS__)

#define X_NON(name, args) TEST_NON_INTERACTIVE(name, STRIP_PARENS args)
#define X_INT(name, sends, gets)                                               \
  TEST_INTERACTIVE(name, STRIP_PARENS sends, STRIP_PARENS gets)
#define X_ERR(name, err) TEST_COMPILATION_FAIL(name, err)

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

  for (int i = 0; i < test_count; i++) {
    if (fork() == 0) {
      scoreboard[i] = tests[i]();
      exit(0);
    }
  }
  while (wait(NULL) > 0)
    ;

  int passed = 0, failed = 0;
  for (int i = 0; i < test_count; i++)
    scoreboard[i] != 1 ? passed++ : failed++;

  printf("Results: %d Passed, %d Failed\n", passed, failed);
  munmap(scoreboard, test_count * sizeof(*scoreboard));
  return failed > 0 ? 1 : 0;
}
