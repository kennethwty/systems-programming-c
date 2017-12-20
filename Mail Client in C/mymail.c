/* Tsz Yan Wong */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<errno.h>

#define DEFAULT_HOST "smtp.uconn.edu"
#define DEFAULT_PORT 25

/* Email struct is a handy way to organize your email fields, the
 * fields are not dynamically sized so things will get truncated */
typedef struct Email{
        char from[100];
        char to[100];
        char message[1000];
} Email;

/* Payload struct just holds a buffer (you'll have to malloc it
 * before use and free it afterwards) and the buffers size. For
 * use with socket_read */
typedef struct Payload{
        char* buf;
        int sz;
} Payload;

/* Socket_send will just take a socket id and a message and keep
 * trying to send until the whole message has sent. */
int   socket_send(int sid, char* message);

/* Socket read takes an initialized payload struct and will resize
 * the buffer as necessary to hold however much information is read
 * from the socket specified. */
int   socket_read(int sid, Payload* p);

/* Prints human-readable error codes when socket acts up. */
void  checkError();
void  array_initializer(char* array);

int main(int argc, char*argv[]){

        /* Decide on server name and port. */
        int port;
        char* servername;
        servername = (argc > 1) ? argv[1] : DEFAULT_HOST;
        port = (argc > 1) ? atoi(argv[2]) : DEFAULT_PORT;
        struct hostent *server = gethostbyname(servername);
        if (server==NULL){
           perror("Error: no such host\n");
           exit(1);
        }

        /* Establish socket connection with server. */
        struct sockaddr_in serv_addr;
        bzero((char*) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *) server->h_addr,
                  (char *) &serv_addr.sin_addr.s_addr,
                  server->h_length);
        int sockfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sockfd<0) exit(-1);
        serv_addr.sin_port = htons(port);
        int status = connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        checkError(status, __LINE__);
        printf("Connected to %s:%d successfully\n", servername, port);

        /* Get host machine's fully qualified domain name (FQDN),
         * stored in fqdn variable */
        struct addrinfo hints;
        struct addrinfo* info;
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1023);
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;
        getaddrinfo(hostname ,"http", &hints, &info);
        char* fqdn = info[0].ai_canonname;
        printf("%s\n", fqdn);

        /* Now that the connection is made in sockfd (variable holding the sockets
         * file descriptor), gather the information to send to the server, talk to
         * the server to read it, and implement mail protocol here! */
        // TODO implement me!

        /* declare structures*/
        Email e;
        Payload* p;

        /* initialize the arrays inside Email structure */
        array_initializer(e.from);
        array_initializer(e.to);

        /* malloc memory for the Payload and Email */
        p = (Payload*)malloc(sizeof(Payload));
        p->buf = (char*)calloc(sizeof(char), 1000);

        /* EHLO */
        char ehlo_arr[1000];
        strcpy(ehlo_arr, "EHLO ");
        strcat(ehlo_arr, hostname);
        strcat(ehlo_arr, "\n");
        socket_send(sockfd, ehlo_arr);
        socket_read(sockfd, p);
        free(p->buf);

        /* get the from and to email addresses */
        printf("From:");
        scanf("%s", &(e.from));
        printf("To:");
        scanf("%s", &(e.to));

        /* logic: send -> malloc -> receive -> free */

        /* for mail from */
        char mailfrom_arr[1000];
        strcpy(mailfrom_arr, "mail from: ");
        /* add in the email address */
        strcat(mailfrom_arr, e.from);
        strcat(mailfrom_arr, "\n");
        /* send */
        socket_send(sockfd, mailfrom_arr);
        p->buf = (char*)calloc(sizeof(char), 1000);
        socket_read(sockfd, p);
        free(p->buf);

        /* for rcpt to */
        char rcpt_arr[1000];
        strcpy(rcpt_arr, "rcpt to: ");
        strcat(rcpt_arr, e.to);
        strcat(rcpt_arr, "\n");

        socket_send(sockfd, rcpt_arr);
        p->buf = (char*)calloc(sizeof(char), 1000);
        socket_read(sockfd, p);
        free(p->buf);

        char data_arr[] =  "DATA\n";
        /* send the DATA command over */
        socket_send(sockfd, data_arr);
        p->buf = (char*)calloc(sizeof(char), 1000);
        socket_read(sockfd, p);
        free(p->buf);

        /* Subject */
        /* In Jon's lab, we used "Message" only without prompting for the Subject */
        /* But just in case we need it, here is the code that actually takes the first line */
        /* before \n as the subject */
        printf("Subject:");
        char subj[300];
        char temp[300]; // holding subject line and then concat to subj and send it to server
        scanf("%s", &temp); // ask for subject line input
        strcpy(subj, "subject: "); // command "subject: "
        strcat(subj, temp); // combine the command and the content in the subject field
        strcat(subj, "\n");

        /* send the subject over to the server */
        socket_send(sockfd, subj);
        p->buf = (char*)calloc(sizeof(char), 1000);
        socket_read(sockfd, p);
        free(p->buf);

        /* for the Message part as shown by the TA */
        // printf("Subject:");
        char line[1000];
        while(strncmp(".\n", line, 2) != 0)
        {
                fgets(line, 1000, stdin);
                strcat(e.message, line);
        }

        /* finally send the message */
        socket_send(sockfd, e.message);
        p->buf = (char*)calloc(sizeof(char), 1000);
        socket_read(sockfd, p);
        free(p->buf);

        /* quit */
        char quit[] = "quit\n";
        socket_send (sockfd, quit);
        p->buf = (char*)calloc(sizeof(char), 10000);
        socket_read(sockfd, p);
        free(p->buf);

        // Clean up!
        freeaddrinfo(info);
        // close the socket
        close(sockfd);
        // free the Payload
        free(p);
        return 0;
}

int socket_send(int sid, char* message) {
        // TODO implement me!
        size_t length = strlen(message);
        /* check if the entire string is sent */
        int size = 0;
        while(size<length)
        {
                /* keep sending the message over until all gone */
                /* message+size: move the pointer */
                size = size + send(sid, (message+size), length-size, 0);
        }
        return size;
}

int  socket_read(int sid, Payload* p) {
        // TODO implement me!
        int result_byte;
        int buf_len = strlen(p->buf);
        /* read the response from the server and store it into buffer */
        /* check whether the buffer is full, if yes, realloc */
        /* use 0 as flag */
        result_byte = recv(sid, p->buf, buf_len, 0);
        while(result_byte != 0)
        {
                /* check the buffer size */
                if(result_byte = p->sz)
                {
                        p->sz = p->sz * 2;
                        p->buf = (char*) realloc(p->buf, p->sz); /* resize the buffer */
                }
        }

        /* check the buffer */
        // while(!(p->buf[p->sz-1] == "\n"))
        return 0;
}

/* this function initializes the from and to array to 0 */
void array_initializer(char* array) {
        for(int i=0; i<strlen(array); i++)
        {
                array[i] = 0;
        }
}

void checkError(int status, int line) {
        if (status < 0){
                printf("socket error(%d)-%d: [%s]\n", getpid(), line, strerror(errno));
                exit(-1);
        }
}
