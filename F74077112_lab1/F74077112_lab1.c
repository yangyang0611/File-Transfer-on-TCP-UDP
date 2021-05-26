#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    clock_t begin = clock();
    time_t now;

    int sockfd, newsockfd, portno, logperfive, log = -25, logcount = 0; /* log is to get degree on process*/
    socklen_t clilen;

    char sendbuf[512];
    char recvbuf[512];
    char buffer[512];
    //unsigned char sendbuf[512];
    //unsigned char recvbuf[512];
    //unsigned char buffer[512];

    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;

    FILE *fp;

    if (strcmp(argv[1], "tcp") == 0) /* TCP*/
    {
        if (strcmp(argv[2], "send") == 0) /*server send*/
        {
            int request = 0, byte_num = 0, file_len, byte = 0, n; /* request: judge if there is request*/
                                                                  /* byte_num: numOfByte of File*/
                                                                  /* file_len: length of input file_name*/
                                                                  /* byte: trensfered bytes*/
                                                                  /* n: return value of read() write()*/

            sockfd = socket(AF_INET, SOCK_STREAM, 0); /* sockfd: store if there is socket in TCP*/
            if (sockfd < 0)
                error("ERROR opening socket\n"); /* if = 0, no socket*/

            /* binding*/
            bzero((char *)&serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = INADDR_ANY;
            serv_addr.sin_port = htons(portno);
            if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR on binding\n"); /* error when port wrong*/

            /* listen*/
            listen(sockfd, 5);

            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); /* newsockfd: store client accept to receive package or not*/
            if (newsockfd < 0)
                error("ERROR on accept\n"); /* if = 0, client no accept*/

            n = read(newsockfd, &request, sizeof(int)); /* n: store want to read the package or not*/
            if (n < 0)
                error("ERROR reading from socket\n"); /* if = 0. no read*/
            //printf("get");
            //if (request) /*  have request*/
            //{
            file_len = strlen(argv[5]) + 1;               /*  get file_length*/
            n = write(newsockfd, &file_len, sizeof(int)); /*  sent(*write to) client the length of input file*/
            if (n < 0)
                error("ERROR writing to socket\n");
            n = write(newsockfd, argv[5], file_len * sizeof(char)); /*  sent(*write to) client the name of input file*/
            if (n < 0)
                error("ERROR writing to socket\n");
            fp = fopen(argv[5], "rb"); /*  open &read file*/
            if (NULL == fp)
            {
                error("Error\n");
            }

            while (1) /*  read file 1 byte by 1 byte(2's binary)*/
            {
                fread(buffer, sizeof(unsigned char), 512, fp);
                if (feof(fp))
                    break;
                byte_num++; /* read 1 byte then byte_num++*/
            }
            int converted = htonl(byte_num);
            logperfive = byte_num * 0.25; /*  calculate how many byte is 25% of whole document*/

            n = write(newsockfd, &converted, sizeof(int)); /*  sent file size to client*/
            if (n < 0)
                error("ERROR writing to socket\n");

            rewind(fp); /*  read file again*/
            while (1)
            {
                n = fread(buffer, sizeof(unsigned char), 512, fp); /*  read again 1 byte by 1 byte*/
                if (feof(fp))
                    break;
                //printf("%d\n", n);
                n = write(newsockfd, buffer, n); /*  divided in sectors*/
                if (n < 0)
                    error("ERROR writing to socket\n");

                

                byte++;     /*  calculate how much bytes is transfered*/
                logcount++; /*  logcount: store if already logperfive*/

                if (logcount == logperfive && log != 150) /*  reached logperfive*/
                {
                    logcount = 0;                                         /*  initial logcount*/
                    time(&now);                                           /*  get time now*/
                    printf("send %d %% data %s", log += 25, ctime(&now)); /*  print how many data is sent now*/
                }
            }
            clock_t end = clock(); /* end of execution*/
            double timeSpend = (double)(end - begin) / CLOCKS_PER_SEC;
            printf("send 100 %% data %s\n", ctime(&now));
            printf("Total transition time: %fms\n", timeSpend * 1000);
            long sizeInByte = ftell(fp);
            long sizeInMB = sizeInByte / (1024 * 1024);
            printf("file size: %ldmb\n", sizeInMB);
            printf("The file was sent successfully\n");
            //}
            //else /*  client to request any print this*/
            //{
            // printf("No client request file.\n");
            //}
            close(newsockfd);
            close(sockfd);
        }
        else if (strcmp(argv[2], "recv") == 0) /*  TCP client - recv*/
        {
            int request = 1, byte = 0, byte_num, file_len, loc, n; /*  loc: store new file name .txt to _receive.txt*/

            char file_name[512], file_name_rev[512], ch[9] = "_receive", ch2[50]; /* ch2: store name of file*/
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket\n");

            portno = atoi(argv[4]);          /*  port num*/
            server = gethostbyname(argv[3]); /*  IP address*/
            if (server == NULL)
            {
                fprintf(stderr, "ERROR, no such host\n");
                exit(0);
            }

            /* doing conection*/
            bzero((char *)&serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(portno);
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("ERROR connecting\n"); /* judge port is avaiable or not*/

            bzero(buffer, 512); /*  clear buffer to 0*/

            n = write(sockfd, &request, sizeof(int)); /*  write() i need request! to server*/
            if (n < 0)
                error("ERROR writing to socket\n");

            n = read(sockfd, &file_len, sizeof(int)); /* read length of file from server*/
            if (n < 0)
                error("ERROR reading from socket\n");

            n = read(sockfd, file_name, file_len * sizeof(char));
            if (n < 0)
                error("ERROR reading from socket\n");

            /* modify file name, change x.txt to x_receive.txt*/
            memset(file_name_rev, '\0', 512);
            loc = strchr(file_name, '.') - file_name;
            strncpy(file_name_rev, file_name, loc);
            strcat(file_name_rev, ch);
            memset(ch2, '\0', 50);
            memcpy(ch2, file_name + loc, strlen(file_name) - loc);
            strcat(file_name_rev, ch2);

            fp = fopen(file_name_rev, "wb"); /*  write to file*/
            if (NULL == fp)
            {
                error("Error\n");
            }

            n = read(sockfd, &byte_num, sizeof(int)); /*  read in byte*/
            //if (n < 0)
              //  error("ERROR reading from socket\n");

           while (byte != byte_num) /*  kept reading byte until reach file_byte_num*/
           // while (1)
            {
                n = read(sockfd, buffer, 512);
                if (n < 0)
                    error("ERROR reading from socket\n");

                /*if((byte & 255)==1)usleep(1);*/
                fwrite(buffer, sizeof(unsigned char), n, fp); /* write to file*/
                byte+=512;                                       /*  calculate how much byte received now*/
            }
            printf("The file was received successfully\n");
            printf("The new file created is %s\n", file_name_rev);
            close(sockfd);
        }
    }
    else if (strcmp(argv[1], "udp") == 0) /* UDP*/
    {
        if (strcmp(argv[2], "send") == 0) /* server send*/
        {
            int request = 0, byte_num = 0, file_len, byte = 0;

            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                error("socket error"); /* UDP socket()function type*/

            /* binding*/
            bzero((char *)&serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(portno);
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                error("bind error");

            clilen = sizeof(cli_addr);
            recvfrom(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&cli_addr, &clilen);

            
                file_len = strlen(argv[5]) + 1;
                sendto(sockfd, &file_len, sizeof(int), 0, (struct sockaddr *)&cli_addr, clilen);
                sendto(sockfd, argv[5], file_len * sizeof(char), 0, (struct sockaddr *)&cli_addr, clilen);

                fp = fopen(argv[5], "rb");
                if (NULL == fp)
                {
                    error("Error\n");
                }

                while (1)
                {
                    fread(sendbuf, sizeof(unsigned char), 512, fp);
                    if (feof(fp))
                        break;
                    byte_num++;
                }
                
                int converted2 = htonl(byte_num);
                logperfive = byte_num * 0.25;

                sendto(sockfd, &converted2, sizeof(int), 0, (struct sockaddr *)&cli_addr, clilen);

                rewind(fp); /* reread*/

                while (1)
                {
                    fread(sendbuf, sizeof(unsigned char), 512, fp);
                    if (feof(fp))
                        break;
                    sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&cli_addr, clilen);
                    /*if((byte & 127)==1)usleep(1);*/

                    byte++;
                    logcount++;
                    if (logcount == logperfive && log != 100)
                    {
                        logcount = 0;
                        time(&now);
                        printf("send %d %% data %s", log += 25, ctime(&now));
                    }
                }
                clock_t end2 = clock();
                double timeSpend2 = (double)(end2 - begin) / CLOCKS_PER_SEC;
                printf("send 100 %% data %s", ctime(&now));
                printf("Total transition time: %fms\n", timeSpend2 * 1000);
                long sizeInByte2 = ftell(fp);
                long sizeInMB2 = sizeInByte2 / (1024 * 1024);
                printf("file size: %ldmb\n", sizeInMB2);
                printf("The file was sent successfully\n");
            
            //else
            //{
              //  printf("No client request file.\n");
            //}
            close(sockfd);
        }
        else if (strcmp(argv[2], "recv") == 0) /* UDP client recv*/
        {
            int request = 1, byte = 0, byte_num, file_len, loc, ret, lose = 0; /*  ret: get return value of recvfrom()*/

            struct timeval timeout = {0, 1}; /* control recvfrom() timeout range -> 1ms*/

            char file_name[512], file_name_rev[512], ch[9] = "_receive", ch2[50];

            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
                error("socket");

            /* binding*/
            bzero((char *)&serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);
            server = gethostbyname(argv[3]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(portno);
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

            bzero(recvbuf, 512); /* initialize buffer to 0*/

            sendto(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            recvfrom(sockfd, &file_len, sizeof(int), 0, NULL, NULL);
            recvfrom(sockfd, file_name, file_len * sizeof(char), 0, NULL, NULL);

            /* change file name*/
            memset(file_name_rev, '\0', 512);
            loc = strchr(file_name, '.') - file_name;
            strncpy(file_name_rev, file_name, loc);
            strcat(file_name_rev, ch);
            memset(ch2, '\0', 50);
            memcpy(ch2, file_name + loc, strlen(file_name) - loc);
            strcat(file_name_rev, ch2);

            fp = fopen(file_name_rev, "wb"); /*  open & write file*/
            if (NULL == fp)
            {
                error("Error\n");
            }

            recvfrom(sockfd, &byte_num, sizeof(int), 0, NULL, NULL);

            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            while (byte != byte_num)
            {
                ret = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, NULL, NULL);

                if (ret == -1)
                {
                    lose++;
                    if (lose > 100)
                        break;
                }
                else
                {
                    fwrite(recvbuf, sizeof(unsigned char), strlen(recvbuf), fp); /*  write data to file*/
                    byte++;
                }
            }
            if (!lose)
                printf("The file was received successfully\n");
            else
                printf("Total packets: %d   lose %d packets \n", byte_num, byte); /* print package lost*/
            printf("The new file created is %s\n", file_name_rev);
            close(sockfd);
        }
    }
    return 0;
}
