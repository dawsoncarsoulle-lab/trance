#ifndef _ERROR_H_
#define _ERROR_H_

typedef enum {
  UNDECLARED_VAR,
  TYPE_MISMATCH,
  REDECLARATION,
  NON_NUMERIC_MATH,
  ERROR_CODE_COUNT
} FacileErrorCode;

// TODO: make macro to pretreat the formatting or make separate construct for
// dynamic strings : ERR_UNDECLARED_VAR
static const char *facile_errors[ERROR_CODE_COUNT] = {
    [UNDECLARED_VAR] = "Undeclared variable '%s' in expression",
    [TYPE_MISMATCH] = "Type mismatch during affectation",
    [REDECLARATION] = "Variable re-declaration is not allowed",
    [NON_NUMERIC_MATH] = "Arithmetic only permitted on integers",
};
#define FACILE_ERR(err_index) facile_errors[(err_index)]

#define FATAL_ERROR(error_message)                                             \
  do {                                                                         \
    yyerror(error_message);                                                    \
    YYABORT;                                                                   \
  } while (0)
#define FATAL_ERROR_DYNAMIC(error_message_format, ...)                         \
  do {                                                                         \
    char error_message_buffer[256];                                            \
    snprintf(error_message_buffer, sizeof(error_message_buffer),               \
             error_message_format, ##__VA_ARGS__);                             \
    yyerror(error_message_buffer);                                             \
    YYABORT;                                                                   \
  } while (0)

// WARNING: uses FacileNode *, -> evaluated_type and also facile_errors
#define FAIL_ON_NON_NUMERIC(left, right)                                       \
  do {                                                                         \
    if (left->evaluated_type != T_INT || right->evaluated_type != T_INT)       \
      FATAL_ERROR(FACILE_ERR(NON_NUMERIC_MATH));                               \
  } while (0)

#endif // _ERROR_H_
