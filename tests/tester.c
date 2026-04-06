#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define FACILE_PATH "../build/facile"
#define ASSEMBLER "ilasm"

#define SENDS(...) {__VA_ARGS__, NULL}
#define GETS(...) {__VA_ARGS__, NULL}

#define TEST_NON_INTERACTIVE(func_name, ...)                                                       \
  int func_name() {                                                                                \
    const char *gets[] = {__VA_ARGS__, NULL};                                                      \
    return run_test_engine(#func_name, NULL, gets);                                                \
  }

#define TEST_INTERACTIVE(func_name, SENDS_ARRAY, GETS_ARRAY)                                       \
  int func_name() {                                                                                \
    const char *sends[] = SENDS_ARRAY, *gets[] = GETS_ARRAY;                                       \
    return run_test_engine(#func_name, sends, gets);                                               \
  }

void clean_string(char *str) {
  if (!str || *str == '\0')
    return;
  int len = strlen(str);
  while (len > 0 && isspace((unsigned char)str[len - 1])) {
    str[--len] = '\0';
  }
}

int assemble_test_file(const char *test_name) {
  char cmd[512];
  system("mkdir -p build");

  snprintf(cmd, sizeof(cmd), "%s %s.facile", FACILE_PATH, test_name);
  if (system(cmd) != 0)
    return -1;

  snprintf(cmd, sizeof(cmd), "%s %s.il /output:build/%s.exe > /dev/null", ASSEMBLER, test_name,
           test_name);
  if (system(cmd) != 0)
    return -1;

  snprintf(cmd, sizeof(cmd), "rm -f %s.il", test_name);
  system(cmd);

  return 0;
}

void build_mono_command(char *cmd, size_t max_length, const char *file_name, const char **sends) {
  cmd[0] = '\0';

  if (sends != NULL) {
    snprintf(cmd, max_length, "printf \"%%b\" \"");
    for (int i = 0; sends[i] != NULL; i++) {
      strncat(cmd, sends[i], max_length - strlen(cmd) - 1);
      strncat(cmd, "\\n", max_length - strlen(cmd) - 1);
    }
    strncat(cmd, "\" | mono build/", max_length - strlen(cmd) - 1);
  } else {
    snprintf(cmd, max_length, "mono build/");
  }

  strncat(cmd, file_name, max_length - strlen(cmd) - 1);
  strncat(cmd, ".exe", max_length - strlen(cmd) - 1);
}

int verify_test_output(const char *file_name, const char *cmd, const char **gets) {
  FILE *mono_pipe = popen(cmd, "r");
  if (!mono_pipe) {
    printf("[FATAL] popen failed to run mono for %s\n", file_name);
    return 0;
  }

  char line[256];
  int line_index = 0;
  int passed = 1;

  while (fgets(line, sizeof(line), mono_pipe) != NULL) {
    clean_string(line);
    if (strlen(line) == 0)
      continue;

    if (gets[line_index] == NULL) {
      printf("[FAIL] %s - Extra output caught: '%s'\n", file_name, line);
      passed = 0;
      break;
    }

    if (strcmp(line, gets[line_index]) != 0) {
      printf("[FAIL] %s (Line %d)\n       Expected: '%s'\n       Got:      '%s'\n", file_name,
             line_index + 1, gets[line_index], line);
      passed = 0;
      break;
    }
    line_index++;
  }
  pclose(mono_pipe);

  if (passed && gets[line_index] != NULL) {
    printf("[FAIL] %s - Missing output, expected '%s' but program exited.\n", file_name,
           gets[line_index]);
    passed = 0;
  }

  return passed;
}

int run_test_engine(const char *file_name, const char **sends, const char **gets) {
  if (assemble_test_file(file_name) != 0) {
    printf("[FATAL] Compilation/Assembly failed for %s\n", file_name);
    return -1;
  }

  char cmd[2048] = {0};
  build_mono_command(cmd, sizeof(cmd), file_name, sends);

  int passed = verify_test_output(file_name, cmd, gets);

  if (passed)
    printf("[PASS] %s\n", file_name);
  return passed;
}

// need to wrap args in () for proper VA_ARGS separation

#define LIST_OF_TESTS(X)                                                                           \
  X(NON, test_booleans, ("1", "2"))                                                                \
  X(NON, test_math, ("25"))                                                                        \
  X(NON, test_types, ("The answer is", "42"))                                                      \
  X(INT, test_read_empty, (SENDS("0")), (GETS("0")))                                               \
  X(INT, test_read_math, (SENDS("20")), (GETS("30")))                                              \
  X(NON, test_read_expr_precedence, ("50", "60", "10"))                                            \
  X(NON, test_if_condition, ("5"))                                                                 \
  X(NON, test_if_else, ("5", "3"))                                                                 \
  X(NON, test_while_loop, ("0", "1", "2", "3", "4", "5", "6", "7", "8", "9"))                      \
  X(NON, test_while_loop_nested, ("5", "5", "4", "3", "2", "1", "4", "3", "2", "1"))               \
  X(INT, test_largest_common_denominator, (SENDS("387", "129")), (GETS("129")))                    \
  X(NON, test_elseif, ("3"))                                                                       \
  X(NON, test_break_continue, ("1", "3"))                                                          \
  X(INT, test_types_intake, (SENDS("1", "2", "bonjur")), (GETS("3", "bonjur")))

#define STRIP_PARENS(...) __VA_ARGS__
#define DEFINE_TEST(type, name, ...) X_##type(name, __VA_ARGS__)

#define X_NON(name, args) TEST_NON_INTERACTIVE(name, STRIP_PARENS args)
#define X_INT(name, sends, gets) TEST_INTERACTIVE(name, STRIP_PARENS sends, STRIP_PARENS gets)

LIST_OF_TESTS(DEFINE_TEST)

#undef X_NON
#undef X_INT

typedef int (*test_func)(void);

int main() {

#define REGISTER_TEST(type, name, ...) name,
  test_func tests[] = {LIST_OF_TESTS(REGISTER_TEST)};

  int test_count = sizeof(tests) / sizeof(tests[0]);
  int *scoreboard = (int *)mmap(NULL, test_count * sizeof(*scoreboard), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

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
    if (scoreboard[i] == 1)
      passed++;
    else
      failed++;

  printf("Results: %d Passed, %d Failed\n", passed, failed);
  munmap(scoreboard, test_count * sizeof(*scoreboard));
  return failed > 0 ? 1 : 0;
}
