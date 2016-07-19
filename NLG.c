/*===========================================================================

FILE: NLG.c

DESCRIPTION
   Nature Language generation.

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




/*----------------------------------------------------------------------------
   Definitions
------------------------------------------------------------------------------*/
#define SEN_MAX_LENGHT 400




/*----------------------------------------------------------------------------
   types/structures
------------------------------------------------------------------------------*/
typedef struct {
  GrammCompE type;
  char *pdata;
  short len;

  SenPartType *next;
}SenPartType;

struct ZhuyuPart {
  SenPartType core;
  SenPartType *dingyu;
};

struct WeiyuPart {
  SenPartType core;
  SenPartType *zhuangyu;
  SenPartType *buyu;
};

struct BingyuPart {
  SenPartType core;
  SenPartType *dingyu;
};

typedef struct {
  struct ZhuyuPart zhuyu;
  struct WeiyuPart weiyu;
  struct BingyuPart bingyu;
}SenFillTableType;


/* the input control structure */
struct NLG_Input {
  WordType subject;
  WordType object;
  WordType process;
  char speechAct; /* 话语行为 */
  char tense;     /* 时态 */
};



/*----------------------------------------------------------------------------
   local varables
------------------------------------------------------------------------------*/
static SenFillTableType senFilltbl;
static char SenBuff[SEN_MAX_LENGHT];

/*----------------------------------------------------------------------------
   global varables
------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/





/*
Status SpeechPlan(LogicQueueType *pQueue, struct NLG_Input *pNLG)
{


}
*/


Status SentencePlan(LogicQueueType *pQueue, SenFillTableType *pSenFilltbl)
{
  /* Logic Queue may have diffenret type datas */

  //(1) 单个动词


  //(2) 连动，两个动词共用主语


  //(3) 给与动词



  //(4) 多个动词



}


Status FillSentence(SenFillTableType *pSenFilltbl, char *buff)
{




  
}




