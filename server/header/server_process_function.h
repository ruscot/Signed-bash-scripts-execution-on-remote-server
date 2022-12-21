#ifndef _SERVER_PROCESS_FUNCTION_H_
#define _SERVER_PROCESS_FUNCTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<signal.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "global_var.h"

/**
 * Function used to check if the
 * certificate of the username is valid
 * return 0 if the certificate is valid
 * */
int check_certificate (int sockfd, char *username);

/**
 * Get the username of the connection
 * initialize through the socket id sockfd
 * */
void get_username (int sockfd, char *username);

/**
 * Create a file filename with the content
 * send through a socket id sockfd
 * */
void write_file (int sockfd, char *filename);

/**
 * Execute the file filename and send his
 * result through the socket id sockfd
 * */
void exec_file (char *filename, int sockfd);

/**
 * Check if the file filename (script + signature) has a valid signature
 * if yes split the signature in signature_filename_bin
 * and the script in script_filename
 * return 0 if the signature is valid
 * */
int
check_file_signature (char *username,
                      char *filename,
                      int sockfd,
                      char *signature_filename_txt,
                      char *script_filename,
                      char *signature_filename_bin);

/**
 * Delete the file filename
 * */
void delete_current_file (char *filename);

#endif //_SERVER_PROCESS_FUNCTION_H_
