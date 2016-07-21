#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netinet/tcp.h>
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/time.h>
#include <fcntl.h>


#define BUF_SIZE 65535
#define READ_LOOPS 100

void die(char *errorMessage)
{
  perror(errorMessage);
  exit(1);
}

char* concat(char *s1, char *s2, char *s3)
{
    char *result = malloc(strlen(s1)+strlen(s2)+2);
    if(strlen(s1) > 0)
       strcpy(result, s1);
    if(strlen(s2) > 0)
       strcat(result, s2);
    if(strlen(s3) > 0)
       strcat(result, s3);
    return result;
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char servIP[10] = "127.0.0.1";
    char *ret,*msgnew;
    int value = 1;

    if (argc < 2){
        fprintf(stderr, "Usage: %s <Port> <Msg>\n", argv[0]);
        exit(1);
    }

    // Convert all *argv after port into one string
    char *msg = concat("", argv[2], " ");
    for( int i = 3; i < argc; i++ ){
       if(i < argc-1)
          msgnew = concat(msg, argv[i], " ");
       else
          msgnew = concat(msg, argv[i], "");
       if(i > 3)
          free(msg);
       msg=msgnew;
    }

    echoServPort = atoi(argv[1]);
    ret = malloc(BUF_SIZE);

    //printf("%d : %s\n",strlen(msg),msg);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        die("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family      = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port        = htons(echoServPort);

    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        die("connect() failed");

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof(int)) < 0)
       die("TCP_NODELAY failed");

//    fcntl(sock, F_SETFL, O_NONBLOCK);

    if (send(sock, msg, strlen(msg), 0) != strlen(msg))
       die("send() failed to send message ");

    /* Give the server a chance */
    usleep(100);

    int loop = 0;
    while(1)
    {
        if(loop ++ > READ_LOOPS) 
            break;
        usleep(10);
        memset(ret,0,BUF_SIZE);
        const int result = recv(sock, ret, BUF_SIZE, 0);
        if(result == -1)
        {
        //    break;
        }
        else if(result == 0)
        {
        //    break;
        }
        if(result > 0){
          printf("%s\n", ret);
        }
    }

    close(sock);
    free(ret);
    exit(0);
}
