#ifndef SLAB_H
#define SLAB_H
/*
==============================================================================

FILE:         slab.h

DESCRIPTION:
  slab interfaces. 

             Copyright @ 2011 Happyman Design.
                    All Rights Reserved.

==============================================================================
Edit History


when       who     what, where, why
--------   ---     -----------------------------------------------------------
*/

/*============================================================================

                     INCLUDE FILES FOR MODULE

============================================================================*/

/* bits value */
#define SLAB_TYPE_SIMPLE 0
#define SLAB_TYPE_SMART  1

/* bit mask */
#define SLAB_FLAG_TYPE 0x1
#define SLAB_SIMPLE_MASK (SLAB_FLAG_TYPE & SLAB_TYPE_SIMPLE)
#define SLAB_SMART_MASK (SLAB_FLAG_TYPE & SLAB_TYPE_SMART)

typedef short SlabCount;

typedef struct {
  SlabCount num;
  SlabCount free; /* index of next free object */
  SlabCount inuse;
  int objsize;
  void *mem;  /* Slab objects' index(discriptor) start */
  void *data;  
  unsigned int flag;
}SlabType;



SlabType * SlabCreat(int objsize, SlabCount num, int flag);
void * SlabAlloc(SlabType *pSlab);
Status SlabFree(SlabType *pSlab, void *pObj);
Status SlabDestroy(SlabType **pSlab);


#endif

