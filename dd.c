// DD WITH PROGRESSION!!!!!!!!!!!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void Say(char *phrase)
{
	char buf[1024] = { 0 };

	sprintf(buf, "%s\n", phrase);
	printf(buf);

	exit(1);
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
	}

	printf("bs is %d\n", bs);
	if (argc < 3)
		Say("Wrong argument count");

	int in_fd, out_fd;

	in_fd = open(argv[1], O_RDONLY);
	if (in_fd < 0)
		Say("Whoaaaaaa!");

	out_fd = open(argv[2], O_WRONLY | O_CREAT);
	if (in_fd < 0)
		Say("Whoaaaa!!!");

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
		printf("\r");
		printf("%d percent", current);
		memset(buf, 0, bytes); // faster
	} while (bytes);

	printf("\r\ndone!\n");
	
	close(out_fd);
	close(in_fd);

	return EXIT_SUCCESS;
}
