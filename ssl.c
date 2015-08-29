//gcc -std=c99 ssl.c -lssl -lcrypto

/*
	SSL easy peasy, it's C see!
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUF_MAX 8192

#define h_addr h_addr_list[0]

void Error(char *fmt, ...)
{
	char message[BUF_MAX] = { 0 };
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(message, BUF_MAX, fmt, ap);
	fprintf(stderr, "Error: %s\n", message);
	va_end(ap);

	exit(EXIT_FAILURE);
}

void init_ssl(void)
{
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();
}
static int debugging = 0;

int use_https_ssl = 1; // 1 for SSL 0 for plain-text

BIO *bio = NULL;

BIO *Connect_SSL(const char *hostname, unsigned int port)
{
	//BIO *bio = NULL;
	char bio_addr[BUF_MAX] = { 0 };
	
	snprintf(bio_addr, sizeof(bio_addr), "%s:%d", hostname, port);
	
	SSL_library_init();
	
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	SSL *ssl = NULL;
	
	bio = BIO_new_ssl_connect(ctx);
	if (bio == NULL)
	{
		Error("BIO_new_ssl_connect");
	}
	
	BIO_get_ssl(bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname(bio, bio_addr);
	
	if (BIO_do_connect(bio) <= 0)
	{
		Error("SSL Unable to connect");
	}

	return bio;
}

int Connect(const char *hostname, unsigned int port)
{
	if (use_https_ssl)
	{
		Connect_SSL(hostname, port);
		return 1;
	}
	
	int sock;
	struct hostent *host;
	struct sockaddr_in host_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		Error("socket()");
	}

	host = gethostbyname(hostname);
	if (host == NULL)
	{
		Error("gethostbyname()");
	}

	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(port);
	host_addr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&host_addr.sin_zero, 0, 8);

	int status = connect(sock, (struct sockaddr *)&host_addr,
			     sizeof(struct sockaddr));

	if (status == 0)
	{
		return sock;
	}

	return 0;
}

ssize_t Read(int sock, char *buf, int len)
{
	if (use_https_ssl)
	{
		return BIO_read(bio, buf, len);
	}
	
	return read(sock, buf, len);
}

ssize_t Write(int sock, char *buf, int len)
{
	if (debugging)
	{
		printf("%s", buf);
	}

	if (use_https_ssl)
	{
		return BIO_write(bio, buf, len);
	}
	
	return write(sock, buf, len);
}

void Disconnect(void)
{
	BIO_free_all(bio);
	bio = NULL;
}

int Close(int sock)
{
	if (use_https_ssl)
	{
		BIO_free_all(bio); bio = NULL;
		return 0;
	}
	
	return close(sock);
}

#define SSL_RDWR 0

int main(void)
{
	init_ssl();
	
	const char *hostname = "google.com";
	unsigned int port = 443;
	
	use_https_ssl = 1; // SSL! 
	
	Connect(hostname, port);
	
	char buf[1024] = { 0 };
	char* msg = "GET / HTTP/1.1\r\nHost:google.com\r\n\r\n";

	ssize_t len = Write(SSL_RDWR, msg, strlen(msg));

	len = Read(SSL_RDWR, buf, sizeof(buf));
	buf[len] = 0;

	// RESPONSE 
	printf("%s\n", buf);
	
	Disconnect();

	return EXIT_SUCCESS;
}
