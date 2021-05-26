#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<ctype.h>
#include<time.h>
#include <arpa/inet.h>


void error(const char *msg) 
{                           
    perror(msg);            
    exit(1);                
}                           

int main(int argc, char *argv[])
{
    time_t now;                                                                                 /* get current time*/

    int sockfd, newsockfd,addrlen, portno , logperfive , log = 0 , logcount = 0;                
    socklen_t clilen;                                                                           /* log is to get the degree on process*/
	
	struct in_addr localInterface;																/* «Ø¥ßmuticast server ©Ò¥Î*/
	struct sockaddr_in groupSock;																/* «Ø¥ßmuticast server ©Ò¥Î*/
	
	struct sockaddr_in localSock;																/*«Ø¥ßmuticast client ©Ò¥Î*/
	struct ip_mreq group;																		/*«Ø¥ßmuticast client ©Ò¥Î*/

    unsigned char sendbuf[512];                                                                 /* data store in send buffer*/
    unsigned char recvbuf[512];                                                                 /* data store in reveive buffer*/
    unsigned char buffer[512];                                                                  
	
	char ip[32];																				/* 32 bits ip address*/

    struct sockaddr_in serv_addr, cli_addr;                                                     /* ¬° «Ø¥ß³sœu ©Ò¥Î*/
    struct hostent *server;                                                                     /* ¬° «Ø¥ß³sœu ©Ò¥Î*/

    FILE *fp;                                                                                   /* open folder */
	
	if(!strcmp(argv[1],"server"))														/* server send*/
	{
		if(!strcmp(argv[2],"multicast"))												/* send via multicast*/
		{
			int byte_num = 0 ,file_len , byte=0;													/* byte_num: numOfByte of file*/
																										/* file_len: length of file name*/
																										/* byte:transfered byte*/
			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            {
                perror("Opening datagram socket error");            /* socketdf: store if there is socket in UDP*/
                exit(1);
            }
            else 
                printf("Opening the datagram socket...OK.\n");
				
			/* binding */
			memset((char *) &groupSock, 0, sizeof(groupSock));								/* groupSock address is 226.1.1.1 via port 4321 */
			groupSock.sin_family = AF_INET;
			groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
			groupSock.sin_port = htons(4321);
				
			localInterface.s_addr = inet_addr("127.0.0.1");										/* Set local interface for outbound multicast datagrams. */
			if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
			{
				error("Setting local interface error");
				exit(1);
			}
            else
                printf("Setting the local interface...OK\n");
				
			printf("Sending datagram message...OK\n");
			file_len = strlen(argv[3])+1;                                                   /*  get file_length*/
			sendto(sockfd, &file_len , sizeof(int) , 0 , (struct sockaddr*)&groupSock, sizeof(groupSock));         /*  send to client via group sent, with file_len */
			sendto(sockfd, argv[3] , file_len*sizeof(char) , 0 , (struct sockaddr*)&groupSock, sizeof(groupSock)); /*  send to client via group sent, with file_name*/
			fp=fopen(argv[3],"rb");                                                         /*  read in file*/
			
            if(NULL == fp)                                                                  /*  no input file detected, print error*/
			{
				error("Error no such file\n");
			}

			while(1)
			{                                                                               /*  send the file 512 bytes per time */
				fread(sendbuf, sizeof(unsigned char), 512, fp);
				if(feof(fp))break;
				    byte_num++;                                                                 /*  byte ++ */
			}
				
			int converted2 = htonl(byte_num);
			logperfive = byte_num*0.05 ;                                                    /*  get process */
																								
			sendto(sockfd, &converted2, sizeof(int), 0 , (struct sockaddr*)&groupSock, sizeof(groupSock));   /*  sent to server via group sent */

			rewind(fp);                                                                         /*  read the file again*/

			while(1)
			{
				fread(sendbuf, sizeof(unsigned char), 512 , fp);                                  /*  read file btye 512 oer 512 */
				/*sendto(sockfd,sendbuf,8*sizeof(unsigned char), 0 , (struct sockaddr*)&groupSock, sizeof(groupSock));  ±NÀÉ®×€À§å¶Ç°eµ¹±µŠ¬ºÝ*/
				sendto(sockfd,sendbuf,sizeof(unsigned char), 0 , (struct sockaddr*)&groupSock, sizeof(groupSock));/*  ±NÀÉ®×€À§å¶Ç°eµ¹±µŠ¬ºÝ*/
				/*ret=recvfrom(sockfd , recvbuf , 8*sizeof(unsigned char), 0, NULL , 0);        «ùÄò±µŠ¬ ¶Ç°eºÝ°ešÓªº žê®Æ*/

				byte++;                                                                         /*  ­pºâ€w¶Ç°eŠh€Öbyte*/
				logcount++;                                                                     /*  logcount ¥ÎšÓ­pºâ¬O§_€wžg¬° logperfive*/
				if(logcount == logperfive && log != 100)                                        /*  ·ílogcount ¬° logperfive ®É €j¬ù€w¶Ç°e 5% ªºdata*/
				{
					logcount = 0;                                                               /*  ±N logcount Âk¹s ­«·s­pºâ*/
					time(&now);                                                                 /*  šú±o¥Ø«e¹qž£®É¶¡*/
					//printf("send %d %% data %s" , log+=5 , ctime(&now));                        /*  ŠL¥X log ¥Ø«e€w¶Ç°eŠh€Ö%ªºdata ¥H€Î ¥Ø«eªº¹qž£®É¶¡*/
				}
				if(feof(fp))break;
			}
				
            long sizeInByte = ftell(fp);
            long sizeInMB = sizeInByte / (1024 * 1024);
            printf("file size: %ldMB\n", sizeInMB);
            /*printf("byte_num is %d\n", byte_num);
            //printf("byte is %d\n", byte);
            //printf("ftell: %ld\n", ftell(fp));*/
            printf("The file was sent successfully\n");
			close(sockfd);
		}
	}
	
	else if(!strcmp(argv[1],"client"))													/* client receive*/
	{
		if(!strcmp(argv[2],"multicast"))												/* if it is a multicast*/
		{
		//	printf("input your local ip address\n");													/* clent's ip address */
		//	scanf("%s",ip);
			
			int byte = 0, byte_num ,file_len , loc , ret , lose=0, package_left=0, byte_left=0, newMB=0;								/* byte_num: total numOfByte of file*/
																									/* file_len: received file length*/
																									/* byte: transfered byte*/
																									/* n  Àò±o write() read() ªº return value*/
																									/*  loc ¥ÎšÓ³B²z±µŠ¬šìªºÀÉ®×ŠWºÙ¡AšÒŠp:a.txt ÅÜŠš a_receive.txt*/
																									/*  ret Àò±o recvfrom() ªº return value*/
																									/*  lose: record of there is packeage lost*/
                                                                                                    /* package_left: numOfPackage which success transfer */
			struct timeval timeout={0,1};                                                       /*  control recvform() timeout range --> 1 microsecond*/

			char file_name[512],file_name_rev[512],ch[9]="_receive",ch2[50];                    /*   file_name: file name from sender */
																									/*  file_name_rev: change file name received */
																									/*  ch,ch2: add 'received' to file received */
				
				
				
			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
                perror("Opening datagram socket error");                  /* receive file form server*/
				exit(1);
            }
            else
                printf("Opening datagram socket....OK.\n");
			int reuse = 1;																			/*¶}±Ò SO_REUSEADDR*/
			if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
			{
				error("Setting SO_REUSEADDR error");
				close(sockfd);
				exit(1);
			}
            else 
                printf("Setting SO_REUSEADDR...OK.\n");
				
			/* binding */ 
			memset((char *) &localSock, 0, sizeof(localSock));										
			localSock.sin_family = AF_INET;
			localSock.sin_port = htons(4321);
			localSock.sin_addr.s_addr = INADDR_ANY;
			if(bind(sockfd, (struct sockaddr*)&localSock, sizeof(localSock)))
			{
				error("Binding datagram socket error");
				close(sockfd);
				exit(1);
			}
            else
                printf("Binding datagram socket...OK.\n");
				
			group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
			group.imr_interface.s_addr = inet_addr("127.0.0.1");
			if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
			{
				error("Adding multicast group error");
				close(sockfd);
				exit(1);
			}
            else
                printf("Adding multicast group...OK.\n");
				
            printf("Reading datagram message...OK.\n");
            //datalen = sizeof(databuf);
            //bzero(recvbuf, 512);
			//sendto(sockfd, &reuse, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            recvfrom(sockfd, &file_len, sizeof(int), 0, NULL , 0);                                    /*  recvfrom()  ±µŠ¬¶Ç°eºÝ ±ý¶Ç°eªºÀÉ®×ŠWºÙªø«×*/
			recvfrom(sockfd, file_name, file_len*sizeof(char), 0, NULL , 0);                         /*  recvfrom()  ±µŠ¬¶Ç°eºÝ ±ý¶Ç°eªºÀÉ®×ŠWºÙ*/

			/* change file name*/
			memset(file_name_rev,'\0',512);                                             
			loc = strchr(file_name,'.') - file_name;                                    
			strncpy(file_name_rev,file_name,loc);                                       
			strcat(file_name_rev,ch);                                                   
			memset(ch2,'\0',50);                                                       
			memcpy(ch2,file_name+loc,strlen(file_name)-loc);                            
			strcat(file_name_rev,ch2);                                                  

			fp = fopen(file_name_rev,"wb");                                             /*  write file */
			if(NULL == fp)
			{
				error("Error\n");                                                       
			}

//			printf("%ld",ftell(fp));
            recvfrom(sockfd, &byte_num, sizeof(int), 0, NULL , NULL);                   /*  receive from server */

			setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));         /*  ±±šî recv()ªº¶W®É*/

			while(byte <= byte_num)                                                     /*  if byte not reache the byte_num */
			{
				ret = recvfrom(sockfd , recvbuf , sizeof(recvbuf), 0, NULL , NULL);       /*  ret: check if recrive from server or not */
				/*ret=recvfrom(sockfd , recvbuf , 8*sizeof(unsigned char), 0, NULL , 0);         «ùÄò±µŠ¬ ¶Ç°eºÝ°ešÓªº žê®Æ*/

				if(ret == -1)                                                             /*  detected there is lost package*/
				{
					lose ++;                                                             /* increase lost byte*/
					if(lose > 100)
                        break;
				}
				else
				{
					fwrite(recvbuf, sizeof(unsigned char), strlen(recvbuf), fp);        /*  write data to file */
					byte++;                                                             /*  increase byte num */
				}
			}
            
            /*long sizeInByte2 = ftell(fp);
			long sizeInMB2 = sizeInByte2 / (1024 * 1024);*/
            if(!lose){
                printf("The file was received successfully\n");                    /* if not lose detected, then print success */
                /*printf("file size :%ldMB\n", sizeInMB2);*/
            }
            else{ 
                //printf("Total packet : %d ,Packet lose : %d\n",byte_num, byte);  /* print lost and received */
                /*package_left = byte_num - byte;
                byte_left = package_left * ftell(fp) / byte_num;
                newMB = sizeInMB2 * package_left / byte_num; 
                printf("file size : %d\n", newMB);*/
                printf("file size is %ldMB\n", ftell(fp)*512/1028/1028);
                /*printf("lose + byte is %d\n", (lose+byte)); */   
            }
            
            //printf("The new file created is %s\n",file_name_rev);
			close(sockfd);
		}
	}
    return 0;
}
