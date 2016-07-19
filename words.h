#ifndef WORDS_H
#define WORDS_H
/*
==============================================================================

FILE:         words.h

DESCRIPTION:
  basic definitions and structures of word

             Copyright @ 2011 Happyman Design.
                    All Rights Reserved.

==============================================================================
Edit History


when       who     what, where, why
--------   ---     -----------------------------------------------------------


TO_BE_DONE:
1. considering to move the 'role' to common feature and boarden the meaning to
   cover more : predicate, relation( for conj, ...). 
   Also may treat nominal's 'role' a passive constrain or discription.

*/

/*============================================================================

                     INCLUDE FILES FOR MODULE

============================================================================*/
#include"sense.h"
#include"list.h"




#define MAX_WORD_LEN 6

/* bytes per Chinese character */
#define CCLEN 2


/*----------------------------------------------------------------------------
    Raw word .
------------------------------------------------------------------------------*/
/* raw word */
typedef struct WordStruct {
  char * pdata; /* real data */
  char len;  /* len in char */
} WordType;


typedef struct WordGroupStruct {
  short     sum;
  WordType *pWords;
}WordGroupType;





/*=============================================================================
  The following are word sense. 
===============================================================================*/

/*----------------------------------------------------------------------------
    nominal word. 
------------------------------------------------------------------------------*/

/* Non-Predicate collocation */
struct NPCollocation {
  SpecialRelationE relation;
  WordGroupType wordGroup[WET_MAX];
  ConstrainType consFeatures;
  struct list_head group;
};


/* Valence grammar */
typedef struct NominalsValenceStruct {

  /* A, normal Predicate constrain */
  RoleType role;               /* constrain of role, what role I can do. passive constrain. */

#if 1 // verbose information for valence matching.
  char predValenSum;           /* constrain of predicate's Valence number */
  ConstrainType predFeature;     /* constrain of predicate */
  ConstrainType *pPredVarFeature;  /* constrain of companion, array size is predValenSum */
#endif   

  /* B, special predicate constrain */
  /* ... symbion relation.  refer to Page136, Lujianming. */
  char ValenSum;            /* nominal valence, don't forget nominal's valence, it's [0,1,2] */
  WordGroupType *pSymbions; /* depends on valenSum, this array has 2 members at most. */
  ConstrainType *pSymbionFeature; /* depends on valenSum, this array has 2 members at most. */
  SpecialRelationE relations[2]; /* Maximum valence of noum word are 2 */
  
  /* ... collocation desire */
  //4  define the collocation . special predicate groups. 
  struct NPCollocation NPColl[REL_MAX];
  
} NomValenType;


/* nominal words relation of collocation */
typedef struct {
  XWordType *pxword;
  struct list_head group;
}SpecialPredVarType;


typedef struct {
  SpecialRelationE relation;
  int8 varSum;
  SpecialPredVarType *pVars;
}SpecialPredType;


/*----------------------------------------------------------------------------
    Predicate word
------------------------------------------------------------------------------*/


struct VarConstrainCore {
  WordGroupType words[WET_MAX];
  ConstrainType consFeatures;

  //4  TO_BO_DONE,  the status change of the valence.
};


//typedef struct PredVariableConstrain {
//}PredVarConstrn;



/* Predicate variable 是一个具有函数形式特征的符号，
   形参是parameter，实参是argument，
   parameter is definition of (constrain) limit and desire ,
   argument is acutally pointed object */
typedef struct PredicateVarStruct {
  WordType * name;
  void *pArg;   /* argument, could be Xword or others  */
  struct list_head group;

  /* predicate variable constrain */ /* parameter */
  RoleType role;
  struct VarConstrainCore blacklist;
  struct VarConstrainCore limits;
  struct VarConstrainCore desire;
//  PredVarConstrn Constrain;  
  
}PredVarType;


typedef struct PredicateValenceStruct {
  char varSum;
  WordType * word;   /* predicate, it's a must, to access predicate's feature */
  short predIndex;   /* for the multi-sense Predicate */
  PredVarType * pVars;   /* variables list */

}PredValenType;




/*----------------------------------------------------------------------------
    complete word
------------------------------------------------------------------------------*/
/* word sense, including features, Valence Grammar */
typedef struct WordFeatureStruct {
  WordFeatureCoreType featureCore; /* word main feature */

  /* Part-of-Speech in featureCore will indicate the content of the valence. */
  /* Predicate refer to the PredValenType, 
     nominal word refer to the NomValenType */  
  void *valence;  

}WordFeatureType;




/* meaningful word entity  */
typedef struct XWordStruct {
  WordType wd;

  /* feature, keep the following two order
     that could convert type to WordFeatureType from here. */
  WordFeatureCoreType featureCore;
  void *valence;

  struct list_head group;
}XWordType;










#endif /* WRODS_H */

