#ifndef DICTIONARY_H
#define DICTIONARY_H

/*
==============================================================================

FILE:         dictionary.h

DESCRIPTION:
  dictionary types. 

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
#include"words.h"

#define MAX_DICT_NAME_LEN 40
#define WNAME_ENDDING '\0'



typedef struct DictIndexWdPool {
  char * pstart;
  int memsize;
  int memfree;
} DictIndexWdPooltype;


typedef struct DictWdIndexItem {
  WordType wd;   /* the head of word entry struct */
    /* word sense address */
}DictWdIndexItemType;



 /*********************************
   This dictionary struct is a runtime
   data info of cached dict in RAM.
 ****/

typedef struct dict_struct {

  char name[MAX_DICT_NAME_LEN];
  char wordnum;          /* dict size of word items */


  /*****************************
    dictionary's index list.
  *******/
  DictWdIndexItemType *pindex;
  DictIndexWdPooltype indexWdPool; /* memory pool for word name sting */


   /*  */

} dict_type;






#endif /* DICTIONARY_H */

