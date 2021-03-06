#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>



int main(int argc, char const *argv[])
{
	int PORT;

	if(argc != 4)
	{
		printf("\nInvalid number of argument .Please input current arguments\n\n");
		exit(0);
	}

	char *str;

	char port[200];
	char receiver[250];

	str = strtok(argv[1],":");
	strcpy(receiver,str);

	str = strtok(NULL, ":");
	strcpy(port,str);

	PORT = atoi(port);

	int server_sock = 0, valread;
	struct sockaddr_in serv_address;

	char buffer[1024] = {0};

	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n socket creatin error\n");
		return -1;
	}

	memset(&serv_address, '0', sizeof(serv_address));

	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1",&serv_address.sin_addr)<=0)
	{
		printf("\nInvalid address/ address not supported \n");
		return -1;
	}

	if(connect(server_sock, (struct sockaddr *)&serv_address, sizeof(serv_address))<0)
	{
		printf("\nconncetion failed\n" );
		return -1;
	}

	//------->>>connection check<<<<--------

	valread = read(server_sock, buffer, 1024);

	printf("server: %s\n",buffer );

	if(buffer[0]!='2')
	{
		close(server_sock);
		exit(0);

	}

	char hostname[250];
	gethostname(hostname);

	printf("Client : HELLO %s\n",hostname );


	///helo message check

	char hello[250]="HELO ";

	strcat(hello,hostname);
	send(server_sock, hello,strlen(hello), 0);

	bzero(buffer,1024);

	valread = read(server_sock, buffer,1024);
	printf("Server : %s\n",buffer );

	if(buffer[0]!='2')
	{
		close(server_sock);
		exit(0);
	}

	//---------->>>>mail from check<<<<<--------

	char user_name[250];
	getlogin_r(user_name,250);

	char mailfrom[250]="MAIL FROM : ";
	strcat(mailfrom,user_name);
	strcat(mailfrom,"@");
	strcat(mailfrom,hostname);

	printf("Client : %s\n", mailfrom );

	send(server_sock, mailfrom, strlen(mailfrom),0);

	bzero(buffer,1024);

	valread = read(server_sock, buffer, 1024);
	printf("Server : %s\n", buffer );
	if(buffer[0]!='2')
	{
		close(server_sock);
		exit(0);

	}

	///RPT check

	char rcptto[250]="RCPT TO : ";
	strcat(rcptto,receiver);
	printf("Client : %s\n", rcptto);

	send(server_sock,rcptto, strlen(rcptto) , 0);
	bzero(buffer,1024);

	valread = read(server_sock, buffer, 1024);
	printf("Server : %s\n", buffer);

	if(buffer[0]!='2')
	{
		close(server_sock);
		exit(0);
	}

	//data check

	char data[250]="DATA";
	printf("Client : %s\n", data);

	send(server_sock, data, strlen(data),0);
	bzero(buffer,1024);

	valread = read(server_sock, buffer, 1024);
	printf("Server : %s\n", buffer);
	if(buffer[0]!='2' && buffer[0]!='3')
	{
		close(server_sock);
		exit(0);
	}

	///file header

	char header[1024];
	bzero(header,1024);

	//time setting

	char date[250];


	time_t now = time(0);

	struct tm tstruct;
	tstruct = *localtime(&now);
	strftime(date,sizeof(date),"%Y-%m-%d ; Time: %X",&tstruct);
	strcat(header, "To : ");
	strcat(header,receiver);
	strcat(header,"\n");
	strcat(header,"From : ");
	strcat(header,user_name);
	strcat(header,"@");
	strcat(header,hostname);
	strcat(header, "\n");
	strcat(header, "Subject : ");
	strcat(header, argv[2]);
	strcat(header,"\n");
	strcat(header,"Date : ");
	strcat(header,date);
	strcat(header,"\n\n");

	send(server_sock,header, sizeof(header),0);

	///data read from file

	FILE *filename = fopen(argv[3],"r");

	char temp[1024];
	bzero(temp,1024);

	while(filename !=NULL && fgets(temp, sizeof(temp),filename)!= NULL)
	{
		printf("Client : %s\n", temp);
		send(server_sock, temp, sizeof(temp),0);
	}

	bzero(temp,1024);
	strcat(temp,".");

	send(server_sock,temp,sizeof(temp),0);

	bzero(buffer,1024);
	valread = read(server_sock,buffer, 1024);
	printf("Server : %s\n", buffer);

	if(filename !=NULL)
	{
		fclose(filename);
	}

	///QUIT

	char quit[250] = "QUIT";
	printf("Client : %s\n", quit);

	send(server_sock, quit, strlen(quit),0);

	bzero(buffer,1024);

	valread = read(server_sock,buffer,1024);
	printf("Server : %s\n", buffer);

	if(buffer[0]!='2')
	{
		close(server_sock);
		exit(0);
	}
return 0;
}
