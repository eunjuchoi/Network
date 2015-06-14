#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 10000
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
	//printf("CONNECTING..... \n<Menu>\n1.credit\n2.send\n3.transrate\n4.exit\n");
        printf("<Menu>\n1.credit\n2.send\n3.exit\n");
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	//close(sock);  
	return 0;
}
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	double Flength = 0;
	int i=0;
	int fSize = 0;
	int fEnd = 0;
	int id = 0;
	char name_msg[NAME_SIZE+BUF_SIZE] = {NULL};
	char t_msg[BUF_SIZE] = {NULL};
	char last_msg[BUF_SIZE] = {NULL};
	char t_name_msg[BUF_SIZE] = {NULL};
        char file_msg[BUF_SIZE]={NULL};
	char noUse[BUF_SIZE] = {NULL};
        int sendRate =0;
        int recvRate = 0;
        int pos=0;
        int trans = 0;

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

                        //restrict rate
                        do{
			printf("\n(Max transfer rate is 10000 bytes/sec.)\n");
                        printf("sendRate(byte):");
                        scanf("%d", &sendRate);
                        printf("recvRate(byte):"); 
                        scanf("%d",&recvRate);
                        }while(sendRate<0||recvRate<0||sendRate>BUF_SIZE||recvRate>BUF_SIZE);
                        printf("OK\n");



			//file size computing
			while(1) {	
				fEnd = fread(noUse, 1 , sendRate, size); 
				fSize += fEnd;

				if(fEnd != sendRate)
					break;
			}
			fclose(size);

			//transfer file size
			printf("File transfer start \n(File Size : %d Byte)\n", fSize); 
			write(sock, &fSize, sizeof(int));
			

			//send the setfname,sendRate , recvRate
			printf("set file name : ");
			scanf("%s",&setfname);
			write(sock,setfname,BUF_SIZE);
                        write(sock,&sendRate,sizeof(int));
                        write(sock,&recvRate,sizeof(int));
			
			//send the file
			fp = fopen(filename, "rb");
			while(Flength<fSize) {	
				pos = fread(file_msg,1,sendRate, fp);
                                Flength += pos;
				write(sock, file_msg, sendRate);
				trans+=sendRate;
				printf("%d/%d bytes transfferd.\n",trans,fSize);
				sleep(1);				///////time
                                pos=0;
			}

			fclose(fp);
			printf("File sending finish \n");
			trans = 0;

                        //recive the file
                        char msg[BUF_SIZE] ={NULL};
                        FILE* fd;
			fd = fopen(setfname, "wb"); 
                        Flength=0;	
			while(Flength<fSize)
			{		
				pos=read(sock, msg, recvRate);
                                Flength+=pos;
				fwrite(msg,1,recvRate, fd);
                                pos=0;
			}
                        printf("File receving finish \n");
                          
			fclose(fd);





                        close(sock);
		}
                fSize=0;

		////////////////////////////////////////////////////////credit
		if(!strcmp(msg,"credit\n"))
		{
			printf("Write ID :");
			scanf("%d", &id);
			switch(id)
			{
                            case 20103154:
                                printf("20103154 Kim Youngho modified file send size to same size with original file, and some other thing.\n");                              break;
			    case 20123423:
				printf("20123423 Choi eun ju made multiple people can access to a server and transmit file from clients to a server.\n");
				break;
		    	    case 20133277:
				printf("Heo Seongsil try to send received file again server to client\n");
				break;
                            case 20133231: 
                                printf("Shin Seungyeol make a coding  for transmission, restricting rate, multiplexing.\n");
				break;
			    case 20103302:
			        printf("I confirm the result by checking the time .\n");
                            default: 
                                printf("student id error\n");
			}
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
