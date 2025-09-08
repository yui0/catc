%{
#include <stdio.h>
#include <stdlib.h>
#include "AST.h"

void yyerror(const char *s);
extern int yylex();
extern FILE *yyin, *yyout;
%}

%union {
    AST *val;
    int type;
}

%right '='
%left '+' '-'
%left '*' '/'

%type <type> type_specifier
%type <val> declarator init_declarator declaration
%type <val> statement compound_statement expression_statement
%type <val> expression assignment_expression additive_expression multiplicative_expression
%type <val> primary_expression

%token IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL
%token INT VOID CHAR FLOAT DOUBLE
%token IF WHILE RETURN
%token '=' '+' '-' '*' '/'

%start translation_unit
%%

/* 翻訳単位 */
translation_unit
    : external_declaration
    | translation_unit external_declaration
    ;

/* 外部宣言（関数定義または変数宣言） */
external_declaration
    : function_definition
    | declaration
        { declareVariable(getSymbol($1->left), $1->right); }
    ;

/* 関数定義 */
function_definition
    : type_specifier declarator compound_statement
        { defineFunction(getSymbol($2), $2->right, $3); }
    ;

/* 変数宣言 */
declaration
    : type_specifier init_declarator ';'
        { $2->sym->type = $1; $$ = $2; }
    ;

/* 初期化子付き宣言 */
init_declarator
    : declarator
    | declarator '=' expression
        { $$ = makeAST(EX_EQ, $1, $3); }
    ;

/* 宣言子 */
declarator
    : IDENTIFIER
        { $$ = makeAST(IDENTIFIER, NULL, NULL); }
    | declarator '(' parameter_list ')'
        { $1->right = $3; $$ = $1; }
    ;

/* パラメータリスト */
parameter_list
    : parameter_declaration
        { $$ = makeList1($1); }
    | parameter_list ',' parameter_declaration
        { $$ = addLast($1, $3); }
    ;

/* パラメータ宣言 */
parameter_declaration
    : type_specifier declarator
        { $$ = $2; }
    ;

/* 型指定子 */
type_specifier
    : VOID      { $$ = VOID; }
    | CHAR      { $$ = CHAR; }
    | INT       { $$ = INT; }
    | FLOAT     { $$ = FLOAT; }
    | DOUBLE    { $$ = DOUBLE; }
    ;

/* ステートメント */
statement
    : compound_statement
    | expression_statement
    | IF '(' expression ')' statement
        { $$ = makeAST(IF_STATEMENT, $3, $5); }
    | WHILE '(' expression ')' statement
        { $$ = makeAST(WHILE_STATEMENT, $3, $5); }
    | RETURN expression ';'
        { $$ = makeAST(RETURN_STATEMENT, $2, NULL); }
    ;

/* 複合ステートメント */
compound_statement
    : '{' '}'
        { $$ = NULL; }
    | '{' block_item_list '}'
        { $$ = makeAST(BLOCK_STATEMENT, $2, NULL); }
    ;

/* ブロックアイテム */
block_item_list
    : block_item
        { $$ = makeList1($1); }
    | block_item_list block_item
        { $$ = addLast($1, $2); }
    ;

/* ブロックアイテム（宣言またはステートメント） */
block_item
    : declaration
    | statement
    ;

/* 式ステートメント */
expression_statement
    : ';'
        { $$ = NULL; }
    | expression ';'
        { $$ = $1; }
    ;

/* 式 */
expression
    : assignment_expression
    ;

/* 代入式 */
assignment_expression
    : additive_expression
    | IDENTIFIER '=' assignment_expression
        { $$ = makeAST(EX_EQ, makeAST(IDENTIFIER, NULL, NULL), $3); }
    ;

/* 加減算式 */
additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression
        { $$ = makeAST(PLUS_OP, $1, $3); }
    | additive_expression '-' multiplicative_expression
        { $$ = makeAST(MINUS_OP, $1, $3); }
    ;

/* 乗除算式 */
multiplicative_expression
    : primary_expression
    | multiplicative_expression '*' primary_expression
        { $$ = makeAST(MUL_OP, $1, $3); }
    | multiplicative_expression '/' primary_expression
        { $$ = makeAST(DIV_OP, $1, $3); }
    ;

/* プライマリ式 */
primary_expression
    : IDENTIFIER
        { $$ = makeAST(IDENTIFIER, NULL, NULL); }
    | I_CONSTANT
        { $$ = makeAST(I_CONSTANT, NULL, NULL); }
    | F_CONSTANT
        { $$ = makeAST(F_CONSTANT, NULL, NULL); }
    | '(' expression ')'
        { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s near line %d\n", s, yylineno);
    exit(1);
}
