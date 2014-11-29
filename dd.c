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
int main(int argc, char **argv)
{
	if (argc != 3)
		Say("Wrong argument count");

	int in_fd, out_fd;

	in_fd = open(argv[1], O_RDONLY);
	out_fd = open(argv[2], O_WRONLY | O_CREAT);

	#define CHUNK 512
	char buf[CHUNK] = { 0 };
	
	ssize_t chunk = 0;
	ssize_t bytes = 0;

	struct stat fstats;
	stat(argv[1], &fstats);
	
	int length = fstats.st_size;
	int percent = length / 100;	

	int total = 0;

	do {
		bytes = read(in_fd, buf, CHUNK);
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
