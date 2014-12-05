/*
	This program is distributed as public domain.
	Author: Al Poole <netstar@gmail.com>
	
	About:
	
	Alternative to "dd".
	Can do local: 
		this if=file.iso of=/dev/sdb
		also!
		this if=http://somesite.com/music.iso of=/dev/sdc
	
	Unbuffered read/write.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

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


int Connect(char *hostname, int port)
{
	int sock;
	struct hostent *host;
	struct sockaddr_in host_addr;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		Scream("socket %s\n", strerror(errno));

	host = gethostbyname(hostname);
	if (host == NULL)
		Scream("gethostbyname");

	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(port);
	host_addr.sin_addr = *((struct in_addr *) host->h_addr);
	memset(&host_addr.sin_zero, 0, 8);

	int status =
	    connect(sock, (struct sockaddr *) &host_addr,
		    sizeof(struct sockaddr));
	if (status == 0) {
		return sock;
	}

	return 0;
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

char *FileFromURL(char *addr)
{
	char *str = NULL;

	char *p = addr;
	if (!p)
		Scream("broken file path");

	str = strstr(addr, "http://");
	if (str) {
		str += strlen("http://");
		char *p = strchr(str, '/');
		if (p) {
			return p;
		}
	}

	if (!p)
		Scream("FileFromURL");

	return p;
}

char *HostFromURL(char *addr)
{
	char *str = strstr(addr, "http://");
	if (str) {
		addr += strlen("http://");
		char *end = strchr(addr, '/');
		*end = '\0';
		return addr;
	}

	Scream("Invalid URL");

	return NULL;
}

#define BLOCK 1024
void Chomp(char *str)
{
	char *p = str;

	while (*p) {
		if (*p == '\r' || *p == '\n') {
			*p = '\0';
		}
		++p;
	}
}

typedef struct header_t header_t;
struct header_t {
	char location[1024];
	char content_type[1024];
	int content_length;
	char date[1024];
	int status;
};

ssize_t ReadHeader(int sock, header_t * headers)
{
	int bytes = -1;
	int len = 0;
	char total[8192] = { 0 };
	char buf[8192] = { 0 };
	while (1) {
		while (buf[len - 1] != '\r' && buf[len] != '\n') {
			bytes = read(sock, &buf[len], 1);
			len += bytes;

		}
		buf[len] = '\0';
		len = 0;

		sscanf(buf, "\nHTTP/1.1 %d", &headers->status);
		sscanf(buf, "\nContent-Type: %s\r", headers->content_type);
		sscanf(buf, "\nLocation: %s\r", headers->location);
		sscanf(buf, "\nContent-Length: %d\r",
		       &headers->content_length);


		if (headers->content_length && strlen(buf) == 2) {
			return 1;								  // found!!
		}

		memset(buf, 0, 8192);
	}
	return 0;										  // not found
}

int Headers(int sock, char *addr, char *file)
{
	char out[8192] = { 0 };
	char buf[8192] = { 0 };
	header_t headers;

	memset(&headers, 0, sizeof(header_t));

	sprintf(out, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file, addr);
	write(sock, out, strlen(out));

	ssize_t len = 0;

	do {
		len = ReadHeader(sock, &headers);
	} while (!len);

	if (!headers.content_length)
		Scream("bad headers!");

	return headers.content_length;
}

int main(int argc, char **argv)
{
	int i;
	unsigned long bs = CHUNK;
	char *infile = argv[1];
	int get_from_web = 0;

	for (i = 0; i < argc; i++) {
		if (0 == strcmp(argv[i], "-bs")) {
			if (argv[i + 1] != NULL)
				bs = atoi(argv[i + 1]);
			i++;
		}
		if (0 == strcmp(argv[i], "-h")
		    || !strncmp(argv[i], "--h", 3))
			Usage();
	}

	if (argc < 3)
		Usage();

	if (!strncmp("http://", infile, 7))
		get_from_web = 1;

	int in_fd, out_fd, sock;
	int length = 0;

	if (get_from_web) {
		char *filename = strdup(FileFromURL(infile));
		char *address = strdup(HostFromURL(infile));
		if (filename && address) {
			sock = in_fd = Connect(address, 80);
			length = Headers(sock, address, filename);
		} else
			Scream("MacBorken URL");
	} else {
		in_fd = open(argv[1], O_RDONLY, 0666);
		if (in_fd < 0)
			Scream(strerror(errno));
	}

	out_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (out_fd < 0)
		Scream(strerror(errno));

	char buf[bs];

	memset(buf, 0, bs);

	ssize_t chunk = 0;
	ssize_t bytes = 0;

	struct stat fstats;
	stat(argv[1], &fstats);
	if (!length)
		length = fstats.st_size;
	int percent = length / 100;

	int total = 0;
	read(in_fd, buf, 1);									  // hack

	do {
		bytes = read(in_fd, buf, bs);
		if (bytes <= 0)
			break;

		chunk = bytes;

		while (chunk) {
			ssize_t count = write(out_fd, buf, chunk);
			if (count <= 0)
				break;

			chunk -= count;
		}

		total += bytes;

		int current = total / percent;
		if (current > 100)
			current = 100;
		printf
		    ("                                                    \r");
		printf("%d%% %d bytes of %d bytes", current, total,
		       length);
		memset(buf, 0, bytes);								  // faster
	} while (length > total);

	printf("\r\ndone!\n");

	close(out_fd);
	close(in_fd);

	return EXIT_SUCCESS;
}
