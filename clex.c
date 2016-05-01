char yytext[100];

/* for debug */
int getChar()
{
	int c;
	c = getc(stdin);
	return c;
}

void ungetChar(int c)
{
	ungetc(c,stdin);
}

int yylex()
{
	int c,n;
	char *p;
again:
	c = getChar();
	if (isspace(c)) {
		goto again;
	}
	switch (c) {
	case '+':
	case '-':
	case '*':
	case '>':
	case '<':
	case '(':
	case ')':
	case '{':
	case '}':
	case ']':
	case '[':
	case ';':
	case ',':
	case '=':
	case EOF:
		return c;
	case '"':
		p = yytext;
		while ((c = getChar()) != '"') {
			*p++ = c;
		}
		*p = '\0';
		yylval.val = makeStr(strdup(yytext));
		return STRING;
	}
	if (isdigit(c)) {
		n = 0;
		do {
			n = n*10 + c - '0';
			c = getChar();
		} while (isdigit(c));
		ungetChar(c);
		yylval.val = makeNum(n);
		return NUMBER;
	}
	if (isalpha(c)) {
		p = yytext;
		do {
			*p++ = c;
			c = getChar();
		} while (isalpha(c));
		*p = '\0';
		ungetChar(c);
		if (strcmp(yytext,"var") == 0) {
			return VAR;
		} else if (strcmp(yytext,"if") == 0) {
			return IF;
		} else if (strcmp(yytext,"else") == 0) {
			return ELSE;
		} else if (strcmp(yytext,"return") == 0) {
			return RETURN;
		} else if (strcmp(yytext,"while") == 0) {
			return WHILE;
		} else if (strcmp(yytext,"for") == 0) {
			return FOR;
		} else if (strcmp(yytext,"println") == 0) {
			return PRINTLN;
		} else {
			yylval.val = makeSymbol(yytext);
			return SYMBOL;
		}
	}
	fprintf(stderr,"bad char '%c'\n",c);
	exit(1);
}

void yyerror()
{
	printf("syntax error!\n");
	exit(1);
}
