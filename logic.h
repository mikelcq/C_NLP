 #ifndef LOGIC_H
#define LOGIC_H
/*
==============================================================================

FILE:         logic.h

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




/*----------------------------------------------------------------------------
   definitions.
------------------------------------------------------------------------------*/


#define LOGIC_HEAD_TYPE 0
#define LOGIC_FOPL_TYPE 1
#define LOGIC_OPEARATOR_TYPE 2
#define LOGIC_RAW_TYPE 3

#define FOPL_NOTYPE_PRED 0
#define FOPL_USUAL_PRED 1
#define FOPL_SPECIAL_PRED 2

/*----------------------------------------------------------------------------
   types and structures..
------------------------------------------------------------------------------*/


/*******************************
  more common logic conceptions
*/

/* universal Logic linker */
typedef struct {
  struct list_head list;
  int8 masterType;   /* 0: Head, 1: FOPL, 2:Operator, 3+:reserved */

}LogicLinkerType;

//#define NAME_MAX_LEN_FOPL_PRED_NAME 14
#define NAME_MAX_LEN_ACTION 20


/*************************************
 The first-order predicate logic, FOPL
 General model.
 */
typedef struct {
  WordType name;
  void * info;

  EntityType *share;

  struct list_head list;
}EntityType;


typedef struct {
  uint8 type;  /* 0:NULL, 1:nomal predicate logic, 2: special defined Logic*/
  uint8 entitySum;
  WordType name;
  struct list_head Entities;
  void * pPredInfo;  /* optional: {PredValenType, SpecialPredType} */

  LogicLinkerType logicLinker;
}FOPL_Type;


/*************************************************
 logic compute, 如果没OR逻辑或，则与运算AND符号可有
 可无，只要前后连起来就表示了AND. 所以用顺序链表来表
 现OR，就需要特殊操作，能够揭示并列关系；
 用结构变形来表达复杂的带括号的OR操作如: 
 ( F1(x) AND F2(x) ) OR ( F3(x) AND F4(x) )
 '()'操作符的结构含有并列链表。

(A OR B) AND (A OR B)又怎么表示呢???

 主要是复杂推理计算怎么实现。
*/

typedef enum FOL_Operation {
  unknown = 0,
  AND,
  OR,
  NOT,

  MAX_OPS
}FOL_OPS;



typedef struct  {
  FOL_OPS ops;  /* operator type */
  char factSum; /* number of variables of operator*/
  FOPL_Type *pFacts; /* no used now. array of the variables, known as facts */
  LogicLinkerType logicLinker;

}FOL_Operator;



typedef struct {
  int logicSum;
  LogicLinkerType LogicHead;

  LogicQueueType *next;
}LogicQueueType;



/* production rules */
typedef struct {
  short preCondSum;  /* just for quick info retrieval */
  short resultSum;
  LogicLinkerType preCondition;  /* prerequests list, linked */ 
  LogicLinkerType result;        /* results list, linked */

  struct list_head list;
}RuleType;



/***********************************
 Situation and Action knowledeg 
 representation based on logic.
 We called scenario.
*/
typedef struct {
  int8 masterType;  /* */
  struct list_head list;
}ActionLinkerType;


typedef struct {
  char name[NAME_MAX_LEN_ACTION];
  LogicLinkerType motions;

  ActionLinkerType actionLinker;
}ActionType;


typedef struct {
  int8 type;    /* IF branch 'if()else' or Loop 'while(){}' control */
  LogicLinkerType conditon;
  ActionType *pYesAction; /* next action or loop action entry if condition True */
  ActionType *pNoAction;  /* next action if condition False */

  ActionLinkerType actionLinker;
}SituationLogicCtlType;


typedef struct {
  ActionLinkerType actionHead;
  
  struct list_head list;

}SCNType;




#endif /* LOGIC_H */

