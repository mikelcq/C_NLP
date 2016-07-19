/*===========================================================================

FILE: word.c

DESCRIPTION
   This file contains the functions of word analysis.

PUBLIC CLASSES:
   N/A

INITIALIZATION AND SEQUENCING REQUIREMENTS:

  word_seg
  show_map
  point_in_map
  map_merge
  AddPoint2Map
  segmap2len
  get_agenda
  put_agenda
  sentence_read
  dict_load
  check_dict_word
  word_comp
  create_wstack
  push
  pop
  dump_dict
  clear_map


       Copyright @2011 Happyman Design.
             All Rights Reserved.
===========================================================================*/




#include"stdio.h"
#include<stdlib.h>
#include"fcntl.h"
#include"string.h"
#include"sys_types"
#include"words.h"

/* Features ====================================================*/
#define CACHE_SUPPORT



/* Definitions ====================================================*/

#define MAX_DICT_NUM 10

/* need to calc from bits to bytes later */
#define WDSEG_MAP_BIT_LEN 
#define WDSEG_MAP_LEN 10 /* bytes */


/* types ====================================================*/

/* structures ====================================================*/



/* internal data of Word segmentation */

typedef struct wdsegmentation {
  uint8 wdseg_map[WDSEG_MAP_LEN];
  int top;  /* top bit */

} wdseg_type;

typedef struct wdseg_full_struct {
  wdseg_type *psegmnt;    /* entry to the segmnt table */
  int num;   /* table size */

}wdseg_full_type;


/* word seg. stack */
typedef wdseg_type wstack_item_type;

typedef struct wstack {
  int topidx;         /* index of the top of the stack */
  int size;           /* size of the stack (multiple of chunksize) */
  int chunksize;      /* size of memory chunks allocated at a time */
  wstack_item_type *array;  /* the stack itself */
  
} wstack_type;


/* global varables ====================================================*/
dict_type dicts[MAX_DICT_NUM]; // now using arry, not link.


/* Functions ====================================================*/
int word_seg(SenType sen, wdseg_full_type *pwdseg, wstack_type *pwstack);
int show_map(wdseg_type *pwdmap);
int point_in_map(int pos, wdseg_type *pdest);
int map_merge(wdseg_type *pdest, wdseg_type *psrc, int pos);
int AddPoint2Map(wdseg_type *pwdseg, int currt_pos, int step);
int segmap2len(wdseg_type wdseg);
int get_agenda(wstack_item_type *pitem, wstack_type *pwstack);
int put_agenda(wstack_item_type *pitem, wstack_type *pwstack);
int sentence_read(SenType *psen);
int dict_load();
int check_dict_word(const dict_type * pdict, const WordType * pword, void * arg);
int word_comp(const WordType * pw1, const WordType * pw2);
int create_wstack(wstack_type *pstack, int size);
int push(wstack_type *pstack, wstack_item_type *pitem);
int pop(wstack_type *pstack, wstack_item_type *pitem);
int dump_dict(dict_type * pdict, int fulldump);
int clear_map(wdseg_type *pwdmap);







int main (int argc, char *argv[])
{
  int i;
  wstack_type wstack;

  char buff[100];
  SenType sentence;

  printf("starting, loading dict \n");

  /* initial */
  if (dict_load() == -1)
  {
    printf("load dict err!\n");
    return 0;
  }
  dump_dict(&dicts[0], 1);


  printf("creating stack...\n");

  create_wstack(&wstack, 50);
  if (!wstack.array)
  {
    printf("stack alloc err!\n");
    goto MAINEND;
  }

  printf("get a input ...\n");
  
  /* reading sentence from a file or input */
  sentence.pdata = buff;
  sentence.len = sizeof(buff);
  if (0 > sentence_read(&sentence))
  {
    printf("reading err!\n");
    goto MAINEND;
  }

  printf("analyzing...\n");

  /* core algorithm of word segmentation */
  wdseg_full_type wdsegmnt;
  word_seg(sentence, &wdsegmnt, &wstack);


  printf("Done, size: %d \n", wdsegmnt.num);

  /* show result */
  for (i = 0; i < wdsegmnt.num; i++)
  {
    printf("the %dth anwser: \n", i);
    show_map(&wdsegmnt.psegmnt[i]);  
  }


MAINEND:
  if (wstack.array)
  {
    printf("free stack \n");
    free(wstack.array);
  }
  if(dicts[0].indexWdPool.pstart)
  {
    printf("free dict data \n");
    free(dicts[0].indexWdPool.pstart);
  }
  if (dicts[0].pindex)
  {
    printf("free dict wd \n");
    free(dicts[0].index);
  }

  return 0;
}





/******************************************************************************
*  Func: Chinese word segmentation.
* 
* Model:
* follow is the model of map for CC postion.
*    x---x---x---x---x---x---x---x---x    [Chinese sentence, x is a Chinese character ]
*  0---1---2---3---4---5---6---7---8---9  [position of axes ]
*  1---0---1---0---0---1---1---0---0---1  [bitmap for word segments: [0,2],[2,5],[5,6],[6,9] => 4 words ]
*******************************************************************************/

int word_seg(SenType sen, wdseg_full_type *pwdseg, wstack_type *pwstack)
{
  int i,ic,it;
  char cached_found = 0;
  WordType tempwd;
  wdseg_type wdseg_cache[20];
  wdseg_type temp_seg, currt_seg;

  int reach_sen_end = 0;
  char *pcurrt = NULL;
  int cachlen = 0;
  int currt_pos = 0;  /* current postion in sentence */

  char debug_wd[MAX_WORD_LEN*2+1];

  /* init datas*/
  clear_map(&temp_seg);
  clear_map(&currt_seg);
  for (it = 0; it < 20; it++)
    clear_map(&wdseg_cache[it]);


  printf("wordseg for sentence(len %d):%s \n", sen.len, sen.pdata);

//  show_map(&currt_seg);


  /* set start point of segmnt map */
  pcurrt = sen.pdata;
  AddPoint2Map(&currt_seg, currt_pos, 0);

//  printf(" current_seg: \n");
//  show_map(&currt_seg);

  for (;;)
  {


    /* step 1. scan.*/
    printf("\n....... step 1  ........ \n");

    
    /* check through cache first */
#ifdef CACHE_SUPPORT
    cached_found = 0;
    for (ic = 0; ic < cachlen; ic++)
    {
      if (point_in_map(currt_pos, &wdseg_cache[ic]))
      {
        printf("cached found!\n");


        
        cached_found++;
        map_merge(&currt_seg, &wdseg_cache[ic], currt_pos);
        wdseg_cache[cachlen - 1 + cached_found] = currt_seg;
        
        show_map(&currt_seg);
      }
    }
    
    if (cached_found > 0) 
      cachlen += cached_found;
#endif

    if (0 == cached_found)
    {


      /* sen end check . */
      reach_sen_end = currt_pos + MAX_WORD_LEN - sen.len/CCLEN;
      if ( reach_sen_end > 0 )/* reach sentence end. */
      {
        printf("reach sen end.");
        i = reach_sen_end; 
      }

        
      for (i = MAX_WORD_LEN; i > 0; i--)
      {
        /* make a word*/
        tempwd.pdata = pcurrt;
        tempwd.len = i*CCLEN;
  
        
  
  
        /* to check the exist of a word in dictionary */
        if (check_dict_word(&dicts[0], &tempwd, NULL))
        {
        
          memcpy(debug_wd, tempwd.pdata, tempwd.len);
          debug_wd[tempwd.len]='\0';
          printf("find in dict: len %d ,[%s] \n", tempwd.len, debug_wd);

          
          /* add new point to segmnt map */
          temp_seg = currt_seg;
          AddPoint2Map(&temp_seg, currt_pos, tempwd.len/CCLEN);
          //show_map(&temp_seg);
  
          /* check sentence end, if success then cache it.  */
          if (segmap2len(temp_seg) == sen.len)
          {

            printf(" got a sucess: \n");
            //show_map(&temp_seg);

            wdseg_cache[cachlen++] = temp_seg;
          }
          else
          {
            printf(" put a agenda : \n");
            show_map(&temp_seg);
            
            put_agenda((wstack_item_type*)&temp_seg, pwstack);
          }
        }
  
       
  
        
      }
    }




   printf("\n........ step 2 ........\n");


    /* step 2. get next agenda, if none, then exit. */
    if (get_agenda(&currt_seg, pwstack) == -1)
    {

      printf(" got none from stack ! \n");
      break; /* the end */

    }

    printf("reset currt_seg: \n");
    show_map(&currt_seg);

    /* sync the current data pointer and map postion*/
    currt_pos = segmap2len(currt_seg)/CCLEN;
    pcurrt = sen.pdata + currt_pos*CCLEN;

    printf("currt_pos:%d, pcurrt:%s \n", currt_pos, pcurrt);

  
  }


  pwdseg->psegmnt = wdseg_cache;
  pwdseg->num = cachlen;

  if (cachlen > 0)
    return 0; /* success */
  else
    return -1;
}



int clear_map(wdseg_type *pwdmap)
{
  
  memset(pwdmap, 0, sizeof(wdseg_type));

}


int show_map(wdseg_type *pwdmap)
{
  short i,j,k,b;
  short unitsize = sizeof(pwdmap->wdseg_map[0])*8;
  short pos = 0;

  //printf("sizeof wdseg_map[0] is:%d \n", unitsize);
  //printf("b[0] is :%d \n", pwdmap->wdseg_map[0]);
  printf("map top is: %d \n", pwdmap->top);
  
  i = pwdmap->top/unitsize;
  j = pwdmap->top%unitsize;

  for (k = 0; k < i; k++)
  {
    for (b = 0; b < unitsize; b++)
    {
      if( ((pwdmap->wdseg_map[k]>>b)&1) == 1)
      {
        printf("[%d] ", pos++);
      }
      else
      {
        printf("- ");
        pos++;
      }
    }
  }

  for (b = 0; b < j+1; b++)
  {
    if( ((pwdmap->wdseg_map[i]>>b)&1) == 1)
    {
      printf("[%d] ", pos++);
    }
    else
    {
      printf("- ");
      pos++;
    }
  }

  printf("\n");
  
  return 0;
}

int point_in_map(int pos, wdseg_type *pdest)
{
  short i,j,k;
  short unitsize = sizeof(pdest->wdseg_map[0])*8;
  
  i = pos/unitsize;
  j = pos%unitsize;
  
  return (pdest->wdseg_map[i] & (1 << j));
}


/* psrc's top should be larger than pdest's */
int map_merge(wdseg_type *pdest, wdseg_type *psrc, int pos)
{
  short i,j,k;
  short unitsize = sizeof(psrc->wdseg_map[0])*8;

  if ((pdest->top) >= (psrc->top))
    return -1;
  else
    pdest->top = psrc->top;
  
  i = pos/unitsize;
  j = pos%unitsize;

  for (k = i + 1; k < (psrc->top+unitsize-1)/unitsize; k++)
    pdest->wdseg_map[k] = psrc->wdseg_map[k];

  pdest->wdseg_map[i] &= ((1<<j)-1); /*low j bits is 1*/
  pdest->wdseg_map[i] |= psrc->wdseg_map[i] & (~(1<<j)+1); /* low j bits is 0*/

  return 0;
}

int AddPoint2Map(wdseg_type *pwdseg, int currt_pos, int step)
{
  int i, j;
  short unitsize = sizeof(pwdseg->wdseg_map[0])*8;

  if ((currt_pos+step) > (pwdseg->top))
    pwdseg->top = (currt_pos+step);
  
  i = pwdseg->top / unitsize;
  j = pwdseg->top % unitsize;
  pwdseg->wdseg_map[i] |= (1 << j);
}


/* return the map's lenght in bytes . */
int segmap2len(wdseg_type wdseg)
{
  int i, j;

#if 1
  return wdseg.top*CCLEN;
#else
  for (i = WDSEG_MAP_LEN - 1; i >= 0 ; i-- )
  if (wdseg.wdseg_map[i])
  {
    if (wdseg.wdseg_map[i] & (sizeof(wdseg.wdseg_map[i])/2)~0) /*err?*/
    {
      for (j = 0; j < sizeof(wdseg.wdseg_map[i])/2; j++)
      if (!(wdseg.wdseg_map[i] >> j + sizeof(wdseg.wdseg_map[i])/2));
	    return (j + sizeof(wdseg.wdseg_map[i])/2 + i*sizeof(wdseg.wdseg_map[i]));
	}
	else
    {
      for (j = 0; j < sizeof(wdseg.wdseg_map[i]); j++)
      if (!(wdseg.wdseg_map[i] >> j));
		return (j + i*sizeof(wdseg.wdseg_map[i]));
	}
  }  	
#endif

}


int get_agenda(wstack_item_type *pitem, wstack_type *pwstack)
{
  return pop(pwstack, pitem);
}



int put_agenda(wstack_item_type *pitem, wstack_type *pwstack)
{
    return push(pwstack, pitem);
}


int sentence_read(SenType *psen)
{
  /* here just read a sentence from a file */

  int fd;
  char * filename = "testinput.txt";
  int count;
  
  fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    printf("open file err:%s \n", filename);
    return -1;
  }

  count = read(fd, psen->pdata, psen->len - 1);
  if (count > 0)
  {
    psen->pdata[count] = '\0';
    psen->len = count;
    printf("read count %d: %s \n", count, psen->pdata);
  }
  else
  {
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}




/* load dictionary */
int dict_load() //char * dname
{
  int i, count;
  int fd = -1;
  int wdidx = 0;
  char * dictpath = "testdict.txt";
  char * dictbuff = NULL;
  char * ptrs = NULL;
  char wordbuff[MAX_WORD_LEN*2+1];
  DictWdIndexItemType * pWdIdx = NULL;
  char foundwd;
  char grabage;

  fd = open(dictpath, O_RDONLY);

  if (fd < 0)
  {
    printf("read file err:%s \n", dictpath);
    goto ERROUT;
  }
  

  dictbuff = malloc(K(4)); /* temp 4k, word data */
  if (!dictbuff)
  {
    printf("alloc mem err:\n");
    goto ERROUT;
  }

  pWdIdx = malloc(sizeof(DictWdIndexItemType)*200); /* 200 words, entries */
  if (!pWdIdx)
  {
    printf("alloc mem wd err:\n");
    goto ERROUT;
  }


  
  ptrs = dictbuff;

  /* start read file and load dict */
  while(ptrs < dictbuff + K(4) )
  {

#if 0 //read with block.
    foundwd = 0;
    count = read(fd, wordbuff, MAX_WORD_LEN*2+1);
    //printf("dict read data: %s \n", wordbuff);
    if (count > 0)
    {
      if (0x0d == wordbuff[0])
      {
        foundwd = 2;
      }
      else
      {
      
        for (i = 0; i < MAX_WORD_LEN*2+1 && i < count; i++)
        {
            printf(" wd:%d ,%s \n", i, wordbuff);
          if (0x0d == wordbuff[i]) /* CR */
          {
            wordbuff[i] = WNAME_ENDDING;
            printf("found wd:%d ,%s \n", i, wordbuff);
            foundwd = 1;
            break;
          }
        }


      }
    }
    else
    {
      /* end of file */
      break;
    }
    
    if(0 == foundwd)
    {
      printf("there are abnormal words. stoping loading.\n");
      //goto ERROUT;
    }
    else if (2 == foundwd)
    {
      continue;
    }


    lseek(fd, i+2-count, SEEK_CUR);
    printf("word reading, i:%d, count:%d, seek back:%d \n", i, count, -1*(count-(i+2)));

#else // read byte per time.
    foundwd = 0;
    for (i = 0; i < MAX_WORD_LEN*2+1 /*CR*/; i++)
    {
      count = read(fd, &wordbuff[i], 1); 
      
      if (count > 0)
      {
        //printf(" char:%02x ,", wordbuff[i]);
        if (0x0d == wordbuff[i])
        {
          read(fd, &grabage, 1); /* 'a' ? */
          wordbuff[i] = WNAME_ENDDING;
          printf("found wd:%d ,%s \n", i, wordbuff);
          foundwd = 1;
          break;
        }
      }
      else
      {
        foundwd = 2;
        break;
      }
    }
    
    if(0 == foundwd)
    {
      printf("there are abnormal words. stoping loading.\n");
      goto ERROUT;
    }
    else if (2 == foundwd)
    {
      break; //end
    }


#endif


    /* memory judy */
    if (ptrs+i+1 > dictbuff+ K(4))
    {
      printf("memory lack, stop loading dict at:%s \n", wordbuff);
      break;
    }
    if (wdidx >= 200)
    {
      printf("memory lack, stop loading dict %d words at:%s \n", wdidx, wordbuff);
      break;
    }
    
    memcpy(ptrs, wordbuff, i+1);
    pWdIdx[wdidx].wd.pdata = ptrs;
    pWdIdx[wdidx].wd.len = i;
    ptrs += (i+1);
    wdidx++;
  };




  /* init dict */
  strcpy(dicts[0].name, "testdict");
  dicts[0].indexWdPool.memsize = K(4);
  dicts[0].indexWdPool.memfree = K(4) - (ptrs-dictbuff) ;
  dicts[0].indexWdPool.pstart = dictbuff;
  dicts[0].pindex = pWdIdx;
  dicts[0].wordnum = wdidx;

  return 0;

ERROUT:
  if (dictbuff)
    free(dictbuff);
  if (pWdIdx)
    free(pWdIdx);
  if (fd > 0)
    close(fd);
  return -1;

}

int dump_dict(dict_type * pdict, int fulldump)
{
  int i;
  
  printf("dict name: %s \n", pdict->name);
  printf("dict size: %d \n", pdict->wordnum);
  printf("dict entry: %x \n", pdict->pindex);
  printf("dict data pool: %x \n", pdict->indexWdPool.pstart);
  printf("dict data pool memsize: %x \n", pdict->indexWdPool.memsize);
  printf("dict data pool memfree: %x \n", pdict->indexWdPool.memfree);
  
  if (1 == fulldump)
  {
    printf("dict items: \n");
    for (i = 0; i < pdict->wordnum; i++)
    {
      printf("[%s] \n", pdict->pindex[i].wd.pdata);
    }
  }



  return 0;
}


/* check the existence of a word in a dict. */
int check_dict_word(const dict_type * pdict, const WordType * pword, void * arg)
{
  int i;
  WordType * pdictw = pdict->pindex.wd;
  
//  assert(pword && pdict);


  for (i = 0; i < pdict->wordnum; i++)
  if (word_comp(&pdictw[i], pword))
      return TRUE;


  return FALSE;
}



int word_comp(const WordType * pw1, const WordType * pw2)
{
  int i;
  
  if (pw1->len != pw2->len)
    return FALSE;

  for (i = 0; i < pw1->len; i++)
  if (pw1->pdata[i] != pw2->pdata[i])
    return FALSE;

  return TRUE;
}


int create_wstack(wstack_type *pstack, int size)
{
  pstack->topidx = -1;
  pstack->size= size;
  pstack->array = NULL;
  pstack->array = malloc(size * sizeof(wstack_item_type));

  if (NULL == pstack->array)
    return -1;


  return 0; 
}


int push(wstack_type *pstack, wstack_item_type *pitem)
{
  if (pstack->topidx + 1 == pstack->size)
  {
    printf("stack no room !\n");
    return -1; // no dynamic alloc for more memory need now.
  }


  pstack->topidx++;
  pstack->array[pstack->topidx] = *pitem;

  return 0;
}


int pop(wstack_type *pstack, wstack_item_type *pitem)
{
  if (pstack->topidx == -1)
  {
    return -1;
    printf("empty stack already! \n");
  }
  

  *pitem = pstack->array[pstack->topidx];
  pstack->topidx--;

  //no dynamic free redundant memory.

  return 0;
}


