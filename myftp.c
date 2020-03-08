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
#include "myftp.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
int es_data(int fd,char entire_input_string[PAYLEN], int data_size,RSA* rsa){

	int BLOCKLEN=RSA_size(rsa)-41;

	int tmplen=0;
	struct encrypt_ftp data;
	
	data.e_data=(unsigned char *)calloc(RSA_size(rsa),sizeof(unsigned char));
	printf("data size: %d, BLOCKLEN: %d\n",data_size,BLOCKLEN);
	printf("(entire_input_string is %s\n )\n",entire_input_string);
	

	unsigned char* input_string=(unsigned char*)calloc(BLOCKLEN,sizeof( char));
	for(int j=0;j<data_size;j++){
		*(input_string+j*sizeof(unsigned char))=(unsigned char*)*(entire_input_string+j*sizeof(char));
	}
	int encrypt_size;
	if(( encrypt_size=RSA_public_encrypt(data_size,input_string,data.e_data,rsa,RSA_PKCS1_OAEP_PADDING))<0){
		printf("error to encrypted data\n");
	}

	// for (int j=0;j<encrypt_size;j++){
	// 	printf("%dth is %d\n",j,*(data.e_data+j));
	// }
	data.length=htonl(sizeof(data.length)+encrypt_size);
	if((tmplen=send(fd,&data.length,sizeof(data.length),0))<0){
		printf("error to send data length\n");
	} 
	data.length=ntohl(data.length);
	// printf("len is %d\n",data.length-sizeof(int));
	int leftlen=encrypt_size;
	while(leftlen!=0){
		if((tmplen=send(fd,data.e_data,encrypt_size,0))<0){
			printf("error to send encrypted data\n");
			exit(1);
		}

		leftlen-=tmplen;
	}
	
	memset(data.e_data,0,RSA_size(rsa));

	free(input_string);

	// printf("end data_size: %d\n", data_size);
	// printf("input:%s\nend\n", entire_input_string);
	return tmplen;
}

int er_data(int fd,char *output_string, int data_size,RSA* rsa){
	int BLOCKLEN=RSA_size(rsa)-41;
	unsigned char* decrypt_string=(unsigned char*)calloc(RSA_size(rsa),sizeof(unsigned char));
	int len;
	struct encrypt_ftp data;
	data.e_data=(unsigned char*)calloc(RSA_size(rsa),sizeof(unsigned char));
	int leftlen=sizeof(data.length);
	while(leftlen!=0){
		if((len=recv(fd,&data.length,sizeof(data.length),0))<0){
				printf("error to recv data length\n");
			}
		leftlen-=len;
	}
	data.length=ntohl(data.length);
	leftlen=data.length-sizeof(data.length);
	// printf("the length of data is %d\n",data.length-sizeof(data.length));
	int encrypt_size=leftlen;
	int decrypt_size;
	unsigned char* e_data_tmp=(unsigned char*)malloc(RSA_size(rsa)*sizeof(unsigned char));
	while(leftlen!=0){
			
			memset(e_data_tmp,0,sizeof(e_data_tmp));
			if((len=recv(fd,e_data_tmp,leftlen,0))<0){
					printf("error to recv e_data\n");
					exit(1);
				}
			
			for(int j=0;j<len;j++)
			{	
				*(data.e_data+((data.length-sizeof(int)-leftlen)+j)*sizeof(unsigned char))=*(e_data_tmp+j*sizeof(unsigned char));
			}
			leftlen-=len;
			


	}
	free(e_data_tmp);
	for(int j=0;j<encrypt_size;j++)
	{	
		
	}
	// printf("can i reach here %d \n",(data.length-sizeof(int)));
	if((decrypt_size=RSA_private_decrypt((data.length-sizeof(int)),data.e_data,decrypt_string,rsa,RSA_PKCS1_OAEP_PADDING))<0){
		printf("can not decrypt the string\n");
	}
	// printf("finish decrypt the data\n");
	// printf("data size: %d\n", data_size);
	// if(decrypt_size!=data_size){
	// 	printf("decrypt_size != data_size, %d %d\n",data_size,decrypt_size);
	// }
	// else{
	// 	printf("decrypt_size == data_size, %d %d\n",data_size,decrypt_size);

	// }
	for(int i=0;i<data_size;i++){
		*((char*)output_string+i*sizeof(char))=(char)*(decrypt_string+i*sizeof(unsigned char));
	}
	// printf("end data_size: %d\n",data_size);
	// printf("output: %s\nend\n",output_string );
	return data_size;
}

void tranp_file_data(int accept_fd, char filename[], char path[],RSA* rsa){
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
	close(file);

	if((file=fopen(filepath,"rb"))<0){
		perror("fail to open the file\n");
	}
	if((len=send(accept_fd,&header,sizeof(header),0))<0){
		perror("cannot send header to client\n");
	}
	 // char character[2]="";
	memset(payload,0,PAYLEN);
	int if_finish=0;
	unsigned int file_length=0;

	header.length = ntohl(header.length);

	int i;
	for(i=0;i<=(header.length-10-1)/(sizeof(payload)-1);i++){
		int j;
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
				if((len=es_data(accept_fd,payload,sizeof(payload)-1,rsa))<0){
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
			while((len=es_data(accept_fd,payload,sizeof(payload)-1,rsa))<0){
				perror("can not send payload to client\n");

			}

		}

		memset(payload,0,PAYLEN);

	}
	printf("Sending finished\n");
	
	close(file);

}

void recv_file_data(int fd, char filename[], char path[],RSA* rsa){
	int len;
	struct message_s header;
	char payload[PAYLEN];
	if((len=recv(fd,&header,sizeof(header),0))<0){
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
	int lenleft=header.length-10;
	while(lenleft!=0){
		printf("file_length: %d, true length: %d, lenleft: %d\n", file_length, header.length-10 , lenleft);
		memset(payload,0, PAYLEN);
		//usleep(1);
		if(lenleft<sizeof(payload)-1){
			if((len=er_data(fd, payload,lenleft,rsa))<0){
				printf("download file error!\n");
			// close(downfile);
			}
		}
		else{
			if((len=er_data(fd, payload,sizeof(payload)-1,rsa))<0){
				printf("download file error!\n");
				// close(downfile);
			}
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
		lenleft-=len;
		// file_length+=len;
	}
	printf("Receiving finished\n");
	fflush(downfile);
	close(downfile);
}
void receive_public_key(RSA* encrypt_rsa,int fd){
	struct message_s header;
	memset(&header,0,sizeof(header));
	int len=0;
	// printf("i can get here too\n");
	// printf("i can get here too: %d\n",RSA_size(encrypt_rsa));

	// unsigned char* n_b;
	// unsigned char* e_b;
	int n_size,b_size;


	if((len=(recv(fd,&header,sizeof(header),0))<0)){
		perror("error to receive header\n");
	}
	if(header.type!=0xD1){
		perror("recv wrong header\n");
		exit(1);
	}
	n_size=ntohl(header.length)-10;
	unsigned char* n_b=(unsigned char*)calloc(n_size,sizeof(unsigned char));
	// while(size_left>0){
	if((len=recv(fd,n_b,n_size,0))<0){
		perror("can not recv n_b");
	}
	// }
	memset(&header,0,sizeof(header));

	if((len=(recv(fd,&header,sizeof(header),0))<0)){
		perror("error to receive header\n");
	}


	if(header.type!=0xD1){
		perror("recv wrong header\n");
		exit(1);
	}
	b_size=ntohl(header.length)-10;
	unsigned char* e_b=(unsigned char*)calloc(b_size,sizeof(unsigned char));

	// while(size_left>0){
	if((len=recv(fd,e_b,b_size,0))<0){
		perror("can not recv n_b");
	}
	// }
	// printf("public: %s\nprivate: %s\n",n_b,e_b);

	encrypt_rsa->n=BN_bin2bn(n_b,n_size,NULL);
	encrypt_rsa->e=BN_bin2bn(e_b,b_size,NULL);

	// printf("n_b: %s\n%d %d\n",n_b,n_size,RSA_size(encrypt_rsa));
	printf("so i get the key\n");
	// PAYLEN=RSA_size(encrypt_rsa)-41;
}
void send_public_key(RSA * rsa, int fd){
	unsigned char* n_b=(unsigned char*)calloc(RSA_size(rsa),sizeof(unsigned char));
	unsigned char* e_b=(unsigned char*)calloc(RSA_size(rsa),sizeof(unsigned char));
	int n_size=BN_bn2bin(rsa->n,n_b);
	int b_size=BN_bn2bin(rsa->e, e_b);
	struct message_s header;
	int len;
	memset(&header,0,sizeof(header));
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xD1;
	
	header.length = 10 + n_size;
	header.length = htonl(header.length);
	if((len=send(fd,&header,sizeof(header),0))<0){
		perror("it can not send the rsa->n\n");
		exit(1);
	}

	if((len=send(fd,n_b,n_size,0))!=n_size){
		perror("it can not send the rsa->n\n");
		exit(1);
	}
	else{
		// printf("yes i have sent %s\n %d %d\n",n_b,n_size,RSA_size(rsa));
	}
	memset(&header,0,sizeof(header));
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xD1;
	
	header.length = 10 + b_size;
	header.length = htonl(header.length);

	if((len=send(fd,&header,sizeof(header),0))<0){
		perror("it can not send the rsa->n\n");
		exit(1);
	}
	if((len=send(fd,e_b,b_size,0))!=b_size){
		perror("it can not send the rsa->n\n");
		exit(1);
	}
	// printf("public: %s\nprivate: %s\n",n_b,e_b);
	// PAYLEN=RSA_size(rsa)-41;
}