%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{
#include <stdio.h>
#include "y.tab.h"

extern void yyerror(char *);  /* prints grammar violation message */
//extern int sym_type(const char *);  /* returns type from symbol table */
#define sym_type(identifier) IDENTIFIER /* with no symbol table, fake it */

#define RET_TYPE(x)				{ yylval.type = x; return(x); }

void comment();
int check_type();
%}

%option yylineno

%%
"/*"					{ comment(); }
"//".*					{ /* consume //-comment */ }

"auto"					{ return(AUTO); }
"break"					{ return(BREAK); }
"case"					{ return(CASE); }
"char"					{ RET_TYPE(CHAR); }
"const"					{ return(CONST); }
"continue"				{ return(CONTINUE); }
"default"				{ return(DEFAULT); }
"do"					{ return(DO); }
"double"				{ return(DOUBLE); }
"else"					{ return(ELSE); }
"enum"					{ return(ENUM); }
"extern"				{ return(EXTERN); }
"float"					{ RET_TYPE(FLOAT); }
"for"					{ return(FOR); }
"goto"					{ return(GOTO); }
"if"					{ return(IF); }
"inline"				{ return(INLINE); }
"int"					{ RET_TYPE(INT); }
"long"					{ RET_TYPE(LONG); }
"register"				{ return(REGISTER); }
"restrict"				{ return(RESTRICT); }
"return"				{ return(RETURN); }
"short"					{ RET_TYPE(SHORT); }
"signed"				{ return(SIGNED); }
"sizeof"				{ return(SIZEOF); }
"static"				{ return(STATIC); }
"struct"				{ return(STRUCT); }
"switch"				{ return(SWITCH); }
"typedef"				{ return(TYPEDEF); }
"union"					{ return(UNION); }
"unsigned"				{ return(UNSIGNED); }
"void"					{ RET_TYPE(VOID); }
"volatile"				{ return(VOLATILE); }
"while"					{ return(WHILE); }
"_Alignas"				{ return ALIGNAS; }
"_Alignof"				{ return ALIGNOF; }
"_Atomic"				{ return ATOMIC; }
"_Bool"					{ RET_TYPE(BOOL); }
"_Complex"				{ RET_TYPE(COMPLEX); }
"_Generic"				{ return GENERIC; }
"_Imaginary"				{ RET_TYPE(IMAGINARY); }
"_Noreturn"				{ return NORETURN; }
"_Static_assert"			{ return STATIC_ASSERT; }
"_Thread_local"				{ return THREAD_LOCAL; }
"__func__"				{ return FUNC_NAME; }

{L}{A}*					{ yylval.val = makeSymbol(yytext); return(IDENTIFIER);/*return check_type();*/ }

{HP}{H}+{IS}?				{ yylval.val = makeNum(atoi(yytext)); return I_CONSTANT; }
{NZ}{D}*{IS}?				{ yylval.val = makeNum(atoi(yytext)); return I_CONSTANT; }
"0"{O}*{IS}?				{ yylval.val = makeNum(atoi(yytext)); return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"		{ yylval.val = makeNum(atoi(yytext)); return I_CONSTANT; }

{D}+{E}{FS}?				{ return F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?			{ return F_CONSTANT; }
{D}+"."{E}?{FS}?			{ return F_CONSTANT; }
{HP}{H}+{P}{FS}?			{ return F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?			{ return F_CONSTANT; }
{HP}{H}+"."{P}{FS}?			{ return F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ yylval.val = makeStr(strdup(yytext)); return STRING_LITERAL; }

"..."					{ return ELLIPSIS; }
">>="					{ return RIGHT_ASSIGN; }
"<<="					{ return LEFT_ASSIGN; }
"+="					{ return ADD_ASSIGN; }
"-="					{ return SUB_ASSIGN; }
"*="					{ return MUL_ASSIGN; }
"/="					{ return DIV_ASSIGN; }
"%="					{ return MOD_ASSIGN; }
"&="					{ return AND_ASSIGN; }
"^="					{ return XOR_ASSIGN; }
"|="					{ return OR_ASSIGN; }
">>"					{ return RIGHT_OP; }
"<<"					{ return LEFT_OP; }
"++"					{ return INC_OP; }
"--"					{ return DEC_OP; }
"->"					{ return PTR_OP; }
"&&"					{ return AND_OP; }
"||"					{ return OR_OP; }
"<="					{ return LE_OP; }
">="					{ return GE_OP; }
"=="					{ return EQ_OP; }
"!="					{ return NE_OP; }
";"					{ return ';'; }
("{"|"<%")				{ return '{'; }
("}"|"%>")				{ return '}'; }
","					{ return ','; }
":"					{ return ':'; }
"="					{ return '='; }
"("					{ return '('; }
")"					{ return ')'; }
("["|"<:")				{ return '['; }
("]"|":>")				{ return ']'; }
"."					{ return '.'; }
"&"					{ return '&'; }
"!"					{ return '!'; }
"~"					{ return '~'; }
"-"					{ return '-'; }
"+"					{ return '+'; }
"*"					{ return '*'; }
"/"					{ return '/'; }
"%"					{ return '%'; }
"<"					{ return '<'; }
">"					{ return '>'; }
"^"					{ return '^'; }
"|"					{ return '|'; }
"?"					{ return '?'; }

{WS}+					{ /* whitespace separates tokens */ }
.					{ /* discard bad characters */ }

%%

int yywrap()		/* called at end of input */
{
	return 1;	/* terminate now */
}

void comment()
{
	int c;

	while ((c = input()) != 0) {
		if (c == '*') {
			while ((c = input()) == '*') ;
			if (c == '/') return;
			if (c == 0) break;
		}
	}
	yyerror("unterminated comment\n");
}

/*int check_type()
{
	switch (sym_type(yytext)) {
	case TYPEDEF_NAME:		// previously defined
		return TYPEDEF_NAME;
	case ENUMERATION_CONSTANT:	// previously defined
		return ENUMERATION_CONSTANT;
	default:			// includes undefined
		return IDENTIFIER;
	}
}*/
