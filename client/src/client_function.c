#include "../header/client_function.h"

void send_file (FILE *fp, int sockfd)
{
  char data[SIZE] = {0};

  while (fgets (data, SIZE, fp) != NULL)
    {
      if (send (sockfd, data, sizeof (data), 0) == -1)
        {
          printf ("[!] Error in sending data\n");
          exit (1);
        }
      bzero (data, SIZE);
    }

  if (send (sockfd, "END", sizeof ("END"), 0) == -1)
    {
      printf ("[!] Error in sending data\n");
      exit (1);
    }
}

void get_result (int sockfd)
{
  int n;
  char buffer[SIZE];

  printf ("[#] Result from server \n");

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
      printf ("%s", buffer);
      bzero (buffer, SIZE);
    }
  return;
}