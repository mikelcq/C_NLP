/*===========================================================================

FILE: syntax.c

DESCRIPTION
   This file contains the functions of syntax analysis.

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
#include"logic.h"


/*----------------------------------------------------------------------------
   Features
------------------------------------------------------------------------------*/
#define _DEBUG_ 0


/*----------------------------------------------------------------------------
   Definitions
------------------------------------------------------------------------------*/


#define PP_CACHE_SIZE 10
#define PP_POS_SIZE 15
#define PP_PARSE_RUlE_SIZE 20


#define ATTR_INTERCEPT_LOOKUP_SIZE (sizeof(attrInterceptLookup)/sizeof(attrInterceptLookup[0]))
#define CACHE_SIZE (sizeof(caches)/sizeof(caches[0]))

#define BindValenVar(pValen,var,pArg) do{pValen->pVars[var].pArg = (void*)(pArg)}while(0)

#define Bind(p,v,n) do{((PredValenType*)(p.valence))->pVars[v].pArg  \
                        =(void*)(&(n));}while(0)
#define XSenBind(pxsen,iv,x,in) Bind(pxsen->pXWds[iv],x,pxsen->pXWds[in])

#define WordPOS(x) (x.featureCore.pos)
#define IsPredWord(x) (WordPOS(x) == POS_V) 
#define IsNomWord(x) ((WordPOS(x) == POS_N)||(WordPOS(x) == POS_R) || (WordPOS(x) == POS_M) || (WordPOS(x) == POS_Q))
#define IsPrepWord(x) (WordPOS(x) == POS_P) 

#define WordIsInComp(w,C) ((w)<(C.wend) && (w)>(C.wstart))
#define IsPOSUDe(x) () //4TO_BE_DONE 


/*----------------------------------------------------------------------------
   types/structures
------------------------------------------------------------------------------*/

typedef int Status;
typedef int16 WordID;



/*
 * records of computations.
 */
struct SyntaxRecords {
  char ambiguSum; //sum of ambigurations.
  char SyntaxError; //
  
};



/*********************************************************************************
 *  following logic bind is to bind the word into FOPL-like Valence compute frame.
 *  These are dynamic compute datas diff with the xword which is static data type
 *    aiming at data storing.
 ********************************************************************************/

#ifdef DYNAMIC_LOGIC_BINDER //dynamic compute data
struct BindNode {
  struct list_head varBinds;
  struct list_head nomBinds;

  WordID predID;
  WordID nomID;
  uint8 varID;
  uint8  flag; //reserved.

};

struct PredBind {
  struct list_head preds;

  struct VarBind {
    struct list_head varBinds;
    uint8  flag;
    uint8 varID;
    uint16 bindSum;
  }varBinder[5]; // Because variable numbers of predicate are limited in about 3, 
                 // so five is enough and array is quicker than link.
  
  WordID id;
};

struct NomBind {
  struct list_head noms;
  struct list_head nomBinds;
  uint16 bindSum;
  WordID id;
  uint8  flag;
};


struct LogicBind {
  struct list_head preds;
  struct list_head noms;
  uint16 predSum;
  uint16 nomSum;

};

#else  // static data structure, it should be much quicker.

#define BINDER_MAX_NOMS 20
#define BINDER_MAX_PREDS 10
#define PRED_MAX_VARS 5
#define NOM_MAX_BINDS 4
#define VAR_MAX_BINDS 5


struct PredBind {
  struct VarBindNode {
    int8 bindSum;
    WordID binds[VAR_MAX_BINDS];
  }vars[PRED_MAX_VARS];

  int8 varSum;
  WordID id;
};


struct NomBind {
  struct NomBindNode{
    WordID bindPred;
    uint8 varID;
  }binds[NOM_MAX_BINDS];

  int8 bindSum;
  WordID id;
};


struct LogicBind {
  int8 predSum;
  int8 nomSum;
  struct PredBind pred[BINDER_MAX_PREDS];
  struct NomBind nom[BINDER_MAX_NOMS];

};

#endif

/*******************************************************
 * PP cache that contain the current PP of the sentence.
 *
 ******************************************************/
typedef struct {
  XWordType *prep;
  WordID id;
  uint8 ruleId;
  SenGrammCompType grammInfo;
}PPType;


struct PP_cache {
  uint8 size;
  PPType PPtable[PP_CACHE_SIZE];

};


/* 
 * PP parse static rules. 
 * Now, sort the rules in descending order of length!
 */
struct PP_ParseRule {
  POS_E POS_List[PP_POS_SIZE];
};



/*****************************************************************
 * syntax control block (SCB) is the main processing info database
 *  , including datas and control info. 
 *****************************************************************/
struct SyntaxCtlBlock {
  XSenType xsen;

  /* static rules */
  struct PP_ParseRule PP_ParRules[PP_PARSE_RUlE_SIZE];

  /* Logic infomation. */
  struct LogicBind binder;
  LogicQueueType logicQueue;

  struct SemFrame semTree;

  /* data Caches */
  struct PP_cache PPcach;

  SlabType *pCacheXWordCrdnt;
  SlabType *pCacheXword;
  
  SlabType *pCacheNomValence;
  SlabType *pCachePredValence;
  SlabType *pCachePredVars;
  SlabType *pCacheSpecialPred;
  SlabType *pCacheSpecialPredVars;
  
  SlabType *pCacheFOPL;
  SlabType *pCacheFOL_Operator;
  
  SlabType *pCacheSemNode;
  SlabType *pCacheAttribute;
  SlabType *pCacheFeatureCore;


};




/***************************************************
  Main tree info of sematic class framework .
  Be aware that the tree is designed as static style.
 ***************************************************/
#ifdef SEMFRAME_LINKED_TREE
#define SEMFRAME_TREE_HIGHT_MAX 10
#define FAMILY_LINK_TAIL 1
#define FAMILY_LINK_HEAD 0
struct SemFrame {
  short levels;
  struct SemNode *family[SEMFRAME_TREE_HIGHT_MAX][2];

};

struct SemNode {
  char * name;
  short  level;
  short  familyID;
  short  siblingID;   /* sibling index */
  struct SemNode *parent; /* when parent is NULL, it's root. */
  struct SemNode *child;
  struct SemNode *sibling;
  struct SemNode *family;
};

#else
/* SEMFRAME_ARRAY_TREE */

struct SemFrame {
  short levels;
  short nodeSum;
  struct SemNode *root; /* root is the first element of the array */
};

struct SemNode {
  char * name;
  short  level;
  short  id;    /* global ID in tree */
  struct SemNode *parent; /* when parent is NULL, it's root. */
  struct SemNode *child;
  struct SemNode *sibling;
};


#endif

/*****************************************
 attribute intercept methods
 ******************************************/
enum AttrInterceptMode
{
  AIM_DEFAULT = 0,
  AIM_MATCH = AIM_DEFAULT,
  AIM_MAP,
  AIM_OPS,

  AIM_MAX
};

struct AttrIntercept {
  char * attrName;
  enum AttrInterceptMode mode;
  bool (* fp) (void*);
};


/*************************************
 Cache table info. 
*************************************/
struct CacheInfo {
  SlabType *pCache;
  int objsize;
  short num;
  unsigned int flag;
};

/*----------------------------------------------------------------------------
   local varables
------------------------------------------------------------------------------*/
/* now static alloc */
static struct SyntaxCtlBlock SCB;  


/*************************************
  caches manual table. 
**************************************/
#define CACHE_SIZE_XWORD 50
#define CACHE_SIZE_NOM_VALEN 30
#define CACHE_SIZE_PRED_VALEN 10
#define CACHE_SIZE_PRED_VARS 25
#define CACHE_SIZE_SPECIAL_PRED 20
#define CACHE_SIZE_SPECAILPRED_VARS 40
#define CACHE_SIZE_FOPL 30
#define CACHE_SIZE_FOL_OPTRS 30
#define CACHE_SIZE_SEMNODES 200
#define CACHE_SIZE_ATTRIBUTES 200
#define CACHE_SIZE_FEATURES 50

#define SCB(x) SCB.x 

static struct CacheInfo caches[] = {
  { SCB(pCacheXWordCrdnt),   sizeof(WCrdntType),      CACHE_SIZE_XWORD,        SLAB_SIMPLE_MASK },
  { SCB(pCacheXword),        sizeof(XWordType),       CACHE_SIZE_XWORD,        SLAB_SIMPLE_MASK },
  
  { SCB(pCacheNomValence),   sizeof(NomValenType),    CACHE_SIZE_NOM_VALEN,    SLAB_SMART_MASK },
  { SCB(pCachePredValence),  sizeof(PredValenType),   CACHE_SIZE_PRED_VALEN,   SLAB_SMART_MASK },
  { SCB(pCacheSpecialPred),  sizeof(SpecialPredType), CACHE_SIZE_SPECIAL_PRED, SLAB_SMART_MASK },
  { SCB(pCachePredVars),     sizeof(PredVarType),     CACHE_SIZE_PRED_VARS,    SLAB_SIMPLE_MASK },
  { SCB(pCacheSpecialPredVars), sizeof(SpecialPredVarType), CACHE_SIZE_SPECAILPRED_VARS, SLAB_SIMPLE_MASK },
  
  { SCB(pCacheFOPL),         sizeof(FOPL_Type),       CACHE_SIZE_FOPL,         SLAB_SMART_MASK  },
  { SCB(pCacheFOL_Operator), sizeof(FOL_Operator),    CACHE_SIZE_FOL_OPTRS,    SLAB_SMART_MASK  },

  { SCB(pCacheSemNode),      sizeof(struct SemNode),  CACHE_SIZE_SEMNODES,     SLAB_SIMPLE_MASK },
  { SCB(pCacheAttribute),    sizeof(AttrType),        CACHE_SIZE_ATTRIBUTES,    SLAB_SIMPLE_MASK },
  { SCB(pCacheFeatureCore),  sizeof(WordFeatureCoreType), CACHE_SIZE_FEATURES, SLAB_SIMPLE_MASK },



};


/*
  atrributes intercept types table.
*/

struct AttrIntercept attrInterceptLookup[] = {
  { "attr1",AIM_DEFAULT, NULL },
  { "attr2",AIM_DEFAULT, NULL },
  { "attr3",AIM_DEFAULT, NULL },
  { "attr4",AIM_DEFAULT, NULL },

};




/*----------------------------------------------------------------------------
   global varables
------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/
SenMapType GenSenMap(wdseg_type wdSeg);
void InitLogicBinder(struct LogicBind *pBinder);
Status ScanPredNom(XSenType *pxsen, struct LogicBind *pBinder);
Status ScanPrep(XSenType *pxsen, struct PP_cache *PPcach);
Status PP_Scan(XSenType *pxsen, struct PP_cache *pPPcach, struct PP_ParseRule *pPP_ParseRules);
Status SyncNom4PP(struct PP_cache *pPPcach, struct LogicBind *pBinder);
Status ProcessNomHead(XSenType *pxsen, struct LogicBind *pBinder, LogicQueueType *pLQueue);
Status PredBindNom(XSenType *pxsen, struct LogicBind *pBinder, LogicQueueType *pLQueue);
Status XsenLoadXwords(XSenType *pxsen);



/*
int main (int argc, char *argv[])
{
  int i;

  return 0;
}

*/




/* centrally control the main syntax process. */
Status CentralControl()
{

  /* init the binder */
  InitLogicBinder(&SCB.binder);
  LogicQueueInit(&SCB.logicQueue);

  /* scan and sort load the POS */
  ScanPredNom(&SCB.xsen, &SCB.binder);
  ScanPrep(&SCB.xsen, &SCB.PPcach);
  PP_Scan(&SCB.xsen, &SCB.PPcach, &SCB.PP_ParRules);
  SyncNom4PP(&SCB.PPcach, &SCB.binder);
  ProcessNomHead(&SCB.xsen, &SCB.binder, &SCB.logicQueue);
  PredBindNom(&SCB.xsen, &SCB.binder, &SCB.logicQueue);

}


/*=============================================================================
FUNCTION: SyntaxAnalyze

DESCRIPTION: Call entry for Main process loop. Called by uplayer main
             NLP engine loop.

PARAMETERS:
    wdSeg : word segmentation result.
    sen   : raw sentence.
RETURN VALUE:

COMMENTS:
  None
=============================================================================*/
LogicQueueType SyntaxAnalyze(wdseg_type wdSeg, SenType sen)
{
  SenMapType senMapNew;
  XSenType *pXsen = &SCB(xsen);

  pXsen->sen = sen;
  pXsen->senMap = GenSenMap(wdSeg);

  XsenLoadXwords(pXsen);

  CentralControl();




  return SCB(logicQueue);

}


/*=============================================================================
FUNCTION: SyntaxRefreshData

DESCRIPTION: Refresh datas: free memory of this analysis session 
             and do some initialization. Called by uplayer main
             process loop.
PARAMETERS:
   None

RETURN VALUE:

COMMENTS:
  None
=============================================================================*/
Status SyntaxRefreshData(void)
{
  XSenType *pXsen = &SCB(xsen);

  pXsen->sen = {0, NULL};
  pXsen->senMap.size = {0, NULL};


  /* Flush the engine dirty datas for next analysis */


  /* xword cache flush */
  FlushXword(void);

  /* word coordinate cache flush  */
  FlushWCrdnt(void);

  /* Nominal Valence data cache flush */
  FlushNomValence(void);

  /* PredValence cache flush  */
  FlushPredValence(void);

  /* PredVars cache flush  */
  FlushPredVars(void);


  /* SpecialPred cache flush*/
  FlushSpecialPred(void);

  /* SpecialPredVars cache flush  */
  FlushSpecialPredVars(void);

  /* FOPL cache IF  */
  FlushFOPL(void);

  /* FOL_Operator cache flush  */
  FlushFOL_Operator(void);

  /* SemNode cache flush  */
  FlushSemNode(void);

  /* Attributes cache flush  */
  FlushAttribute(void);

  /* Feature cache flush  */
  FlushFeatureCore(void);

  return
}



/*=============================================================================
FUNCTION: SyntaxEngineBootup

DESCRIPTION:  syntax engine init, only once when system up.

PARAMETERS:
   None

RETURN VALUE:

COMMENTS:
  None
=============================================================================*/
int SyntaxEngineBootup()
{
  unsigned int size;
  int i;


  /* Memory initializing */
  for (i = 0; i < CACHE_SIZE; i++)
    caches[i].pCache = SlabCreat(caches[i].objsize, caches[i].num, caches[i].flag);

  return OK;
}


/*=============================================================================
FUNCTION: SyntaxEngineDown

DESCRIPTION:  syntax engine deinit, only once when system down.

PARAMETERS:
   None

RETURN VALUE:

COMMENTS:
  None
=============================================================================*/
int SyntaxEngineDown(void)
{
  unsigned int size;
  int i;


  /* Memory free */
  for (i = 0; i < CACHE_SIZE; i++)
    caches[i].pCache = SlabDestroy(&caches[i].pCache);

  return OK;
}



/* Get the standard sentence word segementation map 
   from internal bitmap */
SenMapType GenSenMap(wdseg_type wdSeg)
{
  short i,j,k,b;
  short unitsize = sizeof(wdSeg->wdseg_map[0])*8;
  SenMapType senMap;
  short pos = 0;
  short lastPos = 0;
  short wordSum = 0;
  short wdidx = 0;

  //printf("map top is: %d \n", wdSeg->top);
  
  i = wdSeg->top/unitsize;
  j = wdSeg->top%unitsize;


 /* to calculate the total bit sum. */
  for (k = 0; k < i; k++)
  {
    for (b = 0; b < unitsize; b++)
    {
      if( ((wdSeg->wdseg_map[k]>>b)&1) == 1)
      {
        wordSum++;
      }
    }
  }

  for (b = 0; b < j+1; b++)
  {
    if( ((wdSeg->wdseg_map[i]>>b)&1) == 1)
    {
      wordSum++;
    }
  }
  
  wordSum--;

  /*-----------------------------*/
  senMap.pWcrdnts = AllocWCrdnt(wordSum);
  //senMap.pWcrdnts = malloc(wordSum * sizeof(WCrdntType));
  if (!senMap.pWcrdnts)
  	return senMap; /*error, to check pWcrdnts */


  for (k = 0; k < i; k++)
  {
    for (b = 0; b < unitsize; b++)
    {
      if( ((wdSeg->wdseg_map[k]>>b)&1) == 1)
      {
        senMap.pWcrdnts[wdidx]->ws = pos;
        senMap.pWcrdnts[wdidx]->len = pos - lastPos;
        lastPos = pos;
        pos++;
        wdidx++;
        //printf("[%d] ", pos++);
      }
      else
      {
        //printf("- ");
        pos++;
      }
    }
  }

  for (b = 0; b < j+1; b++)
  {
    if( ((wdSeg->wdseg_map[i]>>b)&1) == 1)
    {
      senMap.pWcrdnts[wdidx]->ws = pos;
      senMap.pWcrdnts[wdidx]->len = pos - lastPos;
      lastPos = pos;
      pos++;
      wdidx++;
      //printf("[%d] ", pos++);
    }
    else
    {
      //printf("- ");
      pos++;
    }
  }


  senMap.size = wordSum;

  return senMap;

}




/********************************************
 Sentence loading words meanings,
 if found in dictionary, corresponding data 
 structure will be allocated automatically
 from the caches.
 ********************************************/
Status ReadFeatureFromDict(XWordType *pXword)
{
  pXword->wd.pdata
}

Status ReadValenceFromDict(XWordType *pXword)
{
  
}


/* Just load word sense, no mem allocate. */
Status LoadWrodSense(XWordType *pXword)
{
  
  ReadFeatureFromDict(pXword);

  ReadValenceFromDict(pXword);

}



Status XsenLoadWordSense(XSenType *pxsen)
{
  int i;

  //4 TO_BE_DONE 
  //Here's a place for an algorithm to load all the sense datas with minmum mem allocate overhead.
  //Only two kinds of sense struct derive from valence type: predicate and nominal type.

  
  for (i = 0; i < pxsen->senMap.size; i++)
    LoadWrodSense(&(pxsen->pXWds[i]));

  return OK;
}



/* init Xwords for sentence  */
Status XsenLoadXwords(XSenType *pxsen)
{
  int i;
  uint16 wordSum;
  Status ret;
  XWordType *pxw = NULL;

  wordSum = pxsen->senMap.size;
  pxw = AllocXword(wordSum);
  //pxw = malloc(sizeof(XWordType)*wordSum);

  if (!pxw)
    return ERR_NO_MEM;

  pxsen->pXWds = pxw;
  for (i = 0; i < wordSum; i++)
  {
    pxw[i].wd.len = pxsen->senMap.pWcrdnts[i].len;
    pxw[i].wd.pdata= pxsen->sen.pdata + pxsen->senMap.pWcrdnts[i].offset;
  }

  ret = XsenLoadWordSense(pxsen); 

  return ret;
  
}




/* Solve conflicts and any remain doubles. */
Status SolveConflict()
{}


void InitLogicBinder(struct LogicBind *pBinder)
{
  int i;

  pBinder->nomSum = 0;
  pBinder->predSum = 0;

  /* following detail clean is not needed actually. */  
  for (i = 0; i < BINDER_MAX_PREDS; i++)
  {
    pBinder->pred[i].varSum = 0;
    //more clean is not necessary.
  }

  for (i = 0; i < BINDER_MAX_NOMS; i++)
  {
    pBinder->nom[i].bindSum = 0;
    //more clean is not necessary.
  }

  return;
}



Status KickoutNomByWd(struct LogicBind *pBinder, WordID wd)
{
  int i, inom;

  for (i = 0; i < pBinder->nomSum; i++)
  {
    if (pBinder->nom[i].id == wd)
    {
      inom = i;
      break;
    }
  }

  if (i == pBinder->nomSum)
    return ERR_NOT_FOUND;

  /* Now we only resort the word ID. */
  for (i = inom; i < pBinder->nomSum; i++)
  {
    pBinder->nom[i].id = pBinder->nom[i+1].id;
  }
  pBinder->nomSum--;

  return OK;
}

Status KickoutNomByID(struct LogicBind *pBinder, int nomid)
{
  int i;

  if (nomid >= pBinder->nomSum)
    return ERR_NOT_FOUND;

  /* Now we only resort the word ID. */
  for (i = nomid; i < pBinder->nomSum; i++)
  {
    pBinder->nom[i].id = pBinder->nom[i+1].id;
  }
  pBinder->nomSum--;

  return OK;
}



/* Alg 1.  xwords is all loaded. */
Status ScanPredNom(XSenType *pxsen, struct LogicBind *pBinder)
{
  int i;
  short wordSum;
  
  wordSum = pxsen->senMap.size;

  
  /* load words  */
  for(i = 0; i < wordSum; i++)
  {
    /* predicate words */
    if (IsPredWord(pxsen->pXWds[i]))
    {
      pBinder->pred[pBinder->predSum++].id = i; 
      assert(pBinder->predSum < BINDER_MAX_PREDS);
    }
    else if (IsNomWord(pxsen->pXWds[i]))
    {
      pBinder->nom[pBinder->nomSum++].id = i;
      assert(pBinder->nomSum < BINDER_MAX_NOMS);
    }
  }

  return OK;
}



Status ScanPrep(XSenType *pxsen, struct PP_cache *PPcach)
{
  int i;
  short wordSum;
  
  wordSum = pxsen->senMap.size;

  
  /* load words  */  //to do aaaa 
  for (i = 0; i < wordSum; i++)
  {
    /* prep. words */
    if (IsPrepWord(WordPOS(pxsen->pXWds[i])))
    {
      PPcach->PPtable[PPcach->size++].id = i;
      PPcach->PPtable[PPcach->size++].prep = &pxsen->pXWds[i];
      assert(PPcach->size < PP_CACHE_SIZE);
    }
  }

  return OK;
}



/*
 * scan for Preposition phrase of a sentence.
 */
Status PP_Scan(XSenType *pxsen, struct PP_cache *pPPcach, struct PP_ParseRule *pPP_ParseRules)
{
  int i,iwd,ir;
  short wordSum;
  PPType *pPP;
  WordID ppstart;
  struct PP_ParseRule *pRule;
  bool matched;

  if(!pPPcach->size)
    return OK;

  wordSum = pxsen->senMap.size;
  pPP = pPPcach->PPtable;
  pRule = pPP_ParseRules;
  
  /*   */
  for (i = 0; i < pPPcach->size; i++, pPP++)
  {
    ppstart = pPP->id;

    /* for each PP parse rule */
    for (ir = 0; ir < PP_PARSE_RUlE_SIZE; ir++, pRule++)
    {
      matched = TRUE;
      
      /* compare the rule and the sentence at the point of PP start */
      for(iwd = 0; (iwd < PP_POS_SIZE) && (pRule->POS_List[iwd] != POS_INVALID); iwd++)
      {
        // if any word not matched.
        if (WordPOS(pxsen->pXWds[ppstart+iwd]) != pRule->POS_List[iwd])
        {
          matched = FALSE
          break;
        }
        
      }

      /* find a PP */
      if (matched)
      {
        //4 TO_BE_DONE , should allow multi-matched and select best one.
        pPP->ruleId = ir;
        pPP->grammInfo.wstart = ppstart;
        pPP->grammInfo.wend = ppstart + iwd - 1;
        pPP->grammInfo.phrase = POS_PP;
        break; // now, just find the first matched one.
      }

    }
  }

  return OK;
}



/*
 *  Synchronize Nominal table (from Binder) for Preposition phrase.
 */
Status SyncNom4PP(struct PP_cache *pPPcach, struct LogicBind *pBinder)
{
  int i,j;

  for (i = 0; i < pBinder->nomSum; i++)
  {
    for (j = 0; j < pPPcach->size; j++)
    {
      /* This Nom word is in PP, so cout it out */
      if (WordIsInComp(pBinder->nom[i].id, pPPcach->PPtable[j].grammInfo))
      {
        KickoutNomByID(pBinder, i);
      }

    }
  }

  return OK;
}





/*
定中心词算法是在谓词论元绑定前作最好，绑定就简单很多了。
定中心词主要是对体词来说的，跟进一步说，是多个体词之间指定
一个中心词，因为谓词可能错误的绑定到修饰成分的体词上, 或者介词短语中的体词上。
当然中心词也可能是并列的，但是实际上还是只指定
一个作为中心词，其他的就 NN() 到这个中心词上。
因此，在定相邻体词关系时，会涉及歧义问题，如: v + n1 + 的 + n2 (咬死猎人的狗--语用歧义，安装灯的人/开关--不同组合切分)
其他的歧义暂时没遇到。
*/


/*
 *  find the head of Nom phrase and do relation process.
 */
Status ProcessNomHead(XSenType *pxsen, struct LogicBind *pBinder, LogicQueueType *pLQueue)
{
  int inom;
  WordID inop, iwd, currWd, ispread, imatch;
  struct NomBind *pnom;
  MatchRelationType mRelation, mr_best;
  FOPL_Type *pLogic;
  SpecialPredType *pSpPred;
  Status ret;

  pnom = &pBinder->nom[0];

  //逆序
  for (inom = pBinder->nomSum - 1; inom >= 0; inom--) /* Cycle*/
  {
    currWd = pnom[inom].id;
    iwd = currWd - 1;

    
    while(iwd >= 0) /* Spread */
    {

      if (IsPOSUDe(pxsen->pXWds[iwd])/* de */
        inop = 1;
      else
        inop = 0;

      if (iwd - inop < 0)
        break; /* recheck with inop for while loop */
    
      /* 若前一个非"的"词是非体词 ，结束spread */
      if (!IsNomWord(pxsen->pXWds[iwd-inop]))
      {
        break;
      }

       /* disambiguation*/
      if ((inop == 1) && (iwd >= 2) && IsPredWord(pxsen->pXWds[iwd-2])) //VerbOnly, v + n1 + 的 + n2
      {
        //4 TO_BE_DONE 
        break;//end this cycle.
      }
     

      mr_best.nice = 0; /* invalid */
      mr_best.relation = REL_UNKNOWN;
      /* to find a best match relation amony past spreaded words */
      for (ispread = iwd+1; ispread < currWd; ispread++)
      {
        mRelation = FindNomRelation(pxsen, iwd-inop, ispread);
        if (mRelation.nice > mr_best.nice && REL_UNKNOWN != mRelation.relation){
          mr_best = mRelation;
          imatch = ispread;
        }
      }


      /* if match a relation, put into record */
      if (mr_best.relation != REL_UNKNOWN)
      {

#if 1 // to be del ??? 
        ret = CreatePredInfo(FOPL_SPECIAL_PRED, 2, &pSpPred);
        if (ret != OK || !pSpPred)
        {
          return ret;
        }
        
        pSpPred->relation = mr_best.relation;
        pSpPred->varSum = 2; //4 Vars is 2 now.
        pSpPred->pVars[0].pxword = &pxsen->pXWds[imatch];   /* Key word */
        pSpPred->pVars[1].pxword = &pxsen->pXWds[iwd-inop]; /* related word */
#endif

        /* allocate a common Logic unit */
        ret = CreateLogicUnit(LOGIC_FOPL_TYPE, 2, &pLogic);
        if (ret != OK || !pLogic)
        {
          return ret;
        }

        pLogic->type = FOPL_SPECIAL_PRED;
        pLogic->pPredInfo = (void *)pSpPred;
        pLogic->name = specialRelationName[mr_best.relation];
        pLogic->pEntities[0].name = pxsen->pXWds[imatch].wd;
        pLogic->pEntities[0].info = &pxsen->pXWds[imatch]; // temply info is point to xword. but may not.
        pLogic->pEntities[1].name = pxsen->pXWds[iwd-inop].wd;
        pLogic->pEntities[1].info = &pxsen->pXWds[iwd-inop];

        LogicQueueAddTail(pLQueue, &pLogic->logicLinker);
        ret = KickoutNomByWd(pBinder, iwd-inop);
        if (ret == OK)
        {
          inom--; /* sync where we are in the nom table. */
        }

      }



      iwd -= inop+1;
    }  /* end of Spread */


    /* Finished this nominal word cycle. and sync.ed nominal table */


  }/* end of Cycle */

}


#ifdef SEMFRAME_LINKED_TREE

bool SemAdd(struct SemFrame *semFrm, struct SemNode *semNode, struct SemNode *parent)
{

  if (parent->child == NULL)
  {
    semFrm->family[parent->level+1][FAMILY_LINK_TAIL]->family = semNode;
  }
  semNode->sibling = parent->child;
  parent->child = semNode;

  semNode->parent = parent;
  semNode->family = NULL;
  semNode->child = NULL;


  return TRUE;
}


/*
 * Sematic Framework tree model.
   pMaster: Uplayer sem.
   pSlave:  downlayer sem.
 */
bool SemMatch(SemClassType *pMaster, SemClassType *pSlave, struct SemFrame *semFrm)//(char levelM, short siblingM, short sIdxM, char levelS, short siblingS, short sIdxS)
{
  short dlevel;
  int i;
  struct SemNode *pSem;

  assert(pMaster->level < SEMFRAME_TREE_HIGHT_MAX && pSlave->level < SEMFRAME_TREE_HIGHT_MAX);

  dlevel = pMaster->level - pSlave->level;
  if (dlevel == 0 && pMaster->familyID == pSlave->familyID && pMaster->siblingID == pSlave->siblingID)
    return TRUE;
  else if (dlevel <= 0 )
    return FALSE;

  /* pointer to the (level, family, sibling)*/
  pSem = semFrm->family[pSlave->level][FAMILY_LINK_HEAD];
  for (i = 0; i < pSlave->familyID; i++)
  {
    if (NULL == pSem->family)
      return FALSE;
    pSem = pSem->family;
  }
  
  for (i = 0; i < pSlave->siblingID;i++)
  {
    if (NULL == pSem->family)
      return FALSE;
    pSem = pSem->sibling;
  }

  /* jump to parents */
  for (i = 0; i < dlevel; i++)
    pSem = pSem->parent;


  if (pSem->familyID == pMaster->familyID && pSem->siblingID == pMaster->siblingID )
    return TRUE;

  return FALSE;
}

#else
/* SEMFRAME_ARRAY_TREE */


bool SemAdd(struct SemFrame *semFrm, struct SemNode *semNode, struct SemNode *parent)
{

  semNode->sibling = parent->child;
  parent->child = semNode;

  semNode->parent = parent;
  semNode->child = NULL;


  return TRUE;
}



/*
 * Sematic Framework tree model.
   pMaster: Uplayer sem.
   pSlave:  downlayer sem.
   As long as pSlave is equal or belong to pMaster, match happens.
 */

 static inline struct SemNode * Id2Sem(struct SemFrame *semFrm, short id)
 {
   return &semFrm->root[id];
 }


bool SemMatch(SemClassType *pMaster, SemClassType *pSlave, struct SemFrame *semFrm)//(char levelM, short siblingM, short sIdxM, char levelS, short siblingS, short sIdxS)
{
  short dlevel;
  int i;
  struct SemNode *pSem;

  assert(pMaster->id < semFrm->nodeSum && pSlave->id < semFrm->nodeSum);

  dlevel = Id2Sem(semFrm, pMaster->id)->level - Id2Sem(semFrm, pSlave->id)->level;
  if (dlevel == 0 && pMaster->id == pSlave->id)
    return TRUE;
  else if (dlevel <= 0 )
    return FALSE;


  /* jump to parents */
  pSem = Id2Sem(semFrm, pSlave->id);
  for (i = 0; i < dlevel; i++)
    pSem = pSem->parent;


  if (pSem->id == pMaster->id)
    return TRUE;

  return FALSE;
}




#endif

inline bool SemClassMatch(SemClassType *pMaster, SemClassType *pSlave)
{
  //4 TO_BE_DONE  gotta use semclass tree search.
  return (0 == strcmp(pMaster->name, pSlave->name));

//  return SemMatch(pMaster, pSlave, &SCB.semTree);

}

inline bool AttrMatch(AttrType *pMaster, AttrType *pSlave)
{
  return ( (0 == strcmp(pMaster->name, pSlave->name)) && 
           (0 == strcmp(pMaster->value, pSlave->value)) );
}

inline bool WordGroupFind(WordGroupType *pG, WordType *pW)
{
  int i;
  
  for (i = 0; i < pG->sum; i++)
  {
    if (word_comp(&pG->pWords[i], pW))
      return TRUE;
  }

  return FALSE;
}
/*
  if (pWgrp->sum)
  {
    for (iwdgroup = 0; iwdgroup < pWgrp->sum; iwdgroup++){
      if (word_comp(&pWgrp->pWords[iwdgroup], &pNomWord->wd)){
        nice += NICE_ONE_SCORE*5;
        break;
      }
    }
  }
  */


/*
 * intercept relation of two attributes.
 */
Status InterceptAttrRelation(AttrType *pMaster, AttrType *pSlave)
{
  char *pstr;
  int i;

  for (i = 0; i < ATTR_INTERCEPT_LOOKUP_SIZE; i++)
  {
    pstr = attrInterceptLookup[i].attrName;
    if (strcmp(pMaster->name,pstr))
      break;
  }
  
  switch (attrInterceptLookup[i].mode)
  {
    case AIM_DEFAULT:
      if (AttrMatch(pMaster, pSlave))
        return OK;
      break;

    case AIM_MAP:
      break;

    case AIM_OPS:
      break;

    default:
      break;
  }
  
  return ERR_FAILED;
}





NiceType ConstrainFind(ConstrainType *pConsFeature, WordFeatureCoreType *pSlaveFeature)
{
  NiceType nice;
  int ivar;
  int iattr, iftr, itmp;
  WordFeatureCoreType *pFeatureCore;
  Status ret;
  
  if (0 == pConsFeature->active)
  {
    return NICE_ZERO_SCORE;
  }


  /* common Attribute checking */
  if (pConsFeature->commAttrSum > 0)
  {

    for (iattr = 0; iattr < pConsFeature->commAttrSum; iattr++)
    {
      /* to intercept common attribute. Now only support simple match and map */
      for (itmp = 0; itmp < pSlaveFeature->attrSum; itmp++)//4 TO_BE_DONE, nominal attr few than pred-var's, so move this loop outside may be better. 
      {
        ret = InterceptAttrRelation(&pConsFeature->pCommonAttrs[iattr], &pSlaveFeature->pAttrs[itmp]);
        if (OK == ret)
          nice += NICE_ONE_SCORE*2;
        
      }
    }
  }


  /* Core features checking */
  if (pConsFeature->wordFeatureSum > 0)
  {
    for (iftr = 0; iftr < pConsFeature->wordFeatureSum; iftr++)
    {
      pFeatureCore = &pConsFeature->pWordFeatures[iftr];

      /* to intercept core feature */
      if (SemClassMatch(pFeatureCore->semClass, pSlaveFeature->semClass))
      {
        nice += NICE_ONE_SCORE*3;

        /* intercept this feature's attributes that live together with above semClass*/
        for (iattr = 0; iattr < pFeatureCore->attrSum; iattr++)
        {
          /* to intercept common attribute. Now only support simple match and map */
          for (itmp = 0; itmp < pSlaveFeature->attrSum; itmp++)
          {
            ret = InterceptAttrRelation(&pFeatureCore->pAttrs[iattr], &pSlaveFeature->pAttrs[itmp]);
            if (OK == ret)
              nice += NICE_ONE_SCORE*2;
            else if (ERR_NEGETIVE_ANSWER == ret)
              nice -= NICE_ONE_SCORE*2;
          }
        }

      }/* end of core feature*/

    }
  }



  return nice;
}




/*
 * Word relation. close location of words.  no valence check.
 * including GN, NN. 
 * word2 is after word1.
 */
MatchRelationType FindNomRelation(XSenType *pxsen, WordID wordM, WordID wordS)
{
  NiceType nice, retNice;
  int iwdgroup;
  XWordType *pWordM, *pWordS;
  NomValenType *pNomValenM, pNomValenS;
  WordGroupType *pWgrp;
  SpecialRelationE relation;
  MatchRelationType mRelation = {REL_UNKNOWN, 0};

  if (!IsNomWord(pxsen->pXWds[wordM]) || !IsNomWord(pxsen->pXWds[wordS]))
    return mRelation;

  pWordM = &pxsen->pXWds[wordM];
  pWordS = &pxsen->pXWds[wordS];
  pNomValenM = (NomValenType*)pWordM->valence;
  pNomValenS = (NomValenType*)pWordS->valence;
  nice = NICE_ZERO_SCORE;
  
  /*-----------------------------------------------------------------
    1st, Nom-valence check the symbions
    -----------------------------------------------------------------*/
  if (pNomValenM->ValenSum == 1)
  {
    /* ... try word instance group first. */
    pWgrp = &pNomValenM->pSymbions[0];
    if (WordGroupFind(pWgrp, &pWordS->wd)){
      nice = NICE_ONE_SCORE*10;
      mRelation.relation = pNomValenM->relations[0];
      goto RELATION_FOUND;
    }

    /* ...... continue search in constrain details */
    retNice = ConstrainFind(&pNomValenM->pSymbionFeature[0], &pWordS->featureCore);
    if (retNice > NICE_ZERO_SCORE){
      nice = retNice;
      mRelation.relation = pNomValenM->relations[0];
    } 
    
  }
  else if (pNomValenM->ValenSum == 2)
  {
    /*************************************************
    *  when reach here, means the matching word is 
    *  a two-valence nominal type, so, each valence
    *  has a words group list and a feature constrain.
    *  This is the maximum complex case.
    **************************************************/

    /* ... try word instance group first. */
    pWgrp = &pNomValenM->pSymbions[0];
    if (WordGroupFind(pWgrp, &pWordS->wd)){
      nice = NICE_ONE_SCORE*10;
      mRelation.relation = pNomValenM->relations[0];
      goto RELATION_FOUND;
    }else{
      /* ...... try again in the second valence word group */
      pWgrp = &pNomValenM->pSymbions[1];
      if (WordGroupFind(pWgrp, &pWordS->wd)){
        nice = NICE_ONE_SCORE*10;
        mRelation.relation = pNomValenM->relations[1];
        goto RELATION_FOUND;
      }
    }

    /* ... continue search in constrain details */
    retNice = ConstrainFind(&pNomValenM->pSymbionFeature[0], &pWordS->featureCore);
    if (retNice > NICE_ZERO_SCORE)
    {
      nice = retNice;
      mRelation.relation = pNomValenM->relations[0];
    }
    
    /* ... try again the second symbion */
    retNice = ConstrainFind(&pNomValenM->pSymbionFeature[1], &pWordS->featureCore);
    if (retNice > nice)
    {
      nice = retNice;
      mRelation.relation = pNomValenM->relations[1];
    }

  }

  

  /*-----------------------------------------------------------------
    2nd, collocation checking.
    -----------------------------------------------------------------*/
  for (relation = REL_GENERAL; relation < REL_MAX; relation++)
  {
    pWgrp = &pNomValenM->NPColl[relation].wordGroup[0];

    if (WordGroupFind(pWgrp, &pWordS->wd)){
      nice = NICE_ONE_SCORE*10;
      mRelation.relation = pNomValenM->NPColl[relation].relation; // equal to relation.
      goto RELATION_FOUND;
    }
  }

  /* gotta return a best relation from the constrains.. */
  for (relation = REL_GENERAL; relation < REL_MAX; relation++)
  {
    retNice = ConstrainFind(&pNomValenM->NPColl[relation].consFeatures, &pWordS->featureCore);
    if (retNice > nice)
    {
      nice = retNice;
      mRelation.relation = pNomValenM->NPColl[relation].relation;
    }
  }


  
RELATION_FOUND:
  mRelation.nice = nice;
  return mRelation;
}





/*
 * Dedicated func for valence checking.
 */
NiceType FindPredValenRelation(XSenType *pxsen, WordID pred, int var, WordID nom)
{
  NiceType nice;
  int ivar;
  int iwdgroup, iattr, iftr, itmp;
  XWordType *pNomWord;
  WordGroupType *pWgrp;
  PredValenType *pPredValen;
  struct VarConstrainCore *pVarCnstrn;
  ConstrainType *pConsFeature;
  WordFeatureCoreType *pFeatureCore;
  Status ret;

  
  pNomWord = &pxsen->pXWds[nom];
  pPredValen = (PredValenType *)(&pxsen->pXWds[pred].valence);

  
  /* 1st, check the Role, it Should match at first. */
  if (pPredValen->pVars[ivar].role.type !=  ((NomValenType*)pNomWord->valence)->role.type)
    return NICE_ZERO_SCORE;
  nice = NICE_ONE_SCORE;


  
  /* 2nd, check the limitation, now only check first weightness */
  /* ... Word instances checking */
  pVarCnstrn = &pPredValen->pVars[ivar].limits;
  pWgrp = pVarCnstrn->words[0];

  if (WordGroupFind(pWgrp, &pNomWord->wd))
    nice += NICE_ONE_SCORE*10;
    //need to return ???




  nice += ConstrainFind(&pVarCnstrn->consFeatures, &pNomWord->featureCore);


#if 0
  /* ... constrain features checking */
  if (pVarCnstrn->consFeatures.active)
  {
    pConsFeature = &pVarCnstrn->consFeatures;

    /* ......common Attribute checking */
    if (pConsFeature->commAttrSum)
    {
      for (iattr = 0; iattr < pConsFeature->commAttrSum; iattr++)
      {
        /* to intercept common attribute. Now only support simple match and map */
        for (itmp = 0; itmp < pNomWord->featureCore.attrSum; itmp++)//4 TO_BE_DONE, nominal attr few than pred-var's, so move this loop outside may be better. 
        {
          ret = InterceptAttrRelation(&pConsFeature->pCommonAttrs[iattr], pNomWord->featureCore.pAttrs[itmp]);
          if (OK == ret)
            nice += NICE_ONE_SCORE*2;
          
        }
      }
    }


    /* ...... Core features checking */
    if (pConsFeature->wordFeatureSum)
    {
      for (iftr = 0; iftr < pConsFeature->wordFeatureSum; iftr++)
      {
        pFeatureCore = &pConsFeature->pWordFeatures[iftr];

        /* to intercept core feature */
        if (SemClassMatch(pFeatureCore->semClass, pNomWord->featureCore.semClass))
        {
          nice += NICE_ONE_SCORE*3;

          /* intercept this feature's attributes that live together with above semClass*/
          for (iattr = 0; iattr < pFeatureCore->attrSum; iattr++)
          {
            /* to intercept common attribute. Now only support simple match and map */
            for (itmp = 0; itmp < pNomWord->featureCore.attrSum; itmp++)
            {
              ret = InterceptAttrRelation(&pFeatureCore->pAttrs[iattr], pNomWord->featureCore.pAttrs[itmp]);
              if (OK == ret)
                nice += NICE_ONE_SCORE*2;
              else if (ERR_NEGETIVE_ANSWER == ret)
                nice -= NICE_ONE_SCORE*2;
            }
          }

          
        }/* end of core feature*/

      }
    }




  }
#endif
  /*end of 2nd checking. */

  return nice;
}




/*
 * Bind of Predicate and nominal words.
 */
Status PredBindNom(XSenType *pxsen, struct LogicBind *pBinder, LogicQueueType *pLQueue)
{
  int iNom, iPred, iNomMatch;
  int ivar, i;
  WordID iwdPred;
  FOPL_Type *pLogic;
  Status ret;
  struct NomBind *pNom;
  struct PredBind *pPred;
  int8 predSum, nomSum;
  NiceType nice, niceBest;
  PredValenType *pPredValen;


  pNom = &pBinder->nom[0];
  pPred = &pBinder->pred[0];
  predSum = pBinder->predSum;
  nomSum = pBinder->nomSum;
  
  for (iPred = 0; iPred < predSum; iPred++)
  {
    iwdPred = pPred[iPred].id;
    pPredValen = (PredValenType*)(pxsen->pXWds[iwdPred].valence);
    

    for (ivar = 0; ivar < pPredValen->varSum; ivar++)
    {

      nice = 0;
      niceBest = 0;


      //4 TO_BE_DONE  more strategies to locate the var-arg.


      for (iNom = 0; iNom < nomSum; iNom++)
      {
        /*  add the binding scope.  subject: front, object:back 
            Should add more check of passive structure.
        */
        if ( pPredValen->pVars[ivar].Constrain.role.type == SUBJECT &&
             iwdPred < pNom[iNom].id)
          continue;

        if ( pPredValen->pVars[ivar].Constrain.role.type == OBJECT &&
             iwdPred > pNom[iNom].id)
          continue;

        
        nice = FindPredValenRelation(pxsen, iwdPred, ivar, iNom);
        if (nice > niceBest)
        {
           iNomMatch = iNom;
           niceBest = nice;
        }
      }



      
      if (niceBest > 0)
      {
        assert (pNom[iNom].bindSum < NOM_MAX_BINDS);
        BindValenVar(pPredValen, ivar, &pxsen->pXWds[pNom[iNomMatch].id]);
        pNom[iNom].binds[ pNom[iNom].bindSum ].bindPred = iPred;
        pNom[iNom].binds[ pNom[iNom].bindSum ].varID = ivar;
        pNom[iNom].bindSum++;
        pPred[iPred].vars[ivar].binds[0] = iNom;
      }
      else
      {
        ;//not matched. what to do ?
      }


      
    }



    /* After bind all valence of predicate vars, generate logic */
    ret = CreateLogicUnit(LOGIC_FOPL_TYPE, pPredValen->varSum, &pLogic);
    if (ret != OK || !pLogic)
    {
      return ret;
    }
    
    pLogic->type = FOPL_USUAL_PRED;
    pLogic->pPredinfo = (void *)pPredValen;
    pLogic->name = pxsen->pXWds[iwdPred].wd;

    for (i = 0; i < pPredValen->varSum; i++)
    {
      pLogic->pEntities[i].name = ((XWordType *)pPredValen->pVars[i].pArg)->wd;
      pLogic->pEntities[i].info = pPredValen->pVars[i].pArg;
    }
    LogicQueueAddTail(pLQueue, &pLogic->logicLinker);

  }



  return OK;
}




/************************************************
 * Pred operation functions.
 * 
 * TO_BE_MOVE to Logic.c ???
 ***********************************************/


/*
 * type : FOPL_SPECIAL_PRED
          FOPL_USUAL_PRED
 */
Status CreatePredInfo(int8 type, int8 varSum, void **pF)
{
  void *p;
  
  if (type == FOPL_SPECIAL_PRED){
    p = (void *)AllocSpecialPred();
    //p = malloc(sizeof(SpecialPredType));
    if (!p)
      return ERR_NO_MEM;

    *pF = p;
    
    if (varSum){
      p = (void*)AllocSpecialPredVars(varSum);
      //p = malloc(sizeof(SpecialPredVarType)*varSum);
      if (!p){
        FreeSpecialPred(*pF)
        //free(*pF);
        return ERR_NO_MEM;
      }
      (SpecialPredType)(*pF)->pVars = p;
    }
    
  }
  else if (type == FOPL_USUAL_PRED){
    p = (void *)AllocPredValence();
    //p = malloc(sizeof(PredValenType));
    if (!p)
      return ERR_NO_MEM;

    *pF = p;
    
    if (varSum){
      p = (void *)AllocPredVars(varSum);
      //p = malloc(sizeof(PredVarType)*varSum);
      if (!p){
        FreePredValence(*pF);
        //free(*pF);
        return ERR_NO_MEM;
      }
      (PredValenType)(*pF)->pVars = p;
    }
    
  }else{
     return ERR_INVLID_ARG;
  }


  return OK;
}



/*
{
  { SCB(pCacheXWordCrdnt),   sizeof(WCrdntType),      CACHE_SIZE_XWORD,        SLAB_SIMPLE_MASK },
  { SCB(pCacheXword),        sizeof(XWordType),       CACHE_SIZE_XWORD,        SLAB_SIMPLE_MASK },
  
  { SCB(pCacheNomValence),   sizeof(NomValenType),    CACHE_SIZE_NOM_VALEN,    SLAB_SMART_MASK },
  { SCB(pCachePredValence),  sizeof(PredValenType),   CACHE_SIZE_PRED_VALEN,   SLAB_SMART_MASK },
  { SCB(pCacheSpecialPred),  sizeof(SpecialPredType), CACHE_SIZE_SPECIAL_PRED, SLAB_SMART_MASK },
  { SCB(pCachePredVars),     sizeof(PredVarType),     CACHE_SIZE_PRED_VARS,    SLAB_SIMPLE_MASK },
  { SCB(pCacheSpecialPredVars), sizeof(SpecialPredVarType), CACHE_SIZE_SPECAILPRED_VARS, SLAB_SIMPLE_MASK },
  
  { SCB(pCacheFOPL),         sizeof(FOPL_Type),       CACHE_SIZE_FOPL,         SLAB_SMART_MASK  },
  { SCB(pCacheFOL_Operator), sizeof(FOL_Operator),    CACHE_SIZE_FOL_OPTRS,    SLAB_SMART_MASK  },

  { SCB(pCacheSemNode),      sizeof(struct SemNode),  CACHE_SIZE_SEMNODES,     SLAB_SIMPLE_MASK },
  { SCB(pCacheAttribute),    sizeof(AttrType),        CACHE_SIZE_ATTRIBUTES,    SLAB_SIMPLE_MASK },
  { SCB(pCacheFeatureCore),  sizeof(WordFeatureCoreType), CACHE_SIZE_FEATURES, SLAB_SIMPLE_MASK },
};
*/

/* Xword  cache IF  */
XWordType * AllocXword(short num)
{
  return (XWordType *)SimpleSlabAlloc(SCB(pCacheXword), num);
}

Status FreeXword(void)
{
  return SimpleSlabFree(SCB(pCacheXword));
}

Status FlushXword(void)
{
  return SimpleSlabFree(SCB(pCacheXword));
}

/* word coordinate cache IF  */
WCrdntType * AllocWCrdnt(short num)
{
  return (WCrdntType *)SimpleSlabAlloc(SCB(pCacheXWordCrdnt), num);
}

Status FreeWCrdnt(void)
{
  return SimpleSlabFree(SCB(pCacheXWordCrdnt));
}

Status FlushWCrdnt(void)
{
  return SimpleSlabFree(SCB(pCacheXWordCrdnt));
}


/* Valence structure better to be dynamic allocated and free back. */
NomValenType * AllocNomValence(void)
{
  return (NomValenType *)SlabAlloc(SCB(pCacheNomValence), num);
}

Status FreeNomValence(void *obj)
{
  return SlabFree(SCB(pCacheNomValence), (void*)obj);
}

Status FlushNomValence(void)
{
  return SlabFlush(SCB(pCacheNomValence));
}


/* PredValence cache IF  */
PredValenType * AllocPredValence(void)
{
  return (PredValenType *)SlabAlloc(SCB(pCachePredValence), num);
}

Status FreePredValence(void *obj)
{
  return SlabFree(SCB(pCachePredValence), (void *)obj);
}

Status FlushPredValence(void)
{
  return SlabFlush(SCB(pCachePredValence));
}


/* PredVars cache IF  */
PredVarType * AllocPredVars(short num)
{
  return (PredVarType *)SimpleSlabAlloc(SCB(pCachePredVars), num);
}

Status FreePredVars(void)
{
  return SimpleSlabFree(SCB(pCachePredVars));
}

Status FlushPredVars(void)
{
  return SimpleSlabFree(SCB(pCachePredVars));
}



/* SpecialPred cache IF  */
SpecialPredType * AllocSpecialPred(void)
{
  return (SpecialPredType *)SlabAlloc(SCB(pCacheSpecialPred));
}

Status FreeSpecialPred(void *obj)
{
  return SlabFree(SCB(pCacheSpecialPred), (void*)obj);
}

Status FlushSpecialPred(void)
{
  return SlabFlush(SCB(pCacheSpecialPred));
}


/* SpecialPredVars cache IF  */
SpecialPredVarType * AllocSpecialPredVars(short num)
{
  return (SpecialPredVarType *)SimpleSlabAlloc(SCB(pCacheSpecialPredVars), num);
}

Status FreeSpecialPredVars(void)
{
  return SimpleSlabFree(SCB(pCacheSpecialPredVars));
}

Status FlushSpecialPredVars(void)
{
  return SimpleSlabFree(SCB(pCacheSpecialPredVars));
}

/* FOPL cache IF  */
FOPL_Type * AllocFOPL(void)
{
  return (FOPL_Type *)SlabAlloc(SCB(pCacheFOPL));
}

Status FreeFOPL(void *obj)
{
  return SlabFree(SCB(pCacheFOPL), (void*)obj);
}

Status FlushFOPL(void)
{
  return SlabFlush(SCB(pCacheFOPL));
}

/* FOL_Operator cache IF  */
FOL_Operator * AllocFOL_Operator(void)
{
  return (FOL_Operator *)SlabAlloc(SCB(pCacheFOL_Operator));
}

Status FreeFOL_Operator(void * obj)
{
  return SlabFree(SCB(pCacheFOL_Operator), (void*)obj);
}

Status FlushFOL_Operator(void)
{
  return SlabFlush(SCB(pCacheFOL_Operator));
}

/* SemNode cache IF  */
struct SemNode * AllocSemNode(short num)
{
  return (struct SemNode *)SimpleSlabAlloc(SCB(pCacheSemNode), num);
}

Status FreeSemNode(void)
{
  return SimpleSlabFree(SCB(pCacheSemNode));
}

Status FlushSemNode(void)
{
  return SimpleSlabFree(SCB(pCacheSemNode));
}


/* Attributes cache IF  */
AttrType * AllocAttribute(short num)
{
  return (AttrType *)SimpleSlabAlloc(SCB(pCacheAttribute), num);
}

Status FreeAttribute(void)
{
  return SimpleSlabFree(SCB(pCacheAttribute));
}

Status FlushAttribute(void)
{
  return SimpleSlabFree(SCB(pCacheAttribute));
}


/* Feature cache IF  */
WordFeatureCoreType * AllocFeatureCore(short num)
{
  return (WordFeatureCoreType *)SimpleSlabAlloc(SCB(pCacheFeatureCore), num);
}

Status FreeFeatureCore(void)
{
  return SimpleSlabFree(SCB(pCacheFeatureCore));
}

Status FlushFeatureCore(void)
{
  return SimpleSlabFree(SCB(pCacheFeatureCore));
}


