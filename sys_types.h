#ifndef SYS_TYPES_H
#define SYS_TYPES_H
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



/*----------------------------------------------------------------------------
  Standard Types
------------------------------------------------------------------------------*/

#ifndef _UINT32_DEFINED
typedef  unsigned  int  uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  signed int    int32;       /* Signed 32 bit value */
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef  signed char        int8;        /* Signed 8  bit value */
#define _INT8_DEFINED
#endif


#define __int64 long long

#ifndef _UINT64_DEFINED
typedef  unsigned __int64   uint64;      /* Unsigned 64 bit value */
#define _UINT64_DEFINED
#endif

#ifndef _INT64_DEFINED
typedef  __int64            int64;       /* Signed 64 bit value */
#define _INT64_DEFINED
#endif

/*----------------------------------------------------------------------------
   size type
------------------------------------------------------------------------------*/
typedef unsigned int size_t;

/*----------------------------------------------------------------------------
   convenient size definition.
------------------------------------------------------------------------------*/

#define KB(x) ((x)*(1<<10))
#define MB(x) ((x)*(1<<20))

/*----------------------------------------------------------------------------
   Bool logic types.
------------------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE   1   /* Boolean true value. */
#endif

#ifndef FALSE
#define FALSE  0   /* Boolean false value. */
#endif

#ifndef NULL
#define NULL  0
#endif



/*----------------------------------------------------------------------------
   Error types.
------------------------------------------------------------------------------*/
#define OK 0
#define ERR_NO_MEM 1
#define ERR_NOT_FOUND 2
#define ERR_INVLID_ARG 3
#define ERR_FAILED 4
#define ERR_NEGETIVE_ANSWER 5


/*----------------------------------------------------------------------------
   Structure location definitions.
------------------------------------------------------------------------------*/


#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ((type *) (((char *) (ptr)) - offsetof(type, member)))




#endif /* SYS_TYPES_H */

