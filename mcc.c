
/*
 * mcc                       by Nagisa Neco Ishiura    *
 *                                                     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lex.h"
#include "tab.h"
#include "code.h"

#define ARRAY_MAXDIMENSION 8
#define STACK_FRAME_RESERVE 3

/* mcc のデバッグ用トレース情報の選択 */
typedef enum
{
  mcc_TRACE_NO,
  mcc_TRACE_LOW,
  mcc_TRACE_MID,
  mcc_TRACE_HIGH,
} mcc_trace_t;

static mcc_trace_t mcc_trace = mcc_TRACE_NO;
static int tmp_stack1716117715 = 13850;

static void arg(int argc, char **argv, char source_f[], char object_f[], mcc_trace_t *trace);
static void argerr();
static void at(char *checkpoint);
static void syntax_error(lex_t *x, char *msg);
static void semantic_error(char *msg);
static int id_isfunc(char *id, tab_t *gt, tab_t *lt);

static void preprocess(code_t *c);
static void postprocess(code_t *c, tab_t *gt);
static void parse_program(code_t *c, lex_t *x, tab_t *gt);
static void parse_declaration_head(code_t *c, lex_t *x, itab_basetype_t *type, int *ptrlevel, char *id);
static void parse_variable_declaration_tail(code_t *c, lex_t *x, tab_t *t, itab_cls_t cls, itab_basetype_t type, int ptrlevel, char *id);
static void parse_function_declaration_tail(code_t *c, lex_t *x, tab_t *t, itab_basetype_t type, int ptrlevel, char *id);
static void parse_function_body(code_t *c, lex_t *x, tab_t *gt, tab_t *lt, int argc);
static void parse_variable_declaration(code_t *c, lex_t *x, tab_t *t, itab_cls_t cls);
static void parse_statement(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_expression(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_expression2(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_expression3(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_expression4(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_expression5(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_variable_reference(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_array_index(code_t *c, lex_t *x, tab_t *gt, tab_t *lt, tab_t *t, int i);
static void parse_assign(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_if(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_while(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_return(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_call(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);
static void parse_lhs_expression(code_t *c, lex_t *x, tab_t *gt, tab_t *lt);

int main(int argc, char **argv)
{
  char source_f[FILENAME_MAX]; /* mini-C プログラムのファイル名 */
  char object_f[FILENAME_MAX]; /* VSM コードのファイル名 */

  /* (0) 変数 x, c, gt, lex_trace の宣言をこの下に書き込む */
  lex_t *x;
  code_t *c;
  tab_t *gt;
  lex_trace_t lex_trace;

  /* (1) 引数の解析 */
  arg(argc, argv, source_f, object_f, &mcc_trace);

  /* (2)〜(7)の処理をこの下に書き込む */
  x = lex_new(source_f);
  c = code_new();
  gt = tab_new(lex_TOKEN_MAXLEN);

  switch (mcc_trace)
  {
  case mcc_TRACE_NO:
    lex_trace = lex_TRACE_NO;
    break;

  case mcc_TRACE_LOW:
    lex_trace = lex_TRACE_BY_CHAR;
    break;

  case mcc_TRACE_MID:
    lex_trace = lex_TRACE_BY_TOKEN;
    break;

  case mcc_TRACE_HIGH:
    lex_trace = lex_TRACE_BY_TOKEN;
    break;

  default:
    assert(0); /* エラー */
  }

  lex_trace_set(x, lex_trace);

  preprocess(c);

  lex_get(x);
  parse_program(c, x, gt);

  postprocess(c, gt);

  code_write(c, object_f);

  tab_delete(gt);
  code_delete(c);
  lex_delete(x);

  return 0;
}

static void argerr()
/* 引数エラー (コマンドラインシンタックスを表示) */
{
  fprintf(stderr, "syntax: mcc [-t TRACE_LEVEL][-o file] PROG.mc\n");
  exit(EXIT_FAILURE);
}

static void arg(
    int argc,
    char **argv,
    char source_f[],
    char object_f[],
    mcc_trace_t *mcc_trace)
/* 引数の解析 */
{
  char *suffix;
  int i = 1;
  strcpy(object_f, "");
  if (i >= argc)
    argerr();
  while (argv[i][0] == '-')
  {
    if (strcmp(argv[i], "-t") == 0)
    {
      i++;
      if (i >= argc)
        argerr();
      switch (atoi(argv[i]))
      {
      case 0:
        *mcc_trace = mcc_TRACE_NO;
        break;
      case 1:
        *mcc_trace = mcc_TRACE_LOW;
        break;
      case 2:
        *mcc_trace = mcc_TRACE_MID;
        break;
      case 3:
        *mcc_trace = mcc_TRACE_HIGH;
        break;
      default:
        *mcc_trace = mcc_TRACE_NO;
      }
      i++;
    }
    else if (strcmp(argv[i], "-o") == 0)
    {
      i++;
      if (i >= argc)
        argerr();
      strcpy(object_f, argv[i]);
      i++;
    }
    else
      argerr();
    if (i >= argc)
      argerr();
  }
  if (i >= argc)
    argerr();
  strcpy(source_f, argv[i]);

  if (strlen(object_f) == 0)
  {
    strcpy(object_f, source_f);
    suffix = object_f + strlen(object_f) - 3;
    if (strcmp(suffix, ".mc") == 0)
    {
      strcpy(suffix, ".vsm");
    }
    else
    {
      strcpy(object_f + strlen(object_f), ".vsm");
    }
  }
}

static void at(char *checkpoint)
/* デバッグ用: チェックポイントの表示 (主に関数の入口用) */
{
  if (mcc_trace >= mcc_TRACE_MID)
  {
    fprintf(stderr, "at %s\n", checkpoint);
  }
}

static void end(char *checkpoint)
/* デバッグ用: チェックポイントの表示 (主に関数の出口用) */
{
  if (mcc_trace >= mcc_TRACE_MID)
  {
    fprintf(stderr, "end %s\n", checkpoint);
  }
}

static void syntax_error(lex_t *x, char *msg)
/* 文法エラー */
{
  fprintf(stderr, "%s:%d: %s (last token '%s')\n",
          x->filename, x->linenum, msg, x->token);
  exit(EXIT_FAILURE);
}

static void semantic_error(char *msg)
/* 意味エラー */
{
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

static void preprocess(code_t *c)
{
  at("preprocess");
  code_append(c, opcode_ISP, 0, 0);
  code_append(c, opcode_LC, 0, 0);
  code_append(c, opcode_SB, 1, 0);
  code_append(c, opcode_CALL, 0, 0);
  code_append(c, opcode_EXIT, 0, 0);
}

static void postprocess(code_t *c, tab_t *gt)
{
  int i = 0;
  at("postprocess");
  code_set(c, 0, opcode_ISP, gt->itab_vsize, 0);
  code_set(c, 1, opcode_LC, gt->itab_vsize, 0);

  i = tab_itab_find(gt, "main");
  if (i == itab_FAIL)
  {
    semantic_error("body of ’main’ not defined");
  }
  else if (gt->itab[i].role == itab_role_VAR)
  {
    semantic_error("’main’ is declared as a variable");
  }
  else if (gt->itab[i].role == itab_role_FUNC)
  {
    code_set(c, 3, opcode_CALL, gt->itab[i].address, 0);
  }
  else
  {
    assert(0);
  }
}

static void parse_program(code_t *c, lex_t *x, tab_t *gt)
{
  itab_basetype_t type;
  int ptrlevel;
  char id[lex_TOKEN_MAXLEN + 1];

  at("parse_program");

  while (x->type != token_EOF)
  {
    parse_declaration_head(c, x, &type, &ptrlevel, id);
    if (x->type == token_LPAREN)
    {
      parse_function_declaration_tail(c, x, gt, type, ptrlevel, id);
    }

    else
    {
      parse_variable_declaration_tail(c, x, gt, itab_cls_GLOBAL, type, ptrlevel, id);
      if (x->type == token_SEMICOLON)
      {
        lex_get(x);
      }
      else
      {
        syntax_error(x, "’;’ expected");
      }
    }
  }
}

static void parse_declaration_head(code_t *c, lex_t *x, itab_basetype_t *type, int *ptrlevel, char *id)
{
  at("parse_declaration_head");

  if (x->type == token_KW_INT)
  {
    *type = itab_basetype_INT;
  }

  else if (x->type == token_KW_CHAR)
  {
    *type = itab_basetype_CHAR;
  }

  else
  {
    syntax_error(x, "type name expected ");
  }
  lex_get(x);

  *ptrlevel = 0;
  while (x->type == token_STAR)
  {
    (*ptrlevel)++;
    lex_get(x);
  }

  if (x->type == token_ID)
  {
    strcpy(id, x->token);
    lex_get(x);
  }
  else
  {
    syntax_error(x, "identifier expected");
  }
}

static void parse_variable_declaration_tail(code_t *c, lex_t *x, tab_t *t, itab_cls_t cls, itab_basetype_t type, int ptrlevel, char *id)
{
  int max[ARRAY_MAXDIMENSION];
  int size = 0;
  int elementsize[ARRAY_MAXDIMENSION];
  int dimension = 0;

  at("parse_variable_declaration_tail");

  while (x->type == token_LBRACK)
  {
    assert(dimension < ARRAY_MAXDIMENSION);

    lex_get(x);

    if (x->type == token_INT)
    {
      max[dimension] = x->val;
      dimension++;
      lex_get(x);
    }
    else
    {
      syntax_error(x, "int expected");
    }

    if (x->type == token_RBRACK)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "] expected");
    }
  }

  if (dimension == 0)
  {
    size = 1;
  }
  else
  {
    elementsize[dimension - 1] = 1;
    for (int d = dimension - 2; 0 <= d; d--)
    {
      elementsize[d] = max[d + 1] * elementsize[d + 1];
    }
    size = max[0] * elementsize[0];
  }

  int i = tab_itab_new(t, id);

  if (i == itab_FAIL)
  {
    char errmsg[lex_TOKEN_MAXLEN + 256];
    sprintf(errmsg, "multiple definition of %s", id);
    syntax_error(x, errmsg);
  }

  t->itab[i].role = itab_role_VAR;
  t->itab[i].cls = cls;
  t->itab[i].basetype = type;
  t->itab[i].ptrlevel = ptrlevel;
  t->itab[i].argc = dimension;
  t->itab[i].size = size;
  t->itab[i].address = t->itab_vsize;
  t->itab_vsize += size;

  if (dimension == 0)
  {
    t->itab[i].aref = -1;
  }
  else
  {
    for (int d = 0; d < dimension; d++)
    {
      int a = tab_atab_append(t, max[d], elementsize[d]);
      if (d == 0)
        t->itab[i].aref = a;
    }
  }
}

static void parse_function_declaration_tail(code_t *c, lex_t *x, tab_t *gt, itab_basetype_t type, int ptrlevel, char *id)
{
  tab_t *lt;

  at("parse_function_declaration_tail");

  lt = tab_new(lex_TOKEN_MAXLEN);

  if (x->type == token_LPAREN)
  {
    lex_get(x);
  }
  else
  {
    syntax_error(x, "( expected");
  }

  int argc = 0;

  while (x->type != token_RPAREN)
  {
    argc++;
    parse_variable_declaration(c, x, lt, itab_cls_ARG);
    if (x->type != token_RPAREN)
    {
      if (x->type == token_COMMA)
      {
        lex_get(x);
      }
      else
      {
        syntax_error(x, ", expected");
      }
    }
  }

  lex_get(x);

  int i = tab_itab_new(gt, id);

  if (i == itab_FAIL)
  {
    char errmsg[lex_TOKEN_MAXLEN + 256];
    sprintf(errmsg, "multiple definition of %s", id);
    syntax_error(x, errmsg);
  }

  gt->itab[i].role = itab_role_FUNC;
  gt->itab[i].cls = itab_cls_GLOBAL;
  gt->itab[i].basetype = type;
  gt->itab[i].ptrlevel = ptrlevel;
  gt->itab[i].argc = argc;
  gt->itab[i].aref = -1;
  gt->itab[i].address = c->size;

  if (mcc_trace >= mcc_TRACE_HIGH)
  {
    fprintf(stderr, "== dump list of gt ==\n");
    tab_dump(gt);
  }

  parse_function_body(c, x, gt, lt, argc);
  gt->itab[i].size = c->size - gt->itab[i].address;

  tab_delete(lt);
}

static void parse_function_body(code_t *c, lex_t *x, tab_t *gt, tab_t *lt, int argc)
{
  at("parse_function_body");
  if (x->type == token_LBRACE)
  {
    lex_get(x);
  }
  else
  {
    syntax_error(x, "'{' expected");
  }

  while (x->type == token_KW_INT || x->type == token_KW_CHAR)
  {
    parse_variable_declaration(c, x, lt, itab_cls_LOCAL);
    if (x->type == token_SEMICOLON)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "';' expected");
    }
  }

  if (mcc_trace >= mcc_TRACE_HIGH)
  {
    fprintf(stderr, "== dump list of lt ==\n");
    tab_dump(lt);
  }

  code_append(c, opcode_ISP, STACK_FRAME_RESERVE + lt->itab_vsize, 0);

  while (x->type != token_RBRACE)
  {
    parse_statement(c, x, gt, lt);
  }

  if (x->type == token_RBRACE)
  {
    lex_get(x);
  }
  else
  {
    syntax_error(x, "'}' expected");
  }

  code_append(c, opcode_RET, 0, 0);
}

static void parse_variable_declaration(code_t *c, lex_t *x, tab_t *t, itab_cls_t cls)
{
  itab_basetype_t type;
  int ptrlevel;
  char id[lex_TOKEN_MAXLEN + 1];

  at("parse_variable_declaration");

  parse_declaration_head(c, x, &type, &ptrlevel, id);
  parse_variable_declaration_tail(c, x, t, cls, type, ptrlevel, id);
}

static void parse_statement(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_statement");
  parse_call(c, x, gt, lt);
  code_append(c, opcode_ISP, -1, 0);
  if (x->type == token_SEMICOLON)
  {
    lex_get(x);
  }
  else
  {
    syntax_error(x, "';' expected");
  }
}

static void parse_expression(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  int type = 0;
  at("parse_expression");
  parse_expression2(c, x, gt, lt);
  type = x->type;
  while (x->type == token_EQEQ || x->type == token_NE || x->type == token_GT || x->type == token_GE || x->type == token_LT || x->type == token_LE)
  {
    lex_get(x);
    parse_expression2(c, x, gt, lt);
    if (type == token_EQEQ)
    {
      code_append(c, opcode_EQ, 0, 0);
    }
    else if (type == token_NE)
    {
      code_append(c, opcode_NE, 0, 0);
    }
    else if (type == token_GT)
    {
      code_append(c, opcode_GT, 0, 0);
    }
    else if (type == token_LT)
    {
      code_append(c, opcode_LT, 0, 0);
    }
    else if (type == token_GE)
    {
      code_append(c, opcode_GE, 0, 0);
    }
    else if (type == token_LE)
    {
      code_append(c, opcode_LE, 0, 0);
    }
    else
    {
      ;
    }
    type = x->type;
  }
}

static void parse_expression2(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  int type = 0;
  at("parse_expression2");
  type = x->type;
  if (x->type == token_PLUS || x->type == token_MINUS)
  {
    lex_get(x);
  }
  parse_expression3(c, x, gt, lt);
  if (type == token_MINUS)
  {
    code_append(c, opcode_INV, 0, 0);
  }

  type = x->type;
  while (x->type == token_PLUS || x->type == token_MINUS)
  {
    lex_get(x);
    parse_expression3(c, x, gt, lt);
    if (type == token_PLUS)
    {
      code_append(c, opcode_ADD, 0, 0);
    }
    else if (type == token_MINUS)
    {
      code_append(c, opcode_SUB, 0, 0);
    }
    else
    {
      ;
    }
    type = x->type;
  }
}

static void parse_expression3(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  int type = 0;
  at("parse_expression3");
  parse_expression4(c, x, gt, lt);
  type = x->type;
  while (x->type == token_STAR || x->type == token_SLASH || x->type == token_PERCENT)
  {
    lex_get(x);
    parse_expression4(c, x, gt, lt);
    if (type == token_STAR)
    {
      code_append(c, opcode_MUL, 0, 0);
    }
    else if (type == token_SLASH)
    {
      code_append(c, opcode_DIV, 0, 0);
    }
    else if (type == token_PERCENT)
    {
      code_append(c, opcode_MOD, 0, 0);
    }
    else
    {
      ;
    }
    type = x->type;
  }
}

static void parse_expression4(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_expression4");
  parse_expression5(c, x, gt, lt);
}

static void parse_expression5(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_expression5");
  if (x->type == token_CHAR || x->type == token_INT || x->type == token_LPAREN || x->type == token_RPAREN)
  {
    if (x->type == token_LPAREN)
    {
      lex_get(x);
      parse_expression(c, x, gt, lt);
      if (x->type == token_RPAREN)
      {
        lex_get(x);
      }
      else
      {
        syntax_error(x, "')' expected");
      }
    }

    else
    {
      code_append(c, opcode_LC, x->val, 0);
      lex_get(x);
    }
  }

  else if (x->type == token_ID)
  {
    parse_call(c, x, gt, lt);
  }

  else
  {
    syntax_error(x, "undefined function");
  }
}

static void parse_variable_reference(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_variable_reference");
}

static void parse_array_index(code_t *c, lex_t *x, tab_t *gt, tab_t *lt, tab_t *t, int i)
{
  at("parse_array_index");
}

static void parse_assign(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_assign");
}

static void parse_if(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_if");
}

static void parse_while(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_while");
}

static void parse_return(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_return");
}

static void parse_call(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_call");

  if (strcmp(x->token, "putchar") == 0)
  {
    lex_get(x);
    if (x->type == token_LPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "'(' expected");
    }

    if (x->type != token_RPAREN)
    {
      parse_expression(c, x, gt, lt);
      if (x->type == token_RPAREN)
      {
        lex_get(x);
      }

      else
      {
        syntax_error(x, "')' expected");
      }
    }

    else if (x->type == token_RPAREN)
    {
      syntax_error(x, "argument expected");
    }
    else
    {
      syntax_error(x, "')' expected");
    }
    code_append(c, opcode_DUP, 0, 0);
    code_append(c, opcode_PUTC, 0, 0);
  }

  else if (strcmp(x->token, "putint") == 0)
  {
    lex_get(x);
    if (x->type == token_LPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "'(' expected");
    }

    if (x->type != token_RPAREN)
    {
      parse_expression(c, x, gt, lt);
      if (x->type == token_RPAREN)
      {
        lex_get(x);
      }

      else
      {
        syntax_error(x, "')' expected");
      }
    }

    else if (x->type == token_RPAREN)
    {
      syntax_error(x, "argument expected");
    }
    
    else
    {
      syntax_error(x, "')' expected");
    }
    code_append(c, opcode_DUP, 0, 0);
    code_append(c, opcode_PUTI, 0, 0);
  }

  else if (strcmp(x->token, "getchar") == 0)
  {
    lex_get(x);
    if (x->type == token_LPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "'(' expected");
    }

    if (x->type != token_RPAREN)
    {
      syntax_error(x, "too many argument");
    }

    if (x->type == token_RPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "')' expected");
    }
    code_append(c, opcode_GETC, 0, 0);
  }

  else if (strcmp(x->token, "getint") == 0)
  {
    lex_get(x);
    if (x->type == token_LPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "'(' expected");
    }

    if (x->type != token_RPAREN)
    {
      syntax_error(x, "too many argument");
    }

    if (x->type == token_RPAREN)
    {
      lex_get(x);
    }
    else
    {
      syntax_error(x, "')' expected");
    }
    code_append(c, opcode_GETI, 0, 0);
  }

  else
  {
    syntax_error(x, "undefined function");
  }
}

static void parse_lhs_expression(code_t *c, lex_t *x, tab_t *gt, tab_t *lt)
{
  at("parse_lhs_expression");
}

static int id_isfunc(char *id, tab_t *gt, tab_t *lt)
/* id が関数名なら 1 を，変数名なら 0 を返す */
{
}
