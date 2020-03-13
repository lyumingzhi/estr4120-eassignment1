#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>	// "struct sockaddr_in"
#include <arpa/inet.h>	// "in_addr_t"
#include <dirent.h>
#include <malloc.h>
#include <pthread.h>

#include <errno.h>
#include <netdb.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
// #include "myftp.h"
// #ifndef MYFTP_H

// #define MYFTP_H
#define PAYLEN 1025

struct message_s{
	unsigned char protocol[5];
	unsigned char type;
	unsigned int length;

}__attribute__((packed));


// void tranp_file_data(ssl_st* accept_fd, char filename[], char path[]);
// void recv_file_data(ssl_st* fd, char filename[],char path[]);
// #endif MYFTP_H
