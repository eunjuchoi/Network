#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 128
#define NAME_SIZE 20
	
void * send_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE]={NULL};
char msg[BUF_SIZE]={NULL};
int setFName = 0;
//pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4) {
		printf("You have to write %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	//pthread_mutex_init(&mutx, NULL);
	// pthread_mutex_lock(&mutx);
	// pthread_mutex_unlock(&mutx);

	sprintf(name, "%s", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

	//transfer client name
	write(sock, name, NAME_SIZE);

	printf("\n\n");
	printf("CONNECTING..... \n<Menu>\n1.credit\n2.send\n3.transrate\n4.exit\n");

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	close(sock);  
	return 0;
}
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	int Flength = 0;
	int i=0;
	int fSize = 0;
	int fEnd = 0;
	int id = 0;
	char name_msg[NAME_SIZE+BUF_SIZE] = {NULL};
	char t_msg[BUF_SIZE] = {NULL};
	char last_msg[BUF_SIZE] = {NULL};
	char t_name_msg[BUF_SIZE] = {NULL};
	char noUse[BUF_SIZE] = {NULL};

	while(1) 
	{
		fgets(msg, BUF_SIZE, stdin);
		
		////////////////////////////////////////////////////////exit
		if(!strcmp(msg,"exit\n")) 
		{
			close(sock);
			exit(0);
		}

		////////////////////////////////////////////////////////send
		if(!strcmp(msg,"send\n")) 
		{
			char filename[BUF_SIZE];
			char setfname[BUF_SIZE];
			FILE *fp;
			FILE *size;

			//Select file
			printf("Write File Name : ");
			scanf("%s", filename);

			size = fopen(filename, "rb");
			if(size == NULL) {
				printf("There is no file named %s\n",filename);
				continue;
			}

			//transfer send signal
			write(sock, "send file", BUF_SIZE);

			//file size
			while(1) {	
				fEnd = fread(noUse, 1 , BUF_SIZE, size);
				fSize += fEnd;

				if(fEnd != BUF_SIZE)
					break;
			}
			fclose(size);

			//transfer file size
			printf("File transfer start \n(File Size : %d Byte)\n", fSize); 
			write(sock, &fSize, sizeof(int));
			fSize = 0;

			//transfer setfname
			printf("set file name : ");
			scanf("%s",&setfname);
			write(sock,setfname,BUF_SIZE);
			
			//transfer file
			fp = fopen(filename, "rb");
			while(1) {		
				Flength = fread(t_msg, 1 , BUF_SIZE, fp);

				if(Flength != BUF_SIZE) {
					for(i=0; i<Flength; i++) {
						last_msg[i] = t_msg[i];
					} 
					write(sock, last_msg, BUF_SIZE); ////////////transfer rate
					write(sock, "file end", BUF_SIZE);
					

					break;
				}
				write(sock, t_msg, BUF_SIZE); 
				printf("%d \n", Flength);
				usleep(1000);				///////time
			}
			fclose(fp);
			printf("File transfer finish \n");			
		}

		////////////////////////////////////////////////////////credit
		if(!strcmp(msg,"credit\n"))
		{
			printf("Write ID :");
			scanf("%d", &id);
			switch(id)
			{
			case 20123423:
				printf("hi\n");
				break;
			default:
				break;
			}
		}
		////////////////////////////////////////////////////////transrate
		if(!strcmp(msg,"transrate\n"))
		{
		}
	}
	return NULL;
}
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
