/*
==============================================================================

FILE:         logic.c

DESCRIPTION:
  Logic structures 

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
#include"word.h"




static inline LogicLinkerType * LogicLinkerNext(LogicLinkerType *pLinker)
{
  if (pLinker->list.next)
    return container_of(pLinker->list.next, LogicLinkerType, list);
  return NULL;
}

static inline LogicLinkerType * LogicLinkerPrev(LogicLinkerType *pLinker)
{
  if (pLinker->list.prev)
    return container_of(pLinker->list.prev, LogicLinkerType, list);
  return NULL;
}



// to be checked. for tailllll
//4 note that the linked list are all circled.

void ConnectLogic(LogicLinkerType *pPrev, LogicLinkerType *pNext)
{
  list_add(&pNext->list, &pPrev->list);
}

void DelLogic(LogicLinkerType *pNode)
{
  list_del(&pNode->list);
}

void LogicQueueInit(LogicQueueType *pQ)
{
  INIT_LIST_HEAD(&pQ->LogicHead);
  
}

/* add a unit append in the tail, note that it's a circle double link */
void LogicQueueAddTail(LogicQueueType *pQ, LogicLinkerType *pNew)
{
  list_add_tail(&pNew->list, &pQ->LogicHead.list);
  pQ->logicSum++;
}

void LogicQueueInsert(LogicQueueType *pQ, LogicLinkerType *pPrev, LogicLinkerType *pNew)
{
  list_add(&pNew->list, &pPrev->list);
  pQ->logicSum++;
}

void LogicQueueDel(LogicQueueType *pQ, LogicLinkerType *pNode)
{
  list_del(&pNode->list);
  pQ->logicSum--;
}


/*
void QueueAddLogicUnitTail(LogicLinkerType *pHead, LogicLinkerType *pNew)
{
  list_add_tail(&pNew->list, &pHead->list);
}
*/



inline void AddEntity(EntityType * prev, EntityType *pNew)
{
  list_add(&pNew->list, &prev->list);
}

inline void DelEntity(EntityType *pNode)
{
  list_del(&pNode->list);
}


/* subType is optional, for FOPL_USUAL and FOPL_SPECIAL */
Status CreateLogicUnit(int8      logicType,  int8 entitySum, void **pLogicUnit)
{
  void *p;
  int i;
  EntityType *pE;
  FOPL_Type *pFOPL;

//  int (* pfFree)(void *obj);

  if (logicType == LOGIC_FOPL_TYPE)
  {
    pFOPL = AllocFOPL();
    if (!pFOPL)
      return ERR_NO_MEM;

    if (0 == entitySum)
      return OK;

    INIT_LIST_HEAD(pFOPL->Entities);
    for (i = 0; i < entitySum; i++)
    {
      pE = AllocEntity();

      if (!pE)
      {
        // TO_BE_DONE need free more
        FreeFOPL(pFOPL);
        return ERR_NO_MEM;
      }
      list_add(&pE->list, &pFOPL->Entities)
    }
    pFOPL->entitySum = entitySum;

    *pLogicUnit = pFOPL;

  }
  else if (logicType == LOGIC_OPEARATOR_TYPE)
  {
    p = AllocFOL_Operator();
    if (!p)
      return ERR_NO_MEM;

    *pLogicUnit = p;
  }
  else
  {
     return ERR_INVLID_ARG;
  }



  




#if 0

  /*********************************************
   Should keep an eye on the second allocating,
   default to auto create FOPL_SPECIAL_PRED.
   *********************************************/

  if (logicType == LOGIC_FOPL_TYPE && subType == FOPL_SPECIAL_PRED)
  {

    if (OK != CreatePredInfo(FOPL_SPECIAL_PRED, 2, &p)){
      pfFree((void*)*pLogicUnit);
      //free(*pLogicUnit);
      return ERR_NO_MEM;
    }
    ((FOPL_Type *)(*pLogicUnit))->pPredInfo = p;
  }
#endif


  return OK;
}





/* variable or constant in the FOPL parameter slot:entity */
inline bool IsVarEntity(EntityType *pEtty)
{
  if (pEtty->share)
    pEtty = pEtty->share;

  if (pEtty->name.len == 0 && pEtty->name.pdata >= 1)
    return TRUE;
  else
    return FALSE;
}






