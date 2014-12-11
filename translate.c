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
	token_t *next;
};

#define INT 0001
#define OPA 0002
#define OPD 0003
#define CAL 0004

int type_from_token(char *token)
{
	
	return 0;
}

token_t *AddToken(token_t *tokens, char *token)
{
	token_t *c = tokens;
	
	int type = type_from_token(token); 

	if (c == NULL) {
		c = calloc(1, sizeof(token_t));
		strcpy(c->token, token);
		c->type = type; //

		return c;
	}

	for (c; c->next; c = c->next);

	if (c->next == NULL) {
		c->next = calloc(1, sizeof(token_t));
		c = c->next;
		strcpy(c->token, token);
		c->type = type; // 

		return c;
	}
}

void TokensList(token_t *tokens)
{	
	token_t *c = tokens->next;

	while (c) {
		printf("%s %d\n", c->token, c->type);
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
		while (*m != ' ' && *m != '\t' && *m != '\r' && *m != '\n') {
			m++;
		}

		*m = '\0'; ++m;

		AddToken(tokens, start); 

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
