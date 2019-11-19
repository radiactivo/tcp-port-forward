#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>

#define DIE(msg) perror(msg); exit(1);
#define P_MSG9_S 34
#define P_MSG7_S 25
#define T 2

struct connect_info {
	int client_socket;
	int forward_socket;
	unsigned char pass;
	pthread_mutex_t lock;
};

char passive_msg_9[P_MSG9_S] = "229 Entering Extended Passive Mode";
char passive_msg_7[P_MSG7_S] = "227 Entering Passive Mode";

void com(struct connect_info * sockets);
int open_forwarding_socket(char *forward_name, int forward_port);
void accept_connection(int server_socket, char *forward_name, int forward_port);
int open_listening_port(int server_port);
void parse_arguments(int argc, char **argv, int *server_port, char **forward_name, int *forward_port);