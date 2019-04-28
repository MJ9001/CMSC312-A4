
/**********************************************************************

   File          : cmsc312-p4-second.c

   Description   : This is second chance page replacement algorithm
                   (see .h for applications)
                   See http://www.cs.cf.ac.uk/Dave/C/node27.html for info

   By            : Trent Jaeger, Yuquan Shan

***********************************************************************/
/**********************************************************************
Copyright (c) 2016 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this softwiare without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

/* Include Files */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>

/* Project Include Files */
#include "cmsc312-p4.h"

/* Definitions */

/* second chance list */

typedef struct second_entry {  
  int pid;
  ptentry_t *ptentry;
  struct second_entry *next;
  struct second_entry *prev;
} second_entry_t;

typedef struct second {
  second_entry_t *first;
} second_t;

second_t *page_list;


int do_secondchance()
{
    second_entry_t *iterator = page_list->first;
  
    while(iterator != NULL)
    {
        iterator->ptentry->bits &= ~REFBIT;
        iterator = iterator->next;
    }
}


/**********************************************************************

    Function    : init_second
    Description : initialize second-chance list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_second( FILE *fp )
{
  page_list = (second_t *)malloc(sizeof(second_t));
  page_list->first = NULL;
  return 0;
}


/**********************************************************************

    Function    : replace_second
    Description : choose victim based on second chance algorithm (first with ref == 0)
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_second( int *pid, frame_t **victim )
{
  second_entry_t *first = page_list->first;
  second_entry_t *iterator = page_list->first;
  second_entry_t *toBeReplaced = first;
  if(first == NULL)
  {
    printf("Empty list error.\n");
  }
  while(iterator != NULL)
  {
     if((iterator->ptentry->bits & REFBIT) > 0 && toBeReplaced == NULL)
     {
       toBeReplaced = iterator;
     }
     iterator = iterator->next;
  }
  if(toBeReplaced == NULL)//use FIFO if all items are in second chance.
    toBeReplaced = page_list->first;
  
  /* return info on victim */
  *victim = &physical_mem[toBeReplaced->ptentry->frame];
  *pid = toBeReplaced->pid;
  
    page_list->first = first->next;
    if(page_list->first != NULL)
      page_list->first->prev = NULL;
  }else{
    if(toBeReplaced->next != NULL)
       toBeReplaced->next->prev = toBeReplaced->prev;
    if(toBeReplaced->prev != NULL)
       toBeReplaced->prev->next = toBeReplaced->next;
    free(toBeReplaced);
  }
  do_secondchance();
  return 0;
}


/**********************************************************************

    Function    : update_second
    Description : update second chance on allocation 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_second( int pid, frame_t *f )
{
  printf("Do we get here.\n");
  /* make new list entry */
  second_entry_t *list_entry = ( second_entry_t *)malloc(sizeof(second_entry_t));
  list_entry->ptentry = &processes[pid].pagetable[f->page];
  list_entry->pid = pid;
  list_entry->next = NULL;
  
  /* put it at the end of the list (beginning if null) */
  if ( page_list->first == NULL ) {
    page_list->first = list_entry;
  }
  /* or really at end */
  else {
    second_entry_t *first = page_list->first;
    second_entry_t *iterator = page_list->first;
    int counter = 0;
    second_entry_t *toBeReplaced = NULL;
  
    while(iterator != NULL)
    {
       if(iterator->ptentry == list_entry->ptentry) {
           iterator->ptentry->bits |= REFBIT;
           return 0;
       }
       if((iterator->ptentry->bits & REFBIT) > 0 && toBeReplaced == NULL)
       {
         toBeReplaced = iterator;
       }
       iterator = iterator->next;
       counter++;
    }
    if(toBeReplaced == NULL)
        toBeReplaced = page_list->first;
    if(counter >= PHYSICAL_FRAMES)
    {
      list_entry->next = toBeReplaced->next;
      list_entry->prev = toBeReplaced->prev;
      if(page_list->first == toBeReplaced)
         page_list->first = list_entry;
      if(toBeReplaced->next != NULL)
         toBeReplaced->next->prev = list_entry;
      if(toBeReplaced->prev != NULL)
         toBeReplaced->prev->next = list_entry;
      free(toBeReplaced);
    }else
    {
      iterator = first;
      while(iterator->next != NULL)
        iterator = iterator->next;
      iterator->next = NULL;
      iterator->prev = list_entry;
      list_entry->next = iterator;
      do_secondchance();
    }
  }
  return 0;
}


