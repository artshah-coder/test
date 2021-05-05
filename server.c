#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#define SZ 1000

static int fd;			/* descriptor for output file				*/
static int sockfd, newsockfd;	/* descriptors for listening and accepted sockets	*/

static void daemonization(void);	/* prototype for process demonization function	*/
static void my_handler(int);		/* prototype for custom signal handler		*/

int main()
{
    int clilen;			/* client address length				*/
    int n;			/* number of recieved symbols				*/
    char line[SZ];		/* buffer for recieved symbols				*/
    struct sockaddr_in servaddr;/* structure for storing server address			*/

    /* daemonize the process				*/
    daemonization();

    /* try to create output file			*/
    if((fd = open("output_file", O_CREAT | O_WRONLY)) < 0)
    {
        perror(NULL);
	exit(1);
    }
    
    /* create TCP-socket				*/
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(NULL);
	close(fd);
	exit(1);
    }

    /* fill server address structure:			*/
    /* TCP/IP protocols family, any network		*/
    /* interface, port number is 60000			*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(60000);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind socket with server address			*/
    if(bind(sockfd, (struct sockaddr *) &servaddr,
			    sizeof(servaddr)) < 0)
    {
        perror(NULL);
	close(sockfd);
	close(fd);
	exit(1);
    }

    /* put the created socket into a listening state	*/
    if(listen(sockfd, 5) < 0)
    {
        perror(NULL);
	close(sockfd);
	close(fd);
	exit(1);
    }

    /* the main server's loop				*/
    while(1)
    {
        /* wait a full established connect		*/
        if((newsockfd = accept(sockfd, NULL, NULL)) < 0)
	{
	    perror(NULL);
	    close(sockfd);
	    close(fd);
	    exit(1);
	}
	/* recieve information from client while	*/
	/* any error will arise (syscall read() returns */
	/* negative value) or client will close		*/
	/* connection (syscall read() returns zero)	*/
	while((n = read(newsockfd, line, SZ)) > 0)
	{
	    write(fd, line, n);
	}
	/* handle error, if it arised in reading process*/
	if(n < 0)
	{
	    perror(NULL);
	    close(sockfd);
	    close(newsockfd);
	    close(fd);
	    exit(1);
	}
        /* close newsockfd				*/
	close(newsockfd);
    }

    return 0;
}

static void daemonization(void)
{
    pid_t pid;

    /* fork off the parent process			*/
    if((pid = fork()) < 0)
    {
        exit(1);
    }
    /* terminate the parent process			*/
    if(pid > 0)
    {
        exit(0);
    }

    /* on success: the child becomes session leader	*/
    if(setsid() < 0)
    {
        exit(1);
    }

    /* handle SIGTERM and SIGHUP signals		*/
    signal(SIGTERM, my_handler);
    signal(SIGHUP, my_handler);

    /* fork off for the second time			*/
    if((pid = fork()) < 0)
    {
        exit(1);
    }
    if(pid > 0)
    {
        exit(0);
    }

    /* set new file permissions				*/
    umask(0);

    /* change working directory				*/
    chdir("/home/");

    /* close all open file descriptors			*/
    int x;
    for(x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
    {
        close(x);
    }
}

static void my_handler(int nsig)
{
    close(fd);
    close(sockfd);
    close(newsockfd);
    exit(0);
}
