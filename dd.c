// DD WITH PROGRESSION!!!!!!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void Scream(char *fmt, ...)
{
	char buf[8192] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	sprintf(buf, "Nooo! %s\n", fmt);
	va_end(ap);
	fprintf(stderr, buf);

	exit(EXIT_FAILURE);
} 

void Say(char *phrase)
{
	char buf[1024] = { 0 };

	sprintf(buf, "%s\n", phrase);
	printf(buf);
}

void Usage(void)
{
	printf("ARGV[0] <from> <to> [OPTION]\n");
	printf("OPTIONS:\n");
	printf("    -bs <block size>\n");
	printf("    -h  help.\n");

	exit(EXIT_FAILURE);
}

	
#define CHUNK 512

int main(int argc, char **argv)
{
	int i;
	unsigned long bs = CHUNK;

	for (i = 0; i < argc; i++) {
		if (0 == strcmp(argv[i], "-bs")) {
			if (argv[i + 1] != NULL)
				bs = atoi(argv[i + 1]);
				i++;
		}
		if (0 == strcmp(argv[i], "-h") || ! strncmp(argv[i], "--h", 3)) 
			Usage();
	}

	if (argc < 3)
		Usage();

	int in_fd, out_fd;

	in_fd = open(argv[1], O_RDONLY, 0666);
	if (in_fd < 0)
		Scream(strerror(errno));

	out_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (in_fd < 0)
		Scream(strerror(errno));

	char buf[bs];

	memset(buf, 0, bs);
	
	ssize_t chunk = 0;
	ssize_t bytes = 0;

	struct stat fstats;
	stat(argv[1], &fstats);
	
	int length = fstats.st_size;
	int percent = length / 100;	

	int total = 0;

	do {
		bytes = read(in_fd, buf, bs);
		if (bytes < 0)
			break;

		chunk = bytes;
		
		while (chunk) {
			ssize_t count = write(out_fd, buf, chunk);
			if (count < 0)
				break;

			chunk -= count;
		}

		total += bytes;
		
		int current = total  / percent;
		printf("                                                    \r");
		printf("%d%% %dK of %dK", current, total >> 8, length >> 8);
		memset(buf, 0, bytes); // faster
	} while (bytes);

	printf("\r\ndone!\n");
	
	close(out_fd);
	close(in_fd);

	return EXIT_SUCCESS;
}
