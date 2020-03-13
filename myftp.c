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
