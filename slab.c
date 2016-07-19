/*===========================================================================

FILE: slab.c

DESCRIPTION
   memory allocation algorithm.

PUBLIC CLASSES:
   N/A

INITIALIZATION AND SEQUENCING REQUIREMENTS:



       Copyright @2011 Happyman Design.
             All Rights Reserved.
===========================================================================*/


#include"stdio.h"
#include<stdlib.h>
#include"fcntl.h"
#include"string.h"
#include"sys_types"
#include"words.h"
#include"slab.h"



/*----------------------------------------------------------------------------
   Definitions
------------------------------------------------------------------------------*/
#define DEBUG 1


#define SLAB_COUNT_END	(((SlabCount)(~0U))-0)
#define SLAB_COUNT_FREE	(((SlabCount)(~0U))-1)
#define	SLAB_COUNT_LIMIT	(((SlabCount)(~0U))-2)


/*----------------------------------------------------------------------------
   types/structures
------------------------------------------------------------------------------*/


/*
  +---------------+------------------+----------------------+
  | Slab          | Object           |  Objects             |
  | discriptor    | discriptors      |                      |
  |               | array            |                      |
  |               |(if Smart type)   |                      |
  +---------------+------------------+----------------------+

 */
 



/*----------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

static inline SlabCount *SlabObjsIdx(SlabType *pSlab)
{
	return (SlabCount *)(pSlab+1);
}


SlabType * SlabCreat(int objsize, SlabCount num, int flag)
{
  SlabType *pSlab;
  unsigned int size;

  if (objsize > KB(512) && objsize*num > MB(10))
    return ERR_INVLID_ARG;

  size = objsize*num;
  if (flag & SLAB_FLAG_TYPE == SLAB_TYPE_SMART)
    size += sizeof(SlabCount)*num;
  size += sizeof(SlabType);

  pSlab = (SlabType *)malloc(size);

  if (NULL == pSlab)
    return ERR_NO_MEM;

  pSlab->mem = (void*)SlabObjsIdx(pSlab);
  pSlab->data = pSlab->mem;
  pSlab->flag = flag;
  pSlab->num = num;
  pSlab->objsize = objsize;

  
  if (flag & SLAB_FLAG_TYPE == SLAB_TYPE_SMART)
  {
    pSlab->data += num*sizeof(SlabCount);
    InitObjs(pSlab);
  }
  else
  {
    pSlab->free = 0;
    pSlab->inuse = 0;
  }

  return pSlab;
}


void InitObjs(SlabType *pSlab)
{
  int i;

	for (i = 0; i < pSlab->num; i++) 
  {
 	  SlabObjsIdx(pSlab)[i] = i+1;
	}

	SlabObjsIdx(pSlab)[i-1] = SLAB_COUNT_END;

  pSlab->free = 0;
  pSlab->inuse = 0;

}

void * SlabAlloc(SlabType *pSlab)
{
  SlabType *pS = pSlab;
  void *pObj = NULL;
  SlabCount next;

//  if (pS->free == SLAB_COUNT_END) //this is used for smart type
  if (pS->inuse >= pS->num)
    return NULL;

  pObj = pS->data + pS->objsize*pS->free;
  if (pS->flag & SLAB_FLAG_TYPE == SLAB_TYPE_SIMPLE)
  {
    pS->free++;
  }
  else /* SMART TYPE */
  {
    next = SlabObjsIdx(pS)[pS->free];
#if DEBUG
    SlabObjsIdx(pS)[pS->free] = SLAB_COUNT_FREE;
#endif
    pS->free = next;
  }


  pS->inuse++;

  return pObj;
}


/* Only support Smart type slab. */
Status SlabFree(SlabType *pSlab, void *pObj)
{
  SlabCount objNew;

  if (pSlab->flag & SLAB_FLAG_TYPE == SLAB_TYPE_SIMPLE)
  {
  //pSlab->free = 0;
  //pSlab->inuse = 0;
    return ERR_INVLID_ARG;
  }

  objNew = (pObj - pSlab->data) / pSlab->objsize;

#if DEBUG
  /* can't free twice */
  assert(!(SlabObjsIdx(pSlab)[objNew] == SLAB_COUNT_FREE));
#endif

  SlabObjsIdx(pSlab)[objNew] = pSlab->free;
  pSlab->free = objNew;
  pSlab->inuse--;

  return OK;
}


/* Only support Smart type slab. */
Status SlabFlush(SlabType *pSlab)
{
  SlabCount objFree;

  if (pSlab->flag & SLAB_FLAG_TYPE == SLAB_TYPE_SIMPLE)
  {
  //pSlab->free = 0;
  //pSlab->inuse = 0;
    return ERR_INVLID_ARG;
  }

  InitObjs(pSlab);

  return OK;
}



void * SimpleSlabAlloc(SlabType *pSlab, SlabCount num)
{
  SlabType *pS = pSlab;
  void *pObj = NULL;
  SlabCount next;


  if (pS->flag & SLAB_FLAG_TYPE != SLAB_TYPE_SIMPLE)
    return NULL;

  if (pS->inuse + num > pS->num)
    return NULL;

  pObj = pS->data + pS->objsize*pS->free;
  pS->free += num;
  pS->inuse += num;

  return pObj;

}

Status SimpleSlabFree(SlabType *pSlab)
{
  if (pSlab->flag & SLAB_FLAG_TYPE != SLAB_TYPE_SIMPLE)
    return ERR_INVLID_ARG

  pSlab->free = 0;
  pSlab->inuse = 0;

  return OK;
}


Status SlabDestroy(SlabType **pSlab)
{
  assert(!*pSlab);

  free(*pSlab);
  *pSlab = NULL;

  return OK;
}


