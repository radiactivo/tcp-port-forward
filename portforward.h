#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>

#define DIE(msg) perror(msg); exit(1);

void com_fuzz(int src, int dst);
void com(int src, int dst);
int open_forwarding_socket(char *forward_name, int forward_port);
void accept_connection(int server_socket, char *forward_name, int forward_port);
int open_listening_port(int server_port);
void parse_arguments(int argc, char **argv, int *server_port, char **forward_name, int *forward_port);