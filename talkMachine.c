/*===========================================================================

FILE: talkMachine.c

DESCRIPTION
   This file contains the key up-level sematic processing.

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
#include"logic.h"




/*----------------------------------------------------------------------------
   Definitions
------------------------------------------------------------------------------*/
#define HASH_TABLE_SIZE 1000
#define LOGIC_QUEUE_LEN_MAX 20
#define CAR_LINE_INVSITEM_NUM_MAX LOGIC_QUEUE_LEN_MAX
#define TCB_RULE_CACHE_SIZE 30
#define TCB_SCN_CACHE_SIZE 20
#define TCB_GEN_HEURISTIC_CACHE_SIZE 100
#define TCB_ACK_HEURISTIC_CACHE_SIZE 30


#define RECORD_MAX_NUMS 50
#define PLAN_MAX_NUMS 10

#define EXPLSPONSOR_STOPER 1
#define EXPLSPONSOR_HOLDER 2

#define ACKTYPE_SEARCH 1
#define ACKTYPE_CHANGE 2

#define HOLDTYPE_ENTER 1
#define HOLDTYPE_USUAL 2

#define PLAN_TYPE_RULE 1
#define PLAN_TYPE_ACT  2
#define PLAN_TYPE_HEURIST 3

#define LAUNCH_TYPE_SAYING 0     /* 一般陈述 */
#define LAUNCH_TYPE_ACKING 1     /* 等待确认建立共同基础 */
#define LAUNCH_TYPE_DIRECTIVE  2 /* 指令语，说话人的目的是使听话人做某事(询问，命令，要求，邀请，建议，祈求 etc)*/
#define LAUNCH_TYPE_COMMISIVE 3  /* 承诺语，说话人对将来的行为作出承诺 (承诺，计划，发誓，打赌，反对 ect)*/
#define LAUNCH_TYPE_EXPRESSIVE 4 /* 表情语，说话人对事情的心理状态(感谢，道歉，欢迎，悲痛 etc)*/
#define LAUNCH_TYPE_DECLARATION 5 /* 宣告语，说话人说出影响外在世界的决定等 (你被开除了。我不干了。) */
#define LAUNCH_TYPE_ASSERTIVE  6  /*  断言语，说话人对事情的表态 (建议，提出，宣誓，自夸，推断 etc) */



#define MustOK(ret)   do{if ((ret) != OK) return (ret);}while(0)
#define CheckAllocPtr(ptr) do{if (!(ptr)) return ERR_NO_MEM;}while(0)
#define LinkerGetFOPL(pLinker) container_of(pLinker, FOPL_Type, logicLinker);
#define GetCarData(type,pCarriage) ((type*)pCarriage->pdata)

#define ERROR_PRINT printf

/*----------------------------------------------------------------------------
   types/structures
------------------------------------------------------------------------------*/


/******************************
 inversed sorted index hashing
*/
typedef struct InvSItem {
  short question;
  short rule;
  short fpred;
  short scn; /* scenario */
  short act;
  char isSCN;
}InvSItemType;

struct InvSTable {
  struct InvSItem *items;
  short sum;
};

struct HashNode {
  char *name;
  struct HashNode * next;

  struct InvSTable invstbl;
};

struct HashTable {
  int sum;
  int takenSum;
  struct HashNode * nodes;  
};


typedef int (*FOPLMacth)(FOPL_Type * pFOPL1, FOPL_Type * pFOPL2);


/* General Linked carriage */
typedef struct Carriage {
  struct list_head list;
  void * pdata;
}CarriageType;

typedef struct CarriageHead {
  struct list_head list;
  int sum;
}CarriageHeadType;






/***************************************************
  Topic-Subtopic index tree.
 ***************************************************/
#ifdef SEMFRAME_ARRAY_TREE

struct TopicFrame {
  short levels;
  short nodeSum;
  struct SubtopicNode *root; /* root is the first element of the array */
};

struct SubtopicNode {
  char * name;
  short  level;
  short  id;    /* global ID in tree */
  struct SubtopicNode *parent; /* when parent is NULL, it's root. */
  struct SubtopicNode *child;
  struct SubtopicNode *sibling;
};


#endif




/*******************************************
*/
struct HeuristicKnowledge {
  LogicQueueType trigger;  /* Logic equation may have AND or OR or NOT */
  short subtopic;
  short question;
//  short rule;
  short scn;
  char isSCN;

  short txt;


  struct list_head list;
};


enum TalkProgress {
 PGS_NOT_STARTED = 0,
 PGS_WALKING,
 PGS_FINISHED,

};


typedef struct PlanItem {
  char type;
  char isSCN;
  short question;
  short rule;
  short scn;
  short act;

  int score;
  RuleType* pRule;
  ActionType* pAct;
  struct HeuristicKnowledge * pHeurist;

  struct list_head list;

}PlanItemType;



typedef struct RecordItem {

  // TCB current state
  char isSCN;
  short question;
  short rule;
  short scn;
  short act;



  // to be decided for how few plan should be record, five?
  short planSum;
  PlanItemType plans[PLAN_MAX_NUMS];

  short entityNUMS;
  struct list_head entities;
  struct list_head list;

/*
  short ruleCacheSize;
  short scnCacheSize;
  struct list_head ruleCache;
  struct list_head scnCache;
*/

}RecordItemType;


struct RegisterLoader {
  char launchType;

  PlanItemType *current;
  short planSum;

  SlabType *pPlanSlab; 

  struct list_head plans;


#if 0
  short planStartIndex;
  short planEndIndex;
  short planIndex[PLAN_MAX_NUMS];     /* for arranging the order of the data, not used now. */
  PlanItemType plans[PLAN_MAX_NUMS];
#endif
};











/********************************************
  Talk machine control block is mainly
  the Core computing database of everything.
********************************************/
struct TalkControlBlock {

  struct RegisterLoader *currLoader;
  struct RegisterLoader *exploreLoader;


  /* current states */
  short cur_question;
  short cur_rule;
  short cur_scn;
  short cur_act;
  char isSCN;
  char talkLevel;

//  enum TalkProgress progress;
  LogicQueueType intention;

  /* Desires, corresponding to the question */
  LogicQueueType desire[5];



  /* Caches */
  //record history should be cycle buffer, link arrary is not good as
  //link list. create a slab cache.
  short recordSum;
//  short recordIndex[];
//  RecordItemType recordMemory[RECORD_MAX_NUMS];
  struct list_head recordMemory;
  SlabType *pRecordSlab; 




  /* Heuristic Knowledge */
  struct HeuristicKnowledge heurist[TCB_GEN_HEURISTIC_CACHE_SIZE];
  struct HeuristicKnowledge acks[TCB_ACK_HEURISTIC_CACHE_SIZE];

  /* current data record */
  //moved to recordItem
  /* Plans *///to be moved to planItem




  struct list_head entities;

  /* */
  struct TopicFrame topicIndexTree;

}TCB;












/*----------------------------------------------------------------------------
   local varables
------------------------------------------------------------------------------*/

static struct HashNode hashArry[HASH_TABLE_SIZE];

static struct HashTable hashtbl = {HASH_TABLE_SIZE, 0, hashArry};

static  struct RegisterLoader loader[2];


/*----------------------------------------------------------------------------
   global varables
------------------------------------------------------------------------------*/
#define InitWord(str) {str, sizeof(str)}
WordType specialRelationName[SpecialRelationE] = {
  InitWord("REL_UNKNOWN"),

  InitWord("REL_ISPO"),
  InitWord("REL_ISATTRO"),
  InitWord("REL_ISSYMBO"),
  InitWord("REL_ISKO"),

  InitWord("REL_HAPPEN_PLACE"),
  InitWord("REL_HAPPEN_TIME"),
  InitWord("REL_LOCATION"),

  InitWord("REL_FSTW"),
  InitWord("REL_FSFW"),
  InitWord("REL_FSPW"),
  InitWord("REL_FSBKW"),
  InitWord("REL_FSAGEW"),

  InitWord("REL_NN"),
  InitWord("REL_AM"),
  InitWord("REL_GN")
    
};



/*----------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/


inline CarriageType * GetNextCar(struct list_head *list)
{
  return container_of(list->next,CarriageType, list);
}

inline CarriageType * GetCar(struct list_head *list)
{
  return container_of(list,CarriageType, list);
}

void FreeCarriageList(CarriageHeadType *pHead, void (*FreeCarData)(void*))
{
  struct list_head * pos, *head;
  CarriageType *pCar;

  head = &pHead->list;
  list_for_each(pos, head)
  {
    pCar = GetCar(pos);

    if (FreeCarData)
      FreeCarData(pCar->pdata);

    FreeCarriage(pCar);
  }
  INIT_LIST_HEAD(head);
}


/******************************************************
 Loading data from database, specificly now is from
 the inversed sort file.
*******************************************************/
Status LoadHashIndex(struct HashTable htbl)
{



}


#define LOGICUNITREADBUFFSIZE 100
/* Allocate object automatically */
Status ReadFOPLByInvSItem(InvSItemType *pItem, FOPL_Type **pFOPL)
{
  char buff[LOGICUNITREADBUFFSIZE];
  FOPL_Type readUnit;
  static FOPL_Type *pFRead = NULL;
  Status ret;

  /* get a maximum case unit for local use. */
  if (!pFRead){
    ret = CreateLogicUnit(LOGIC_FOPL_TYPE, 3, &pFRead);
    MustOK(ret);
  }
    
  if(pItem->isSCN)
  {
    /* read the logic unit --FOPL-- by the location (SCN,act,fpred) */
    pItem->scn
    pItem->act
    pItem->fpred
    
  }
  else
  {
    /* read the logic unit --FOPL-- by the location (rule,fpred) */
    pItem->rule
    pItem->fpred
  }

  pFOPL = pFRead;

}

Status ReadRuleByInvSItem(InvSItemType *pItem, RuleType **pFOPL)
{
  

}


Status ReadActionByInvSItem(InvSItemType *pItem, ActionType **pAct)
{
  

}


Status ReadSCNByInvSItem(InvSItemType *pItem, SCNType **pSCN)
{
  

}


inline int InvSItemCmp(InvSItemType *pItem1, InvSItemType *pItem2)
{
  if (pItem1->question != pItem2->question ||
      pItem1->isSCN != pItem2->isSCN)
    return -1;

  if (pItem1->isSCN == TRUE)    
  {
    if (pItem1->scn != pItem1->scn ||
        pItem1->act != pItem1->act)
      return -1;
  }
  else
  {
    if (pItem1->rule != pItem1->rule)
      return -1;
  }

  return 0;
}



static uint32 NameHash(char * name, short len)
{
  //BKDRHash(char *str)
  int i;
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

  for (i = 0; i < len; i++)
	{
		hash = hash * seed + (*name++);
	}

	return (hash & 0x7FFFFFFF);

}

Status  CheckHash(uint32 hashval, WordType *pwd)
{
  struct HashNode *pNode;
  
  if (hashval > hashtbl.sum)
    return ERR_NOT_FOUND;

  pNode = &hashtbl.nodes[hashval];
  while(pNode){
    if(strlcmp(pNode->name, pwd->pdata, pwd->len)== 0)
      return OK;
    pNode = pNode->next;
  }
  return ERR_NOT_FOUND;
}




/* About how to match between two FOPL, we adopt the classic
 * logic mathematics.
 */
Status FOPL_MatchStrict(FOPL_Type *pFOPL1, FOPL_Type *pFOPL2)
{
  int i;
  uint8 entitySum;
  EntityType *pEntt1,*pEntt2;

  if (pFOPL1->entitySum != pFOPL2->entitySum)
    return ERR_FAILED;

  if (word_comp(&pFOPL1->name, &pFOPL2->name))
    return ERR_FAILED;

  entitySum = pFOPL1->entitySum;
  
  /* Here we don't use the Micro list_for_each() because we
     are rolling two lists synchroniclly for comparing */
  pEntt1 = list_entry(pFOPL1->Entities->next, EntityType, list);
  pEntt2 = list_entry(pFOPL2->Entities->next, EntityType, list);
  for (i = 0; i < entitySum; i++)
  {
    pEntt1 = list_entry(pEntt1->next, EntityType, list);
    pEntt2 = list_entry(pEntt2->next, EntityType, list);

    if (!IsVarEntity(pEntt1) && !IsVarEntity(pEntt2))
    {
      if (!word_comp(&pEntt1->name, &pEntt2->name))
        return ERR_FAILED;
    }
  }

  return OK;
}



Status QueryLogic(LogicQueueType *pQueue, CarriageHeadType *pCarriageHead)
{
  Status ret;
  LogicLinkerType *pLL;

  assert(pQueue && pQueue->logicHead && pQueue->logicSum > 0);

  pLL = LogicLinkerNext(&pQueue->LogicHead);
  if (pQueue->logicSum == 1)
  {
    ret = QueryLogicSingle(pLL, FOPL_MatchStrict, pCarriageHead);
  }
  else
  {
    ret = QueryLogicMulti(pQueue, FOPL_MatchStrict, pCarriageHead);
  }

  return ret;
}


/*
*  This is a primary query interface, upon which could be modifed for more functions.
*/
Status QueryLogicSingle
(
  LogicLinkerType *pLinker, 
  FOPLMacth fmatch,
  CarriageHeadType *pCarHeadInvSItem
)
{
  int i;
  Status ret;
  FOPL_Type *pFOPL;
  FOPL_Type *pFRead = NULL;
  uint32 hashIndex;
  struct InvSTable *pInvStbl;
  CarriageType *pCarriage;

  if (pLinker->masterType != LOGIC_FOPL_TYPE 
      || !fmatch
      || !pLinker
      || !pCarHeadInvSItem)
    return ERR_INVLID_ARG;
  
  pFOPL = LinkerGetFOPL(pLinker);
  // no need to care the pFOPL->type : FOPL_NOTYPE_PRED FOPL_USUAL_PRED FOPL_SPECIAL_PRED 

  hashIndex = NameHash(pFOPL->name.pdata, pFOPL->name.len);
  ret = CheckHash(hashIndex, &pFOPL->name);
  MustOK(ret);

  
  /********************************************
   The index is all in RAM cache now, later
   if the RAM limitted, only keep the hash
   table and maintain dynamic inversed sort 
   tabel in the RAM.
  */
  pInvStbl = &hashtbl.nodes[hashIndex].invstbl;


  /* to choose several items in the inversed table. 
     This is a query algorithom, could be updated. */

  pCarHeadInvSItem->sum = 0;
  INIT_LIST_HEAD(&pCarHeadInvSItem->list);

  for (i = 0; i < pInvStbl->sum; i++)
  {
    ret = ReadFOPLByInvSItem(&pInvStbl->items[i], &pFRead);
    if (ret != OK)
      continue;

    if (fmatch(pFOPL, pFRead))
    {
      pCarriage = ALLOCCarriage();
      CheckAllocPtr(pCarriage);

      pCarriage->pdata = &pInvStbl->items[i];
      list_add_tail(&pCarriage->list, &pCarHeadInvSItem->list);
      pCarHeadInvSItem->sum++;
    }

  }


  return OK;
}


Status QueryLogicMulti
(
  LogicQueueType *pQueue, 
  FOPLMacth fmatch,
  CarriageHeadType *pCarHeadInvSItem
 )
{
  int i, iCar, iCarLine, iShortest;
  int shortestLen, CarLineNums;
  CarriageHeadType CarHeadInvSItem[CAR_LINE_INVSITEM_NUM_MAX];
  LogicLinkerType *pLink, *pLinkHead;
  CarriageType *pCar,*pCarLast, *pCarx;
  struct list_head *pos, *head;
  Status ret;
  char foundCommon;

  if (!pQueue || !fmatch || !pCarHeadInvSItem)
    return ERR_INVLID_ARG;

  if (pQueue->logicSum > LOGIC_QUEUE_LEN_MAX)
    return ERR_INVLID_ARG;


  /* for each FOPL, get the result list */
  pLinkHead = &pQueue->LogicHead;
  //pLink = pLinkHead
  pLink = LogicLinkerNext(pLinkHead);
  CarLineNums = 0;


  for (i = 0, ; i < pQueue->logicSum; i++)
  {
    if (pLink == pLinkHead)
    {
      //terrible Something wrong!
      return ERR_INVLID_ARG;
    }
    
    if (pLink->masterType == LOGIC_FOPL_TYPE)
    {
      ret = QueryLogicSingle(pLink,fmatch,&CarHeadInvSItem[CarLineNums]);

      if (ret = OK && CarHeadInvSItem[CarLineNums].sum > 0)
        CarLineNums++;
    }

    pLink = LogicLinkerNext(pLink);
  }

  
  pCarHeadInvSItem->sum = 0;
  INIT_LIST_HEAD(&pCarHeadInvSItem->list);

  /****************************************
  * for the Multi list intersect compute,
  * we start with the shortest list, and
  * keep the common members.
  *****************************************/
  /* get the shortest list */
  shortestLen = CarHeadInvSItem[0].sum;
  for (i = 1; i < CarLineNums; i++)
  {
    if (CarHeadInvSItem[i].sum < shortestLen){
      shortestLen = CarHeadInvSItem[i].sum;
      iShortest = i;
    }
  }


  /* get the  AnBnC... of all the lists: info of common Rule/SCN  */
  pCar = GetNextCar(&CarHeadInvSItem[iShortest].list);
  for (iCar = 0; iCar < shortestLen; iCar++)
  {
    foundCommon = 0;
    for (iCarLine = 0; iCarLine < CarLineNums; iCarLine++)
    {
      if (iCarLine == iShortest)
        continue;

      /* loop in list to check each */
      head = &CarHeadInvSItem[iCarLine].list;
      list_for_each(pos, head)
      {
        pCarx = GetCar(pos);
        if (InvSItemCmp(pCar->pdata, pCarx->pdata))
        {
          foundCommon = 1;
          break;
        }
      }

      /* if not found in anyone list, it's not in common */
      if (!foundCommon)
        break;

    }

    pCarLast = pCar;
    pCar = GetNextCar(&pCar->list);

    if (foundCommon){
      list_del(&pCarLast->list);
      list_add_tail(&pCarLast->list, &pCarHeadInvSItem->list);
      pCarHeadInvSItem->sum++;
    }

  }







}



/* Topic index Tree */


#ifdef SEMFRAME_ARRAY_TREE


bool SubtopicAdd(struct TopicFrame *topicFrm, struct SubtopicNode *semNode, struct SubtopicNode *parent)
{

  semNode->sibling = parent->child;
  parent->child = semNode;

  semNode->parent = parent;
  semNode->child = NULL;


  return TRUE;
}



/*
 * Topic Framework tree model.
   pMaster: Uplayer subtopic.
   pSlave:  downlayer subtopic.
   As long as pSlave is equal or belong to pMaster, match happens.
 */

 static inline struct SubtopicNode * Id2Topic(struct TopicFrame *topicFrm, short id)
 {
   return &topicFrm->root[id];
 }


bool SubtopicMatch(void *pMaster, void *pSlave, struct TopicFrame *topicFrm)
{
  short dlevel;
  int i;
  struct SubtopicNode *pSubtopic;

  assert(pMaster->id < topicFrm->nodeSum && pSlave->id < topicFrm->nodeSum);

  dlevel = Id2Topic(topicFrm, pMaster->id)->level - Id2Topic(topicFrm, pSlave->id)->level;
  if (dlevel == 0 && pMaster->id == pSlave->id)
    return TRUE;
  else if (dlevel <= 0 )
    return FALSE;


  /* jump to parents */
  pSubtopic = Id2Topic(topicFrm, pSlave->id);
  for (i = 0; i < dlevel; i++)
    pSubtopic = pSubtopic->parent;


  if (pSubtopic->id == pMaster->id)
    return TRUE;

  return FALSE;
}




#endif


int HeuristQueueMatch(LogicQueueType *trigger, LogicQueueType *pQueue)
{
  
}


int FOPL_FineMatchInRule(LogicQueueType *pQueue, RuleType *pRule)
{
}


int FOPL_FineMatchInAction(LogicQueueType *pQueue, ActionType *pAct)
{
  
}



inline PlanItemType * GetPlan(struct list_head * list)
{
  return container_of(list, PlanItemType, list);
}


inline PlanItemType * GetNextPlan(struct list_head * list)
{
  return container_of(list->next, PlanItemType, list);
}


inline Status PutPlan2Loader(struct RegisterLoader *pLoader, PlanItemType * pPlan)
{
  /* add to list tail, FIFO */
  list_add_tail(&pPlan->list, &pLoader->plans);
  pLoader->planSum++;
}


//DelPlan == FreePlan + PopPlan;
inline void DelPlan(PlanItemType * pPlan)
{
  FreepPlanAttachments(pPlan);
  FreepPlan(pPlan);
  list_del(&pPlan->list);
}


inline void FreePlan(PlanItemType * pPlan)
{
  FreepPlanAttachments(pPlan);
  FreepPlan(pPlan);
}


inline void PopPlan(PlanItemType * pPlan)
{
   list_del(&pPlan->list);
}


inline PlanItemType * GetFreePlanFromLoader(struct RegisterLoader *pLoader)
{

  return AllocPlan(pLoader->pPlanSlab);
}




#ifdef __ARRAY_PLAN_CACHE__

//队列的结构
inline PlanItemType * GetPlanFromLoader(struct RegisterLoader *pLoader, short idx)
{
  short i;
  
  if (pLoader->planSum - 1 > idx)
  {
    i = pLoader->planIndex[idx];
    if (i != -1)
      return &pLoader->plans[i];
  }

  return NULL;
}

inline PlanItemType * PopPlanFromLoader(struct RegisterLoader *pLoader, short idx)
{
  short i;
  
  if (pLoader->planSum - 1 > idx)
  {
    i = pLoader->planIndex[idx];
    if (i != -1)
      return &pLoader->plans[i];
  }
  return NULL;
}


inline PlanItemType * PopNextPlanFromLoader(struct RegisterLoader *pLoader)
{
  short i;
  
  if (pLoader->planSum > 0)
  {
    for (;;){
      i = pLoader->planIndex[pLoader->planStartIndex];
      pLoader->planStartIndex++;
      if (i != -1) // -1 is flag of poped.
        break;
    }

    pLoader->planIndex[pLoader->planStartIndex] = -1;
    pLoader->planSum--;
    pLoader->current = i;
    return &pLoader->plans[i];
  }

  return NULL;
}


inline PlanItemType * GetCurrentPlanFromLoader(struct RegisterLoader *pLoader)
{
  short i;
  
  if (pLoader->planSum > 0)
  {
    i = pLoader->current;
    return ((i==-1) ? NULL : &pLoader->plans[i]);
  }

  return NULL;
}


inline PlanItemType * GetFreePlanFromLoader(struct RegisterLoader *pLoader/*, bool overwrite*/)
{
  short i;
  
  if (pLoader->planEndIndex < PLAN_MAX_NUMS-1)
  {
    pLoader->planEndIndex++;
    pLoader->planSum++;
    i = pLoader->planIndex[pLoader->planEndIndex];
    return &pLoader->plans[i];
  }

/*  else if (overwrite)
  {
    i = pTCB->planIndex[pTCB->planSum-1];
    return &pTCB->plans[i];
  }
*/
  return NULL;

}

#endif


inline void InitPlan(PlanItemType * plan)
{
  plan->pHeurist = NULL;
  //plan->rule->;
  
}


inline RecordItemType * GetRecord(struct TalkControlBlock *pTCB)
{
  short i;
  
  if (pTCB->recordSum > 0)
  {
/*
    i = pTCB->recordIndex[pTCB->recordSum-1];
    pTCB->recordSum++;
    return &pTCB->recordMemory[i];
*/
    return container_of(pTCB->recordMemory->next, RecordItemType, list)
  }

  return NULL;
}


inline Status PutRecord(struct TalkControlBlock *pTCB, RecordItemType * pRecord)
{
  
//  if (pTCB->recordSum >= RECORD_MAX_NUMS)
//  {
//    return ERR_NO_MEM;
//  }

  /* add to list head, FILO stack */
  list_add(&pRecord->list, &pTCB->recordMemory);
  pTCB->recordSum++;
}

inline void DelRecord(RecordItemType * pRecord)
{
  FreeRecordAttachments(pRecord);
  FreeRecord(pRecord);
  list_del(&pRecord->list);

}

inline RecordItemType * GetFreeRecord(struct TalkControlBlock *pTCB)
{

  return AllocRecord(pTCB->recordSlab);

/*
  short i;
  if (pTCB->recordSum < RECORD_MAX_NUMS)
  {
    i = pTCB->recordIndex[pTCB->recordSum];
    return &pTCB->recordMemory[i];
  }
  return NULL;
*/

}

inline void InitRecord(RecordItemType * record)
{
  record->pHeurist = NULL;
  //plan->rule->;
  
}




void ReportNoIdea(void)
{
  //
}


void ReportGoOn()
{
  
}


void ToggleLoader(struct TalkControlBlock *pTCB)
{
  struct RegisterLoader * p;

  p = pTCB->currLoader;
  pTCB->currLoader = pTCB->exploreLoader;
  pTCB->exploreLoader = p;

}

void FillRecord(RecordItemType *pRecord, struct TalkControlBlock *pTCB)
{
  int i;
  struct list_head *pos, *head;
  PlanItemType * plan;


  pRecord->question = pTCB->cur_question;
  pRecord->isSCN = pTCB->isSCN;
  pRecord->scn = pTCB->scn;
  pRecord->act = pTCB->act;
  pRecord->rule = pTCB->rule;


  i = 0;
  head = &pTCB->currLoader->plans;
  list_for_each(pos, head)
  {
    plan = GetPlan(pos);

    pRecord->plans[i].type = plan->type;
    pRecord->plans[i].question = plan->question;
    pRecord->plans[i].isSCN = plan->isSCN;
    pRecord->plans[i].rule = plan->rule;
    pRecord->plans[i].scn = plan->scn;
    pRecord->plans[i].act = plan->act;
    pRecord->plans[i].pRule = plan->pRule;
    pRecord->plans[i].pAct = plan->pAct;

    i++;

    if (i >= PLAN_MAX_NUMS)
      break;
  }

  pRecord->planSum = i;

}



Status LaunchLoader(struct RegisterLoader *pLoader)
{
  char type;
  PlanItemType *plan;
  
  type = pLoader->launchType;
  plan = pLoader->current;

  if (plan->type == PLAN_TYPE_RULE)
  {
    SpeakOut(plan->pRule, );
  }
  else if (plan->type == PLAN_TYPE_ACT)
  {
    SpeakOut(plan->pAct, );
  }
  else if (plan->type == PLAN_TYPE_HEURIST)
  {
    SpeakOut(plan->pHeuristd->txt, );
  }

  if (type == LAUNCH_TYPE_ACKING)
  {
    DailogueQuery(,,);
  }
  
}






/*****************************************************
 Reasoning Rules:
 1. BDI of a rule Knowledge.
 2. Grounding and query to User if not sure of several
     options.
 3. Current relativity First. Then Topic tree relation.
 4. Fast heuristic cache hit.

 Apply order in descending.
****/

typedef enum {
  THK_UNKNWON = 0,
  THK_STOPED,
  THK_EXPLORE,
  THK_ACKING,
  THK_HOLD,

  THK_MAX
}ThinkStateE;



#define TCB(x) TCB.x
#define QUESTION_CHANGE_THRESHOLD 3
Status LogicThinkerCore(  LogicQueueType *pQueue)
{
  CarriageHeadType CarriageHead;
  CarriageType *pCar;
  InvSItemType *pInvSItem;
  static struct TalkControlBlock *pTCB = &TCB;
  struct HeuristicKnowledge *pHeuristKnow;
  PlanItemType * pPlan;
  RecordItemType *pRecord;
  struct list_head *pos, *head;
  RuleType *pRuleRead;
  ActionType *pActionRead;
  Status ret;
  int i, bestHit;
  int scores, bestScores, thresholdScores;
  bool reply, find;

  //state machine. need to be in TCB ?
  static ThinkStateE state = THK_STOPED;
  static char exploreSponsor;
  static char ackType;
  static char holdType;
  static char holdEnterSponsor;
  bool machineLoop = FALSE;
  bool holdLoop;


  if (state != THK_ACKING)
  {
    /* Reasoning Rule 4. */
    bestScores = 0;
    bestHit = -1;
    for (i = 0; i < TCB_GEN_HEURISTIC_CACHE_SIZE; i++)
    {
      pHeuristKnow = &pTCB->heurist[i];
      scores = HeuristQueueMatch(&pHeuristKnow->trigger, pQueue);
      if (scores > bestScores)
      {
        bestHit = i;
        bestScores = scores;
      }
    }
  
    /*Found a good one in heurestics.
      Now we just do one heurist and 
      no InvStbl query */
    if (bestHit != -1)
    {
      pPlan = GetFreePlanFromLoader(pTCB->exploreLoader);
      if (pPlan){
        InitPlan(pPlan);
        pPlan->pHeurist = &pTCB->heurist[bestHit];
        pPlan->score = bestScores;
        pPlan->type = PLAN_TYPE_HEURIST;
        goto FINISH_MATCH:
      }
      else
      {
        ERROR_PRINT("No plan cache ! \n");
        goto FINISH_MATCH:
      }
    }
  
  
    /*---------------------------------------
     InvStbl query from database, need to sort
     in Priority of the results, because 
     the results may so long that need cut 
     -------------------------------------*/
    ret = QueryLogic(pQueue, &CarriageHead);
    if (ret != OK){
      FreeCarriageList(&CarriageHead);
      goto FINISH_MATCH:
    }
    
    head = &CarriageHead.list;
    list_for_each(pos, head)
    {
      pCar = GetCar(pos);
      pInvSItem = GetCarData(InvSItemType, pCar);

      if (pInvSItem->isSCN == 0)
      {
        ReadRuleByInvSItem(pInvSItem, &pRuleRead);
        if (!pRuleRead)
        {
          ERROR_PRINT("Read Rule failed!\n ");
          continue; // or break; ???
        }
        
        ret = FOPL_FineMatchInRule(pQueue, &pRuleRead);
        if (ret == OK)
        {
          pPlan = GetFreePlanFromLoader(pTCB->exploreLoader);
          if (!pPlan)
          {
            ERROR_PRINT("No plan cache ! \n");
            goto FINISH_MATCH:
          }
  
          InitPlan(pPlan);
          pPlan->question = pInvSItem->question;
          pPlan->isSCN = pInvSItem->isSCN;
          pPlan->rule = pInvSItem->rule;
          pPlan->scn = pInvSItem->scn;
          pPlan->act = pInvSItem->act;
          pPlan->pRule = pRuleRead;
          pPlan->type = PLAN_TYPE_RULE;
  
        }
        else
        {
          FreeRule(pRuleRead);
        }
  
  
      }
      else if (pInvSItem->isSCN == 1)
      {
        ReadActionByInvSItem(pInvSItem, &pActionRead);
        if (!pRuleRead)
        {
          ERROR_PRINT("Read Rule failed!\n ");
          continue; // or break; ???
        }
  
  
        ret = FOPL_FineMatchInAction(pQueue, &pActionRead);
        if (ret == OK)
        {
          pPlan = GetFreePlanFromLoader(pTCB->exploreLoader);
          if (!pPlan)
          {
            ERROR_PRINT("No plan cache ! \n");
            goto FINISH_MATCH:
          }
  
          InitPlan(pPlan);
          pPlan->question = pInvSItem->question;
          pPlan->isSCN = pInvSItem->isSCN;
          pPlan->rule = pInvSItem->rule;
          pPlan->scn = pInvSItem->scn;
          pPlan->act = pInvSItem->act;
          pPlan->pAct = pActionRead;
          pPlan->type = PLAN_TYPE_ACT;
  
        }
        else
        {
          FreeAction(pActionRead);
        }
  
  
      }
  
    }
  
    /* Cardata:invsItem is in RAM, no need to free.*/
    FreeCarriageList(&CarriageHead, NULL);

  }

FINISH_MATCH:


 /*****************************************
  * State Machine main loop. 
  * 
  *****************************************/


  do{
    switch(state)
    {


      case THK_EXPLORE:

        /* Lanuch Loader to send out */
        //*((PlanItemType*)pTCB->exploreLoader) = *GetPlan(pTCB);
        if (pTCB->exploreLoader->planSum == 0){
          ReportNoIdea();
          find = FALSE;
        }
        else{
          find = TRUE;
          pTCB->exploreLoader->current = GetNextPlan(&pTCB->exploreLoader->plans);
          pTCB->exploreLoader->launchType = LAUNCH_TYPE_ACKING; //for acking.
          LaunchLoader(pTCB->exploreLoader);
        }


        if (exploreSponsor == EXPLSPONSOR_STOPER)
        {
          if (find){
            ackType = ACKTYPE_SEARCH;
            state = THK_ACKING;
          }
        }
        else if (exploreSponsor == EXPLSPONSOR_HOLDER)
        {
          if (find)
          {
            ackType = ACKTYPE_CHANGE;
            state = THK_ACKING;
          }
          else
          {// switch failed, back to hold.
            state = THK_HOLD;
          }
        }

        machineLoop = FALSE;

        break;





      case THK_ACKING:
        /* grounding -- check the answer */
        bestScores = 0;
        bestHit = -1;
        reply = FALSE;
        for (i = 0; i < TCB_ACK_HEURISTIC_CACHE_SIZE; i++)
        {
          pHeuristKnow = &pTCB->acks[i];
          scores = HeuristQueueMatch(&pHeuristKnow->trigger, pQueue);
          if (scores > bestScores)
          {
            bestHit = i;
            bestScores = scores;
          }
        }
        
        if (bestHit >= 0) 
        {
          reply = IsYesReply(&pTCB->acks[bestHit]);
        }

        
        if (ackType == ACKTYPE_SEARCH)
        {
          if (reply){
            //ToggleLoader();
            state = THK_HOLD;
            holdType = HOLDTYPE_ENTER;
            holdEnterSponsor = ACKTYPE_SEARCH;
            machineLoop = TRUE;
          } else {
            state = THK_EXPLORE;
            machineLoop = TRUE;
          }
        }
        else if (ackType == ACKTYPE_CHANGE)
        {
          if (reply){//change sucessful, 
            //ToggleLoader();
            state = THK_HOLD;
            holdType = HOLDTYPE_ENTER;
            holdEnterSponsor = ACKTYPE_CHANGE;
            machineLoop = TRUE;
            
          } else {
            // switch failed, back to hold.
            state = THK_HOLD; // back to hold, and pop a plan.
            machineLoop = TRUE;
          }
        }

        break;

      case THK_HOLD:


        //grounded.
        if (holdType == HOLDTYPE_ENTER)
        {
          //do some init.
          if (holdEnterSponsor == ACKTYPE_CHANGE)
          {
           // put into record before toggle.
           FillRecord(pRecord, pTCB);
           PutRecord(pTCB, pRecord);
          }
          ToggleLoader(pTCB);

          pPlan = pTCB->currLoader->current;
          assert(pPlan);

          pTCB->talkLevel = 0;
          pTCB->cur_question = pPlan->question;
          pTCB->isSCN = pPlan->isSCN;
          pTCB->cur_scn = pPlan->scn;
          pTCB->cur_act = pPlan->act;
          pTCB->cur_rule = pPlan->rule;


          if (pTCB->currLoader->planSum <= 1) //there are no more plans.
            ReportGoOn();
            find = FALSE;
            holdType = HOLDTYPE_USUAL;
            machineLoop = FALSE;
            break;
          }

          // send out a plan.
          pPlan = GetNextPlan(&pTCB->currLoader->current->list){
          pTCB->currLoader->launchType = LAUNCH_TYPE_SAYING;
          LaunchLoader(pTCB->currLoader);

          holdType = HOLDTYPE_USUAL;
          machineLoop = FALSE;
          break;
        }


        /*---------------------------------------------------
          Usual holding topic talking.
        */

        if (pTCB->exploreLoader->planSum == 0)){
          if (pTCB->talkLevel > 0)
            pTCB->talkLevel--;
          ReportNoIdea();
          break; //Failed exitence;
        }


        find = FALSE;

        bool rescan = FALSE;

        head = &pTCB->exploreLoader->plans;

        do{

          list_for_each(pos, head)
          {
  
            
            pPlan = GetPlan(pos);
  
            if (rescan== FALSE)
            {
                if (pPlan->question == pTCB->cur_question)
                  find = TRUE;
            }
            else
            {
  
                /* Check condition for change/switch */
                if (pPlan->type == 1)//heurist
                {
                  if ( pPlan->question != pTCB->cur_question && pPlan->score > QUESTION_CHANGE_THRESHOLD)
                  {
                    //change/switch the subtopic.
                    state = THK_EXPLORE;
                    exploreSponsor = EXPLSPONSOR_HOLDER;
                    machineLoop = TRUE;
                    break; // change exitence;
                  }
      
                }
                else if (pPlan->type == 2 || pPlan->type == 3)
                {
      
                  if (pPlan->question != pTCB->cur_question)
                  {
                    if (pTCB->talkLevel <= 0)
                    {
                      //change/switch the subtopic.
                      state = THK_EXPLORE;
                      exploreSponsor = EXPLSPONSOR_HOLDER;
                      machineLoop = TRUE;
                      break;// change exitence;
                    }
                  }
                }
    
            }
  
            // wait to find the first one and accept it.
            if (!find){
              continue;
            }
              
  
            /* Main of Holding talk */
            // how to record the much data history ???
            // put into record before toggle.
            pRecord = GetFreeRecord(pTCB);
            if (!pRecord)
            {
              
            }
  
  
            FillRecord(pRecord, pTCB);
            PutRecord(pTCB, pRecord);
            ToggleLoader(pTCB);
    

            pTCB->isSCN = pPlan->isSCN;
            pTCB->cur_scn = pPlan->scn;
            pTCB->cur_act = pPlan->act;
            pTCB->cur_rule = pPlan->rule;

            // here may add more complex smart dialogue-act deduction model.
            //get intention and send out.
            pTCB->currLoader->launchType = LAUNCH_TYPE_SAYING;
            LaunchLoader(pTCB->currLoader);
    
    
            pTCB->talkLevel += 1;
            break;
          }

          /* First scan result check */
          if (rescan == FALSE && !find){
              rescan = TRUE;
            if (pTCB->talkLevel > 0)
              pTCB->talkLevel--;
          }else if (rescan == TRUE)
            rescan == FALSE;

        }while(rescan);

        machineLoop = FALSE;
        break;







      case THK_STOPED:
        
        state = THK_EXPLORE;
        exploreSponsor = EXPLSPONSOR_STOPER;
        machineLoop = TRUE;
        break;

      default:
        machineLoop = FALSE;
        break;

    }

  }while(machineLoop);







}











Status StartTalkMachine()
{
  /* Memory Init */



}


Status StopTalkMachine()
{
  /* Memory flush */



}


