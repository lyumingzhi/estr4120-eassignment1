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
#include "myftp.h"
#define RSA_SERVER_CERT     "cert.pem"
#define RSA_SERVER_KEY      "key.pem"

#define ON         1
#define OFF        0

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0;
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

void list_reply(SSL* accept_fd){
	char payload[PAYLEN];
	memset(payload,0,PAYLEN);
	int len;
	DIR * dir;
	struct dirent * ptr;
	struct message_s header;
	int fn_len=1;
	header.length=0;
	// printf("i am in\n");
	if ((dir=opendir("data"))==NULL){
		perror("can not find the responding data");
	}
	while((ptr=readdir(dir))!=NULL){
		if(strlen(payload)+strlen(ptr->d_name)<=PAYLEN-1){
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			//printf("name: %s\n",ptr->d_name);
		}
		else{
			fn_len+=1;
			memset(payload,0,PAYLEN);
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			//printf("name: %s\n",ptr->d_name);
			
		}
	}
	// if(fn_len==0){
	// 	fn_len=1;
	// }
 
	//printf("what is wrong\n");
	// printf("the file list is: %s",filelist);
	closedir(dir);

	header.type=0xA2;
	// char tmpmessage[]="myftp";
	// header.protocol=(unsigned char *)tmpmessage ;
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.length=(strlen(payload)+10);

	header.length = htonl(header.length);


	// printf("header: %d\n",header.length);
	memset(payload,0,PAYLEN);
	//strcpy(message_to_client.payload,filelist);
	if((len=(SSL_write(accept_fd,&header, sizeof(header))))<0){
		perror("can not send request to client");
	}
	else{
		if ((dir=opendir("data"))==NULL){
			perror("can not find the responding data");
		}
		while((ptr=readdir(dir))!=NULL){
		if(strlen(payload)+strlen(ptr->d_name)<=PAYLEN-1){
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			//printf("name: %s\n",payload);
		}
		else{
			//printf("name: %s\n",payload);
			if((len=(SSL_write(accept_fd,payload,sizeof(payload))))<0){
				perror("can not send the file name to client\n");
			}
			memset(payload,0,PAYLEN);
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			//printf("name: %s\n",ptr->d_name);
			
		}
		
	}
	if(fn_len==1){
			if((len=(SSL_write(accept_fd,payload,sizeof(payload))))<0){
				perror("can not send the file name to client\n");
			}
		}
	}
	closedir(dir);
	//printf("Sending finished\n");
	// free(message_to_client.payload);
}


void reply_request_file(SSL* accept_fd, struct message_s buf, char payload[]){
	DIR * dir;
	struct dirent * ptr;
	struct message_s get_reply;
	int f_exist=0;
	int len;
	if ((dir=opendir("data"))==NULL){
		perror("can not find the responding data");
	}
	while((ptr=readdir(dir))!=NULL){
		// printf("file name:%s\n", buf.payload);
		if(strcmp(payload,ptr->d_name)==0){
			f_exist=1;
			get_reply.type=0xB2;
			get_reply.protocol[0]='m';
			get_reply.protocol[1]='y';
			get_reply.protocol[2]='f';
			get_reply.protocol[3]='t';
			get_reply.protocol[4]='p';
			get_reply.length=5+1+4;
			printf("find the file\n");
			break;
		}
	}

	get_reply.length = htonl(get_reply.length);

	if(f_exist==0){
		get_reply.type=0xB3;
		printf("cannot find the file\n");
	}
	if((len=(SSL_write(accept_fd,&get_reply,sizeof(get_reply))))<0){
		perror("can not send request to client");
	}
	if(f_exist==1){
		char path[]="data/";
		tranp_file_data( accept_fd, payload,path);
	}
	//printf("Sending finished\n"); 
}

void put_recv_file(SSL* accept_fd){
	struct message_s header;
	int len;
	char payload[PAYLEN];
	char filename[PAYLEN];
	if((len=SSL_read(accept_fd,payload,sizeof(payload)))<0){
		perror("cannot send header to client\n");
	}
	strcpy(filename,payload);

	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';
	header.type=0xC2;
	header.length=10;

	header.length = htonl(header.length);

	if((len=SSL_write(accept_fd,&header,sizeof(header)))<0){
		perror("cannot send header to client\n");
	}
	char path[]="data/";
	recv_file_data(accept_fd,filename,path);

}

void *pthread_loop(void* sDescriptor){
	SSL * accept_fd =(SSL*) sDescriptor;
	int err;
	int len;
	struct message_s buf;
	 
		if ((len = (SSL_read(accept_fd,&buf,sizeof(buf))))<0){
			perror("can not recv the commend");
			pthread_mutex_lock(&mutex);
				client_count--;
			pthread_mutex_unlock(&mutex);
		}
		
		if(buf.type==0xA1){
			list_reply(accept_fd);
		}
		if(buf.type==0xB1){
			char payload[PAYLEN];
			if ((len=(SSL_read(accept_fd,payload,sizeof(payload))))<0){
				perror("can not recv the commend\n");
				pthread_mutex_lock(&mutex);
					client_count--;
				pthread_mutex_unlock(&mutex);
			}
			reply_request_file( accept_fd, buf,payload);
		}
		if(buf.type==0xC1){
			put_recv_file(accept_fd);
		}

	err = SSL_shutdown(accept_fd);
	if (err == -1) {
		ERR_print_errors_fp(stderr); 
		exit(1);
	}
	pthread_mutex_lock(&mutex);
		client_count--;
	pthread_mutex_unlock(&mutex);
	//printf("%d\n",client_count);
	return NULL;
}

void main_loop(unsigned short port){
	int     err;
	int     verify_client = OFF; /* To verify a client certificate, set ON */
	int     listen_sock;
	int     sock;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	socklen_t client_len;
	char    *str;
	char     buf[4096];

	SSL_CTX         *ctx;
	SSL            *ssl;
	SSL_METHOD      *meth;
	X509            *client_cert = NULL;

	short int       s_port = 5555;
	
	/*----------------------------------------------------------------*/
	/* Register all algorithms */
	OpenSSL_add_all_algorithms();
	// OpenSSL_add_ssl_algorithms();
	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth = (SSL_METHOD*)SSLv23_method();

	/* Create a SSL_CTX structure */
	// printf("can i create\n");
	ctx = SSL_CTX_new(meth);
		// printf("can i create\n");

	if (!ctx) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	// printf("can i create\n");

	/* Load the server certificate into the SSL_CTX structure */
	if (SSL_CTX_use_certificate_file(ctx, RSA_SERVER_CERT, SSL_FILETYPE_PEM)
			<= 0) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* set password for the private key file. Use this statement carefully */
	SSL_CTX_set_default_passwd_cb_userdata(ctx, (char*)"4430");

	/* Load the private-key corresponding to the server certificate */
	if (SSL_CTX_use_PrivateKey_file(ctx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <=
			0) { 
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* Check if the server certificate and private-key matches */
	if (!SSL_CTX_check_private_key(ctx)) {
		fprintf(stderr,
				"Private key does not match the certificate public key\n");
		exit(1);
	}

	if(verify_client == ON) {
		/* Load the RSA CA certificate into the SSL_CTX structure */
		if (!SSL_CTX_load_verify_locations(ctx, "server_ca.crt", NULL)) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}
		/* Set to require peer (client) certificate verification */
		SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);

		/* Set the verification depth to 1 */
		SSL_CTX_set_verify_depth(ctx,1);
	}
	
	/* ----------------------------------------------- */
	/* Set up a TCP socket */

	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);   
	if (listen_sock == -1) {
		perror("socket");
		exit(-1);
	}
	memset (&sa_serv, 0, sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (port);          /* Server Port number */

	err = bind(listen_sock, (struct sockaddr*)&sa_serv,sizeof(sa_serv));
	if (err == -1) {
		perror("bind");
		exit(-1);
	}

	/* Wait for an incoming TCP connection. */
	err = listen(listen_sock, 5);
	if (err == -1) {
		perror("listen");
		exit(-1);
	}


	// int fd, accept_fd, count;
	// struct sockaddr_in addr;
	// struct sockaddr_in tmp_addr;
	// unsigned int addrlen= sizeof(struct sockaddr_in);
	// fd=socket(AF_INET, SOCK_STREAM,0);
	// if (fd==-1){
	// 	perror("socket()");
	// 	exit(1);
	// }

	// memset(&addr, 0, sizeof(struct sockaddr_in));
	// addr.sin_family= AF_INET;
	// addr.sin_addr.s_addr=htonl(INADDR_ANY);
	// addr.sin_port=htons(port);

	// if(bind(fd, (struct sockaddr *) &addr, sizeof(addr))==-1)
	// {
	// 	perror("bind()");
	// 	exit(-1);
	// }
	// if (listen(fd, 1024)==-1){
	// 	perror("listen()");
	// 	exit(1);
	// }
	// printf("[To stop the server: press Ctrl + C]\n");
	
	pthread_t thr;
	while(1){
		
		if(client_count<11){
	
			/* Socket for a TCP/IP connection is created */
			printf("Accepting connection...\n");

			client_len = sizeof(struct sockaddr_in);
			sock = accept(listen_sock, (struct sockaddr*)&sa_cli, &client_len);
			if (sock == -1) {
				perror("accept");
				exit(-1);
			}

			// terminating the listen socket
			// close (listen_sock);
			// printf ("Connection from %lx, port %x\n", 
			// 		(unsigned long) sa_cli.sin_addr.s_addr, 
			// 		sa_cli.sin_port);

			/* ----------------------------------------------- */
			/* TCP connection is ready. */
			/* A SSL structure is created */
			ssl = SSL_new(ctx);
			if (ssl == NULL) {
				fprintf(stderr, "ERR: unable to create the ssl structure\n");
				exit(-1);
			}

			/* Assign the socket into the SSL structure (SSL and socket without BIO) */
			SSL_set_fd(ssl, sock);

			/* Perform SSL Handshake on the SSL server */
			err = SSL_accept(ssl);
			if (err == -1) {
				ERR_print_errors_fp(stderr); 
				exit(1);
			}

			/* Informational output (optional) */
			// printf("SSL connection using %s\n", SSL_get_cipher (ssl));

			if (verify_client == ON) {
				/* Get the client's certificate (optional) */
				client_cert = SSL_get_peer_certificate(ssl);
				if (client_cert != NULL) {
					printf ("Client certificate:\n");     
					str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
					RETURN_NULL(str);
					printf ("\t subject: %s\n", str);
					free (str);
					str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
					RETURN_NULL(str);
					printf ("\t issuer: %s\n", str);
					free (str);
					X509_free(client_cert);
				} else {
					printf("The SSL client does not have certificate.\n");
				}
			}

			// if((accept_fd=accept(fd,(struct sockaddr *) &tmp_addr, &addrlen))==-1){
			// 	perror("accept()");
			// 	exit(1);
			// }
			pthread_mutex_lock(&mutex);
				client_count++;
			pthread_mutex_unlock(&mutex);
			if(pthread_create(&thr, NULL, pthread_loop, ssl)!=0){
				printf("fail to create thread\n");
				err = SSL_shutdown(ssl);
				if (err == -1) {
					ERR_print_errors_fp(stderr); 
					exit(1);
				}
				pthread_mutex_lock(&mutex);
					client_count--;
				pthread_mutex_unlock(&mutex);
			}
		}
	}
	err = close(sock);
	if (err == -1) {
		perror("close");
		exit(-1);
	}
	/* Free the SSL structure */
	SSL_free(ssl);

	/* Free the SSL_CTX structure */
	SSL_CTX_free(ctx);
}
int main(int argc, char **argv){
	unsigned short port;
	if (argc!=2){
		fprintf(stderr, "Usage: %s [port]\n",argv[0]);
		exit(1);
	}
	port=atoi(argv[1]);
	main_loop(port);
	return 0;
} 
