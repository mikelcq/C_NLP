#ifndef SENTENCE_H
#define SENTENCE_H
/*
==============================================================================

FILE:         sentence.h

DESCRIPTION:
  basic definitions and structures of sentence. 

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
 Sentence classes. 
------------------------------------------------------------------------------*/

/* raw sentence */
typedef struct SentenceStruct {
  char * pdata;
  int len;  /* len in char */
}SenType;


/* this is the word location info in the sentence. */
typedef struct WCoordinateStruct {
  char offset;  /* x of [x,y] for coordinate */
  char len;  /* len of word, is (y -x) */
}WCrdntType;


/* sentence map of words. */
typedef struct SenMapStruct {
  short size;
  WCrdntType *pWcrdnts;  /* word coordinates array */
}SenMapType;



typedef enum GrammComp {
  COMP_UNKNOWN = 0,
  COMP_SUB,
  COMP_PRED,
  COMP_OBJ,
  COMP_BU,
  COMP_ZHUANG,
  COMP_DING,
  
  COMP_MAX
} GrammCompE;


/* Grammar component */
typedef struct SenGrammCompStruct {
  GrammCompE comp;
  POS_E phrase;
  short wstart;
  short wend;
  struct list_head group;

}SenGrammCompType;


/* meaningful sentence */
typedef struct XSentenceStruct {

  /* basic info */
  SenType sen;
  SenMapType senMap;

  /* grammer info .... */
  short senCompSize;
  SenGrammCompType *senComp;


  /* sematic info array, size is senMap.size, index is ´Ê±ê  .... */
  XWordType *pXWds;

  
}XSenType;








#endif /* SENTENCE_H */

