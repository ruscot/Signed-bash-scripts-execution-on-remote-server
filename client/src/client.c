#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "../header/client_function.h"

int main (int argc, char **argv)
{
  char *script_filename;
  char *signature_filename;
  char username[21];

  if (argc < 4)
    {
      printf ("[!] You have to pass the name of your script in the argument of this program, the signature file, and your username that correspond to your public key name \n");
      printf ("Run command :\n./client <script-name> <signature-file> <username>\n");
      exit (1);
    }
  else
    {
      script_filename = argv[1];
      signature_filename = argv[2];
      strncpy (username, argv[3], 20);
    }

  //Run command to create the file to send with the help of the script bash script_send_signature_and_file.sh
  char *command = malloc (strlen (script_filename) +
                          strlen (username) +
                          strlen (signature_filename) +
                          sizeof ("./script_send_signature_and_file.sh ") +
                          sizeof (" ") * 2 + 1);
  strcpy (command, "./script_send_signature_and_file.sh ");
  strcat (command, signature_filename);
  strcat (command, " ");
  strcat (command, script_filename);
  strcat (command, " ");
  strcat (command, username);

  if (system (command) == -1)
    {
      printf ("[!] Failed to run the command %s \n", command);
      exit (1);
    }
  free (command);

  char *filename = malloc (strlen (username) +
                           sizeof ("/file_to_send") +
                           1);
  strcpy (filename, username);
  strcat (filename, "/file_to_send");

  //Server IP address
  char *ip = "127.0.0.2";
  //Server port
  int port = 8080;
  int e;

  int sockfd;
  struct sockaddr_in server_addr;
  FILE *fp;
  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      printf ("[!] Error in socket\n");
      exit (1);
    }
  printf ("[#] Socket server creation done \n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr (ip);

  //Connect to socket server
  e = connect (sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr));
  if (e == -1)
    {
      printf ("[!] Error in while connecting to server \n");
      exit (1);
    }
  printf ("[#] Connected to server\n");

  //Check is the connection is accepted by the server
  char buffer[2000] = {0};
  if (recv (sockfd, buffer, 2000, 0) < 0)
    {
      printf ("[!] Error while receiving server response \n");
    }
  if (strcmp ("Connection accepted", buffer) != 0)
    {
      printf ("[!] Error server is full try later \n");
    }

  //Send username to the server
  char data[SIZE];
  memset (data, '\0', sizeof (username));
  strncpy (data, username, 20);
  if (send (sockfd, data, SIZE, 0) == -1)
    {
      printf ("[!] Error in sending data\n");
      exit (1);
    }
  printf ("[#] Username send successfully \n");
  if (send (sockfd, "END", sizeof ("END"), 0) == -1)
    {
      printf ("[!] Error in sending data\n");
      exit (1);
    }
  printf ("[#] Username send END successfully \n");

  if (recv (sockfd, buffer, 2000, 0) < 0)
    {
      printf ("[!] Error while receiving server response \n");
    }
  if (strcmp ("Good username", buffer) != 0)
    {
      printf ("[!] Error wrong username send\n");
    }


  //Send script with signature to the server
  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      printf ("[!] Error while reading file \n");
      exit (1);
    }
  send_file (fp, sockfd);
  printf ("[#] Data send successfully to the server \n");

  //Get the result from the server
  get_result (sockfd);

  //Delete all temporary file
  if (remove (filename) == 0)
    {
      printf ("[#] Deleted successfully\n");
    }
  else
    {
      printf ("[!] Error unable to delete the file %s\n", filename);
    }

  if (rmdir (username) == 0)
    {
      printf ("[#] Deleted successfully\n");
    }
  else
    {
      printf ("[!] Error unable to delete the directory %s\n", username);
    }

  //Free data and close socket connection
  free (filename);
  close (sockfd);
  printf ("[#] Disconnected from the server \n");

  return 0;

}