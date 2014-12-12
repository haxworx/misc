#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void Failure(char *fmt, ...)
{
	char buf[8192] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	sprintf(buf, "ERR: %s\n", fmt);
	fprintf(stderr, buf);
	va_end(ap);

	exit(EXIT_FAILURE);
}

typedef struct token_t token_t;
struct token_t {
	char token[128];
	int type;
	int end_instruction;
	token_t *next;
};

#define TK_OP_ADD 1
#define TK_OP_DEL 2
#define TK_OP_MUL 3
#define TK_OP_DIV 4
#define TK_OP_ASG 5
#define TK_OP_STR 6

#define TK_VAR_VAL 7 
#define TK_VAR_NAM 8

int type_from_token(char *token)
{
	char *t = token;
	int result = 0;	
	
	switch (*t) {
		case '+':
			return TK_OP_ADD;
			break;
		case '-':
			return TK_OP_DEL;
			break;
		case '*':
			return TK_OP_MUL;
			break;
		case '/':
			return TK_OP_DIV;
			break;
		case '=':
			return TK_OP_ASG;
			break;
		
	}	

	if (*t == '"' && t[strlen(t) - 1] == '"')
		return TK_OP_STR;


	if (isdigit(*t))
		return TK_VAR_VAL;
	else
		return TK_VAR_NAM;

	return 0; 
}

token_t *AddToken(token_t *tokens, char *token, int end_of_line)
{
	token_t *c = tokens;
	
	int type = type_from_token(token); 

	if (c == NULL) {
		c = calloc(1, sizeof(token_t));
		strcpy(c->token, token);
		c->type = type; //
		c->end_instruction = end_of_line;
		return c;
	}

	for (c; c->next; c = c->next);

	if (c->next == NULL) {
		c->next = calloc(1, sizeof(token_t));
		c = c->next;
		strcpy(c->token, token);
		c->type = type; // 
		c->end_instruction = end_of_line;
		return c;
	}
}

void TokensList(token_t *tokens)
{	
	token_t *c = tokens->next;

	while (c) {
		printf("%s %d %d\n", c->token, c->type, c->end_instruction);
		c = c->next;
	}
}


token_t *Tokenize(char *file, ssize_t length)
{
	char *map = calloc(1, 1 + length);

	token_t *tokens = calloc(1, sizeof(token_t));

	FILE *f = fopen(file, "r");		
	if (!f)
		Failure("input file issue");

	fread(map, length, 1, f);	

	fclose(f);

	char *m = map;

	while (*m) {
		char *start = m;
		int have_quote = 0;
		int end_of_line = 0;

		while (*m != ',' && *m != ' ' && *m != '\t' && *m != '\r') {
			if (*m == '\n') {
				end_of_line = 1;
				break;
			}
			if (*m == '"') {
				m++; // onwards christian soldier!
				while (*m != '"') {
					++m;
				}
				m++;
			} else 
				m++;
			
		}

		*m = '\0'; ++m;

		AddToken(tokens, start, end_of_line); 

		while (*m == ' ' || *m == '\t' || *m == '\r' || *m == '\n') {
			++m;
		}			
	}

	return tokens;
}

int main(void)
{
	token_t *tokens = calloc(1, sizeof(token_t)); // our tokens

	char *infile = "test.al";

	struct stat fstats;

	if (stat(infile, &fstats))
		Failure("input file issue");

	ssize_t length = fstats.st_size;

	tokens = Tokenize(infile, length);	

	TokensList(tokens);

	return 0;
}
