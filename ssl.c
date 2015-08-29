/* 
   Copyright (c) 2015, Al Poole <netstar@gmail.com> All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 2. Redistributions in 
   binary form must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other
   materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

 */

/*
	SSL easy peasy, it's C see!
	
	gcc -std=c99 ssl.c -lssl -lcrypto
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
		return NULL;
	}
	
	BIO_get_ssl((BIO *) bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname((BIO *) bio, bio_addr);
	
	if (BIO_do_connect((BIO *) bio) <= 0)
	{
		return NULL;
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
	
	char buf[BUF_MAX] = { 0 };
	char* msg = "GET / HTTP/1.1\r\nHost: google.com\r\n\r\n";
	
	SSL *ssl = Connect_SSL(hostname, port);
	if (ssl == NULL)
	{
		Error("We could not connect!");
	}

	ssize_t len = Write_SSL(ssl, msg, strlen(msg));

	len = Read_SSL(ssl, buf, sizeof(buf));
	buf[len] = 0;

	// RESPONSE 
	printf("%s\n", buf);
	
	Disconnect_SSL(ssl);

	return EXIT_SUCCESS;
}
