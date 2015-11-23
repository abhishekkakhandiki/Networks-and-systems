/******************************************************************************/
/* Important CSCI 402 usage information:                                      */
/*                                                                            */
/* This fils is part of CSCI 402 warmup programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *
 * @(#)$Id: listtest.c,v 1.1 2015/08/21 03:52:20 william Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "cs402.h"

#include "my402list.h"


/* ----------------------- Utility Functions ----------------------- */

/*
 * 
 * name: My402listlength
 * @param My402lis*
 * @return int
 * Returns the number of elements in the list.
 */

int  My402ListLength(My402List* listname)
 {
		 return(listname->num_members);
	 
 }
 
/*
 * 
 * name: My402listempty
 * @param My402list*
 * @return int
 * Returns TRUE if the list is empty. Returns FALSE otherwise. 
 */
 

int  My402ListEmpty(My402List* listname)
{
	if(listname->num_members == 0)
	{
		return TRUE;
	}
	return FALSE;
	
}
/*
 * 
 * name: My402ListAppend
 * @param My402list* void*
 * @return int
 * If list is empty, just add obj to the list. Otherwise, add obj after Last(). 
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise. 
 */

int  My402ListAppend(My402List* listname, void* data)
{
	
	
		My402ListElem *p2= NULL;
		p2 = &listname->anchor;
		My402ListElem* temp = malloc(sizeof(My402ListElem));
		My402ListElem* Lastelement;
		//printf("inside Append");
  if (My402ListEmpty(listname))
 {
	    //printf("inside Append : List empty \n");
	  temp = malloc(sizeof(My402ListElem));
	   
	  listname->anchor.next = temp;   	  												
	  listname->anchor.prev = temp;	
	  
	  temp->next = p2;
	  temp->prev = p2;
	  
	  temp->obj = data;
	  								
	  listname->num_members++;
	  //printf("Append Print IF %s\n",temp->obj);
	  return TRUE;
	  	  	  
 }	
      
	else
{ 
	//printf("inside Append list not empty \n");
	  Lastelement = My402ListLast(listname); 
	  temp->obj =  data;
	  
	  Lastelement->next = temp; 
	       				
	  temp->prev = Lastelement;				
	  temp->next =  p2;
	  					
	  p2-> prev = temp;			
	  listname->num_members++;
	  //printf("Append Print ELSE %s\n",temp->obj);
	  return TRUE;
	  
} 
	
		return FALSE;
}

/*
 * 
 * name: My402ListPrepend
 * @param My402List* void*
 * @return int 
 * 
 */

int  My402ListPrepend(My402List* listname, void* data)
{

	My402ListElem *p2 = &listname->anchor;
	My402ListElem* temp = NULL;
	My402ListElem* temp1 = NULL;
	My402ListElem* Firstelement = NULL;
	if (My402ListEmpty(listname))
 {    
	  temp = malloc(sizeof(My402ListElem));
	  temp->obj = data;
	  
	  temp->next = p2; 	
	  temp->prev = p2;
	  						
	  p2->next = temp; 					
	  p2->prev = temp;
	  					
	  listname->num_members++;
	  return TRUE;
	  	  	  
  }	   
      else
  {   
	  Firstelement = My402ListFirst(listname);
	  temp1 = malloc(sizeof(My402ListElem));
	  temp1->obj =  data;
	  
	  temp1->next = Firstelement;	
	  temp1->prev = p2;
	  										
	  Firstelement->prev = temp1;
	  																			
	  p2->next = temp1;
	  
	  listname->num_members++;
	  return TRUE;
  }  
  return FALSE;			

}
/*
 * 
 * name: My402listUnlink
 * @param My402list* My402listElem*
 * @return void
 * Unlink and delete elem from the list. 
 * will not delete the object pointed to by elem and will not check if elem is on the list. 
 */

void My402ListUnlink(My402List* listname1, My402ListElem* elem)
{
	My402ListElem* previous = NULL;
	My402ListElem* nextElem = NULL;

	
	
		nextElem = elem->next;
		previous = elem->prev;
		
		previous->next = nextElem;
		nextElem->prev = previous;
		
		free(elem);
	
		listname1->num_members --;
}
/*
 * 
 * name: My402ListUnlinkAll
 * @param My402List*
 * @return void
 * Unlink and delete all elements from the list and makes the list empty. 
 * will not delete the objects pointed to be the list elements. 
 */

void My402ListUnlinkAll(My402List* listname)
{
	int j ;
	int i = My402ListLength(listname);
	My402ListElem* first = My402ListFirst(listname);
	for( j =0 ; j<i;j++)
	{
		My402ListUnlink(listname,first);
		first = My402ListNext(listname,first);
	}
}


/*
 * 
 * name: My402ListInsertAfter
 * @param My402list* void* My402listelem*
 * @return int
 * Insert obj between elem and elem->next. If elem is NULL, then this is the same as Append().
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise. 
 * will not check if elem is on the list.
 */

int  My402ListInsertAfter(My402List* listname1, void* obj, My402ListElem* elem) 
{

	 My402ListElem* newer = NULL;
	 My402ListElem* next = NULL;
	 if(elem == NULL)
	 {
		 My402ListAppend(listname1,obj);
		 return TRUE;	
	 }
	else
	{
		newer = malloc(sizeof(My402ListElem));
		next = elem->next;
		
		newer->obj = obj;
		newer->prev = elem; 
		newer->next = next;  
		
		next->prev = newer;       
		elem->next = newer;
		
		listname1->num_members ++ ;
		return TRUE;
	}
    return FALSE;
}

/*
 * 
 * name: My402ListInsertBefore
 * @param My402list*.void*,My402Listelem*
 * @return int
 * Insert obj between elem and elem->prev. If elem is NULL, then this is the same as Prepend(). 
 * This function returns TRUE if the operation is performed successfully and returns FALSE otherwise. 
 * will not check if elem is on the list. 
 */

int  My402ListInsertBefore(My402List* listname1, void* obj, My402ListElem* elem) //Insert obj bw elem and elem prev 
{
	
	 My402ListElem* newer = NULL;
	 My402ListElem* prev  = NULL;
	 if(elem == NULL)
	 {
		 My402ListPrepend(listname1,obj);
		 return TRUE;	
	 }
	 else
	 {
		newer = malloc(sizeof(My402ListElem));
		newer->obj = obj;
	    
	    prev = elem->prev;
		newer->prev = prev;  
		newer->next = elem;   
		
		prev->next = newer;
		 
		elem->prev = newer;	
		listname1->num_members++;
		return TRUE;
	}
	return FALSE;
}

/*
 * 
 * name: My402ListFirst
 * @param My402List*
 * @return My402ListElem*
 * Returns the first list element or NULL if the list is empty. 
 */

My402ListElem *My402ListFirst(My402List* listname)
{  

	if(My402ListEmpty(listname))
	{
		return (NULL);
	}
	else
	{		
		return (listname->anchor.next);
	}
}

/*
 * 
 * name:My402ListLast
 * @param My402List*
 * @return My402ListElem*
 * Returns the last list element or NULL if the list is empty. 
 */

My402ListElem *My402ListLast(My402List* listname)
{
	if(My402ListEmpty(listname))
	{
		return (NULL);
	}
	else
	{
		return (listname->anchor.prev);
	}
}

/*
 * 
 * name: My402ListNext
 * @param My402List*,My402ListElem*
 * @return My402ListElem*
 * Returns elem->next or NULL if elem is the last item on the list. 
 * Will not check if elem is present.
 */

My402ListElem *My402ListNext(My402List* listname1, My402ListElem* element)  //returns elem->next or null
{
	
	if(element==My402ListLast(listname1))
	{
	   return(NULL);
	} 
	 return (element->next);        //check
}
/*
 * 
 * name: My402Listprev
 * @param My402List*,My402ListElem*
 * @return My402ListElem*
 * Returns elem->prev or NULL if elem is the last item on the list. 
 * Will not check if elem is present.
 */


My402ListElem *My402ListPrev(My402List* listname1, My402ListElem* elem)     
{
	
	if(elem==My402ListFirst(listname1))
	{
	   return(NULL);
	}
	
	return (elem->prev);          
}

/*
 * 
 * name: My402ListFind
 * @param My402List*,void*
 * @return My402ListElem*
 * Returns the list element elem such that elem->obj == obj.
 * Will not check if elem is present.
 */

My402ListElem *My402ListFind(My402List*listname, void* elem)
{
	My402ListElem* element = NULL;
	for(element = My402ListFirst(listname);element != NULL;element = My402ListNext(listname,element))
	{
		if(element->obj == elem)
		return(element);
	}
	 return (NULL);                              
}

/*
 * 
 * name: My402ListInit
 * @param My402List*
 * @return int
 * Initialize the list into an empty list
 */



int My402ListInit(My402List* listname)
{
	
		My402ListElem *anch=&listname->anchor;
	    listname->num_members = 0;
		listname->anchor.next=anch;
		listname->anchor.prev=anch;
		listname->anchor.obj=NULL;
		return TRUE;							
}


