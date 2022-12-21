//
// Created by anthony.martinez on 12/19/2022.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include "../header/server_process_function.h"

void get_username (int sockfd, char *username)
{
  int n;
  char buffer[SIZE];
  printf ("[#] Client get username\n");
  while (1)
    {
      n = recv (sockfd, buffer, SIZE, 0);
      if (n <= 0)
        {
          break;
          return;
        }
      else if (strcmp (buffer, "END") == 0)
        {
          break;
        }
      memset (username, '\0', sizeof (username));
      strncpy (username, buffer, 20);
      bzero (buffer, SIZE);
    }
  printf ("[#] Client username get successfully\n");
  return;
}

void write_file (int sockfd, char *filename)
{
  printf ("[#] Server write file %s\n", filename);
  int n;
  FILE *fp;
  char buffer[SIZE];

  fp = fopen (filename, "w");
  if (fp == NULL)
    {
      printf ("[!] Error in creating file \n");
      exit (1);
    }
  while (1)
    {
      n = recv (sockfd, buffer, SIZE, 0);
      if (n <= 0)
        {
          break;
          return;
        }
      else if (strcmp (buffer, "END") == 0)
        {
          break;
        }
      fprintf (fp, "%s", buffer);
      bzero (buffer, SIZE);
    }
  fclose (fp);
  printf ("[#] Server write file %s END\n", filename);
  return;
}

int check_certificate (int sockfd, char *username)
{
  printf ("[#] Server check certificate\n");
  //Certificate filename
  char *cert_file = malloc (strlen (username) +
                            sizeof ("certificate/cert-") +
                            sizeof (".pem"));
  strcpy (cert_file, "certificate/cert-");
  strcat (cert_file, username);
  strcat (cert_file, ".pem");
  //Check if the certificate file exist
  if (access (cert_file, 0) != 0)
    {
      printf ("[!] No certificate file for user\n");
      char *response = "You have no certificate on the server\n";
      if (send (sockfd, response, strlen (response), 0) == -1)
        {
          printf ("[!] Error in sending data\n");
        }
      printf ("[!] No certificate file for user send\n");
      free (cert_file);
      return 1;
    }

  //Command to check if the certificate is correct using the bash script check_cert.sh
  char *command = malloc (strlen (username) +
                          strlen (cert_file) +
                          sizeof ("./check_cert.sh ") +
                          sizeof (" "));
  strcpy (command, "./check_cert.sh ");
  strcat (command, cert_file);
  strcat (command, " ");
  strcat (command, username);

  //Execute the command
  FILE *fp;
  /* Open the command for reading. */
  fp = popen (command, "r");
  if (fp == NULL)
    {
      printf ("[!] Error while reading file \n");
      char *response = "Error with program run";
      send (sockfd, response, strlen (response), 0);
      return 1;
    }

  //Check if the result is 1 otherwise the certificate is not good
  char data[SIZE] = {0};
  while (fgets (data, SIZE, fp) != NULL)
    {
      if (data[0] == '1')
        {
          break;
        }
      else
        {
          printf ("[!] Certificate not valid\n");
          char *response = "Your certificate is not valid\n";
          if (send (sockfd, response, strlen (response), 0) == -1)
            {
              printf ("[!] Error in sending data\n");
            }
          printf ("[!] Certificate not valid send\n");
          return 1;
        }
    }

  printf ("[#] Certificate is correct\n");
  pclose (fp);
  free (command);
  free (cert_file);
  printf ("[#] Server check certificate END\n");
  return 0;
}

int
check_file_signature (char *username,
                      char *filename,
                      int sockfd,
                      char *signature_filename_txt,
                      char *script_filename,
                      char *signature_filename_bin)
{
  FILE *fp;
  fp = fopen (filename, "r");
  //Create file with signature
  FILE *fp_signature;
  fp_signature = fopen (signature_filename_txt, "w");
  //Create file with script
  FILE *fp_script;
  fp_script = fopen (script_filename, "w");
  ssize_t read;
  if (fp == NULL)
    {
      printf ("[!] Problem while trying to open %s\n", filename);
      close (sockfd);
      exit (EXIT_FAILURE);
    }
  if (fp_signature == NULL)
    {
      printf ("[!] Problem while trying to open %s\n", signature_filename_txt);
      close (sockfd);
      exit (EXIT_FAILURE);
    }
  if (fp_script == NULL)
    {
      printf ("[!] Problem while trying to open %s\n", script_filename);
      close (sockfd);
      exit (EXIT_FAILURE);
    }

  char data[SIZE] = {0};
  //Split the script and the signature in the 2 files created
  while (fgets (data, SIZE, fp) != NULL)
    {
      if (strstr (data, "#!/bin/bash") != NULL)
        {
          fprintf (fp_script, "%s", data);
          break;
        }
      else
        {
          fprintf (fp_signature, "%s", data);
        }
    }
  while (fgets (data, SIZE, fp) != NULL)
    {
      fprintf (fp_script, "%s", data);
    }

  fclose (fp);
  fclose (fp_signature);
  fclose (fp_script);

  //Convert text signature to binary
  char *conversion_file_command = malloc (strlen (username) +
                                          strlen (signature_filename_txt) +
                                          sizeof ("LC_ALL=C tr -cd 0-9a-fA-F < ") +
                                          sizeof (" | xxd -r -p > ") +
                                          strlen (signature_filename_bin));
  strcpy (conversion_file_command, "LC_ALL=C tr -cd 0-9a-fA-F < ");
  strcat (conversion_file_command, signature_filename_txt);
  strcat (conversion_file_command, " | xxd -r -p > ");
  strcat (conversion_file_command, signature_filename_bin);

  if (system (conversion_file_command) == -1)
    {
      printf ("[!] conversion_file_command exec failed\n");
      close (sockfd);
      exit (EXIT_FAILURE);
    }
  printf ("[#] conversion_file_command exec successfully\n");

  //Create the command to check if the signature is valid
  char *check_signature_command = malloc (strlen (username) * 3 +
                                          sizeof ("openssl dgst -sha1 -verify pub_keys/public-key-") +
                                          sizeof (".pem -signature ") +
                                          strlen (signature_filename_bin) +
                                          sizeof (" ") +
                                          strlen (script_filename));
  strcpy (check_signature_command, "openssl dgst -sha1 -verify pub_keys/public-key-");
  strcat (check_signature_command, username);
  strcat (check_signature_command, ".pem -signature ");
  strcat (check_signature_command, signature_filename_bin);
  strcat (check_signature_command, " ");
  strcat (check_signature_command, script_filename);

  FILE *check_signature_command_fp;

  /* Open the command for reading. */
  check_signature_command_fp = popen (check_signature_command, "r");

  if (check_signature_command_fp == NULL)
    {
      printf ("[!] Error while reading file \n");
      char *response = "Error with program run";
      send (sockfd, response, strlen (response), 0);
      return 1;
    }
  //If the command output 'Verified OK' the signature is valid otherwise it is not
  while (fgets (data, SIZE, check_signature_command_fp) != NULL)
    {
      if (strstr (data, "Verified OK") != NULL)
        {
          printf ("[#] Signature is correct\n");
          pclose (check_signature_command_fp);
          free (check_signature_command);
          free (conversion_file_command);
          return 0;
        }
      else
        {
          printf ("[!] Signature not valid\n");
          char *response = "Signature of the script is not valid\n";
          if (send (sockfd, response, strlen (response), 0) == -1)
            {
              printf ("[!] Error in sending data\n");
            }
          printf ("[!] Signature not valid send\n");
          free (check_signature_command);
          free (conversion_file_command);
          return 1;
        }
    }
  printf ("[!] Signature not valid\n");
  char *response = "Signature of the script is not valid\n";
  if (send (sockfd, response, strlen (response), 0) == -1)
    {
      printf ("[!] Error in sending data\n");
    }
  printf ("[!] Signature not valid send\n");
  free (check_signature_command);
  free (conversion_file_command);
  return 1;
}

void exec_file (char *filename, int sockfd)
{
  //Create command to execute the file
  char *command = malloc (strlen (filename) + sizeof ("./"));
  strcpy (command, "./");
  strcat (command, filename);

  FILE *fp;

  /* Open the command for reading. */
  fp = popen (command, "r");
  if (fp == NULL)
    {
      printf ("[!] Error while reading file \n");
      char *response = "Error with program run";
      send (sockfd, response, strlen (response), 0);
      return;
    }

  char data[SIZE] = {0};
  //Send the file output on the sockfd
  while (fgets (data, SIZE, fp) != NULL)
    {
      if (send (sockfd, data, sizeof (data), 0) == -1)
        {
          printf ("[!] Error in sending data\n");
          exit (1);
        }
      bzero (data, SIZE);
    }
  /* close */
  pclose (fp);
  free (command);
}

void delete_current_file (char *filename)
{
  if (remove (filename) == 0)
    {
      printf ("[#] Deleted successfully\n");
    }
  else
    {
      printf ("[!] Error unable to delete the file %s\n", filename);
    }
}