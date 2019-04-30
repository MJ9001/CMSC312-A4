
/**********************************************************************

   File          : cmsc312-p4-lfu.c

   Description   : This is least frequently used replacement algorithm
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

typedef struct lfu_entry{  
  int pid;
  ptentry_t *ptentry;
  struct lfu_entry *next;
  struct lfu_entry *prev;
} lfu_entry_t;

typedef struct lfu{
  lfu_entry_t *first;
} lfu_t;

lfu_t *page_list;

/**********************************************************************

    Function    : init_lfu
    Description : initialize lfu list
    Inputs      : fp - input file of data
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int init_lfu( FILE *fp )
{
  page_list = (lfu_t *)malloc(sizeof(lfu_t));
  page_list->first = NULL;
  return 0;
}


/**********************************************************************

    Function    : replace_lfu
    Description : choose victim based on lfu algorithm, take the frame 
                  associated the page with the smallest count as victim 
    Inputs      : pid - process id of victim frame 
                  victim - frame assigned from fifo -- to be replaced
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int replace_lfu( int *pid, frame_t **victim )
{
    
  printf("And then we got here.\n");
  lfu_entry_t *first = page_list->first;
  lfu_entry_t *iterator = page_list->first;
  lfu_entry_t *toBeReplaced = first;
  int lowest_count = 2147483647;
  printf("Are we here?\n");
  if(first == NULL)
  {
    printf("FIRST WAS NULL, OMG.\n");
  }
  while(iterator != NULL)
  {
     printf("Let us loop!\n");
     if(iterator->ptentry->ct < lowest_count)
     {
       lowest_count = iterator->ptentry->ct;
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
    if(page_list->first != NULL)
      page_list->first->prev = NULL;
  }else{
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

    Function    : update_lfu
    Description : create container for the newly allocated frame (and 
                  associated page), and insert it to the end (with respect
                  to page_list->first) of page list 
    Inputs      : pid - process id
                  f - frame
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int update_lfu( int pid, frame_t *f )
{
  /* make new list entry */
  lfu_entry_t *list_entry = ( lfu_entry_t *)malloc(sizeof(lfu_entry_t));
  list_entry->ptentry = &processes[pid].pagetable[f->page];
  list_entry->pid = pid;
  list_entry->next = NULL;
  
  /* put it at the end of the list (beginning if null) */
  if ( page_list->first == NULL ) {
    page_list->first = list_entry;
  }
  /* or really at end */
  else {
    lfu_entry_t *first = page_list->first;
    lfu_entry_t *iterator = page_list->first;
    int counter = 0;
    int lowest_count = 2147483647;
    lfu_entry_t *toBeReplaced = first;
  
    while(iterator != NULL)
    {
       if(iterator->ptentry == list_entry->ptentry)
           return 0;
       if(iterator->ptentry->ct < lowest_count)
       {
         lowest_count = iterator->ptentry->ct;
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
