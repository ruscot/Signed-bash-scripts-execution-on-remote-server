#ifndef _CLIENT_FUNCTION_H_
#define _CLIENT_FUNCTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define SIZE 1024

/**
 * Function used to send a file through
 * a socket with id sockfd
 * */
void send_file (FILE *fp, int sockfd);

/**
 * Function used to get a a result
 * from a socket with id sockfd
 * */
void get_result (int sockfd);

#endif //_CLIENT_FUNCTION_H_
