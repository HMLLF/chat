#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
struct joint								//保存接收信息
{
	char recvbuf[128];
	char name[32];
};
int client_fd[32]={0};						//保存客户端描述符
char client_name[32][32] = {0};				//保存客户端用户名
void*Accept(void*arg)
{
	int i1=*((int *)arg);
	//recv send
	
	int send_i;
	struct joint recv_data;
	int t = 0;

	while(1)
	{
		
		char join[256] = {0};
		char who[32];
		int rec = recv(client_fd[i1],&recv_data,sizeof(recv_data),0);				//接收客户端信息
		if(0 == strcmp(recv_data.recvbuf,"to one"))									//如果客户端输入to one
		{
			send(client_fd[i1],"please tell me who is your want to talk:\n",41,0);	//则发送信息询问要向哪个客户端发送信息
			//continue;
			recv(client_fd[i1],who,32,0);											//收到客户端输入的客户名保存在who中
			printf("is %s\n",who);
			t = 1;																	//并将状态置为1
			continue;
		}
		if(0 == strcmp(recv_data.recvbuf,"to everyone"))							//输入to everyone将状态置为2
		{
			t = 0;
			continue;
		}
		sprintf(join,"%s:%s",recv_data.name,recv_data.recvbuf);
		if(rec > 0)
		{
			printf("%s\n",join);
		}
		if(0 == rec)																//若客户端退出，则在公屏展示
		{
			client_fd[i1] = 0;
			char quit_man[32];
			sprintf(quit_man,"%s退出群聊",recv_data.name);
			printf("%s\n",quit_man);
			for(send_i = 0;send_i < 32;send_i++)
			{
				send(client_fd[send_i],quit_man,sizeof(quit_man),MSG_NOSIGNAL);
			}
			break;
		}
		if(0 == t)																	//群嗨
		{
			for(send_i = 0;send_i < 32;send_i++)
			{
				if(client_fd[send_i] != 0)
				{
					send(client_fd[send_i],join,sizeof(join),MSG_NOSIGNAL);
				}
			}
		}
		if(1 == t)																	//私聊
		{
			for(send_i = 0;send_i < 32;send_i++)
			{
				if(0 == strcmp(client_name[send_i],who))							//轮询，向客户端输入的用户名用户发送信息
				{
					send(client_fd[send_i],join,sizeof(join),MSG_NOSIGNAL);
				}
			}
		}
	}
}
int main(int argc,char**argv)
{
	//create
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == server_fd)
	{
		perror("server socket error");
		return -1;
	}
	//set
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	//bind
	bind(server_fd,(struct sockaddr*)&server_addr,
		sizeof(server_addr));
	//listen
	listen(server_fd,10);
	//accept
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	pthread_t pid[32];
			
	int i=0;
	while(1)
	{
		client_fd[i] = accept(server_fd,
								(struct sockaddr*)&client_addr,		//链接客户端套接字
								&len);
		recv(client_fd[i],client_name[i],sizeof(client_name),0);	//接收客户端用户名保存在二维数组client_name中
		if(-1 == client_fd[i])
		{
			perror("client accept error");
			close(server_fd);
			return -1;
		}
		else
		{
			pthread_create(&pid[i],NULL,Accept,					//创建线程来收发信息
							(void*)&i);
			sleep(1);
			i++;
		}
	}
	
	//close
	close(server_fd);
	
}