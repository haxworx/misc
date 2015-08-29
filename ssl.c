//gcc -std=c99 ssl.c -lssl -lcrypto

/*
	SSL easy peasy, it's C see!
*/

#include <stdio.h>
#include <stdarg.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUF_MAX 8192

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

int use_https_ssl = 1; // 1 for SSL 0 for plain-text

SSL *Connect_SSL(const char *hostname, unsigned int port)
{
	SSL *bio = NULL;
	char bio_addr[BUF_MAX] = { 0 };
	
	snprintf(bio_addr, sizeof(bio_addr), "%s:%d", hostname, port);
	
	SSL_library_init();
	
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
	SSL *ssl = NULL;
	
	bio = (SSL *) BIO_new_ssl_connect(ctx);
	if (bio == NULL)
	{
		Error("BIO_new_ssl_connect");
	}
	
	BIO_get_ssl((BIO *) bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname((BIO *) bio, bio_addr);
	
	if (BIO_do_connect((BIO *) bio) <= 0)
	{
		Error("SSL Unable to connect");
	}

	return bio;
}

ssize_t Read_SSL(SSL *ssl, char *buf, int len)
{
	return BIO_read((BIO *) ssl, buf, len);
}

ssize_t Write_SSL(SSL *ssl, char *buf, int len)
{
	return BIO_write((BIO *) ssl, buf, len);
}

void Disconnect_SSL(SSL *ssl)
{
	BIO_free_all((BIO *) ssl);
	ssl = NULL;
}

int main(void)
{
	init_ssl();
	
	const char *hostname = "google.com";
	unsigned int port = 443;
	
	SSL *ssl = Connect_SSL(hostname, port);
	
	char buf[1024] = { 0 };
	char* msg = "GET / HTTP/1.1\r\nHost:google.com\r\n\r\n";

	ssize_t len = Write_SSL(ssl, msg, strlen(msg));
	len = Read_SSL(ssl, buf, sizeof(buf));
	
	buf[len] = 0;

	// RESPONSE 
	printf("%s\n", buf);
	
	Disconnect_SSL(ssl);

	return EXIT_SUCCESS;
}
