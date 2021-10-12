static char sccsid[] = "@(#)25  1.5  src/bos/usr/lib/nls/loc/imt/tfep/timprof.c, libtw, bos411, 9428A410j 4/21/94 02:30:55";
/*
 *
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: timproce.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
*                                                                           *
*       Module Name      : TIMProf                                          *
*                                                                           *
*       Description Name : Chinese Input Method - Profile search            *
*                                                                           *
*       Function         : ...............                                  *
*                                                                           *
*       Module Type      : C                                                *
*                                                                           *
*       Compiler         : AIX C                                   *
*                                                                           *
*       Author           : Jim Roung                                        *
*                                                                           *
*       Status           : Chinese Input Method Version 1.0                 *
*                                                                           *
*       Change Activity  : T.B.W.                                           *
*                                                                           *
****************************************************************************/

/* ---------------------------------------------------------------- */
/* ------------------   Include files   --------------------------- */
/* ---------------------------------------------------------------- */

#include "taiwan.h"
#include "fcntl.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "ctype.h"


/* ---------------------------------------------------------------- */
/* =================  Global Variables  =====================      */
/* ---------------------------------------------------------------- */

static list    *head = NULL, *tail = NULL, *temp = NULL;
void            closeprofile();
void            destroyprofile();
char           *getoptvalue();
void            makewordtree(), destroywordtree();
int             openprofile();
char           *searchprofile();
/* ---------------------------------------------------------------- */
/* ---------------  Serach the location for profile  ------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : searchprofile()                                            *
* Description : To search where the profile is.                         *
*               - 1st : Search the specified system environment         *
*                       variable (TIMPROFILE=).                         *
*               - 2nd : Search the HOME directory.                      *
*               - 3rd : Using the default path name.                    *
* External Reference :                                                  *
* Input  :                                                              *
*         access_mode : The access mode of specified file.              *
* Output :                                                              *
*         file : The pointer of this found profile.                     *
*                                                                       *
*************************************************************************/


char           *searchprofile(access_mode)
   int             access_mode;
{
   char           *pathfind[PATHFIND];
   char           *file;
   int             loop_counter;


   /* set the path name for environment setting or default file name */

   pathfind[0] = pathfind[1] = pathfind[2] = NULL;

   pathfind[0] = (char *) getenv(ENVVAR);

   file = (char *) getenv("HOME");
   pathfind[1] = malloc(strlen(file) + sizeof(FILENAME1) + 1);
   (void) sprintf(pathfind[1], "%s/%s", file, FILENAME1);

   pathfind[2] = DEFAULTPROFILE;

   /* check the file exist or not                                   */

   for (loop_counter = 0; loop_counter < PATHFIND; loop_counter++)
   {
      if (!*pathfind[loop_counter] || !pathfind[loop_counter])
         continue;
      file = malloc((unsigned int) strlen(pathfind[loop_counter]) + 1);
      (void) strcpy(file, pathfind[loop_counter]);
      if (access(file, access_mode) >= 0)
         break;                                  /* can read the file */
      free(file);
      file = NULL;
   }                                             /* for */

   free(pathfind[1]);
   return file;

}                                                /* searchprofile() */

/* V4.1 V4.1 V4.1, searchlearnfile() is designd for V4.1 purpose        *
 * (To search where the user learning file.                             */
/************************************************************************
*                                                                       *
* Function : searchlearnfile()                                          *
* Description : To search where the user learning file                  *
*               - 1st : Search the specified system environment         *
*                       variable (LEARNFILE=).                          *
*               - 2nd : Search the HOME directory.                      *
*               - 3rd : Using the default path name.                    *
* External Reference :                                                  *
* Input  :                                                              *
*         access_mode : The access mode of specified file.              *
* Output :                                                              *
*         file : The pointer of this found profile.                     *
*                                                                       *
*************************************************************************/


char           *searchlearnfile(access_mode)
int            access_mode;
{
   char           *pathfind[2];
   char           *file;
   uint            loop_counter;


   /* set the path name for environment setting or default file name */

   pathfind[0] = pathfind[1] = NULL;
   file = NULL;
   pathfind[0] = (char *) getenv(LEARNFILE);
   if (( pathfind[0] ) && strlen(pathfind[0]))
   {
     file = malloc((unsigned int) strlen(pathfind[0]) + 1);    /*  V410 */
     (void) strcpy(file, pathfind[0]);                         /*  V410 */
     if (access(file, access_mode) >0 )                       /*  @big5 */
     {                                                        /*  @big5 */
       free(file);                                            /*  @big5 */
       file = NULL;                                           /*  @big5 */
     }                                                        /*  @big5 */
    }

   return file;

}                                                /* searchprofile() */

/* ---------------------------------------------------------------- */
/* -------------------  Open the specified profile  -------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : openprofile()                                              *
* Description : To open this found profile.                             *
* External Reference :                                                  *
* Input  :                                                              *
*         profile : The pointer of this found profile.                  *
* Output :                                                              *
*         NULL : Can not open this specified profile.                   *
*         TRUE : This open operation is successful.                     *
*                                                                       *
*************************************************************************/

int             openprofile(profile)
   char           *profile;
{
   int             fd;
   struct stat     file_buffer;

   if ((fd = open(profile, O_RDONLY)) == -1)
      return NULL;
   if ((fstat(fd, &file_buffer)) == -1)
      return NULL;
   if ((profile_ptr = malloc(file_buffer.st_size)) == NULL)
      return NULL;
   read(fd, profile_ptr, file_buffer.st_size);
   profile_size = file_buffer.st_size;
   profile_offset = profile_ptr;
   close(fd);                                    /* close user profile           */
   if (yyparse() == 1)
   {
      if (profile_ptr)
         free(profile_ptr);
      destroywordtree();
      return NULL;
   }
    /* if */
   else
      return TRUE;

}                                                /* open profile */
/* ---------------------------------------------------------------- */
/* ----------------  Find System/User Dictionary  ---------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : FindSysUserDict()                                          *
* Description : To search where the system/user dictionay is.           *
*               - 1st : Search the specified system environment         *
*                       variable (TIMPROFILE=).                         *
*               - 2nd : Search the HOME directory.                      *
*               - 3rd : Using the default path name.                    *
* External Reference :                                                  *
* Input  :                                                              *
*         dict_type   : Which dictionary will be searched.              *
*         access_mode : The access mode of this searching dictionary.   *
* Output :                                                              *
*         file : The pointer of this found profile.                     *
*                                                                       *
*************************************************************************/

char           *FindSysUserDict(dict_type, access_mode)
   char            dict_type;
   char            access_mode;
{
   char           *pathfind[PATHFIND];
   char           *file;
   int             loop_counter;

   switch (dict_type)
   {

   case PH_USER_DICT:
      pathfind[0] = (char *) getenv(PHUSERDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = malloc(strlen(file) + sizeof(FILENAME2) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME2);
      pathfind[2] = DEFAULTPHUSERDICT;
      break;
   case TJ_USER_DICT:
      pathfind[0] = (char *) getenv(TJUSERDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = malloc(strlen(file) + sizeof(FILENAME3) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME3);
      pathfind[2] = DEFAULTTJUSERDICT;
      break;
   case PH_SYS_DICT:                             /* PH system dictionary path => default only */
      pathfind[0] = DEFAULTPHSYSDICT;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case TJ_SYS_DICT:                             /* TJ system dictionary path => default only */
      pathfind[0] = DEFAULTTJSYSDICT;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   default:
      break;
   }                                             /* switch */

   for (loop_counter = 0; loop_counter < PATHFIND; loop_counter++)
   {
      if (!*pathfind[loop_counter])
         continue;
      file = (char *) malloc((unsigned int) strlen(pathfind[loop_counter]) + 1);
      (void) strcpy(file, pathfind[loop_counter]);
      if (access(file, access_mode) >= 0)
         break;
      free(file);
      file = (char *) NULL;
   }                                             /* for */

   free(pathfind[1]);
   return file;

}                                                /* FindSysUserDict () */

/* ---------------------------------------------------------------- */
/* ---------------  Parser for user defined profile -------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : parse()                                                    *
* Description : To simple analysis the contents of this found profile.  *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A.                                                          *
* Output :                                                              *
*         N/A.                                                          *
*                                                                       *
*************************************************************************/


/* ---------------------------------------------------------------- */
/* -----------------  Make word tree by parser  ------------------ */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : makewordtree()                                             *
* Description : To make a word-tree for those reserved words in profile.*
* External Reference :                                                  *
* Input  :                                                              *
*         str1 : Reserved word -> initchar/initsize/initbeep.           *
*         str2 : Phonetic/Tsang-Jye/Alpha-numeric. or                   *
*                Full/Half or                                           *
*                Beepon/Beepoff                                         *
* Output :                                                              *
*         N/A.                                                          *
*                                                                       *
*************************************************************************/

void            makewordtree(str1, str2)
   char           *str1, *str2;
{

   if ((tail = (list *) malloc(sizeof(list))) == NULL)
      exit(1000);                                /* Error code for internal debugging */
   if (head == NULL)                             /* create a linked list for this word tree */
   {
      head = (list *) malloc(sizeof(list));      /* The 1st allocation for tree */
      temp = head;
   }                                             /* if */

   temp->leftword = str1;                        /* Make single linked list */
   temp->rightword = str2;
   temp->next = tail;
   temp = tail;
   temp->next = NULL;                            /* debby */
   temp->leftword=NULL;                          /* debby */
   temp->rightword = NULL;

}                                                /* makewordtree() */

/* ---------------------------------------------------------------- */
/* -----------  Search word tree for specified value  ------------ */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : getoptvalue()                                              *
* Description : To get the wanted keyword.                              *
* External Reference :                                                  *
* Input  :                                                              *
*         target : initchar/initsize/initbeep.                          *
* Output :                                                              *
*         temp_ptr->rightword : Alphanumeric/Phonetic/Tsang-Jye.        *
*                               Half/Full.                              *
*                               BeepOn/BeepOff                          *
*                                                                       *
*************************************************************************/

char           *getoptvalue(target)
   char           *target;
{                                                /* retrivel the keyword from word tree */
   list           *temp_ptr = head;

   for (; temp_ptr != NULL; temp_ptr = temp_ptr->next)
   {
      if (strcmp(temp_ptr->leftword, target))
         continue;
      else
         return (temp_ptr->rightword);
   }

   return NULL;                                  /* target not found */

}                                                /* getoptvalue() */

/* ---------------------------------------------------------------- */
/* ----------------  Destroy the word tree  ---------------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : destroywordtree()                                          *
* Description : To destroy the maked linked list word tree              *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A.                                                          *
* Output :                                                              *
*         N/A.                                                          *
*                                                                       *
*************************************************************************/

void            destroywordtree()
{
   list           *ptr, *next_ptr;

   if ((next_ptr = head) == NULL)
      return;
   do
   {
      ptr = next_ptr;
      free(ptr->leftword);
      free(ptr->rightword);
      next_ptr = ptr->next;
      free(ptr);

   } while (next_ptr);

   head = tail = NULL;                           /* reset to NULL pointer        */

}                                                /* destroywordtree() */

/* ---------------------------------------------------------------- */
/* ------------------  Close user define profile  ---------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : closeprofile()                                             *
* Description : To close the opened profile.                            *
* External Reference :                                                  *
* Input  :                                                              *
*         N/A.                                                          *
* Output :                                                              *
*         N/A.                                                          *
*                                                                       *
*************************************************************************/

void            closeprofile()
{
   void            destroywordtree();

   if (profile_ptr)
      free(profile_ptr);
   destroywordtree();

}

/* ********************************************************************** *
 *
 * nextchar() ...                                                  * *
 *
********************************************************************* */



int             nextchar()
{
   if              ((profile_offset - 1) >= (profile_ptr + profile_size))
                      return (EOF);
   else
      return *profile_offset++;
}                                                /* nextchar()  */

