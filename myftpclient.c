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
#include <malloc.h>
#include <dirent.h>
#include "myftp.h"

#include <errno.h>
#include <netdb.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
 
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#define RSA_CLIENT_CA_CERT      "cacert.pem"
 
#define ON      1
#define OFF     0
void tranp_file_data(ssl_st* accept_fd, char filename[], char path[]){
	// printf("can i reach here\n");
	char filepath[100]="";
	strcat(filepath,path);
	strcat(filepath,filename);
		// printf("can i reach here\n");
	FILE * file=NULL;
	// printf("can i reach here\n");
	if((file=fopen(filepath,"rb"))<0){
		perror("fail to open the file\n");
	}
	// printf("can i reach here\n");
	unsigned int filelength=0;
	char c;
	c=fgetc(file);
	while(!feof(file)){
		filelength+=1;
		// printf("char is %X\n",c);
		c=fgetc(file);
	}
	struct message_s header;
	int len;
	char payload[PAYLEN];
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';
	header.type=0xFF;
	header.length=filelength+10;

	header.length = htonl(header.length);

	printf("file length is %d\n",filelength);
	fclose(file);

	if((file=fopen(filepath,"rb"))<0){
		perror("fail to open the file\n");
	}
	if((len=SSL_write(accept_fd,&header,sizeof(header)))<0){
		perror("cannot send header to client\n");
	}
	 // char character[2]="";
	memset(payload,0,PAYLEN);
	int if_finish=0;
	unsigned int file_length=0;

	header.length = ntohl(header.length);

	unsigned int i;
	for(i=0;i<=(header.length-10-1)/(sizeof(payload)-1);i++){
		unsigned int j;
		for(j=0;j<sizeof(payload)-1;j++){
			// memset(character,0,sizeof(character));
			// character[0]=fgetc(file);
			payload[j]=fgetc(file);
			//printf("char: %c",payload[j]);
			if(!feof(file)){
				// printf("can i reach here,%d %d %s\n",j,strlen(payload), character);
				// printf("char is %X\n",character[0]);
				// strcat(payload,character);
				file_length+=1;
				// printf("can i reach there,\n");
			}
			else{
				if((len=SSL_write(accept_fd,payload,sizeof(payload)-1))<0){
					perror("can not send payload to client\n");
				}
				memset(payload,0,PAYLEN);
				if_finish=1;
				break;
			}
		}
		/*
		int k;
		for(k=0;k<sizeof(payload)-1;k++){
			printf("i : %dth, %X\n",k, payload[k] );
		}
		exit(1);
		*/
		if(if_finish==0){
			while((len=SSL_write(accept_fd,payload,sizeof(payload)-1))<0){
				perror("can not send payload to client\n");

			}

		}

		memset(payload,0,PAYLEN);

	}
	printf("Sending finished\n");
	
	fclose(file);

}

void recv_file_data(ssl_st* fd, char filename[], char path[]){
	int len;
	struct message_s header;
	char payload[PAYLEN];
	if((len=SSL_read(fd,&header,sizeof(header)))<0){
		perror("cannot recv the header from server\n");
	}

	header.length = ntohl(header.length);

	printf("length: %d\n",header.length);
	if(header.type!=0xFF){
		
		perror("type is wrong \n");

	}
	FILE * downfile=NULL;
	char filepath[100]="";
	strcat(filepath,path);
	strcat(filepath,filename);
	//printf("cat the filename\n");
	if((downfile=fopen(filepath,"wb"))==NULL){
		printf("open file: %s\n",filepath);
	}
	unsigned int file_length=0;
	int i;
	// for( i=0; i<=(header.length-10-1)/(sizeof(payload)-1);i++){
	// 	usleep(1);
	// 	printf("i: %d\n",i);
	// 	if((len=recv(fd,payload,sizeof(payload),0))<0){
	// 		perror("can not recv the payload from server\n");
	// 	}
	// 	/*
	// 	for (int j=0; j<sizeof(payload);j++){
	// 		// printf("payload: %di, %X\n",j,payload[j]);
	// 	}
	// 	*/
	// 	//printf("i: %d %d\n",i,(header.length-10-1)/(sizeof(payload)-1));
	// 	if (i!=(header.length-10-1)/(sizeof(payload)-1)){
	// 		//printf("not last step i: %d\n",i);
	// 		if(fwrite(payload,sizeof(payload)-1,1, downfile)<0)
	// 		{
	// 			perror("can not write the payload into file\n");
	// 		}
	// 		file_length+=sizeof(payload)-1;
			
	// 	}
	// 	else{
					
	// 		if ((header.length-10)%(sizeof(payload)-1)!=0)
	// 		{
	// 			printf("last step i: %d %d\n",i, (header.length-10)%(sizeof(payload)-1));
	// 			if(fwrite(payload,(header.length-10)%(sizeof(payload)-1),1, downfile)<0)
	// 			{
	// 				perror("can not write the payload into file\n");
	// 			}
	// 			file_length+=(header.length-10)%(sizeof(payload)-1);
	// 		}
	// 		else{
	// 			printf("last step i: %d %d\n",i, (sizeof(payload)-1));
	// 			if(fwrite(payload,(sizeof(payload)-1),1, downfile)<0)
	// 			{
	// 				perror("can not write the payload into file\n");
	// 			}
	// 			file_length+=(sizeof(payload)-1);

	// 		}
	// 	}

	
	// 	memset(payload,0,PAYLEN);
	// }
	while(file_length<header.length-10){
		//printf("file_length: %d, true length: %d\n", file_length, header.length-10 );
		memset(payload,0, PAYLEN);
		//usleep(1);
		if((len=SSL_read(fd, payload,sizeof(payload)-1))<0){
			printf("download file error!\n");
			// close(downfile);
		}
		if (file_length+len<header.length-10){
			if((fwrite(payload,len,1,downfile))<0){
				printf("wrong to write file!\n");
			}
			file_length+=len;
		}
		else{
			if((fwrite(payload,header.length-10-file_length,1,downfile))<0){
				printf("wrong to write file!\n");
			}
			file_length+=header.length-10-file_length;
		}
		// file_length+=len;
	}
	printf("Receiving finished\n");
	fflush(downfile);
	fclose(downfile);
}

void list_file(SSL* fd, struct message_s header){
	// memset(command,0,5);
	int len;
	int i;
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xA1;
	// buf.type=0xA1;
	header.length=sizeof(header);

	header.length = htonl(header.length);
	// buf=htonl(buf);
	if((len=(SSL_write(fd,&header,sizeof(header))))<0){
		perror("can not send request for list\n");
	}
	// printf("send the command to server\n");
	struct message_s message_from_server;
	// printf("size : %d %d\n",sizeof(message_from_server),sizeof(message_from_server.payload));
	if((len=(SSL_read(fd,&message_from_server,sizeof(message_from_server))))<0){
		perror("can not recv the message from server\n");
	}
	// printf("recv the message\n");
	else{
		char payload[PAYLEN];
		message_from_server.length = ntohl(message_from_server.length);
		//printf("size:%d %d \n",message_from_server.length,(int)(message_from_server.length-10-1)/sizeof(payload));
		for( i=0;i<=(int)(message_from_server.length-10-1)/sizeof(payload);i++){
			//printf("the %dth\n",i);
			if((len=(SSL_read(fd,payload,sizeof(payload))))<0){
				perror("can not receive the file list from server\n");
			}
			else{
				printf("%s\n",payload);
			}
		}
	}

	printf("Done\n");
}

void get_file(SSL* fd,   char * command){
	struct message_s header;
	int i;
	int len;
	if(command[3]!=' '){
		printf("c:%c\n%s",command[3],command);
		perror("wrong format\n");
		exit(1);
	}
	for (i=3;i<strlen(command);i++){
		if(command[i]!=' '){
			break;
		}
	}
	const char *tmp=&command[i];
	char filename[32];
	char payload[PAYLEN]="";
	strcpy(filename,tmp);
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xB1;
	
	strcpy(payload,filename);
	header.length=10+strlen(filename);

	header.length = htonl(header.length);

	if((len=SSL_write(fd,&header,sizeof(header)))<0){
		perror("can not send request for file\n");
	}
	else{
		// printf("Done to send request for file\n");

		// for(int i=0;i<=((header.length-10-1)/sizeof(payload));i++){
		// 	printf("%dth iteration\n",i);
			if((len=SSL_write(fd,payload,sizeof(payload)))<0){
				perror("can not send the payload to server\n");
			}
		// }
		struct message_s result_of_get;
		if((len=(SSL_read(fd,&result_of_get,sizeof(result_of_get))))<0){
			perror("cannot receive message from server\n");

		}
		if(result_of_get.type==0xB2){
			printf("successfully find the file\n");
			recv_file_data(fd, filename,"");
		}
		else{
			printf("fail to find the file\n");
		}
	}

}

void put_file(SSL* fd, char command[]){
	struct message_s header;
	int i;
	int len;
	if(command[3]!=' '){
		printf("c:%s\n",command[3]);
		perror("wrong format\n");
		exit(1);
	}
	for (i=3;i<strlen(command);i++){
		if(command[i]!=' '){
			break;
		}
	}
	const char *tmp=&command[i];
	char filename[32];
	char payload[PAYLEN]="";
	int f_exist=0;
	strcpy(filename,tmp);
	strcpy(payload,filename);
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xC1;
	
	header.length = 10 + strlen(filename);
	header.length = htonl(header.length);

	DIR * dir;
	struct dirent * ptr;
	if ((dir=opendir("."))==NULL){
		perror("can not find the responding data\n");
	}
	while((ptr=readdir(dir))!=NULL){
		if(strcmp(payload,ptr->d_name)==0){
			f_exist=1;
			break;
		}
	}
	if(f_exist==0){
		// printf("can not find the file locally\n" );
		perror("can not find the file locally\n");
		exit(1);
	}
	else{
		printf("find the file\n");
		if((len=(SSL_write(fd,&header,sizeof(header))))<0){
			perror("can not send request to client");
		}
		//printf("send the header\n");
		if((len=(SSL_write(fd,payload,sizeof(payload))))<0){
			perror("can not send request to client");
		}
		//printf("send the file name\n");
		struct message_s message_from_server;
		if((len=(SSL_read(fd,&message_from_server,sizeof(message_from_server))))<0){
			perror("can not recv request to client");
		}
		if(message_from_server.type==0xC2){
				tranp_file_data(fd,filename,"");
			}
	}
}
void main_task(in_addr_t ip, unsigned short port)
{
	int     err;
	int     verify_client = OFF; /* To verify a client certificate, set ON */
	int     sock;
	struct sockaddr_in server_addr;
	char  *str;
	char    buf [4096];
	// char    hello[80];

	SSL_CTX         *ctx;
	SSL             *ssl;
	SSL_METHOD      *meth;
	X509            *server_cert;

	short int       s_port = 5555;
	const char      *s_ipaddr = "127.0.0.1";

	/*----------------------------------------------------------*/
	// printf ("Message to be sent to the SSL server: ");
	// fgets (hello, 80, stdin);

	/* Register all algorithm */
	OpenSSL_add_all_algorithms();

	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Create an SSL_METHOD structure (choose an SSL/TLS protocol version) */
	meth = (SSL_METHOD*)SSLv23_method();

	/* Create an SSL_CTX structure */
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		fprintf(stderr, "ERR: Unable to create ctx\n");
		exit(-1);
	}

	/*----------------------------------------------------------*/
	if(verify_client == ON) {
		/* Load the client certificate into the SSL_CTX structure */
		if (SSL_CTX_use_certificate_file(ctx, "client.crt", 
					SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}

		/* Load the private-key corresponding to the client certificate */
		if (SSL_CTX_use_PrivateKey_file(ctx, "client.key", 
					SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}

		/* Check if the client certificate and private-key matches */
		if (!SSL_CTX_check_private_key(ctx)) {
			fprintf(stderr, 
					"Private key does not match the certificate public key\n");
			exit(1);
		}
	}

	/* Load the RSA CA certificate into the SSL_CTX structure */
	/* This will allow this client to verify the server's     */
	/* certificate.                                           */

	if (!SSL_CTX_load_verify_locations(ctx, RSA_CLIENT_CA_CERT, NULL)) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* Set flag in context to require peer (server) certificate */
	/* verification */

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	SSL_CTX_set_verify_depth(ctx, 1);

	/* ------------------------------------------------------------- */
	/* Set up a TCP socket */

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);       
	if (sock == -1) {
		perror("socket");
		exit(-1);
	}

	memset (&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons(port);       /* Server Port number */
	server_addr.sin_addr.s_addr = ip; /* Server IP */
	
	/* Establish a TCP/IP connection to the SSL client */
	err = connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)); 
	if (err == -1) {
		perror("connect");
		exit(-1);
	}
	
	/* ----------------------------------------------- */
	/* An SSL structure is created */

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "ERR: cannot create ssl structure\n");
		exit(-1);
	}

	/* Assign the socket into the SSL structure */
	SSL_set_fd(ssl, sock);

	/* Perform SSL Handshake on the SSL client */
	printf("SSL handshake starts\n");
	err = SSL_connect(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Informational output (optional) */
	// printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

	/* Get the server's certificate */
	server_cert = SSL_get_peer_certificate(ssl);    
	if (server_cert != NULL) {
		printf ("Server certificate:\n");

		str = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
		if (str == NULL) {
			exit(-1);
		}
		printf ("\t subject: %s\n", str);
		free (str);

		str = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
		if (str == NULL) {
			exit(-1);
		}
		printf ("\t issuer: %s\n", str);
		free(str);

		X509_free (server_cert);
	} else {
		printf("The SSL server does not have certificate.\n");
	}


	struct message_s header;
	// struct all_data buf;
	// int fd;
	char command[100];
	memset(command,0,100);
	struct sockaddr_in addr;
	unsigned int addrlen=sizeof (struct sockaddr_in);
	int len;
	// fd=socket(AF_INET, SOCK_STREAM,0);
	// if(fd== -1){
	// 	perror("socket()");
	// 	exit(1);
	// }
	// scanf("%s", command);
	fgets(command,100,stdin);
	// printf("command: %s",command);
	command[strlen(command)-1]='\0';
	// memset(&addr, 0, sizeof(struct sockaddr_in));
	// addr.sin_family=AF_INET;
	// addr.sin_addr.s_addr=ip;
	// addr.sin_port=htons(port);
	// if(connect(fd, (struct sockaddr *) &addr, addrlen)==-1){
	// 	perror("connect()");
	// 	exit(1);
	// }
	while(strcmp(command,"list")!=0 && strncmp(command,"get",3)!=0&& strncmp(command,"put",3)!=0){
		printf("command: (1) list, (2) get filename, or (3) put filename\n");
		// scanf("%s", command);
		// gets(command);
		fgets(command,100,stdin);
		command[strlen(command)-1]='\0';
		// printf("command: %s\n",command);
		// scanf("%s", command);
	}
	// printf("what is wrong\n");
	if (strcmp(command,"list")==0){
		// printf("what is wrong\n");
		list_file(ssl,header);
	}
	if (strncmp(command,"get",3)==0){
		get_file(ssl,command);
	}
	if (strncmp(command,"put",3)==0){
		put_file(ssl,command);
		// printf("Sent nothing\n");
	}

	
	err = SSL_shutdown(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	/* Terminate communication on a socket */
	err = close(sock);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Free the SSL structure */
	SSL_free(ssl);

	/* Free the SSL_CTX structure */
	SSL_CTX_free(ctx);

	
}
int main(int argc, char **argv){
	in_addr_t ip;
	unsigned short port;
	if (argc !=3 ){
		fprintf(stderr,"Usage: %s [IP address] [port]\n", argv[0]);
		exit(1);
	}
	if ((ip=inet_addr(argv[1]))==-1){
		perror("inet_addr()");
		exit(1);
	}
	port=atoi(argv[2]);
	main_task(ip,port);
	return 0;
}
