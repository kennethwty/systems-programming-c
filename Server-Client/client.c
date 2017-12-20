/* Client */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#define DEFAULT_HOST "localhost"

/* Check error */
void checkError(int status)
{
        if (status < 0)
        {
                printf("socket error: [%s]\n",strerror(errno));
                exit(-1);
        }
}

/* Send function */
/* From Lab 7 */
int socket_send(int sock_id, char* cmd, int cmd_len)
{
        int sent = 0; // Bytes we've sent out
        //int remain_byte = cmd_len;
        int current;
        /* Send the data out */
        while(sent < cmd_len)
        {
                current = send(sock_id, (cmd+sent), cmd_len-sent, 0);
                checkError(current); /* Check if bytes sent is > 0 */
                sent = sent + current;
                //remain_byte = remain_byte - current;
        }
        return sent; /* return 0 if sent successfully */
}

int main(int argc,char* argv[])
{
        /* Check the number of arguments to determent the name of the host */

        //if(argc != 2)
        //{
        //      printf("\n Usage: %s <hostname> \n", argv[0]);
        //      return 1;
        //}

        /* Takes the default hostname defined above if the user forgets to enter one */
        char *servername = (argc > 1) ? argv[1] : DEFAULT_HOST;

        /* Create a socket */
        int sid = socket(PF_INET,SOCK_STREAM,0);
        struct sockaddr_in srv;
        struct hostent* server = gethostbyname(servername); /* hostname */
        // srv.sin_len = sizeof(srv); not working with this statement
        srv.sin_family = AF_INET;
        srv.sin_port = htons(8025); /* Port number 8025 */
        memcpy(&srv.sin_addr.s_addr,server->h_addr,server->h_length);

        /* Connect the server and check whether it is successful */
        int status = connect(sid,(struct sockaddr*)&srv,sizeof(srv));
        checkError(status);

        /* for Select() */
        fd_set readfds;
        struct timeval timeout;

        /* Read the command from standard input */
        while(1)
        {
                /* get the command from STDIN */
                char cmd[1000];
                //printf("Command: ");
                fgets(cmd, 1000, stdin);
                strcat(cmd, "\0");

                /* select */
                FD_ZERO(&readfds);
                FD_SET(sid, &readfds);
                /* set the timeout */
                timeout.tv_sec = 0;
                timeout.tv_usec = 100000; // wait for 0.1 second as stated in Q&A

                //memcpy(&readfds, sid, sizeof(readfds));
                int active; // -1: error, 0: none of them is ready, 1: ready
                socket_send(sid, cmd, strlen(cmd));

                /* wait for output from server, if none (touch hello.txt), continue to next loop */
                /* 10 max fds, only for read */
                active = select(10, &readfds, NULL, NULL, &timeout);
                if(active==0) //none is ready
                {
                        //printf("Timeout");
                        continue; // if timeout continue to the next loop for next cmd
                        //break;
                } else if(active>0){ //ready

                        /* Create a buffer to hold the data from the server */
                        char* buf= (char*)calloc(sizeof(char), 1000);
                        int buf_len = 1000;

                        /* Read */
                        read(sid, buf, buf_len);
                        printf("%s", buf); // print the output
                        free(buf); // free the buffer

                        /* brute force way of comparing the "exit" */
                        if(cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't')
                        {
                                return 0;
                        }
                } else{ // active < 1
                        printf("select() failed");
                        break;
                }
        }
        close(sid);
        return 0;
}
