 #ifndef SENSE_H
#define SENSE_H
/*
==============================================================================

FILE:         sense.h

DESCRIPTION:
  sematics conceptions 

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
#include"list.h"

#define ATTR_NAME_LEN 10
#define ATTR_VALUE_LEN 10
#define SEMCLASS_NAME_LEN 10

#define NICE_ZERO_SCORE 0
#define NICE_ONE_SCORE 1
/**********************************************
  POS
*********************************************/

/*part of speech*/
typedef enum POS {
  POS_UNKNOWN = 0,
  POS_INVALID = POS_UNKNOWN,

#if 0 
  NOUN,
  VERB,
  ADJ,
  ADV,
  PRON, //Pronoun
  NUM,  //Numeral
  QUAN, //Quantifier
  TIME, //
  LOCA, //Localizer
  PLACE, //chu suo ci
  AUX,  //Auxiliary
  Z,    //zhuangtaici
  CONJ, //Conjunction
  PREP, //preposition
  MORPH, //morpheme
  PUNC,  //punctuation
#endif

  POS_A,  /*���ݴ�*/
  POS_B,  /*�����*/
  POS_C,  /*����*/
  POS_D,  /*����*/
  POS_F,  /*��λ��*/
  POS_G,  /*����*/
  POS_M,  /*����*/
  POS_N,  /*����*/
  POS_P,  /*���*/
  POS_Q,  /*����*/
  POS_R,  /*����*/
  POS_S,  /*������*/
  POS_T,  /*ʱ���*/
  POS_U,  /*����*/
  POS_V,  /*����*/
  POS_W,  /*���*/
  POS_Z,  /*״̬��*/
  
  
  POS_PHRASE,
  POS_AP = PHRASE,  //���ݴ��Զ���
  POS_MCP, //�����Զ���
  POS_SP,  //�������Զ���
  POS_DP,  //�����Զ���
  POS_NP,  //�����Զ���
  POS_TP,  //ʱ����Զ���
  POS_DJ,  //��ν����
  POS_MP,  //��������
  POS_PP,  //��ʶ���
  POS_VP,  //�����Զ���
  
  
  POS_MAX
} POS_E;


/**********************************************
  Variable roles
*********************************************/
typedef enum Role {
  UNKNOWN = 0,
  SUBJECT,
  OBJECT,
  

  MAX_ROLE
}RoleE;

typedef struct RoleStruct {
  RoleE type;

}RoleType;


typedef enum {
  WET_First,
  WET_Second,
  WET_Third,
  WET_Fourth,
  WET_Fifth,
 

  WET_MAX
}WeightnissE;

typedef int NiceType;


/**********************************************
  Sematic for word sense
*********************************************/

typedef struct SematicClassStruct {
  char name[SEMCLASS_NAME_LEN];  //4 need to think about the hardcoded type or string name.

#ifdef SEMFRAME_LINKED_TREE
  short level;     /* level in sematic tree */
  short familyID;   /* sibling group index of a level */
  short siblingID;  /* sibling index in family */
#else
  /* SEMFRAME_ARRAY_TREE */
  short id;
#endif

}SemClassType;


typedef struct Attribute {
//  short type;   //4 gotta define the long type list for future using and expanding.
  char name[ATTR_NAME_LEN];

  char value[ATTR_VALUE_LEN];
  struct list_head group;
}AttrType;


/* feature of word */
//4 TO_BE_DONE  to define the feature struct with dynamic store model. 
typedef struct  {
  POS_E pos;  /* Part-of-Speech will indicate the content of the valence. */
  SemClassType semClass;       /* Sematic class  */
  short     attrSum;           /* attributes numbers */
  AttrType *pAttrs;            /* attributes array */
  struct list_head group;

} WordFeatureCoreType;


typedef struct  {
  char      active;                      /* flag of this condition on. */
  short     wordFeatureSum;              /* single feature numbers */
  short     commAttrSum;                 /* common attrs numbers */
  WordFeatureCoreType *pWordFeatures;    /* array of single features */
  AttrType *pCommonAttrs;                /* array of common attributes */
  
} ConstrainType;



/**********************************************
  Special relations for
*********************************************/

typedef enum SpecialRelation {
  REL_UNKNOWN = 0,

  REL_VALEN_START , //???
  REL_ISPO = REL_VALEN_START,  /* IS part of */
  REL_ISATTRO,                 /* Is attribute of */
  REL_ISSYMBO,                 /* Is symbion of */
  REL_VALEN_END,

  REL_GENERAL = REL_VALEN_END,

  REL_ISKO = REL_GENERAL,        /* Is Kind Of,  ISKO(x, y) */
  REL_HAPPEN_PLACE,              /* F happen at place, HPLACE(x, F ) */
  REL_HAPPEN_TIME,               /* F happen at time, HTIME(x, F) */
  REL_LOCATION,                  /* y 's location is x, LOCATION(x, y) */

  REL_FSSTART,
  REL_FSTW = FSSTART,  /* From Same Topic With, FSTW(x, y) */
  REL_FSFW,            /* From Same Feild With, FSTW(x, y) */
  REL_FSPW,            /* From Same Place With, FSTW(x, y) */
  REL_FSBKW,           /* From Same Book With, FSTW(x, y) */
  REL_FSAGEW,          /* From Same Age With, FSTW(x, y) */


  REL_NN, //���е����ʹ�ϵ
  REL_AM, //���Σ���Ҫ��һ����ǰ���ݴʡ�
  REL_GN, //������ ���� ����NP�����õ�N�������Ϣ

  REL_MAX
}SpecialRelationE;



typedef struct {
  SpecialRelationE relation;
  NiceType nice;

}MatchRelationType;






#endif /* SENSE_H */

