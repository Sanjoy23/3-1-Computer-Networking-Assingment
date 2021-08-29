#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>


int main(int argc, char const *argv[])
{
	int server_socket = socket (AF_INET, SOCK_STREAM,0);

	if(server_socket < 0)
	{
		printf("socket error\n");
		return 0;
	}

	struct  sockaddr_in server_addr;

	bzero((char *) & server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = 0;

	if(bind(server_socket,(struct sockaddr *)& server_addr, sizeof(server_addr))<0)
	{
		perror("bind failed..");
		exit(EXIT_FAILURE);
	}

	socklen_t len = sizeof(server_addr);

	if(getsockname (server_socket, (struct sockaddr *)& server_addr, &len) == -1)
	{
		perror("getsockname");
		return 0;
	}

	printf("Waiting for Client connection.\n\n port number %d\n\n",ntohs(server_addr.sin_port));

	if(listen(server_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	int new_socket,valread;
	int addrlen = sizeof(server_addr);
	//------>>>>>>>waiting for connection from client<<<<<<<-------

	if((new_socket = accept(server_socket, (struct sockaddr *) & server_addr,(socklen_t*)&addrlen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	printf("connection established.\n");

	char hostname[250];
	gethostname(hostname, 250);

	//------->>>>>>>connection established.<<<<<<<------

	char buffer[1024] = {0};
	char sender[1024] = "220 OK, from ";

	strcat(sender,hostname);
	send(new_socket, sender , strlen(sender),0);

	//-------->>>>>Hello reply<<<<<<<<-------

	bzero(buffer,1024);
	valread = read(new_socket , buffer , 1024);
	bzero(sender,1024);

	char * str;
	char ptr[250];
	str = strtok(buffer," ");
	strcpy(ptr,str);
	str = strtok(NULL," ");
	strcpy(buffer, str);

	if(strcmp(ptr,"HELO") == 0)
	{
		strcat(sender, "250 Hello ");
		strcat(sender,buffer);
		strcat(sender,", pleased to meet you.");
		send(new_socket, sender, strlen(sender), 0 );
	}
	else
	{
		strcat(sender,"500 Syntax error command unrecognized.Required \"HELO\" command.\n");
		send(new_socket, sender, strlen(sender), 0);
		printf("connection closed\n\n");
		return main(argc, argv);

	}

	//-------->>>>>>Mail from reply<<<<<<<-------

	bzero(buffer, 1024);
	valread = read (new_socket , buffer, 1024);
	bzero(sender,1024);

	char ptr2[250];

	str = strtok(buffer, " ");
	strcpy(ptr,str);
	str=strtok(NULL," ");
	strcpy(ptr2,str);
	str = strtok(NULL," ");
	str = strtok(NULL," ");
	strcpy(buffer,str);

	if(strcmp(ptr, "MAIL") == 0 && strcmp(ptr2, "FROM") == 0)
	{
		printf("To: %s\n",buffer );
		strcat(sender , "250 ");
		strcat(sender,buffer);
		strcat(sender,"...sender ok.");

		send(new_socket , sender, strlen(sender),0);
	}
	else{
		strcat(sender,"500 Syntax error command unrecognized.Required \"MAIL FROM\" command.\n");
		send(new_socket, sender, strlen(sender), 0);
		printf("connection closed\n");
		return main(argc,argv);
	}

	//RCPT to reply

	FILE *fp;

	bzero(buffer, 1024);
	valread = read(new_socket, buffer, 1024);
	bzero(sender, 1024);

	str = strtok(buffer," ");
	strcpy(ptr,str);
	str = strtok(NULL," ");
	strcpy(ptr2,str);
	str = strtok(NULL," ");
	str = strtok(NULL," ");
	strcpy(buffer,str);

	char receiver[250];
	strcpy(receiver,buffer);

	if(strcmp (ptr,"RCPT") == 0 && strcmp(ptr2,"TO") == 0)
	{
		//--->>>>>>file create<<<<<----

		str = strtok(receiver,"@");
		char filename[250];
		strcpy(filename,str);
		strcat(filename,".txt");

		fp = fopen(filename,"a");

		if(fp == NULL)
		{
			strcat(sender,"404 can't open user's mail file.\n");
			send(new_socket, sender, strlen(sender), 0);

			printf("connection closed.\n\n");
			return main(argc,argv);
		}

		strcat(sender, "250");
		strcat(sender,buffer);
		strcat(sender,".Recipient ok.");

		send(new_socket, sender, strlen(sender),0);
	}
	else{
		strcat(sender,"500 Syntax error command unrecognized.Required\"RCPT\" command.\n");
		send(new_socket, sender, strlen(sender),0);
		printf("connection closed.\n\n");
		return main(argc,argv);

	}

	//------>>>>>>data check<<<<-----

	bzero(buffer,1024);
	valread = read(new_socket,buffer,1024);
	bzero(sender,1024);

	char * str2;
	str2 = strtok(buffer," ");
	strcpy(ptr,str2);
	str2 = strtok(NULL, " ");
	char header[1024];

	if(strcmp(ptr,"DATA") == 0)
	{
		strcat(sender, "354 Enter mail, end with \.\ on a line by itself.");
		send(new_socket, sender, strlen(sender), 0);
	}
	else{
	strcat(sender,"500 Syntax error command unrecognized.Required\"RCPT\" command.\n");
		send(new_socket, sender, strlen(sender),0);
		printf("connection closed.\n\n");
		return main(argc,argv);

	}
	//------>>>>>>file header<<<<<<--------
	bzero(header,1024);
	valread = read(new_socket,header,1024);
	fwrite(header, sizeof(char),strlen(header),fp);

	//------->>>>>File write<<<<<<<------

	int count = 0;

	while(1)
	{
		bzero(buffer,1024);
		 valread = read(new_socket,buffer,1024);

		 if(strcmp(buffer,".") == 0)
		 {
		 	bzero(sender,1024);
		 	strcat(sender,"250 Message accepted and stored in user's file. Total Message line : ");
		 	char num[200];
		 	sprintf(num,"%d",count);
		 	strcat(sender, num);

		 	send(new_socket,sender, strlen(sender), 0);
			bzero(buffer,1024);
			strcat(buffer, "\n\t-----------\n");

			fwrite(buffer,sizeof(char), strlen(buffer), fp);

			break;

		 }
		 count++;
 		 fwrite(buffer,sizeof(char), strlen(buffer), fp);


	}
	fclose(fp);


	//------->>>>>>quit check.<<<<<<<--------

	bzero(buffer,1024);
	valread = read(new_socket, buffer , 1024);
	bzero(sender,1024);

	if(strcmp(buffer,"QUIT") == 0)
	{
		strcat(sender,"221 ");
		strcat(sender,hostname);
		strcat(sender,"closing connection");

		send(new_socket,sender,strlen(sender),0);
	}else{

		strcat(sender,"500 Syntax error command unrecognized.Required\"QUIT\" command.\n");
		send(new_socket, sender, strlen(sender),0);
		printf("connection closed.\n\n");
		return main(argc,argv);
	}

close(new_socket);
close(server_socket);
printf("connection closed\n");
return main(argc,argv);
}
