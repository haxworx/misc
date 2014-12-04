// DD WITH PROGRESSION!!!!!!!!!!!

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
	printf("hostname: %s\n", hostname);
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

        int status = connect(sock, (struct sockaddr *) &host_addr, sizeof(struct sockaddr));
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

void DownloadHTTP(int sock, char *addr, char *file)
{
	char out[8192] = { 0 };
	char in[8192] = { 0 };
	sprintf(out, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file, addr);
	write(sock, out, strlen(out));
	read(sock, in, 1024);
	
	char *p = in;
/*HTTP/1.1 200 OK
Date: Thu, 04 Dec 2014 22:15:14 GMT
Last-Modified: Tue, 16 Jul 2013 15:53:43 GMT
Content-Length: 7282
Content-Type: text/plain; charset=utf-8
Server: tachyon
*/
	int len;
	char buf[8192] = { 0 };
	sscanf(in, "Content-Length: %d", &len);
	sscanf(in, "Server: %d\", string");
	
	exit(22);
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
		if (0 == strcmp(argv[i], "-h") || ! strncmp(argv[i], "--h", 3)) 
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
			DownloadHTTP(sock, address, filename);	
		} else
			Scream("MacBorken URL");
	} else {
		in_fd = open(argv[1], O_RDONLY, 0666);
		if (in_fd < 0)
			Scream(strerror(errno));
	}

	out_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (in_fd < 0)
		Scream(strerror(errno));

	char buf[bs];

	memset(buf, 0, bs);
	
	ssize_t chunk = 0;
	ssize_t bytes = 0;

	struct stat fstats;
	stat(argv[1], &fstats);
	
	length = fstats.st_size;
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
