#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<signal.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "../header/server_process_function.h"

//Number of process runnning
int fork_pool_number = 0;
//Used to control the processus created
sem_t mutex;

void create_process_for_program (int sockfd)
{
  //Check if we can create new process
  if (fork_pool_number < MAX_FORK)
    {
      char buffer[SIZE];
      printf ("[#] Server socket send response. \n");
      strncpy (buffer, "Connection accepted", sizeof ("Connection accepted"));
      send (sockfd, buffer, SIZE, 0);
      bzero (buffer, SIZE);
      //We create our new process
      if (fork () == 0)
        {
          //In the new process
          char filename[1000];
          snprintf (filename, 1000, "%d", sockfd);

          char username[21];
          username[0] = '\0';
          //Get the username of the client
          get_username (sockfd, username);
          if (username[0] == '\0')
            {
              //If the username is not good end connection
              strncpy (buffer, "Wrong username end connection", sizeof ("Wrong username end connection"));
              printf ("[#] Server socket send response \"Wrong username end connection\" \n");
              send (sockfd, buffer, SIZE, 0);
              bzero (buffer, SIZE);
              printf ("[#] Server socket send response \"Wrong username end connection\" END\n");
              if (send (sockfd, "END", sizeof ("END"), 0) == -1)
                {
                  printf ("[!] Error in sending data\n");
                  exit (1);
                }

              printf ("[#] Send end to client\n");
            }
          else
            {
              //We have the good username
              strncpy (buffer, "Good username", sizeof ("Good username"));
              printf ("[#] Server socket send response \"Good username\" \n");
              send (sockfd, buffer, SIZE, 0);
              bzero (buffer, SIZE);
              printf ("[#] Server socket send response \"Good username\" END\n");

              //Get the script with the signature
              write_file (sockfd, filename);

              //Create filename to split the signature and the script
              char *signature_filename_txt = malloc (sizeof ("signature_file/signature-") +
                                                     strlen (username) +
                                                     sizeof (".txt") +
                                                     1);
              strcpy (signature_filename_txt, "signature_file/signature-");
              strcat (signature_filename_txt, username);
              strcat (signature_filename_txt, ".txt");

              char *signature_filename_bin = malloc (sizeof ("signature_file/signature-") +
                                                     strlen (username) +
                                                     sizeof (".bin") +
                                                     1);
              strcpy (signature_filename_bin, "signature_file/signature-");
              strcat (signature_filename_bin, username);
              strcat (signature_filename_bin, ".bin");

              char *script_filename = malloc (sizeof ("script-") +
                                              strlen (username) +
                                              sizeof (".sh") +
                                              1);
              strcpy (script_filename, "script-");
              strcat (script_filename, username);
              strcat (script_filename, ".sh");

              char *client_public_key = malloc (sizeof ("pub_keys/public-key-") +
                                                strlen (username) +
                                                sizeof (".pem") +
                                                1);
              strcpy (client_public_key, "pub_keys/public-key-");
              strcat (client_public_key, username);
              strcat (client_public_key, ".pem");

              //Check if the certificate is valid
              if (check_certificate (sockfd, username) == 0)
                {
                  //Certificate of the client valid now we check the signature
                  if (check_file_signature (username,
                                            filename,
                                            sockfd,
                                            signature_filename_txt,
                                            script_filename,
                                            signature_filename_bin) == 0)
                    {
                      //The signature is valid now we run the script and send the result to the client
                      exec_file (script_filename, sockfd);

                      //Delete all temporary file we have used
                      delete_current_file (signature_filename_bin);
                      delete_current_file (signature_filename_txt);
                      delete_current_file (script_filename);
                    }
                }

              delete_current_file (client_public_key);

              free (client_public_key);
              delete_current_file (filename);
              if (send (sockfd, "END", sizeof ("END"), 0) == -1)
                {
                  printf ("[!] Error in sending data\n");
                  exit (1);
                }

              printf ("[#] Send end to client\n");

              free (signature_filename_txt);
              free (script_filename);
              free (signature_filename_bin);
            }

          //Decrement the number of process running
          sem_wait (&mutex);
          fork_pool_number--;
          sem_post (&mutex);
          close (sockfd);
        }
      else
        {
          //incr the number of process running
          sem_wait (&mutex);
          fork_pool_number++;
          sem_post (&mutex);
        }
    }
  else
    {
      //We can't create new process connection refused
      char buffer[SIZE];
      strncpy (buffer, "Connection refused", sizeof ("Connection refused"));
      printf ("[#] Server socket send response. \n");
      send (sockfd, buffer, SIZE, 0);
      if (send (sockfd, "END", sizeof ("END"), 0) == -1)
        {
          printf ("[!] Error in sending data\n");
          exit (1);
        }

      printf ("[#] Send end to client\n");
      close (sockfd);
    }
}

int main ()
{
  //Init of the mutex
  sem_init (&mutex, 1, 1);
  char *ip = "127.0.0.2";
  int port = 8080;
  int e;

  int sockfd, new_sock;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;

  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      printf ("[!] Error in socket\n");
      exit (1);
    }
  printf ("[#]Server socket created. \n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr (ip);

  //Bind our socket to our server address
  e = bind (sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr));
  if (e < 0)
    {
      printf ("[!] Error in Binding");
      exit (1);
    }
  printf ("[#] Binding Successfull.\n");

  while (1)
    {
      e = listen (sockfd, 10);
      if (e == 0)
        {
          printf ("[#] Listening...\n");
        }
      else
        {
          printf ("[!] Error in Binding\n");
          exit (1);
        }
      addr_size = sizeof (new_addr);
      //Accept new socket connection
      new_sock = accept (sockfd, (struct sockaddr *) &new_addr, &addr_size);

      create_process_for_program (new_sock);
    }
}