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

#define TEST_NON_INTERACTIVE(func_name, file_name, ...)                                            \
  void func_name() {                                                                               \
    const char *gets[] = {__VA_ARGS__, NULL};                                                      \
    run_test_engine(file_name, NULL, gets);                                                        \
  }

#define TEST_INTERACTIVE(func_name, file_name, SENDS_ARRAY, GETS_ARRAY)                            \
  void func_name() {                                                                               \
    const char *sends[] = SENDS_ARRAY, *gets[] = GETS_ARRAY;                                       \
    run_test_engine(file_name, sends, gets);                                                       \
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

  snprintf(cmd, sizeof(cmd), "%s build/%s.il < %s", FACILE_PATH, test_name, test_name);
  if (system(cmd) != 0)
    return -1;

  snprintf(cmd, sizeof(cmd), "%s build/%s.il /output:build/%s.exe > /dev/null", ASSEMBLER,
           test_name, test_name);
  if (system(cmd) != 0)
    return -1;

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

void run_test_engine(const char *file_name, const char **sends, const char **gets) {
  if (assemble_test_file(file_name) != 0) {
    printf("[FATAL] Compilation/Assembly failed for %s\n", file_name);
    return;
  }

  char cmd[2048] = {0};
  build_mono_command(cmd, sizeof(cmd), file_name, sends);

  int passed = verify_test_output(file_name, cmd, gets);

  if (passed) {
    printf("[PASS] %s\n", file_name);
  }
}

TEST_NON_INTERACTIVE(test_math, "test-math", "25")
TEST_INTERACTIVE(test_read_empty, "test-read_empty", SENDS("0"), GETS("0"))
TEST_INTERACTIVE(test_read_math, "test-read_math", SENDS("20"), GETS("30"))
TEST_NON_INTERACTIVE(test_read_expr_precedence, "test-read_expr_precedence", "50", "60", "10")
TEST_NON_INTERACTIVE(test_if_condition, "test-if_condition", "5")
TEST_NON_INTERACTIVE(test_if_else, "test-if_else", "5", "3")
TEST_NON_INTERACTIVE(test_while_loop, "test-while_loop", "0", "1", "2", "3", "4", "5", "6", "7",
                     "8", "9")
TEST_NON_INTERACTIVE(test_while_loop_nested, "test-while_loop_nested", "5", "5", "4", "3", "2", "1",
                     "4", "3", "2", "1")
TEST_INTERACTIVE(test_largest_common_denominator, "test-largest_common_denominator",
                 SENDS("387", "129"), GETS("129"))
TEST_NON_INTERACTIVE(test_elseif, "test-elseif", "3")
TEST_NON_INTERACTIVE(test_break_continue, "test-break_continue", "1", "3")

typedef void (*test_func)(void);

int main() {
  test_func tests[] = {test_math,
                       test_read_empty,
                       test_read_math,
                       test_read_expr_precedence,
                       test_if_condition,
                       test_if_else,
                       test_while_loop,
                       test_while_loop_nested,
                       test_largest_common_denominator,
                       test_elseif,
                       test_break_continue};

  int test_count = sizeof(tests) / sizeof(tests[0]);
  int *scoreboard = mmap(NULL, test_count * sizeof(int), PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  for (int i = 0; i < test_count; i++)
    scoreboard[i] = -1;

  for (int i = 0; i < test_count; i++) {
    if (fork() == 0) {
      tests[i]();
      scoreboard[i] = 0;
      exit(0);
    }
  }
  while (wait(NULL) > 0)
    ;

  int passed = 0, failed = 0;
  for (int i = 0; i < test_count; i++) {
    if (scoreboard[i] == 0)
      passed++;
    else
      failed++;
  }

  printf("Results: %d Passed, %d Failed\n", passed, failed);
  munmap(scoreboard, test_count * sizeof(int));
  return failed > 0 ? 1 : 0;
}
