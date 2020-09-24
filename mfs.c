/* Samuel Adegoke
  ID 1001541794 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports five arguments

int main()
{
  int cmdCT, pidCT, histVal;
  char CmdHIST[15][100];
  char **fillerString;
  int pidHIST[MAX_COMMAND_SIZE];
  int pidArray[100];

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
  int len;
  len = strlen(cmd_str);

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    // int token_index  = 0;
    // for( token_index = 0; token_index < token_count; token_index ++ )
    // {
    //   printf("token[%d] = %s\n", token_index, token[token_index] );
    // }
    if (token[0] != NULL)
    {
      if (strcmp(token[0], "history") == 0)
      {
        for (int i = 0; i < cmdCT; i++)
        {
          printf("%d: %s", i, CmdHIST[i]);
        }
        cmdCT++;
      }

      else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0) // to exit the shell
      {
        return 0;
      }

      else if (strcmp(token[0], "cd") == 0)
      {
        chdir(token[1]);
        if (cmdCT <= 14)
        {
          strcpy(CmdHIST[cmdCT], cmd_str);
        }
        cmdCT++;
      }

      else if (strcmp(token[0], "showpids") == 0)
      {
        pidCT++;
        for (int i = 0; i < pidCT; i++)
        {
          printf("%d: %d\n", i, pidArray[i]);
        }
        if (cmdCT <= 14)
        {
          strcpy(CmdHIST[cmdCT], cmd_str);
        }
        cmdCT++;
      }

      else if (strstr(cmd_str, "!") != NULL)
      {
        cmd_str = cmd_str + 1;
        histVal = atoi(cmd_str);
        if (histVal > cmdCT)
        {
          printf("Not in bounds. \n");
        }
        else
        {
          fillerString = malloc(sizeof(char) * 100);
          strcpy(fillerString, CmdHIST[histVal]);
          pid_t pid = fork();

          // [ls,-alt] = args
          // arg = "ls"
          // fillerString = "ls -alt"
          // execvp("ls", char ** args)
          execvp(fillerString[0], fillerString);
          //execvp(fillerString[0], fillerString);
          printf("%s\n", fillerString);
          //execvp(fillerString[0], fillerString);
        }
      }

      else
      {
        pid_t pid = fork();
        if (pid == 0)
        {
          if (execvp(token[0], token) == -1)
          {
            printf("Command not found: %s\n", token[0]);
            return 0;
          }
          else
          {
            execvp(token[0], token);
          }
        }
        else
        {
          pidArray[cmdCT] = pid;
          pidCT++;
          if (cmdCT <= 14)
          {
            strcpy(CmdHIST[cmdCT], cmd_str);
          }
          int status;
          wait(&status);
          cmdCT++;
        }
      }

      if (cmdCT <= 14)
      {
        strcpy(CmdHIST[cmdCT], cmd_str);
      }

      free(working_root);
    }
  }

  return 0;
}
