#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

struct joint								//用结构体来保存发送信息
{
	char sendbuf[128];
	char name[32];
};
void*Recv(void*arg)
{
	int fd = *(int *)arg;
	while(1)
	{
		char recvbuf[128] = {0};
		recv(fd,recvbuf,sizeof(recvbuf),0);
		printf("%s\n",recvbuf);
	}
}
int main(int argc,char **argv)
{
	if(4 != argc)
	{
		printf("lack parameter\n");
		return -1;
	}
	
	//create
	int client_fd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == client_fd)
	{
		perror("client socket error");
		return -1;
	}
	//set 
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	//connect
	connect(client_fd,(struct sockaddr*)&server_addr,
			sizeof(server_addr));
	//send recv
	pthread_t pid[32];
	int i = 0;
	struct joint send_data;
	char client_name[32] = {0};
	strcpy(client_name,argv[3]);
	send(client_fd,client_name,sizeof(client_name),0);  		//将客户端用户名发送给服务器
	while(1)
	{
		strcpy(send_data.name,argv[3]);
		
		
		pthread_create(&pid[i],NULL,Recv,(void*)&client_fd);	//创建线程来接收从服务器返回的信息
		
		fgets(send_data.sendbuf,128,stdin);						//键盘获取输入
		
		send_data.sendbuf[strlen(send_data.sendbuf)-1] = 0;
		send(client_fd,&send_data,sizeof(send_data),0);		//发送
		
		i++;
	}
	close(client_fd);
}