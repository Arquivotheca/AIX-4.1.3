static char sccsid[] = "@(#)74	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhimprof.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:39:03";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: FindSysUserDict
 *		UdFindIM
 *		closeprofile
 *		destroywordtree
 *		getoptvalue
 *		makewordtree
 *		nextchar
 *		openprofile
 *		searchprofile
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
*                                                                           *
*       Module Name      : ZHIMProf                                         *
*                                                                           *
*       Description Name : Chinese Input Method - Profile search            *
*                                                                           *
****************************************************************************/

/* ---------------------------------------------------------------- */
/* ------------------   Include files   --------------------------- */
/* ---------------------------------------------------------------- */

#include "chinese.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

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
/* ----------------  Serach the location for profile  ------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : searchprofile()                                            *
* Description : To search where the profile is.                         *
*               - 1st : Search the specified system environment         *
*                       variable (ZHIMPROFILE=).                        *
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
   pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME1) + 1);
   (void) sprintf(pathfind[1], "%s/%s", file, FILENAME1);

   pathfind[2] = DEFAULTPROFILE;

   /* check the file exist or not                                   */
   for (loop_counter = 0; loop_counter < PATHFIND; loop_counter++)
   {
      if (!*pathfind[loop_counter] || !pathfind[loop_counter])
         continue;
      file = (unsigned char*)malloc((unsigned int) strlen(pathfind[loop_counter]) + 1);
      (void) strcpy(file, pathfind[loop_counter]);
      if (access(file, access_mode) >= 0)
         break;                                  /* can read the file */
      free(file);
      file = NULL;
   }                                             /* for */

   free(pathfind[1]);
   return file;

}                                                /* searchprofile() */

/* ---------------------------------------------------------------- */
/* --------------------  Open the specified profile  -------------- */
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
   if ((profile_ptr = (unsigned char*)malloc(file_buffer.st_size)) == NULL)
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
/* -----------------  Find System/User Dictionary  ---------------- */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : FindSysUserDict()                                          *
* Description : To search where the system/user dictionay is.           *
*               - 1st : Search the specified system environment         *
*                       variable (ZHIMPROFILE=).                        *
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

   case PY_USER_DICT:
      pathfind[0] = (char *) getenv(PYUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME2) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME2);
      pathfind[2] = DEFAULTPYUSRDICT;
      break;
   case TJ_USER_DICT:
      pathfind[0] = (char *) getenv(TJUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME17) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME17);
      pathfind[2] = DEFAULTTJUSRDICT;
      break;
   case LE_USER_DICT:
      pathfind[0] = (char *) getenv(LEUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME3) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME3);
      pathfind[2] = DEFAULTLEUSRDICT;
      break;
   case EN_USER_DICT:
      pathfind[0] = (char *) getenv(ENUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME4) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME4);
      pathfind[2] = DEFAULTENUSRDICT;
      break;
   case ABC_USER_REM:
      pathfind[0] = (char *) getenv(ABCUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME5) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME5);
      pathfind[2] = NULL;
      break;
   case ABC_USER_DICT:
      pathfind[0] = (char *) getenv(ABCUSRDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME6) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME6);
      pathfind[2] = NULL;
      break;
   case UD_DICT:
      pathfind[0] = (char *) getenv(UDDICT);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAME16) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAME16);
      pathfind[2] = DEFAULTUDDICT;
      break;
   case PY_SYS_DICT_COMM:        /* PY system dictionary path => default only */
      pathfind[0] = DEFAULTPYSYSDICT1;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case PY_SYS_DICT_GB:          /* PY system dictionary path => default only */
      pathfind[0] = DEFAULTPYSYSDICT2;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case PY_SYS_DICT_CNS:         /* PY system dictionary path => default only */
      pathfind[0] = DEFAULTPYSYSDICT3;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case PY_SYS_DICT_JK:          /* PY system dictionary path => default only */
      pathfind[0] = DEFAULTPYSYSDICT4;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case TJ_SYS_DICT:             /* TJ system dictionary path => default only */
      pathfind[0] = DEFAULTTJSYSDICT;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case LE_SYS_DICT:            /* LE system dictionary path => default only */
      pathfind[0] = DEFAULTLESYSDICT;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case EN_SYS_DICT:             /* EN system dictionary path => default only */
      pathfind[0] = DEFAULTENSYSDICT;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case ABC_SYS_DICT_CWD_S:     /* ABC system dictionary path => default only */
      pathfind[0] = DEFAULTABCSYSDICT1;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case ABC_SYS_DICT_CWD_T:     /* ABC system dictionary path => default only */
      pathfind[0] = DEFAULTABCSYSDICT2;
      pathfind[1] = pathfind[2] = NULL;          /* reset pointer */
      break;
   case ABC_SYS_DICT_OVL:       /* ABC system dictionary path => default only */
      pathfind[0] = DEFAULTABCSYSDICT3;
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
   if((file == NULL) &&(dict_type == ABC_USER_REM||dict_type == ABC_USER_DICT)) 
   {
      file = (char *) malloc((unsigned int) strlen(pathfind[1]) + 1);
      (void) strcpy(file, pathfind[1]);
   }

   free(pathfind[1]);
   return file;

}                                                /* FindSysUserDict () */

/* ---------------------------------------------------------------- */
/* ----------------  Parser for user defined profile -------------- */
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
/* ------------------  Make word tree by parser  ------------------ */
/* ---------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : makewordtree()                                             *
* Description : To make a word-tree for those reserved words in profile.*
* External Reference :                                                  *
* Input  :                                                              *
*         str1 : Reserved word -> initchar/initsize/initbeep.           *
*         str2 : Pinyin/English-Chinese/ABC/Alpha-numeric. or           *
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
   temp->next = NULL;

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
*       target : initchar/initsize/initbeep.                            *
* Output :                                                              *
*       temp_ptr->rightword : Alphanumeric/Pinyin/English-Chinese/ABC.  *
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
/* -----------------  Destroy the word tree  ---------------------- */
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

/**********************************************************************
 *                                                                    *
 * nextchar() ...                                                     *
 *                                                                    *
 **********************************************************************/



int             nextchar()
{
   if              ((profile_offset - 1) >= (profile_ptr + profile_size))
                      return (EOF);
   else
      return *profile_offset++;
}                                                /* nextchar()  */


/************************************************************************
*                                                                       *
* Function : UdFindIM()                                                 *
* Description : To search where the User defined IM mode  is.           *
*               - 1st : Search the specified system environment         *
*                       variable (ZHIMPROFILE=).                        *
*               - 2nd : Search the HOME directory.                      *
*               - 3rd : Using the default path name.                    *
* External Reference :                                                  *
* Input  :                                                              *
* Output :                                                              *
*         file : The pointer of this found profile.                     *
*                                                                       *
*************************************************************************/

char       *UdFindIM()
{

   char           *pathfind[PATHFIND];
   char           *file;
   int             loop_counter;

      pathfind[0] = (char *) getenv(USERDEFINEDIM);
      file = (char *) getenv("HOME");
      pathfind[1] = (unsigned char*)malloc(strlen(file) + sizeof(FILENAMEUDIM) + 1);
      (void) sprintf(pathfind[1], "%s/%s", file, FILENAMEUDIM);
      pathfind[2] = DEFAULTUDIM;


   for (loop_counter = 0; loop_counter < PATHFIND; loop_counter++)
   {
      if (!*pathfind[loop_counter])
         continue;
      file = (char *) malloc((unsigned int) strlen(pathfind[loop_counter]) + 1);
      (void) strcpy(file, pathfind[loop_counter]);
      if (access(file, 1) >= 0)
         break;
      free(file);
      file = (char *) NULL;
   }                                             /* for */

   free(pathfind[1]);
   return file;

}
