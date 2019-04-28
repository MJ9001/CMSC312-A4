
/**********************************************************************

   File          : cmsc312-p4-mfu.c

   Description   : This is most frequently used replacement algorithm
                   (see .h for applications)

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

typedef struct mfu_entry{  
  int pid;
  ptentry_t *ptentry;
  struct mfu_entry *next;
  struct mfu_entry *prev;
} mfu_entry_t;

typedef struct mfu{
  mfu_entry_t *first;
} mfu_t;

mfu_t *page_list;

/**********************************************************************

    Function    : init_mfu
    Description : initialize mfu list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_mfu( FILE *fp )
{
  page_list = (mfu_t *)malloc(sizeof(mfu_t));
  page_list->first = NULL;
  return 0;
}


/**********************************************************************

    Function    : replace_mfu
    Description : choose victim based on mfu algorithm, take the frame 
                  associated the page with the largest count as victim 
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_mfu( int *pid, frame_t **victim )
{
    
  printf("And then we got here.\n");
  mfu_entry_t *first = page_list->first;
  mfu_entry_t *iterator = page_list->first;
  mfu_entry_t *toBeReplaced = first;
  int highest_count = 0;
  printf("Are we here?\n");
  if(first == NULL)
  {
    printf("FIRST WAS NULL, OMG.\n");
  }
  while(iterator != NULL)
  {
     printf("Let us loop!\n");
     if(iterator->ptentry->ct > highest_count)
     {
       highest_count = iterator->ptentry->ct;
       toBeReplaced = iterator;
     }
     iterator = iterator->next;
  }
  printf("We have looped.\n");
  
  /* return info on victim */
  *victim = &physical_mem[toBeReplaced->ptentry->frame];
  *pid = toBeReplaced->pid;
  printf("Return info set.\n");
  
  //list_entry->next = toBeReplaced->next;
  //list_entry->prev = toBeReplaced->prev;
  if(first == toBeReplaced){
    page_list->first = first->next;
    page_list->first->prev = NULL;
  } else {
    if(toBeReplaced->next != NULL)
       toBeReplaced->next->prev = toBeReplaced->prev;
    if(toBeReplaced->prev != NULL)
       toBeReplaced->prev->next = toBeReplaced->next;
    printf("Let's free up some space.\n");
    free(toBeReplaced);
  }
  printf("And finished.\n");
  return 0;
}


/**********************************************************************

    Function    : update_mfu
    Description : create container for the newly allocated frame (and 
                  associated page), and insert it to the end (with respect
                  to page_list->first) of page list 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_mfu( int pid, frame_t *f )
{
  printf("Do we get here.\n");
  /* make new list entry */
  mfu_entry_t *list_entry = ( mfu_entry_t *)malloc(sizeof(mfu_entry_t));
  list_entry->ptentry = &processes[pid].pagetable[f->page];
  list_entry->pid = pid;
  list_entry->next = NULL;
  
  /* put it at the end of the list (beginning if null) */
  if ( page_list->first == NULL ) {
    page_list->first = list_entry;
  }
  /* or really at end */
  else {
    mfu_entry_t *first = page_list->first;
    mfu_entry_t *iterator = page_list->first;
    int counter = 0;
    int highest_count = 0;
    mfu_entry_t *toBeReplaced = first;
  
    while(iterator != NULL)
    {
       if(iterator->ptentry == list_entry->ptentry)
           return 0;
       
       if(iterator->ptentry->ct > highest_count)
       {
         highest_count = iterator->ptentry->ct;
         toBeReplaced = iterator;
       }
       iterator = iterator->next;
       counter++;
    }
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
    }
  }
  return 0;
}
