#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#define SZ 1000

int main(int argc, char **argv)
{
    int fd;			/* file descriptor for file	*/
    int sockfd;			/* socket file descriptor	*/
    int n;			/* amount of sent/read symbols	*/
    int i;			/* loop counter			*/
    char sendline[SZ];		/* buffer for sending line	*/
    struct sockaddr_in servaddr;/* structure for server address	*/

    /* Check existence of command line arguments		*/
    if(argc != 4)
    {
        printf("Usage: %s <FILENAME> <SERVER IP> <SERVER PORT>\n", *argv);
	exit(1);
    }

    /* Try to open file for reading				*/
    if((fd = open(*(argv + 1), O_RDONLY)) < 0)
    {
        perror(NULL);
	exit(1);
    }
    
    /* Create TCP-socket					*/
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror(NULL);
	close(fd);
	exit(1);
    }

    /* Fill structure for server address:			*/
    /* TCP/IP protocols family, IP and port - from command line	*/
    /* arguments						*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if(atoi(*(argv + 3)) == 0 ||
		    atoi(*(argv + 3)) > 65535)
    {
        printf("Invalid port number\n");
	close(sockfd);
	close(fd);
	exit(1);
    }
    else
    {
        servaddr.sin_port = htons(atoi(*(argv + 3)));
    }
    if(inet_aton(*(argv + 2), &servaddr.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
	close(sockfd);
	close(fd);
	exit(1);
    }

    /* Establish logical connect with server via created socket	*/
    if(connect(sockfd, (struct sockaddr *) &servaddr,
			    sizeof(servaddr)) < 0)
    {
        perror(NULL);
	close(sockfd);
	exit(1);
    }

    while((n = read(fd, sendline, SZ)) > 0)
    {
        if((n = write(sockfd, sendline, n)) < 0)
	{
	    perror("Can't write to socket\n");
	    close(sockfd);
	    close(fd);
	    exit(1);
	}
    }

    return 0;
}
