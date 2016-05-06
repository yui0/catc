%{
#include <stdio.h>
#include <stdlib.h>
#include "AST.h"
#ifdef DEBUG
#define YYERROR_VERBOSE 1
#define YYDEBUG 1
#endif
%}

%union {
	AST *val;
	int type;
}

%right '='
%left '<' '>'
%left '+' '-'
%left '*' '/'
%right '!'

%type <type> type_specifier
%type <val> declarator init_declarator init_declarator_list initializer
%type <val> declaration declaration_list parameter_list parameter_declaration declaration_specifiers
%type <val> pointer type_qualifier_list
%type <val> statement compound_statement jump_statement iteration_statement
%type <val> expression_statement selection_statement
%type <val> direct_declarator block_item_list block_item
%type <val> identifier_list parameter_type_list
%type <val> expression postfix_expression argument_expression_list assignment_expression
%type <val> multiplicative_expression additive_expression cast_expression shift_expression
%type <val> equality_expression relational_expression unary_expression
%type <val> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF


%token	IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%start translation_unit
%%

primary_expression
	: IDENTIFIER
	| constant
	| string
	| '(' expression ')'
	| generic_selection
	;

constant
	: I_CONSTANT		/* includes character_constant */
	| F_CONSTANT
	| ENUMERATION_CONSTANT	/* after it has been defined as such */
	;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER
	;

string
	: STRING_LITERAL
	| FUNC_NAME
	;

generic_selection
	: GENERIC '(' assignment_expression ',' generic_assoc_list ')'
	;

generic_assoc_list
	: generic_association
	| generic_assoc_list ',' generic_association
	;

generic_association
	: type_name ':' assignment_expression
	| DEFAULT ':' assignment_expression
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	  { $$ = makeAST(CALL_OP, $1, 0); }
	| postfix_expression '(' argument_expression_list ')'
	  { $$ = makeAST(CALL_OP, $1, $3); }
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	| '(' type_name ')' '{' initializer_list '}'
	| '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list
	: assignment_expression
	  { $$ = makeList1($1); }
	| argument_expression_list ',' assignment_expression
	  { $$ = addLast($1, $3); }
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	| ALIGNOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	  { $$ = makeAST(MUL_OP, $1, $3); }
	| multiplicative_expression '/' cast_expression
//	  { $$ = makeAST(DIV_OP, $1, $3); }
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	  { $$ = makeAST(PLUS_OP, $1, $3); }
	| additive_expression '-' multiplicative_expression
	  { $$ = makeAST(MINUS_OP, $1, $3); }
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	  { $$ = makeAST(LT_OP, $1, $3); }
	| relational_expression '>' shift_expression
	  { $$ = makeAST(GT_OP, $1, $3); }
	| relational_expression LE_OP shift_expression
	  { $$ = makeAST(LE_OP, $1, $3); }
	| relational_expression GE_OP shift_expression
	  { $$ = makeAST(GE_OP, $1, $3); }
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	  { $$ = makeAST(EX_EQ, $1, $3); }
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression
//	  { $$ = makeList1($1); }
	| expression ',' assignment_expression
//	  { $$ = addLast($1, $3); }
	;

constant_expression
	: conditional_expression	/* with constraints */
	;

declaration
	: declaration_specifiers ';'
	| declaration_specifiers init_declarator_list ';'
	  { $2->left->sym->type = $1; $$ = $2; /*printf("[%s]\n",$2->left->sym->name);*/ }
	| static_assert_declaration
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
	| storage_class_specifier
	| type_specifier declaration_specifiers
	| type_specifier
	| type_qualifier declaration_specifiers
	| type_qualifier
	| function_specifier declaration_specifiers
	| function_specifier
	| alignment_specifier declaration_specifiers
	| alignment_specifier
	;

init_declarator_list
	: init_declarator
	  { $$ = addList($1);/*printf("[%s]\n",$1->sym->name);*//*printf("[%x]\n",$1->right);*/ }
	| init_declarator_list ',' init_declarator
	  { $$ = addLast($1, $3);/*printf("[%s]\n",$3->sym->name);*/ }
	;

init_declarator
	: declarator '=' initializer
	  { $$ = makeList1($1); $$ = addLast($$, makeAST(EX_EQ, $1, $3)); }	// int a = 10
	| declarator
	;

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

type_specifier
	: /* empty! */
	| VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| BOOL
	| COMPLEX
	| IMAGINARY	  	/* non-mandated extension */
	| atomic_type_specifier
	| struct_or_union_specifier
	| enum_specifier
	| TYPEDEF_NAME		/* after it has been defined as such */
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list ';'	/* for anonymous struct/union */
	| specifier_qualifier_list struct_declarator_list ';'
	| static_assert_declaration
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: ':' constant_expression
	| declarator ':' constant_expression
	| declarator
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER '{' enumerator_list '}'
	| ENUM IDENTIFIER '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant '=' constant_expression
	| enumeration_constant
	;

atomic_type_specifier
	: ATOMIC '(' type_name ')'
	;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

function_specifier
	: INLINE
	| NORETURN
	;

alignment_specifier
	: ALIGNAS '(' type_name ')'
	| ALIGNAS '(' constant_expression ')'
	;

declarator
	: pointer direct_declarator
	  { $2->sym->pointer = 1/*$1->sym->pointer*/; $$ = $2; }
	| direct_declarator
	;

direct_declarator
	: IDENTIFIER
//	  { $$ = makeList1($1); }
	| '(' declarator ')'
	| direct_declarator '[' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '(' parameter_type_list ')'
	  { $1->right = $3; }	// int func(int x)
	| direct_declarator '(' ')'
	| direct_declarator '(' identifier_list ')'
//	  { $$ = addLast($1, $3); }
	  { $1->right = $3; }	// func(x)
	;

pointer
	: '*' type_qualifier_list pointer
	| '*' type_qualifier_list
	| '*' pointer
//	  { $1->sym->pointer++; }	// char **a;
	| '*'
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list ',' ELLIPSIS
	| parameter_list
	;

parameter_list
	: parameter_declaration
	  { $$ = makeList1($1); }
	| parameter_list ',' parameter_declaration
	  { $$ = addLast($1, $3); }
	;

parameter_declaration
	: declaration_specifiers declarator
	  { $$ = $2; }
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	  { $$ = makeList1($1); }
	| identifier_list ',' IDENTIFIER
	  { $$ = addLast($1, $3); }
	;

type_name
	: specifier_qualifier_list abstract_declarator
	| specifier_qualifier_list
	;

abstract_declarator
	: pointer direct_abstract_declarator
	| pointer
	| direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' '*' ']'
	| '[' STATIC type_qualifier_list assignment_expression ']'
	| '[' STATIC assignment_expression ']'
	| '[' type_qualifier_list STATIC assignment_expression ']'
	| '[' type_qualifier_list assignment_expression ']'
	| '[' type_qualifier_list ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' '*' ']'
	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_abstract_declarator '[' type_qualifier_list ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	| assignment_expression
	;

initializer_list
	: designation initializer
	| initializer
	| initializer_list ',' designation initializer
	| initializer_list ',' initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
	;

static_assert_declaration
	: STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';'
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{'  block_item_list '}'
	  { $$ = makeAST(BLOCK_STATEMENT, $2, 0); printAST($$); }
	;

block_item_list
	: block_item
	  { $$ = addList($1); }	// declaration is LIST
//	  { if ($1->op!=LIST) $$ = makeList1($1); else $$ = $1; }	// declaration is LIST
	| block_item_list block_item
	  { $$ = addLast($1, $2); }
	;

block_item
	: declaration
//	  { /*del LIST*/ $$ = $1->left; free($1); }
	| statement
	;

expression_statement
	: ';'
	| expression ';'
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement
	  { $$ = makeAST(IF_STATEMENT, $3, makeList2($5, $7)); }
	| IF '(' expression ')' statement
	  { $$ = makeAST(IF_STATEMENT, $3, makeList2($5, NULL)); }
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	  { $$ = makeAST(WHILE_STATEMENT, $3, $5); }
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	  { $$ = makeAST(FOR_STATEMENT, makeList3($3, $4, $5), $7); }
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	  { $$ = makeAST(RETURN_STATEMENT, NULL, NULL); }
	| RETURN expression ';'
	  { $$ = makeAST(RETURN_STATEMENT, $2, NULL); }
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
//	  { defineFunction(getSymbol($2), $3, $4); }
	| declaration_specifiers declarator compound_statement
	  { defineFunction(getSymbol($2), $2->right, $3); }
//	  { defineFunction(getSymbol($2->left), $2->right, $3); }
//	| declarator compound_statement	// [empty!] func()
//	  { defineFunction(getSymbol($1), 0, $2); }
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%
#include <stdarg.h>
void error(char *fmt, ...)
{
	fprintf(stderr, "\033[1m\033[31m");
	va_list argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	fprintf(stderr, "\033[0m");
	exit(1);
}

#if 1
#include "lex.yy.c"

// retrieve from '# digits text'
static char *source; // current input file name
yymark()
{
	if (source) free(source);
	source = (char *)calloc(yyleng, sizeof(char));
	if (source) {
		sscanf(yytext, "# %d %s", &yylineno, source);
	}
}

void yyerror(char *s)
{
	fflush(stdout);
	fprintf(stderr, "\033[1m\033[31m");
	fprintf(stderr, "*** %d: %s near '%s'\n", yylineno, s, yytext);
	fprintf(stderr, "\033[0m");
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
	yydebug = 1;
#endif
	if (!strcmp(argv[1], "-S")) {
		argv++;
		argc = 1;
		yyin = fopen(argv[1], "r");
	}

	char name[3][256];
	if (argc>1) {
		yyin = fopen(argv[1], "r");
		if (argc>2) {
			yyout = fopen(argv[2], "w");
		} else {
			sscanf(argv[1], "%[^.]]", name[0]);
			strcpy(name[1], name[0]);
			strcat(name[1], ".o");
			strcpy(name[2], name[0]);
			//strcat(name[0], ".asm");
			strcat(name[0], ".s");	// gas
			yyout = fopen(name[0], "w");
		}
	}

	if (yyparse()) {
		fprintf(stderr, "*** fatal error!\n");
		return 1;
	}

	if (argc>1) {
		fclose(yyout);
		fclose(yyin);

		char cmd[256];
		//snprintf(cmd, 256, "as %s -o %s", name[0], name[1]);
		//system(cmd);
		//snprintf(cmd, 256, "gcc %s -o %s", name[1], name[2]);
		snprintf(cmd, 256, "gcc %s -o %s", name[0], name[2]);
		system(cmd);
	}

	return 0;
}
#else

#include "clex.c"
int main(int argc, char *argv[])
{
	yyparse();
	return 0;
}
#endif
