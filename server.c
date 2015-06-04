#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 128
#define MAX_CLNT 2
#define NAME_SIZE 20

void * handle_clnt(void * arg);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
char clnt_name[NAME_SIZE]= {NULL};
char clnt_names[MAX_CLNT][NAME_SIZE]= {NULL};


pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock, i;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
	
		if(clnt_cnt >= MAX_CLNT) {
			printf("CONNECT FAIL : %d \n", clnt_sock);
			write(clnt_sock, "too many users. sorry", BUF_SIZE);
			continue;
		}


		pthread_mutex_lock(&mutx);

		clnt_socks[clnt_cnt]=clnt_sock;
		//receive client name
		read(clnt_sock, clnt_name, NAME_SIZE);		
		strcpy(clnt_names[clnt_cnt++], clnt_name);
		pthread_mutex_unlock(&mutx);

		
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
		printf("Connected client name : %s \n", clnt_name);
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
        int str_str=0;
	int fSize = 0;
	const char sig_file[BUF_SIZE] = {"send file"};
	const char Fmsg_end[BUF_SIZE] = {"file end"};
	char msg[BUF_SIZE] = {NULL};
	char file_msg[BUF_SIZE] = {NULL};
	char setfname[BUF_SIZE] = {NULL};

	while((str_len=read(clnt_sock, msg, BUF_SIZE))!=0) 
	{

		if(!strcmp(msg, sig_file))
		{
			int j;
			int fileGo = NULL;
			char tmpName[NAME_SIZE]= {NULL};

			//receive file size, name
			read(clnt_sock, &fSize, sizeof(int));
			printf("File size %d Byte\n", fSize);
			read(clnt_sock, setfname, BUF_SIZE);
			printf("received file name : %s\n",setfname);
			
			FILE *fp;
			fp = fopen(setfname, "wb"); 
		
			//receive and save file
			while(1)
			{		
				str_str = read(clnt_sock, file_msg, BUF_SIZE);
                                printf("file_msg :%d str_str: %d\n",strlen(file_msg), str_str);
				if(str_str < 0){ 
                                    printf("Receive Error\n");
				    break;
                                }
                                if(str_str == 0){
                                    printf("Receive OK\n");
                                    break;
                                }
				fwrite(file_msg, 1, str_str, fp);
			}

			fclose(fp);


			pthread_mutex_unlock(&mutx);
			printf("File data transfered \n");

		}
	}

	////////
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1) {
				clnt_socks[i]=clnt_socks[i+1];
				strcpy(clnt_names[i], clnt_names[i+1]);
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
