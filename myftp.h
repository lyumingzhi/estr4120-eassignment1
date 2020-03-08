#ifndef MYFTP_H
#define MYFTP_H
#define PAYLEN 87
#include <openssl/bn.h>
#include <openssl/rsa.h>
struct message_s{
	unsigned char protocol[5];
	unsigned char type;
	unsigned int length;

}__attribute__((packed));

struct encrypt_ftp{
	unsigned int length;
	unsigned char* e_data;
}__attribute__((packed));

struct thread_input{
	int fd;
	RSA* rsa_server;
}__attribute__((packed));

void tranp_file_data(int accept_fd, char filename[], char path[],RSA* rsa);
void recv_file_data(int fd, char filename[],char path[],RSA* rsa);
int er_data(int fd,char *output_string, int data_size,RSA* rsa);
int es_data(int fd,char entire_input_string[], int data_size,RSA* rsa);
// extern int PAYLEN;
void send_public_key(RSA * rsa, int fd);
void receive_public_key(RSA* encrypt_rsa,int fd);
#endif MYFTP_H
