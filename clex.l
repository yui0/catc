%{
int yywrap()
{
	return 1;
}

void count();
%}

digit				[0-9]
letter				[a-zA-Z_]
letter_or_digit			[a-zA-Z_0-9]
hex				[a-fA-F0-9]
E				([Ee][+-]?{D}+)
P				([Pp][+-]?{D}+)
FS				(f|F|l|L)
IS				((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

white_space			[ \t\n]
blank				[ \t]
other				.

%%
"/*"				{ /*comment()*/; }
"//"[^\n]*			{ /* consume //-comment */ }

"auto"				{ count(); return(AUTO); }
"_Bool"				{ count(); return(BOOL); }
"break"				{ count(); return(BREAK); }
"case"				{ count(); return(CASE); }
"char"				{ count(); return(CHAR); }
"_Complex"			{ count(); return(COMPLEX); }
"const"				{ count(); return(CONST); }
"continue"			{ count(); return(CONTINUE); }
"default"			{ count(); return(DEFAULT); }
"do"				{ count(); return(DO); }
"double"			{ count(); return(DOUBLE); }
"else"				{ count(); return(ELSE); }
"enum"				{ count(); return(ENUM); }
"extern"			{ count(); return(EXTERN); }
"float"				{ count(); return(FLOAT); }
"for"				{ count(); return(FOR); }
"goto"				{ count(); return(GOTO); }
"if"				{ count(); return(IF); }
"_Imaginary"			{ count(); return(IMAGINARY); }
"inline"			{ count(); return(INLINE); }
"int"				{ count(); return(INT); }
"long"				{ count(); return(LONG); }
"register"			{ count(); return(REGISTER); }
"restrict"			{ count(); return(RESTRICT); }
"return"			{ count(); return(RETURN); }
"short"				{ count(); return(SHORT); }
"signed"			{ count(); return(SIGNED); }
"sizeof"			{ count(); return(SIZEOF); }
"static"			{ count(); return(STATIC); }
"struct"			{ count(); return(STRUCT); }
"switch"			{ count(); return(SWITCH); }
"typedef"			{ count(); return(TYPEDEF); }
"union"				{ count(); return(UNION); }
"unsigned"			{ count(); return(UNSIGNED); }
"void"				{ count(); return(VOID); }
"volatile"			{ count(); return(VOLATILE); }
"while"				{ count(); return(WHILE); }
"println"			{ count(); return(PRINTLN); }

L?\"(\\.|[^\\"\n])*\"		{ count(); yylval.val = makeStr(strdup(yytext)); return(STRING); }
{letter}{letter_or_digit}*	{ count(); yylval.val = makeSymbol(yytext); return(SYMBOL); }

{digit}+			{ count(); yylval.val = makeNum(atoi(yytext)); return(NUMBER); }

"..."				{ count(); return(ELLIPSIS); }
">>="				{ count(); return(RIGHT_ASSIGN); }
"<<="				{ count(); return(LEFT_ASSIGN); }
"+="				{ count(); return(ADD_ASSIGN); }
"-="				{ count(); return(SUB_ASSIGN); }
"*="				{ count(); return(MUL_ASSIGN); }
"/="				{ count(); return(DIV_ASSIGN); }
"%="				{ count(); return(MOD_ASSIGN); }
"&="				{ count(); return(AND_ASSIGN); }
"^="				{ count(); return(XOR_ASSIGN); }
"|="				{ count(); return(OR_ASSIGN); }
">>"				{ count(); return(RIGHT_OP); }
"<<"				{ count(); return(LEFT_OP); }
"++"				{ count(); return(INC_OP); }
"--"				{ count(); return(DEC_OP); }
"->"				{ count(); return(PTR_OP); }
"&&"				{ count(); return(AND_OP); }
"||"				{ count(); return(OR_OP); }
"<="				{ count(); return(LE_OP); }
">="				{ count(); return(GE_OP); }
"=="				{ count(); return(EQ_OP); }
"!="				{ count(); return(NE_OP); }
";"				{ count(); return(';'); }
("{"|"<%")			{ count(); return('{'); }
("}"|"%>")			{ count(); return('}'); }
","				{ count(); return(','); }
":"				{ count(); return(':'); }
"="				{ count(); return('='); }
"("				{ count(); return('('); }
")"				{ count(); return(')'); }
("["|"<:")			{ count(); return('['); }
("]"|":>")			{ count(); return(']'); }
"."				{ count(); return('.'); }
"&"				{ count(); return('&'); }
"!"				{ count(); return('!'); }
"~"				{ count(); return('~'); }
"-"				{ count(); return('-'); }
"+"				{ count(); return('+'); }
"*"				{ count(); return('*'); }
"/"				{ count(); return('/'); }
"%"				{ count(); return('%'); }
"<"				{ count(); return('<'); }
">"				{ count(); return('>'); }
"^"				{ count(); return('^'); }
"|"				{ count(); return('|'); }
"?"				{ count(); return('?'); }

{white_space}+

^"#"{blank}*{digit}+({blank}+.*)?\n	yymark();
{other}				return yytext[0];

%%

int column = 0;
void count()
{
	for (int i=0; yytext[i] != '\0'; i++) {
		if (yytext[i] == '\n') {
			column = 0;
		} else if (yytext[i] == '\t') {
			column += 8 - (column % 8);
		} else {
			column++;
		}
	}
}
