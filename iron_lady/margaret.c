#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void error_quit(char *error)
{
	fprintf(stderr, "Error: %s\n", error);
	exit(EXIT_FAILURE);	
}


void chomp(char *str)
{
	while(*str) {
		if(*str == '\n') {
			*str = '\0';
			return;	
		}	
		str++;
	}	
}

char **read_quotes(char *quote_file, int *num)
{
	FILE *fp;
	char buf[512];
	char *p;
	int i = 0;
	char **quotes = NULL;

	fp = fopen(quote_file, "r");
	if(!fp)
		error_quit("Unable to open quote file!");
	
	
	while((p = fgets(buf, sizeof(buf), fp)) != NULL) {
			chomp(buf);
			
			if(!strlen(buf))
				continue;
		
			quotes = realloc(quotes, 1 + i * sizeof(char **));
			quotes[i++] = strdup(buf);
	}

	*num = --i;	
	
	fclose(fp);

	return quotes;
}

void display_quote(char *data)
{
	char *p = data;
	int c_count = 0;
	
	putchar('\n');
	printf(" \"");
	
	while(*p) {
				
		if(c_count >= 65 && *p == ' ') {
			printf("  \n");
			c_count = 0;	
		}
		
		putchar(*p);
	
		c_count++;
		p++;
	}
	
	printf("\"\n\n");
	
	puts(" Margaret Thatcher (The Iron Lady)\n");
}

void show_quote(char **quotes, int num)
{
	int i = 0;
	
	srandom(time(NULL));
	
	i = (int) (num * (random() / (RAND_MAX + 1.0)));
	
	display_quote(quotes[i]);
}

void free_quotes(char **quotes, int num)
{
	for(; num >= 0; num--)
		free(quotes[num]);	
}

int main(int argc, char **argv)
{
	char *quote_file = "quotes.txt";
	char **quotes;
	int num;
	
	quotes = read_quotes(quote_file, &num);	

	show_quote(quotes, num);	
	
	free_quotes(quotes, num);
	
	return EXIT_SUCCESS;	
}