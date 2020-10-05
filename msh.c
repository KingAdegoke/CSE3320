/* Samuel Adegoke
  ID 1001541794 */

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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
#define MAX_NUM_ARGUMENTS 11 // Mav shell supports 11 arguments. Ten arguments plus addition to the command (rq 7)

void shift(char *string, char historyArr[16][100], int counter) //function that shifts array history when it is above 15
{
  if (counter > 14)
  {
    for (int i = 0; i < 14; i++)
    {
      strcpy(historyArr[i], historyArr[i + 1]);
    }
    strcpy(historyArr[14], string);
  }
  else
  {
    strcpy(historyArr[counter], string);
  }
}

void pidShift(pid_t pid, int pArr[100], int pCounter) //function that shifts pid array when it is above 15
{
  if (pCounter > 14)
  {
    for (int i = 0; i < 14; i++)
    {
      pArr[i + 1] = pArr[i];
    }
    pArr[14] = getppid();
  }
  else
  {
    pArr[pCounter] = getppid();
  }
}

void exclamation(char *input, int hV, int CC, char historyArr[16][100]) //function for when the user enters an exclamation point
{
  hV = 0;
  input = input + 1; //deleting the ! from the input
  hV = atoi(input);  //turning the number into a integer since it is already a string
  if (hV > CC - 1)   // detects if the history value is within the bounds
  {
    printf("Not in bounds. \n");
    strcpy(input, ""); //copys nothing into the command so that it is able to go through execvp
  }
  else
  {
    strcpy(input, historyArr[hV]); // stores the command from the history array into your input to store into execvp
  }
}

int main()
{
  int cmdCT, pidCT;
  int histVal;
  char CmdHIST[16][100];
  int pidArray[100];

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

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

    // put this if statement here so that cmd_str can be tokenized/parsed before being put into execvp
    if (strstr(cmd_str, "!") != NULL)
    {
      cmd_str = cmd_str + 1;   //deleting the ! from the input
      histVal = atoi(cmd_str); //turning the number into a integer since it is already a string
      if (histVal > cmdCT - 1) // detects if the history value is within the bounds
      {
        printf("Not in bounds. \n");
        strcpy(cmd_str, ""); //copys nothing into the command so that it is able to go through execvp
      }
      else
      {
        strcpy(cmd_str, CmdHIST[histVal]); // stores the command from the history array into your input to store into execvp
      }
    }

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    // for ls -a -l etc
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

    // check to see if there is any input
    if (token[0] != NULL)
    {
      if (strcmp(token[0], "history") == 0)
      {
        shift(cmd_str, CmdHIST, cmdCT);
        cmdCT++;
        for (int i = 0; i < cmdCT; i++)
        {
          if (i < 15)
          {
            printf("%d: %s", i, CmdHIST[i]);
          }
        }
      }

      // exit the shell
      else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0) // to exit the shell
      {
        return 0;
      }

      else if (strcmp(token[0], "cd") == 0)
      {
        shift(cmd_str, CmdHIST, cmdCT);
        cmdCT++;
        chdir(token[1]);
      }
      // showpids command that prints out the pids from the array
      else if (strcmp(token[0], "showpids") == 0)
      {
        pid_t pid = fork();
        if (pid == 0)
        {
          for (int i = 0; i < pidCT; i++)
          {
            if (i < 15)
            {
              printf("%d: %d\n", i, pidArray[i]);
            }
          }
          return 0;
        }
        shift(cmd_str, CmdHIST, cmdCT);
        cmdCT++;
        pidShift(pid, pidArray, pidCT);
        pidCT++;
        int status;
        wait(&status);
      }
      // calls execvp in the child. ls, pwd, and clear commands.
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
          pidShift(pid, pidArray, pidCT);
          pidCT++;
          shift(cmd_str, CmdHIST, cmdCT);
          int status;
          wait(&status);
          cmdCT++;
        }
      }
      // free so that the user is able to take input
      free(working_root);
    }
  }
  return 0;
}
