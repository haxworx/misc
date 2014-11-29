#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void Bork(char *fmt, ...)
{
	char buf[8192] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	sprintf(buf, "ERR: %s\n", fmt);
	fprintf(stderr, buf);
	va_end(ap);

	exit(EXIT_FAILURE);
}

typedef struct code_t code_t;
struct code_t {
	char *name;
	char *arg;
	int type;
	union Value {
		int integer;
		char string[8192];
	} value;
	char *assign;
	code_t *next;
};

code_t *AddInstruction(struct code_t *codes, code_t * code)
{
	code_t *c = codes;

	if (c == NULL) {
		c = calloc(1, sizeof(code_t));
		c->name = strdup(code->name);
		c->type = code->type;
		if (c->type == 2) {
			strcpy(c->value.string, code->value.string);
		} else if (c->type == 3) {
			c->arg = code->arg;
		} else
			c->value.integer = code->value.integer;
		return c;
	}

	while (c->next)
		c = c->next;

	if (c->next == NULL) {
		c->next = calloc(1, sizeof(code_t));
		c = c->next;
		c->name = strdup(code->name);
		c->type = code->type;
		if (c->type == 2) {
			c->value.integer = code->value.integer;
		} else if (c->type == 3) {
			c->name = code->name;
			c->arg = code->arg;
		} else
			strcpy(c->value.string, code->value.string);

	}
}

void ShowInstructions(code_t * codes)
{
	code_t *c = codes->next;	// FIRST NULLITY

	while (c) {
		printf("name: %s\n", c->name);
		if (c->type == 2)
			printf("value: %d\n", c->value.integer);
		else if (c->type == 3) {
			printf("%s(%s);\n", c->name, c->arg);
		} else
			printf("value: %s\n", c->value.string);
		printf("type: %d\n", c->type);
		printf("arg: %s\n", c->arg);
		c = c->next;
	}
}


void ParsePseudoCode(char *path, ssize_t length)
{
	FILE *f = fopen(path, "r");
	if (!f)
		Bork("ERR: %s\n", strerror(errno));

	char *map = calloc(1, length + 1);
	if (!map)
		Bork("calloc()");

	code_t *codes = calloc(1, sizeof(code_t));
	code_t code;

	fread(map, length, 1, f);

	char *p = map;

	while (*p) {
		char *line = p;
		while (*line == ' ' || *line == '\t')
			++line;

		char *eol = strchr(line, '\n');
		if (eol)
			*eol = '\0';
		else
			Bork("ERR: no newline at end of file!");


		char *s = line;

		int has_changed = 0;

		if (0 == strncmp(s, "str", 3)) {
			while (*s == ' ' || *s == '\t')
				s++;								  // skip spaces
			code.type = 1;
			s += 4;
			char *e = strchr(s, ' ');
			if (e) {
				*e = '\0';
				code.name = s;
			} else {
				code.name = s;
			}
			s += strlen(code.name) + 1;
			s = strchr(s, '=');
			if (s) {
				while (*s == ' ' || *s == '=')
					++s;

				sscanf(s, "%s\"", code.value.string);
				code.arg = "NULL";
				has_changed = 1;
			}
		}

		if (0 == strncmp(s, "var", 3)) {
			while (*s == ' ' || *s == '\t')
				++s;
			code.type = 2;
			s += 4;
			char *e = strchr(s, ' ');
			if (e) {
				*e = '\0';
				code.name = s;
			} else {
				code.name = s;
			}

			s += strlen(code.name) + 1;
			s = strchr(s, '=');
			if (s) {
				while (*s == ' ' || *s == '=')
					++s;
				sscanf(s, "%d", &code.value.integer);
				code.arg = "NULL";
				has_changed = 1;
			}
		}
		char proto[1024] = { 0 };

		sprintf(proto, "%%s(%%s)");
		char name[128] = { 0 };
		char arg[128] = { 0 };
		int res = sscanf(s, proto, name, arg);
		if (!has_changed && res) {
			printf("%s\n", proto);
			code.name = name;
			code.arg = arg;
			code.type = 3;
			has_changed = 1;
		}
		if (has_changed)
			AddInstruction(codes, &code);
		p += (eol - line) + 1;
	}

	ShowInstructions(codes);

	free(map);
	fclose(f);
}

int main(int argc, char **argv)
{
	char *inf, *ouf = { NULL };

	if (argc != 3)
		Bork("Usage: ARGV[0] <source> <output>\n");

	inf = argv[1];
	ouf = argv[2];

	struct stat fstats;

	stat(inf, &fstats);

	ssize_t length = fstats.st_size;

	ParsePseudoCode(inf, length);

	return EXIT_SUCCESS;
}
