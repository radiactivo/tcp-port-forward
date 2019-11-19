/* Licensed under MIT
 * Original source: https://github.com/antialize/utils/blob/master/portforward.c
 * It was later licensed:  https://github.com/antialize/utils/issues/1
 */
#include "portforward.h"
/**/

/**
 * Send the traffic from src socket to dst socket fuzzing the incoming
 * content of the packet with radamsa
 * When done or on any error, this dies and will kill the forked process.
 */
void com(struct connect_info * sockets) 
{
    char buf[1024 * 4];
    char * c = NULL;
    int r=0, i=0, j=0, src, dst;

    //Critical Section 1
    pthread_mutex_lock(&sockets->lock);

    if (sockets->pass == 0) 
    {
        src = sockets->client_socket;
        dst = sockets->forward_socket;
        sockets->pass = '\x01';
    }
    else
    {
        src = sockets->forward_socket;
        dst = sockets->client_socket;
    }

    pthread_mutex_unlock(&sockets->lock);
    r = read(src, buf, 1024 * 4);

    while (r > 0) 
    {
        c = strstr(buf, passive_msg_7);
    
        if ( c != NULL )
        {
            //Open again two threads for the passive mode
           fprintf(stdout,"Never expected ha?\n");
           fflush(stdout);
        }
        i = 0;

        fprintf(stdout, "%s\n", buf );
        fflush(stdout);
        while (i < r) 
        {
            j = write(dst, buf + i, r - i);

            if (j == -1)
            {
                DIE("write"); // TODO is errno EPIPE
            }

            i += j;
        }

        r = read(src, buf, 1024 * 4);
    }

    if (r == -1)
    {
        DIE("read");
    }

    shutdown(src, SHUT_RD);
    shutdown(dst, SHUT_WR);
    close(src);
    close(dst);
    exit(0);
}

/**
 * Opens a connection to the destination.
 *
 * On any error, this dies and will kill the forked process.
 */
int open_forwarding_socket(char *forward_name, int forward_port) 
{
    int forward_socket;
    struct hostent *forward;
    struct sockaddr_in forward_address;

    forward = gethostbyname(forward_name);

    if (forward == NULL)
    {
        DIE("gethostbyname");
    }

    bzero((char *) &forward_address, sizeof(forward_address));
    forward_address.sin_family = AF_INET;
    bcopy((char *)forward->h_addr, (char *) &forward_address.sin_addr.s_addr, forward->h_length);
    forward_address.sin_port = htons(forward_port);

    forward_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (forward_socket == -1)
    {
        DIE("socket");
    }

    if (connect(forward_socket, (struct sockaddr *) &forward_address, sizeof(forward_address)) == -1)
    {
        DIE("connect");
    }

    return forward_socket;
}


/**
 * Forwards all traffic from the client's socket to the destination
 * host/port.  This also initiates the connection to the destination.
 */
void forward_traffic(int client_socket, char *forward_name, int forward_port)
{
    int forward_socket, i=0, err =0;
    pthread_t tid[T];
    pid_t down_pid;
    struct connect_info * sockets;

    if( pthread_mutex_init( (pthread_mutex_t *restrict) &sockets->lock, (const pthread_mutexattr_t *restrict) NULL) )
    {
        DIE("mutex");
    }

    forward_socket = open_forwarding_socket( forward_name, forward_port );
    sockets = (struct connect_info *) calloc(1, sizeof(struct connect_info));
    // Fork - child forwards traffic back to client, parent sends from client
    // to forwarded port ====> The parent process will spawn two threads, 
    // one for senc back to client other to send from client to forwarded port
    sockets->client_socket = client_socket;
    sockets->forward_socket = forward_socket;



    while(i < 2)
    {
        err = pthread_create( &tid[i], NULL, (void *) &com, (void *) sockets );
        if (err != 0)
        {
            DIE("thread");
        }
        i++;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);

    free(sockets);
}

/**
 * Handles a single connection.  Will fork in order to handle another
 * connection while the child starts to forward traffic.
 */
void accept_connection(int server_socket, char *forward_name, int forward_port)
{
    int client_socket;
    pid_t up_pid;

    client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1)
    {
        DIE("accept");
    }

    // Fork - Child handles this connection, parent listens for another
    up_pid = fork();

    if (up_pid == -1)
    {
        DIE("fork");
    }

    if (up_pid == 0)
    {
        forward_traffic(client_socket, forward_name, forward_port);
        exit(1);
    }

    close(client_socket);
}

/**
 * Opens the listening port or dies trying.
 */
int open_listening_port(int server_port) 
{
    struct sockaddr_in server_address;
    int server_socket, bind_socket;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) 
    {
        DIE("socket");
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) 
    {
        DIE("bind");
    }

    if ( listen(server_socket, 40) == -1) { DIE("listen"); }

    return server_socket;
}

/**
 * Argument parsing and validation
 */
void parse_arguments(int argc, char **argv, int *server_port, char **forward_name, int *forward_port) 
{
    if (argc < 3) 
    {
        fprintf(stderr, "Not enough arguments\n");
        fprintf(stderr, "Syntax:  %s listen_port forward_host [forward_port]\n", argv[0]);
        exit(1);
    }

    *server_port = atoi(argv[1]);

    if (*server_port < 1 | *server_port > 65000) 
    {
        fprintf(stderr, "Listen port is invalid\n");
        exit(1);
    }

    *forward_name = argv[2];

    if (argc == 3) 
    {
        *forward_port = *server_port;
    }
    else
    {
        *forward_port = atoi(argv[3]);

        if (*forward_port < 1 | *forward_port > 65000) 
        {
            fprintf(stderr, "Forwarding port is invalid\n");
            exit(1);
        }
    }
}


/**
 * Coordinates the effort
 */
int main(int argc, char **argv) 
{
    int server_port, forward_port, server_socket;
    char *forward_name;

    parse_arguments(argc, argv, &server_port, &forward_name, &forward_port);
    signal(SIGCHLD,  SIG_IGN);
    server_socket = open_listening_port(server_port);

    while (1) 
    {
        accept_connection(server_socket, forward_name, forward_port);
    }

    return 0; 
}
