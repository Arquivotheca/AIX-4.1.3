static char sccsid[] = "@(#)84	1.43  src/bos/usr/ccs/lib/libodm/odmi.c, libodm, bos41J, 9512A_all 3/17/95 15:41:55";
/*
 *   COMPONENT_NAME: (LIBODM) OBJECT DATA MANAGER LIBRARY
 *
 * FUNCTIONS:
 *      odm_set_perms, odm_set_path, odm_initialize, odm_terminate
 *      odm_open_class, odm_create_class, init_class, odm_rm_class
 *      odm_close_class, odm_rm_by_id, odm_rm_obj, odm_change_obj
 *      odm_get_by_id, odm_get_list, odm_get_obj, odm_get_first,
 *      odm_get_next, odm_add_obj, odm_free_list, note_class
 *      odm_mount_class, get_offsets
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1989 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <memory.h>

#include <odmi.h>
#include "odmlib.h"
#include "odmtrace.h"
#include "odmhkids.h"

#define LOCK_AVAILABLE 0

extern int number_of_locks;      /* from odmlock.o */
extern int *lock_table;         /* from odmlock.o */

int odmerrno;
int odmtrace = 0;
int odm_read_only;

/*--------------------*/
/* int odmerrno = -1; bdn */
/*--------------------*/
char *odmcf_errstr = NULL;


/* The following variables (adds,fetches,deletes, and
                           changes) are used for getting a summary with a
                           debugger of how many of each operation have been
                           done by the library in this process for performance
                           monitoring */
static int adds = 0;
static int fetches = 0;
static int deletes = 0;
static int changes = 0;
static char *Namelist[1024];

static  struct Class    *Classlist[1024];
static numClasses = 0;
char repospath[MAX_ODM_PATH] = "";


/* mode variables for the advanced options */
int odmcf_perms_op = 0664; /* default permissions for creates */

struct Class *class_cur_shmated = NULL;         /* flags for classes    */
struct StringClxn *clxn_cur_shmated = NULL;     /*   that have been     */
                                                /*   shmat'd.           */

/*
 *   NAME:     ODM_SET_PERMS
 *   FUNCTION: set odm mode variable so that classes are opened
 *             with the specified permissions
 *
 *   RETURNS: Permission value before the change.
 */
int odm_set_perms(perm)
int perm;        /* New permissions */
{
    int prev_perms;

    START_ROUTINE(ODMHKWD_ODM_SET_PERMS);
    TRC("odm_set_perms","Setting new perms %d",perm,
        "previous perm %d",odmcf_perms_op);


    prev_perms = odmcf_perms_op;
    odmcf_perms_op = perm;

    TRC("odm_set_perms","Permissions set","","","");
    STOP_ROUTINE;
    return(prev_perms);
}


/*
 *   NAME:     ODM_SET_PATH 
 *   FUNCTION: set odm mode variable so that classes are opened
 *             in a particular repository directory
 *   RETURNS:  Previous value of the path before the change was made.
 */
char *
odm_set_path(
char *reppath)          /* path to repository */
{
    char *path;
    char *cp;


    START_ROUTINE(ODMHKWD_ODM_SET_PATH);
    TRC("odm_set_path","Setting path %s",reppath,
        "Prev. path %s",repospath);

    if (reppath != NULL && strlen(reppath) > MAX_ODM_PATH -1)
    {
        TRC("odm_set_path","Path is too long!","","","");
        odmerrno = ODMI_INVALID_PATH;
        STOP_ROUTINE;
        return((char *) -1);
    } /* endif */

    cp = (char *)malloc(strlen(repospath)+1);
    if (  cp == NULL )
    {
        TRC("odm_set_path","Malloc failed! size %d",strlen(repospath),"","");
        odmerrno = ODMI_MALLOC_ERR;
        STOP_ROUTINE;
        return((char *) -1);
     }

    /*if_err_ret_err(cp,(char *),ODMI_MALLOC_ERR);*/
    cp[0] = '\0';
    strncat(cp,repospath,MAX_ODM_PATH);
    if(reppath)
    {
        repospath[0] = '\0';
        strncat(repospath,reppath,MAX_ODM_PATH);
    }
    else
    {
        path = getenv("ODMDIR");
        if(path)
        {
            repospath[0] = '\0';
            strncat(repospath,path,MAX_ODM_PATH);
        }
    }

    TRC("odm_set_path","Path set to repospath %s",repospath,"","");
    STOP_ROUTINE;
    return(cp);
}

/*
 *   NAME:     ODM_INITIALIZE
 *   FUNCTION: initialize the database - gets repos path from environment
 *   RETURNS:  Zero always.
 *
 */
int odm_initialize()
{
    int i;
    char *p;

    START_ROUTINE(ODMHKWD_ODM_INITIALIZE);
    TRC("odm_initialize","initializing ODM","","","");

    for(i=0;i<MAX_CLASSES;i++)Classlist[i] = NULL;
    numClasses = 0;

    if ( (p = odm_set_path(NULL)) == (char *)-1)
    {
        TRC("odm_initialize","Could not set path! err %d",odmerrno,"","");
        STOP_ROUTINE;
        return(-1);
    } 
    else
	free ((void *)p);

#ifdef DEBUG
    /*--------------------------------------------------------*/
    /* Turn on tracing if the external variable ODMERR exists */
    /*--------------------------------------------------------*/
    if (getenv("ODMERR") )
        odmtrace = 1;
#endif
    TRC("odm_initialize","Successful initialization","","","");
    STOP_ROUTINE;
    return(0);
}

/*
 *   NAME:     ODM_TERMINATE
 *   FUNCTION: terminate the odm.  Does the necessary cleanup to shutdown
 *             the odm session.
 *   RETURNS:  A 0 if successful, -1 otherwise.
 */
int odm_terminate()
{
    int size;
    int index;
    struct ClassFileHdr *cur;
    struct ClxnFileHdr *clxn_cur;
    int returnstatus;


    /*
      This function should at least detach any object class currently
      in shared memory.
   */
    START_ROUTINE(ODMHKWD_ODM_TERMINATE);
    TRC("odm_terminate","Shutting down ODM.","","","");


    if ( class_cur_shmated )
    {

        /*---We do not have to ftruncate the file----------------*/
        /*---to set the file size--------------------------------*/
        /* cur = (struct ClassFileHdr *) class_cur_shmated->hdr; */
        /* size =  (int)(cur->Class.data                         */
        /*     + cur->Hdr.ndata * cur->Class.structsize);        */
        /* size = (size+4096)&(-4096);                           */
        /* ftruncate(class_cur_shmated->fd, size);               */

        shmdt((char *)class_cur_shmated->hdr);
        class_cur_shmated->hdr = NULL;
        class_cur_shmated = NULL;
    }

    if ( clxn_cur_shmated )
    {
        /*---We do not have to ftruncate the file--------------------------*/
        /*---to set the file size------------------------------------------*/
        /* clxn_cur = (struct ClxnFileHdr *) clxn_cur_shmated->hdr;        */
        /* size =  (int)(clxn_cur->StringClxn.data + clxn_cur->Hdr.ndata); */
        /* size = (size+4096)&(-4096);                                     */
        /* ftruncate(clxn_cur_shmated->fd, size);                          */
        /*-----------------------------------------------------------------*/

        shmdt((char *) clxn_cur_shmated->hdr);
        clxn_cur_shmated->hdr = NULL;
        clxn_cur_shmated = NULL;
    }

    /*-----------------------------------------------------------------------*/
    /* Check the lock table to see if any locks have been set.  If there are */
    /* locks, unlock them.                                                   */
    /*-----------------------------------------------------------------------*/
    if (number_of_locks > 0)
    {
        for (index = 0; index < number_of_locks; index++ )
        {
            if ( lock_table[index] == LOCK_AVAILABLE)
            	continue;
            returnstatus = odm_unlock(lock_table[index]);
            if (returnstatus < 0)
            {
                TRC("odm_terminate","Cannot free lock %d",lock_table[index],
                    "error %d",odmerrno);

                odmerrno = ODMI_UNLOCK;
                STOP_ROUTINE;
                return(-1);
            } /* endif */

        } /* endfor */

        number_of_locks = 0;
        free((void *) lock_table);
        lock_table = NULL;

    } /* endif */

    TRC("odm_terminate","Checking for open files. numClasses %d",numClasses,
                       "","");


    for(index=0;index<numClasses;index++)
    {
       if (Classlist[index] != NULL &&
           verify_class_structure(Classlist[index]) >= 0)
       {

          /* sync the files to disk before closing */
          returnstatus = fsync(Classlist[index]->fd);
          if (returnstatus < 0)
             TRC("odm_terminate","Could not sync class to disk! err %d",odmerrno,
                 "","");

          TRC("odm_terminate","Closing class %s",
                              Classlist[index]->classname,"","");


	  if ((Classlist[index])->crit) {
	     TRC("odm_terminate","free (Classlist[index])->crit 0x%08x",
			(Classlist[index])->crit,"","");
	     free( (void *)(Classlist[index])->crit);
	     (Classlist[index])->crit = 0;
	     (Classlist[index])->ncrit = 0;
	  }
          returnstatus = raw_close_class(Classlist[index],FALSE);
          if (returnstatus < 0)
          {
             TRC("odm_terminate","Could not close class! err %d",odmerrno,"","");
             STOP_ROUTINE;
             return(-1);
          } /* endif */

       } /* endif */

       free((void *) Namelist[index]);
       Classlist[index] = NULL;

    } /* endfor */

    numClasses = 0;

    TRC("odm_terminate","Successful termination","","","");
    STOP_ROUTINE;
    return(0);
}
  
/*
 *    NAME:    ODM_OPEN_CLASS
 *    FUNCTION: open an odm object class in the mode used by
 *              odm, returning its symbol.
 *    RETURNS:  The same Class pointer passed in (classp) if successful,
 *              -1 otherwise.
 *
 */
extern int open_RDONLY;
struct Class *odm_open_class(classp)
struct Class *classp;   /* CLASS_SYMBOL of the object class to open */
{
    int rc;
    int fd;
    int open_mode;
    char pathname[MAX_ODM_PATH];
    static int first = 1;
    struct stat statbuf;
 
    START_ROUTINE(ODMHKWD_ODM_OPEN_CLASS);

    TRC("odm_open_class","Opening class ","","","");

    if (verify_class_structure(classp) <  0)
    {
        TRC("odm_open_class","Invalid structure %x",classp,"","");
        STOP_ROUTINE;
        return((struct Class *) -1);
    } /* endif */


    rc = note_class ( classp );
    if ( rc == -1 )
    {
        TRC("odm_open_class","Could not note class!","","","");
        STOP_ROUTINE;
        return ((struct Class *) -1);
    }

    /*if_err_ret_err(note_class(classp),(struct Class *),0);*/

    if (!classp->open)
    {

       if(repospath[0])
          sprintf(pathname,"%s/%s",repospath,classp->classname);
       else
          strcpy(pathname,classp->classname);

       if ( (stat(pathname,&statbuf)) == -1 )
            odm_searchpath(pathname,classp->classname);

       TRC("odm_open_class","Object class path %s",pathname,"","");
       /* Defect 62044 */
       /* if calling routine is odmget then set open mode for READ ONLY */
       open_mode = O_RDWR;
/*
	if (open_RDONLY)
		{
        	open_mode = O_RDONLY;
		open_RDONLY = 0;
		}
*/
       fd = open(pathname,open_mode ,odmcf_perms_op);
       if (fd == -1 && (errno == EACCES || errno == EROFS))
       {
          /*----------------------------------------------------------*/
          /* The file could not be opened because of the permissions. */
          /* Attempt to open the file as READ-ONLY                    */
          /*----------------------------------------------------------*/
          TRC("odm_open_class","Read-write open failed! Attempting read-only",
                         "","","");

          open_mode = O_RDONLY;
          fd = open(pathname,open_mode ,odmcf_perms_op);

       } /* endif */

       if(fd == -1)
       {
          if (errno == ENOENT)
             odmerrno = ODMI_CLASS_DNE;
          else if (errno == EACCES)
              odmerrno = ODMI_CLASS_PERMS;
          else if (errno == ENOTDIR)
              odmerrno = ODMI_INVALID_PATH;
          else
              odmerrno = ODMI_OPEN_ERR;

          TRC("odm_open_class","Could not open class. err %d",
              odmerrno,"errno %d",errno);

          STOP_ROUTINE;
          return((struct Class *)-1);
        } /* endif */

        classp->fd = fd;
        classp->hdr = NULL;
        classp->open = CLASS_IS_OPEN;

        if (open_mode == O_RDONLY)
        {
           /* Set the read-only bit as well */
           TRC("odm_open_class","Class is read-only %s",classp->classname,"","");
           classp->open = classp->open | OPENED_AS_READ_ONLY;
        } /* endif */

    } /* endif !classp->open */

    TRC("odm_open_class","Opened class. address %x",classp,"","");
    STOP_ROUTINE;
    return(classp);
}

/*
 *    NAME:    ODM_CREATE_CLASS
 *    FUNCTION: create an empty object class
 *    RETURNS:  A 0 if successful, -1 otherwise.
 *
 */
int odm_create_class(classp)
struct Class *classp;   /* Pointer to the object class to close */
{
    int rc;
    char *mem = NULL;
    int fd = -1;
    int size = -1;
    mode_t old_perms;
    char pathname[MAX_ODM_PATH + MAX_ODMI_NAME + 1];
    int class_created = FALSE;  /* Boolean indicating if the class
                                                     is on the disk */

    START_ROUTINE(ODMHKWD_ODM_CREATE_CLASS);
    TRC("odm_create_class","Opening class","","","");

    if (verify_class_structure(classp) < 0)
    {
        TRC("odm_create_class","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if(repospath[0])
        sprintf(pathname,"%s/%s",repospath,classp->classname);
    else
        strcpy(pathname,classp->classname);

    if(!access ( pathname, F_OK))
    {
       TRC("odm_create_class","Class already exists!","","","");
       odmerrno = ODMI_CLASS_EXISTS;
       STOP_ROUTINE;
       return(-1);
    }

    /* Set system file permissions to take whatever user has specified */
    /* in odmcf_perms_op                                               */
    old_perms = umask ((mode_t) 0);

    fd = open(pathname,O_RDWR | O_CREAT,odmcf_perms_op);

    if(fd == -1)
    {
       if (errno == ENOENT)
           odmerrno = ODMI_INVALID_PATH;
       else if (errno == EACCES)
           odmerrno = ODMI_CLASS_PERMS;
       else
           odmerrno = ODMI_OPEN_ERR;

       TRC("odm_create_class","Could not open class. err %d",
            odmerrno,"errno %d",errno);

       goto ERROR_CLEANUP_FOR_odm_create_class;

    }

    /*--------------------------------------------------------------*/
    /* Set class_created to TRUE so that we know to remove the file */
    /* in an error condition.                                       */
    /*--------------------------------------------------------------*/

    class_created = TRUE;

    mem = shmat(  fd  , (char *) 0 , SHM_MAP );
    if((int)mem == -1)
    {
       if (errno == EACCES)
           odmerrno = ODMI_CLASS_PERMS;
       else
            odmerrno = ODMI_OPEN_ERR;

       TRC("odm_create_class","Could not shmat class. err %d",errno,"","");

       goto ERROR_CLEANUP_FOR_odm_create_class;
    }

    size = init_class(classp,mem);
    if (size < 0)
    {
       TRC("odm_create_class","Could not init class! %d",odmerrno,"","");
       goto ERROR_CLEANUP_FOR_odm_create_class;
    } /* endif */

    size = (size+4096)&(-4096);

    ftruncate(fd,size);

    shmdt ( mem );
    mem = NULL;

    close(fd);
    fd = -1;

    TRC("odm_create_class","Class created, size %d",size,"","");

    if(classp->clxnp)
    {

       TRC("odm_create_class","Class has collection","","","");
       rc = create_clxn ( classp->clxnp );
       if ( rc == -1 )
       {
          TRC("odm_create_class","Could not make collection, err %d",
              odmerrno,"","");
          goto ERROR_CLEANUP_FOR_odm_create_class;
       } /* endif */

      /*if_err_ret_err(create_clxn(classp->clxnp),(int),0);*/
    }

    /* Reset system permissions to whatever they were PRIOR to creating */
    /* the file                                                         */
    (void) umask (old_perms);

    TRC("odm_create_class","Class successfully created","","","");
    STOP_ROUTINE;
    return(0);

ERROR_CLEANUP_FOR_odm_create_class:
    if (fd > 0)
        close(fd);

    if (class_created)
        unlink(pathname);


    (void) umask (old_perms);

    TRC("odm_create_class","Class not created!","","","");

    STOP_ROUTINE;
    return(-1);
}


/*
 *    NAME:    INIT_CLASS
 *    FUNCTION: write headers, etc into empty object class file
 *    RETURNS:  The length of the header space in the empty object class
 *              if successful, -1 otherwise.
 *
 */

int init_class(classparm,mem)
struct  Class   *classparm;    /* Pointer to the object class CLASS_SYMBOL */
char    *mem;                  /* Address of the data space in memory      */
{
    struct  Class           *classp;
    struct  ClassElem       *elemp;
    int     i;
    char    *strings;

    /*-----------------------------------------------------------------*/
    /* This routine sets up the header information in the object       */
    /* class on the disk.  The object class file will look like:       */
    /*                                                                 */
    /*   Start of file -->  ______________________                     */
    /*                      |  Header information                      */
    /*                      |  (struct ClassHdr)                       */
    /*                      |---------------------                     */
    /*                      |  Class structure                         */
    /*                      |   (struct Class)                         */
    /*                      |---------------------                     */
    /*                      |  Elements of objects                     */
    /*                      |  One structure for each column           */
    /*                      |                                          */
    /*                      |  number_of_elements * (struct ClassElem) */
    /*                      |                                          */
    /*                      |---------------------                     */
    /*                      |  Strings from the structures             */
    /*                      |  (classname, elemname, etc.)             */
    /*                      |---------------------                     */
    /*                      |                                          */
    /*                      |  Object class data                       */
    /*                      |                                          */
    /*                      .                                          */
    /*                      .                                          */
    /*                      .                                          */
    /*    End of file -->   |_____________________                     */
    /*-----------------------------------------------------------------*/


    START_ROUTINE(ODMHKWD_INIT_CLASS);
    TRC("init_class","Initializing the class","","","");

    ((struct ClassHdr *)mem)->magic = ODMI_MAGIC;
    ((struct ClassHdr *)mem)->ndata = 0;
    ((struct ClassHdr *)mem)->version = 0;

    classp = (struct Class*)(mem + sizeof(struct ClassHdr));
    bcopy(classparm,classp,sizeof(struct Class));
    elemp = (struct ClassElem *)
        (mem + sizeof(struct ClassHdr) + sizeof (struct Class));
    bcopy(classparm->elem,elemp,sizeof(struct ClassElem)*classp->nelem);
    strings = (char *)elemp+sizeof(struct ClassElem)*classp->nelem;

    /* clean up class structure incase someone reads it and tries to use it */
    classp->clxnp = NULL;
    classp->open = FALSE;
    classp->hdr = NULL;

    /* fix up Class structure so all addresses are relative to file */
    classp->classname = strings-(ulong)mem;
    strcpy(strings,classparm->classname);
    strings += strlen(strings)+1;
    classp->elem = (struct ClassElem *)((char*)elemp - mem);

    /* fix up ClassElem structures */
    for(i=0;i<classp->nelem;i++)
    {
       /*copy old value of elemname */
       strcpy(strings,elemp[i].elemname);
       elemp[i].elemname = strings-(ulong)mem;
       strings += strlen(strings)+1;
       if(elemp[i].type == ODM_LINK)
       {
          /*copy name of link instead of struct ptr*/
          strcpy(strings,elemp[i].col);
          elemp[i].col = strings-(ulong)mem;
          strings += strlen(strings)+1;
          strcpy(strings,elemp[i].link->classname);
          elemp[i].link = (struct Class *)(strings-(ulong)mem);
          strings += strlen(strings)+1;
       }
    }

    /*round up to aligned boundary*/
    strings = (char*)(((ulong)strings + sizeof(ulong))&(-sizeof(ulong)));

    classp->data = (char*)(strings - mem);

    TRC("init_class","The class has been initialized","","","");
    STOP_ROUTINE;

    return((int)classp->data);

}

/*
 *    NAME:    ODM_RM_CLASS
 *    FUNCTION: eliminate an object class
 *    RETURNS:  A 0 if successful, -1 otherwise.
 *
 */

int odm_rm_class(classp)
struct Class *classp;    /* Pointer to the object class to remove */
{
    int rc;
    int destroy_clxn_returned = 0;
    int size;
    char pathname[MAX_ODMI_NAME + MAX_ODM_PATH + 2];
    struct ClassFileHdr *cur;
    int index;

    START_ROUTINE(ODMHKWD_ODM_RM_CLASS);
    TRC("odm_rm_class","Removing class","","","");

    if (verify_class_structure(classp) < 0)
    {
        TRC("odm_rm_class","Bad class structure!","","","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if(repospath[0])
        sprintf(pathname,"%s/%s",repospath,classp->classname);
    else
        strcpy(pathname,classp->classname);

    TRC("odm_rm_class","Path is %s",pathname,"","");

    /*
           Need to detach the object class from the shared memory
           segment if it is the one that is currently attached.
        */

    if ( class_cur_shmated == classp )
    {

       /*--We do not have to ftruncate--------------------------*/
       /*--to set the file size---------------------------------*/
       /*  cur= (struct ClassFileHdr *) class_cur_shmated->hdr; */
       /* size =  (int)(cur->Class.data                         */
       /*     + cur->Hdr.ndata * cur->Class.structsize);        */
       /* size = (size+4096)&(-4096);                           */
       /* ftruncate(class_cur_shmated->fd, size);               */
       /*-------------------------------------------------------*/

       shmdt((char *) class_cur_shmated->hdr);
       class_cur_shmated->hdr = NULL;
       class_cur_shmated = NULL;
    }

    /* Close this class */
    rc = raw_close_class(classp,FALSE);
    if (rc < 0)
    {
       TRC("odm_rm_class","Could not close class! err %d",odmerrno,"","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    /* set Classlist entry for the removed object class to null */

    for(index=0;index<numClasses;index++)
    {
       if (Classlist[index] != NULL &&
           verify_class_structure(Classlist[index]) >= 0)
       {
          if (0==strcmp(Classlist[index]->classname,classp->classname))
          {
             free( (void *) Namelist[index] );
             Classlist[index] = NULL;
             break;
          } /* endif */
       } /* endif */
    } /* endfor */


    if(classp->clxnp)
    {
       destroy_clxn_returned = destroy_clxn(classp->clxnp);
       /*---------------------------------------------------------------*/
       /* Check the return code after the object class has been deleted */
       /*---------------------------------------------------------------*/
    }

    rc = unlink ( pathname );
    if ( rc == -1 )
    {
       if (errno == ENOENT)
          odmerrno = ODMI_INVALID_PATH;
       else if (errno == EACCES)
          odmerrno = ODMI_CLASS_PERMS;
       else
          odmerrno = ODMI_UNLINKCLASS_ERR;

       TRC("odm_rm_class","Could not unlink class! err %d",errno,"","");

       STOP_ROUTINE;
       return(-1);
    }
    /*if_err_ret_err(unlink(pathname),,ODMI_UNLINKCLASS_ERR);*/


    /*if_err_ret_err(rv,,0);*/

    if ( destroy_clxn_returned == -1 )
    {
       TRC("odm_rm_class","Could not unlink collection! err %d",odmerrno,"","");

       STOP_ROUTINE;
       return(-1);
    }
    STOP_ROUTINE;
    return(0);
}

/*
 *    NAME:    ODM_CLOSE_CLASS
 *    FUNCTION: close a odmcf odm object class
 *    RETURNS:  A 0 if successful, -1 otherwise.
 *
 */

int odm_close_class(classp)
struct Class *classp;   /* Pointer to the object class to close */
{
    int rc;

    START_ROUTINE(ODMHKWD_ODM_CLOSE_CLASS);
    TRC("odm_close_class","Closing class","","","");
    if (verify_class_structure(classp) < 0)
    {
       TRC("odm_close_class","Bad class structure!","","","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */


    rc = raw_close_class(classp,0);
    if (rc < 0)
    {
       TRC("odm_close_class","Could not close class! err %d",odmerrno,
           "","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */


    STOP_ROUTINE;
    return(0);
}

/*
 *    NAME:     ODM_RM_BY_ID
 *    FUNCTION: deletes a particular object in a class
 *              given the class ptr and the id
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
int odm_rm_by_id(classp,id)
struct Class *classp;   /* Pointer to the object class to be deleted */
long  id;               /* memory object of obj to be deleted */
{
    int rc;
    struct Class *return_class;
    char *offset;
    int was_open;
    int temp_error;

    START_ROUTINE(ODMHKWD_ODM_RM_BY_ID);
    TRC("odm_rm_by_id","Removing id %d",id,"","");


    if (verify_class_structure(classp) < 0)
    {
        TRC("odm_rm_by_id","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if (id < 0)
    {
       TRC("odm_rm_by_id","Invalid id!","","","");
       odmerrno = ODMI_PARAMS;
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    was_open = classp->open;

    return_class = raw_addr_class ( classp );

    if ( (int) return_class == -1 )
    {
       TRC("odm_rm_by_id","Could not addr class! err %d",odmerrno,"","");
       STOP_ROUTINE;
       return(-1 );
    }

    if (classp->open & OPENED_AS_READ_ONLY)
    {
       /*----------------------------------------------------------*/
       /* If the class has been opened as read only, objects cannot*/
       /* be removed.                                              */
       /*----------------------------------------------------------*/
       TRC("odm_rm_by_id","Class is read only!","","","");
       raw_close_class(classp,was_open);
       odmerrno = ODMI_READ_ONLY;
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    /*if_err_ret_err(raw_addr_class(classp),(int),0);*/

    offset = raw_find_byid ( classp, id );
    if ( (int) offset == -1 )
    {
       TRC("odm_rm_by_id","could not find by id! err %d",odmerrno,"","");

       temp_error = odmerrno;
       raw_close_class ( classp, was_open );
       odmerrno = temp_error;

       STOP_ROUTINE;
       return ( (int) -1 );
    }

    /*if_err_ret_err(offset = raw_find_byid(classp,id),(int),0);*/
    TRC("odm_rm_by_id","Offset is %d",offset,"","");

    *(long *)offset = -1;
    ++deletes;

    rc =  raw_close_class ( classp, was_open );
    if ( rc == -1 )
    {
       TRC("odm_rm_by_id","Could not close class! err %d",odmerrno,"","");
       STOP_ROUTINE;
       return ( (int) -1 );
    }

    /*if_err_ret_err(raw_close_class(classp,was_open),(int),0);*/

    TRC("odm_rm_by_id","Object sucessfully deleted. deletes %d",deletes,
        "","");
    STOP_ROUTINE;
    return(0);
}

/*
 *    NAME:     ODM_RM_OBJ
 *    FUNCTION: delete all the objects in a class meeting a criterion
 *              given the classname
 *    RETURNS:  The number of objects deleted if successful, -1 otherwise.
 */
int odm_rm_obj(classp,crit)
struct Class *classp;  /* Object class containing objects to be deleted. */
char *crit;            /* Critieria to determine objects to delete.*/
{
    int was_open;
    int rc;
    int rv;
    int temp_error;
    struct Class *return_class;
    struct Class *NEWclassp;  

    START_ROUTINE(ODMHKWD_ODM_RM_OBJ);


    TRC("odm_rm_obj","Removing objects. Crit %s",crit,"","");
    if (verify_class_structure(classp) < 0)
    {
       TRC("odm_rm_obj","Invalid class structure!","","","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    if (!classp->reserved)
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
          TRC("odm_rm_obj","could not open class %s",classp->classname,"","");
          STOP_ROUTINE;
          return(-1);
       }
 
       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 
       copyinfo(NEWclassp, classp);              /* Defect 100497 */
    }

    if (reserved_type(classp))
        convert_to_vchar(classp);

    was_open = classp->open;

    return_class = raw_addr_class ( classp );
    if ( (int) return_class == -1 )
    {
       TRC("odm_rm_obj","Could not addr class! err %d",odmerrno,"","");
       if (reserved_type(classp))
            convert_to_char(classp,NULL);   
       STOP_ROUTINE;
       return((int) -1);
    }
    /*if_err_ret_err( raw_addr_class(classp), (int), 0);*/

    if (classp->open & OPENED_AS_READ_ONLY)
    {
       /*----------------------------------------------------------*/
       /* If the class has been opened as read only, objects cannot*/
       /* be removed.                                              */
       /*----------------------------------------------------------*/
       TRC("odm_rm_obj","Class is read only!","","","");
       if (reserved_type(classp))
            convert_to_char(classp,NULL);   
       raw_close_class(classp,was_open);
       odmerrno = ODMI_READ_ONLY;
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    rv = raw_rm_obj ( classp, crit );
    if ( rv == -1 )
    {
       TRC("odm_rm_obj","Could not remove object! err %d",odmerrno,"","");

       if (reserved_type(classp))
            convert_to_char(classp,NULL);   
       temp_error = odmerrno;
       raw_close_class(classp,was_open);
       odmerrno = temp_error;

       STOP_ROUTINE;
       return((int) -1);
    }

    /*if_err_ret_err( rv = raw_rm_obj(classp,crit), (int), 0);*/

    rc = raw_close_class ( classp, was_open );
    if ( rc == -1 )
    {
       TRC("odm_rm_obj","Could not close class! err %d",odmerrno,"","");
       if (reserved_type(classp))
            convert_to_char(classp,NULL);   
       STOP_ROUTINE;
       return((int) -1);
    }
    /*if_err_ret_err( raw_close_class(classp,was_open), (int), 0);*/

    TRC("odm_rm_obj","Removed %d objects",rv,"","");

    if (reserved_type(classp)) 
       add_convert_to_char(classp);

    STOP_ROUTINE;
    return(rv);
}

/*
 *    NAME:     ODM_CHANGE_OBJ
 *    FUNCTION: changes a particular objects in a class by its id
 *              given the class ptr and the memory object
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
int odm_change_obj(classp,cobj)
struct Class *classp;   /* Object class containing object to change */
void *cobj;             /* memory object of obj to be deleted */
{
    int rc;
    register int i,nd,rv;
    struct ClassElem *e;
    long id;                 /* object id */
    char *dbobj;
    int was_open;
    int temp_error;
    struct Class *return_class;
    char *cobj_copy;
    struct Class *NEWclassp;   


    START_ROUTINE(ODMHKWD_ODM_CHANGE_OBJ);

    if (cobj == NULL)
    {
       TRC("odm_change_obj","Null change object!","","","");
       odmerrno = ODMI_PARAMS;
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    if (verify_class_structure(classp) < 0)
    {
       TRC("odm_change_obj","Invalid class structure","","","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    cobj_copy = (char *)malloc(classp->structsize+1); 
    bcopy(cobj,cobj_copy,classp->structsize);

    if (!classp->reserved)
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
           TRC("odm_change_obj","could not open class %s",classp->classname,"","");
           STOP_ROUTINE;
           return(-1);
       }

       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 

       copyinfo(NEWclassp, classp);             /* Defect 100497 */

    }

    if (reserved_type(classp))
    {
       if (add_convert_to_vchar(classp,cobj_copy) < 0)
       {
          TRC("odm_change_obj","add_convert_to_vchar failed","","","");
          STOP_ROUTINE;
          return(-1);
       } /* endif */
    }

    TRC("odm_change_obj","Changing object","","","");
    if (verify_class_structure(classp) < 0)
    {
       TRC("odm_change_obj","Invalid class structure","","","");
       STOP_ROUTINE;
       return(-1);
    } /* endif */
/*
    if (cobj == NULL)
      {
        TRC("odm_change_obj","Null change object!","","","");
        odmerrno = ODMI_PARAMS;
        STOP_ROUTINE;
        return(-1);
      } endif */

    was_open = classp->open;

    return_class = raw_addr_class ( classp );
    if ( (int) return_class == -1 )
    {
        TRC("odm_change_obj","Could not addr class! err %d",odmerrno,"","");
        if (reserved_type(classp))
            convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((int) -1);
    }

    if (classp->open & OPENED_AS_READ_ONLY)
    {
        /*----------------------------------------------------------*/
        /* If the class has been opened as read only, it cannot be  */
        /* changed.                                                 */
        /*----------------------------------------------------------*/
        TRC("odm_change_obj","Class is read only!","","","");
        if (reserved_type(classp))
            convert_to_char(classp,NULL);   
        raw_close_class(classp,was_open);
        odmerrno = ODMI_READ_ONLY;
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    id = *(long *)cobj_copy;   
    TRC("odm_change_obj","Id to change %d",id,"","");

    dbobj = raw_find_byid ( classp, id );
    if ( (int) dbobj == -1 )
    {
        TRC("odm_change_obj","Could not find by id! err %d",odmerrno,"","");

        if (reserved_type(classp))
            convert_to_char(classp,NULL);   
        temp_error = odmerrno;
        raw_close_class(classp,was_open);
        odmerrno = temp_error;

        STOP_ROUTINE;
        return(-1);

    }
    /*if_err_ret_err(dbobj = raw_find_byid(classp,id),,0);*/

    nd = classp->nelem;
    TRC("odm_change_obj","Number of elements %d",nd,"","");

    if(classp->clxnp)
    {
        TRC("odm_change_obj","Changing Collection ","","","");

        for(e = classp->elem,i=0;i<nd;i++,e++)
        {
           if(e->type == ODM_VCHAR)
           {
              rv = change_vchar(classp, e, cobj_copy, dbobj);
              if ( rv == -1 )
              {
                  TRC("odm_change_obj","Could not find by id! err %d",
                      odmerrno,"","");
                  if (reserved_type(classp))
                      convert_to_char(classp,NULL);   

                  temp_error = odmerrno;
                  raw_close_class(classp,was_open);
                  odmerrno = temp_error;

                  STOP_ROUTINE;
                  return(-1);

              } /* endif */
              /*if_err_ret_err(rv,(int),0);*/
           } /* endif */

       }  /* endfor */
    } /* endif */


    bcopy(cobj_copy, dbobj, classp->structsize);
    /* zero out link ptr & listinfo ptr*/

    for(e = classp->elem,i=0;i<nd;i++,e++)
    {
       if(e->type == ODM_LINK)
       {
           *(char **)(dbobj + e->offset ) = NULL;
           *(char **)(dbobj + e->offset + LINK_INFO_OFFSET) = NULL;
       }
    } /* endfor */

    if(classp->clxnp)
    {
       TRC("odm_change_obj","Fixing collection pointers","","","");

       for(e = classp->elem,i=0;i<nd;i++,e++)
       {
           if(e->type == ODM_VCHAR)
               *(char **)(dbobj + e->offset) = e->holder;
        } /* endfor */
    } /* endif */

    ++changes;

    rc = raw_close_class ( classp, was_open );
    if ( rc == -1 )
    {
        TRC("odm_change_obj","Could not close class! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((int) -1);
    }

    /*if_err_ret_err(raw_close_class(classp,was_open),(int),0);*/
    TRC("odm_change_obj","Changed object. changes %d",changes,"","");

    if (reserved_type(classp))
        add_convert_to_char(classp);

    free(cobj_copy);
    STOP_ROUTINE;

    return(0);
}

/*
 *    NAME:     ODM_GET_BY_ID
 *    FUNCTION: gets a particular objects in a class by its id
 *    RETURNS:  A pointer to the object if successful, -1 otherwise.
 */
void *
odm_get_by_id(
struct Class *classp,   /* Pointer to the object class to get object from */
long id,                /* id of obj to be deleted */
void *p)                /* area of memory to put memory object */
{
    int rc;
    int i;                  /* scratch loop counter */
    int size;               /* size of the object */
    int was_open;
    char *pov;      /* variable to receive results of get */
    struct ClassElem *e;
    int temp_error;
    int malloced_space = FALSE;  /* So we know if we need to free p */
    struct Class *return_class;
    struct Class *NEWclassp;   


    START_ROUTINE(ODMHKWD_ODM_GET_BY_ID);


    TRC("odm_get_by_id","Getting by id %d",id,"","");
    if (verify_class_structure(classp) < 0)
    {
       TRC("odm_get_by_id","Invalid class structure!","","","");
       STOP_ROUTINE;
       return((void *) -1);
    } /* endif */

    if (id < 0)
    {
       TRC("odm_get_by_id","Invalid id!","","","");
       odmerrno = ODMI_PARAMS;
       STOP_ROUTINE;
       return((void *)-1);
    } /* endif */

    if (!classp->reserved)
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
           TRC("odm_get_by_id","could not open class %s",classp->classname,"","");
           STOP_ROUTINE;
           return(-1);
       }

       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 

       copyinfo(NEWclassp, classp);            /* Defect 100497 */

    }

    if (reserved_type(classp))
        convert_to_vchar(classp);

    was_open = classp->open;
    return_class = raw_addr_class ( classp );
    if ( (int) return_class == -1 )
    {
        TRC("odm_get_by_id","Could not addr class! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((void *) -1);
    }

    /*if_err_ret_err(raw_addr_class(classp),(char *),0);*/

    pov = raw_find_byid ( classp, id );

    if ( (int) pov  == -1 )
    {
        TRC("odm_get_by_id","Could not find by id! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        temp_error = odmerrno;
        raw_close_class(classp,was_open);
        odmerrno = temp_error;
        STOP_ROUTINE;
        return((void *) -1);
    } /* endif */

    /*if_err_ret_err(pov = raw_find_byid(classp,id),(char *),0);*/

    size = classp->structsize;
    TRC("odm_get_by_id","Structure size %d",size,"","");

    if(p == NULL)
    {
       p = (char *)malloc(size);
       if (  p == NULL )
       {
           TRC("odm_get_by_id","Could not malloc! size %d",size,"","");
           raw_close_class(classp,was_open);
           odmerrno = ODMI_MALLOC_ERR;
           STOP_ROUTINE;
           return((void *) -1);
       } /* endif */

       malloced_space = TRUE;
       /*if_err_ret_err(p,(char *),ODMI_MALLOC_ERR);*/
    } 

    bcopy(pov,p,size);

    if(classp->clxnp)
    {
       TRC("odm_get_by_id","Getting from collection","","","");

       for(e = classp->elem,i=0;i<classp->nelem;i++,e++)
       {
           if(e->type == ODM_VCHAR)
           {
              TRC("odm_get_by_id","Getting column %s",e->elemname,"","");
              rc = get_vchar ( classp, e, p );
              if ( rc == -1 )
              {
                  TRC("odm_get_by_id","Could not get vc! err %d",
                      odmerrno,"","");
                  if (malloced_space)
                     free((void *) p);

                  temp_error = odmerrno;
                  raw_close_class(classp,was_open);
                  odmerrno = temp_error;

                  STOP_ROUTINE;
                  return((void *) -1);
               }  /* endif */

               /*if_err_ret_err(get_vchar(classp,e,p),(char *),0);*/
            } /* endif */

       } /* endfor */
    } /* endif */

    rc = raw_close_class ( classp, was_open );
    if ( rc == -1 )
    {
        TRC("odm_get_by_id","Could not close class! err",odmerrno,"","");
        if (malloced_space)
            free((void *) p);
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((void *) -1);
    }
    /*if_err_ret_err(raw_close_class(classp,was_open),(char *),0);*/

    TRC("odm_get_by_id","Got object","","","");

    STOP_ROUTINE;

    if (reserved_type(classp))
        convert_to_char(classp,p);

    return((void *)p);
}

/*
 *    NAME:     ODM_GET_LIST
 *    FUNCTION: get the set of objects meeting a criterion into
 *              an array of C structures, returning results in an
 *              information structure.  Recurse for LINKS and VLINKS
 *              down to a specified depth.
 *   RETURNS:   A pointer to an array of objects matching the criteria
 *              if successful, -1 otherwise.
 */

#include <sys/stat.h>
void *
odm_get_list (
struct Class *classp,           /* CLASS_SYMBOL for the class               */
char *criteria,                 /* Criterion for the query                  */
struct listinfo *info,          /* Place to return summary of results       */
int max_expect,                 /* Prediction of number of obj meeting crit */
int depth)                      /* number of levels down to recurse         */

{
   int rc;                      /* return code from subroutines             */
   int i;                       /* scratch variable                         */
   int nd;                      /* number of descriptors                    */
   int size;                    /* number bytes in the C struct for this    */
                                /* object class                             */
   char *pov;                   /* variable to receive results of odmget    */
   char crit[MAX_CRITELEM_LEN]; /* string to hold criterion for recursive   */
                                /* query                                    */
   int ntries;                  /* total number of times through the main   */
                                /* loop so far                              */
   int mex;                     /* the list has been expanded to be able to */
                                /* hold this number of objects              */
   int bufsiz;                  /* list memory has expanded to this size in */
                                /* bytes                                    */
   char *p;                     /* pointer to an output buffer where objects*/
                                /* will be returned if nchar field is used. */
                                /* pointer to a working buffer if nchar is  */
                                /* used                                     */
   char *pi, *pend;             /* pointer index, final pointer             */
   char *d;                     /* scratch pointer                          */ 
   char *q;                     /* pointer to an output buffer if nchar     */
                                /* is used.                                 */
   char *qi;                    /* pointer index                            */
   struct listinfo *temp_info;

   struct ClassElem *e;         /* pointer to area in ODM struct where clc  */
                                /* saved info about how to represent this   */
                                /* particular descriptor in a memory object */
   int was_open;                /* indicate class was open                  */
   int link_was_open;
   int class_is_open;           /* Indicates if ODM has the class open      */
   int returncode;
   int temp_error;
   struct Class *return_class;
   long id;
   struct Class *NEWclassp;     /* class returned by ODM_MOUNT_CLASS routine*/ 

   START_ROUTINE(ODMHKWD_ODM_GET_LIST);

   TRC("odm_get_list","Getting list","","","");

   if (verify_class_structure(classp) < 0)
   {
      TRC("odm_get_list","Invalid class structure!","","","");
      STOP_ROUTINE;
      return((void *) -1);
   } /* endif */

   TRC("odm_get_list","Max expect is %d",max_expect,
       "Depth is %d",depth);

   if (info == NULL || max_expect <= 0)
   {
      TRC("odm_get_list","Invalid params!","","","");
      odmerrno = ODMI_PARAMS;
      return((void *) -1);
   } /* endif */

   info->valid = FALSE; /* So that if this routine fails */
   info->classname[0] = '\0';
   info->crit[0] = '\0';
   info->class = NULL;
   info->num = 0;

   if (!classp->reserved)
   {
      if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
          NEWclassp == (CLASS_SYMBOL) -1  )
      {
         TRC("odm_get_list","could not open class %s",classp->classname,"","");
         STOP_ROUTINE;
         return(-1);
      }

      /* Defect 100497: classp->reserved is used to indicate the object
         class has already been mounted, classp->reserved is also used
         to indicate the object class has nchar type. classp->reserved
         and other data is set in odm_mount_class, the data needs to 
         be copied to the classp, so that the content can be returned to 
         the calling routine. */ 

      copyinfo(NEWclassp, classp);           /* Defect 100497 */
   }

   if (reserved_type(classp))
      convert_to_vchar(classp);

   was_open = classp->open;

   return_class = raw_addr_class ( classp );
   if ( (int) return_class == -1 )
   {
      TRC("odm_get_list","Could not addr class! err %d",odmerrno,"","");
      if (reserved_type(classp))
         convert_to_char(classp,NULL);   
      STOP_ROUTINE;
      return((void *) -1 );
   }

   class_is_open = TRUE;

   nd = classp->nelem;
   size = classp->structsize; 
   info->num = 0;
   mex = 0;
   ntries = 0;
   p = pi = NULL;

   TRC("odm_get_list","Number of elements %d",nd,"Structure size %d",size);
   TRC("odm_get_list","Criteria %s",criteria,"","");

   while(1)
   {
      if (ntries++ == 0)
        pov = raw_find_obj(classp,criteria,TRUE);
      else
        pov = raw_find_obj(classp,criteria,FALSE);

      if (!pov)                          /* no object found */
        break;

      if ( (int) pov == -1 )
      {
        TRC("odm_get_list","Could not find obj! err %d",odmerrno,"","");
        goto ERROR_CLEANUP_FOR_odm_get_list;
      }

      /* if we got one ( or,later, more than we allocated
         room for) allocate (realloc) memory in increments of
         the expected number of objects. This way no malloc
         is done when no object meets the crit, and a NULL
         can be returned */

      if (!(info->num % max_expect))
      {
         mex += max_expect;
         bufsiz =mex * size;

         TRC("odm_get_list","Allocating size %d",bufsiz,
             "Number of structs %d",mex);

         if (!info->num)
         {                          /* if num of object found is 0    */
            p =  (char *)malloc(bufsiz); 

            /* If object class contains NCHAR types then malloc space */
	    if (reserved_type(classp))
               q =  (char *)malloc(classp->reserved*max_expect);
            info->valid = TRUE;
         }
         else
         {
            p =  realloc(p,bufsiz);
	    /* If object class contains NCHAR types then malloc space */
	    if (reserved_type(classp))
               q =  realloc(q,classp->reserved*mex);
         }

         if (( p == NULL ))  
         {
            TRC("odm_get_list","Alloc failed! err %d",errno,"","");
            /*--------------------------------------------------*/
            /* Exit directly from here.  Do not go to the error */
            /* cleanup since we cannot free the list.           */
            /*--------------------------------------------------*/

            odmerrno = ODMI_MALLOC_ERR;
            raw_close_class(classp,was_open);
            odmerrno = ODMI_MALLOC_ERR;
            STOP_ROUTINE;
            return((void *) -1);
         }

	 if ((reserved_type(classp)) && (q == NULL))
         {
            TRC("odm_get_list","Alloc failed! err %d",errno,"","");
            /*--------------------------------------------------*/
            /* Exit directly from here.  Do not go to the error */
            /* cleanup since we cannot free the list.           */
            /*--------------------------------------------------*/

            odmerrno = ODMI_MALLOC_ERR;
            raw_close_class(classp,was_open);
            odmerrno = ODMI_MALLOC_ERR;
            STOP_ROUTINE;
            return((void *) -1);
         }

         pi = p + (info->num)*size;
      } /* endif */

      info->num++;
      TRC("odm_get_list","Number of elements so far %d",info->num,"","");

      bcopy(pov,pi,size);
      pi += size;

   } /* end while */

   pend = pi;
   if (classp->clxnp)
   {
      TRC("odm_get_list","Getting from collection","","","");

      for(e = classp->elem,i=0;i<nd;i++,e++)
      {
         if (e->type == ODM_VCHAR)
         {
            TRC("odm_get_list","Getting vchar %s",e->elemname,"","");

            for (pi = p; pi <pend; pi += size)
            {
               rc = get_vchar ( classp, e, pi );
               if (rc == -1 )
               {
                  TRC("odm_get_list","Could not get vchar #%d",
                       (pi - p)/size , "","");
                  goto ERROR_CLEANUP_FOR_odm_get_list;
               } /* endif */

               /*if_err_ret_err(get_vchar(classp, e, pi),(char *),0);*/
            } 
         }  
      } 
   } 

   /* close now since we don't need addressability any more */
   returncode = raw_close_class(classp,was_open);
   if (returncode < 0)
   {
      TRC("odm_get_list","Could not close class! err %d",odmerrno,"","");
      goto ERROR_CLEANUP_FOR_odm_get_list;
   } /* endif */

   class_is_open = FALSE;

   /* After we have gotten all the objects at top level,
      if depth is > 1, we will think about recursing */

   TRC("odm_get_list","Got first level","","","");

   if (depth > 1)
   {
      TRC("odm_get_list","Going deeper. Depth %d",depth,"","");

      for (e = classp->elem,i=0;i<nd;i++,e++)
      {
         if (e->type == ODM_LINK)
         {
            TRC("odm_get_list","Expanding link %s",e->elemname,"","");

            if (e->link == 0)
            {
               odmerrno = ODMI_LINK_NOT_FOUND;
               goto ERROR_CLEANUP_FOR_odm_get_list;
            }
            /*if_null_ret_err(e->link, (char *),ODMI_LINK_NOT_FOUND);*/

            /* do all the links from this objs to an
               object class one after another to help
               with the virtual open performance */
            link_was_open = e->link->open;

            return_class = raw_addr_class ( e->link );
            if ( (int) return_class == -1 )
            {
               TRC("odm_get_list","Could not addr link! err %d",odmerrno,"","");
               goto ERROR_CLEANUP_FOR_odm_get_list;
            }

            /*if_err_ret_err(raw_addr_class(e->link),(char *),0);*/

            for (pi = p; pi <pend; pi += size)
            {

               /* d = address for pointer returned
                  by the recursive call to odm_get_list */
               d = pi + e->offset;

               /* value spec in criterion is different depending 
                  whether linked to column is numerical or string */

               /* malloc info struct*/
               temp_info = (struct listinfo *)
                            malloc(sizeof (struct listinfo));
               if (temp_info == NULL)
               {
                  TRC("odm_get_list","Could not malloc for link!","","","");
                  raw_close_class(e->link,link_was_open);
                  odmerrno = ODMI_MALLOC_ERR;
                  goto ERROR_CLEANUP_FOR_odm_get_list;
               }

               /*if_null_ret_err(m,(char *),ODMI_MALLOC_ERR);*/

               /* recurse */
               memcpy((void *) temp_info,(void *) '\0',
                      (size_t) sizeof(struct listinfo));

               *(struct listinfo **)(d + LINK_INFO_OFFSET) = temp_info;

               /* if link has no value don't follow it */

               TRC("odm_get_list","Link value is %s",d + LINK_VAL_OFFSET,"","");

               if (!strlen(d + LINK_VAL_OFFSET))
                  continue;

               if (e->linktype == ODM_SHORT ||
                   e->linktype == ODM_LONG ||
                   e->linktype == ODM_ULONG ||
                   e->linktype == ODM_DOUBLE)
                  sprintf(crit,"%s = %s",e->col,d+LINK_VAL_OFFSET);

               else
                  sprintf(crit,"%s = '%s'",
                               e->col,d+LINK_VAL_OFFSET);

               TRC("odm_get_list","Getting link with crit %s",crit,"","");

               *(void **)d = odm_get_list(e->link, crit,
                                          temp_info,1,depth-1);

               if ( (int) d == -1 )
               {
                  TRC("odm_get_list","Could not expand link! err %d",
                       odmerrno,"","");

                  temp_error = odmerrno;
                  raw_close_class(e->link,link_was_open);
                  odmerrno = temp_error;

                  goto ERROR_CLEANUP_FOR_odm_get_list;
               }

               TRC("odm_get_list","Expanded link","","","");

               /*if_err_ret_err(*(char **)d, (char *),0);*/
               /* if it fails, bubble err up */
            } /* endfor */

            info->class = e->link;
            raw_close_class(e->link,link_was_open);
         } /* endif e->type == LINK */
      } /* end for */
   } /* endif depth > 1 */


   /*  fill in info struct as necessary */

   info->class = classp;
   info->valid = TRUE;
   strcpy(info->classname,classp->classname);
   strcpy(info->crit,criteria);

   /* successful - return the address of the structure list */
   TRC("odm_get_list","Get was successful. Num objs %d",info->num,
       "Object list at %x",p);

   if (reserved_type(classp))
   {
      qi = q;
      if (p==NULL)                       /* No object found */
      {                                  
          convert_to_char(classp,NULL);  /* Defect 110204   */  
          STOP_ROUTINE;
          return((void *)p);
      }

      for (pi = p; pi <pend; pi += size)
      {
         bcopy(pi,qi,size);
         convert_to_char_data(classp,qi); 
         id = *(long *) qi;	
         qi += classp->reserved;
      }

      convert_to_char_struct(classp);  
      odm_free_list(p,info);
      STOP_ROUTINE;
      return((void *)q);
   }

   STOP_ROUTINE;
   return((void *)p);

ERROR_CLEANUP_FOR_odm_get_list:

    TRC("odm_get_list","In error cleanup! err %d",odmerrno,"","");

    temp_error = odmerrno;

    if (reserved_type(classp))
       convert_to_char(classp,NULL);   

    /*--------------------------*/
    /* Free the space allocated */
    /*--------------------------*/
    odm_free_list(p,info);

    if (class_is_open)
        raw_close_class(classp,was_open);

    odmerrno = temp_error;
    TRC("odm_get_list","Exiting error cleanup! err %d",odmerrno,"","");

    STOP_ROUTINE;
    return((void *) -1);
} /* end of odm_get_list */

/*
 *    NAME:     ODM_GET_OBJ
 *    FUNCTION: get the data for an object from the odm into a C structure.
 *    RETURNS:  A pointer to an object if successful, -1 otherwise.
 */
int open_RDONLY = 0;
void *
odm_get_obj(
struct Class *classp,   /* class handle */
char *crit,             /* criterion string for the query */
void *p,                /* area of memory to put memory object */
int first)              /* boolean - true: get first obj, false: get next obj */
{
    int rc;
    int size,i;
    char *pov; /* variable to receive results of odmget */
    struct ClassElem *e;
    int was_open;
    int malloced_space = FALSE; /* Indicates if we have malloced for p */
    int temp_error;
    struct Class *return_class;
    struct Class *NEWclassp;   


    /* Make sure database is opened read only */
    /* Defect 62044 */
    open_RDONLY = 1;
    START_ROUTINE(ODMHKWD_ODM_GET_OBJ);
    TRC("odm_get_obj","Getting object","","","");

    if (verify_class_structure(classp) < 0)
    {
        TRC("odm_get_obj","Invalid class structure!","","","");
        STOP_ROUTINE;
        return((void *) -1);
    } /* endif */


    TRC("odm_get_obj","Criteria %s",crit,"First %d",first);

    if (!classp->reserved)
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
          TRC("odm_get_obj","could not open class %s",classp->classname,"","");
          STOP_ROUTINE;
          return(-1);
       }

       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 

       copyinfo(NEWclassp, classp);           /* Defect 100497 */

    }

    if (reserved_type(classp))
        convert_to_vchar(classp);

    was_open = classp->open;
    return_class = raw_addr_class ( classp );
    if ( (int) return_class == -1 )
    {
        TRC("odm_get_obj","Could not addr class! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((void *) -1);
    }
    /*if_err_ret_err( raw_addr_class(classp),(char *),0);*/
    pov = raw_find_obj(classp,crit,first);

    if(!pov)
    {
        TRC("odm_get_obj","No objects","","","");

        /* Defect 107645: If class has nchar type, we need to convert       */
        /* the class structure back to char type before we exit. Otherwize, */
        /* the structsize and some other data field will be incorrect; We   */
        /* saw this problem when "putattr" routine called odm_get_first,    */
        /* the classp returned by odm_get_first is changed when no object   */
        /* found. Since the classp didn't convert back to char for nchar    */
        /* field, some data populated by odm_add_obj will garage.           */  
        /* We have to convert data back to char whenever convert_to_vchar   */
        /* was called.                                                      */

        if (reserved_type(classp))
             convert_to_char(classp,NULL);   /* convert data back to char   */
        raw_close_class(classp,was_open);
        STOP_ROUTINE;
        return(0);
    }

    if ( (int) pov == -1 )
    {
        TRC("odm_get_obj","bad return from raw_find! err %d",
            odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        temp_error = odmerrno;
        raw_close_class(classp,was_open);
        odmerrno = temp_error;
        STOP_ROUTINE;
        return((void *) -1);
    }
    /*if_err_ret_err(pov,( char *),0);*/

    size = classp->structsize;
    if(!p)
    {
        TRC("odm_get_obj","Allocating space. size %d",size,"","");
        /* must allocate enough space for expanded structure size if
         the user does not allocate any */

        p = (char *)malloc(size>classp->reserved?size+1:classp->reserved+1);
        if (  p == NULL )
        {
           TRC("odm_get_obj","Malloc failed! err %d",errno,"","");

           raw_close_class(classp,was_open);
           odmerrno = ODMI_MALLOC_ERR;
           STOP_ROUTINE;
           return((void *) -1);
        }
        malloced_space = TRUE;
        /*if_err_ret_err(p,(char *),ODMI_MALLOC_ERR);*/
    }

    bcopy(pov,p,size);

    for(e = classp->elem,i=0;i<classp->nelem;i++,e++)
    {
       if(e->type == ODM_VCHAR)
       {
           TRC("odm_get_obj","Getting vchar %s",e->elemname,"","");

           rc = get_vchar ( classp, e, p );
           if ( rc == -1 )
           {
               TRC("odm_get_obj","Get vchar failed! err %d",odmerrno,"","");
               temp_error = odmerrno;

               if (reserved_type(classp))
                   convert_to_char(classp,NULL);   
               raw_close_class(classp,was_open);
               if (malloced_space)
                  free((void *) p);

               odmerrno = temp_error;
               STOP_ROUTINE;
               return((void *) -1);
           } /* endif */

           /*if_err_ret_err(get_vchar(classp,e,p),(char *),0);*/
       } /* endif */
    } /* endfor */


    rc = raw_close_class ( classp, was_open );
    if ( rc == -1 )
    {
        TRC("odm_get_obj","Could not close class! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   

        if (malloced_space)
           free((void *) p);

        STOP_ROUTINE;
        return((void *) -1);
    }
    /*if_err_ret_err(raw_close_class(classp,was_open),(char *),0);*/

    TRC("odm_get_obj","Successful get","","","");

    if (reserved_type(classp))
        convert_to_char(classp,p);

    STOP_ROUTINE;
    return((void *) p);
}

/*
 *   NAME:     ODM_GET_FIRST
 *   FUNCTION: Retrieve the first object which matches a criteria.
 *   RETURNS:  A pointer to the object if successful, -1 otherwise.
 *
 */
void *
odm_get_first(
struct Class *classp,   /* class symbol */
char *crit,             /* criterion string for the query */
void *p)                /* area of memory to put memory object */
{
    char *rc;

    START_ROUTINE(ODMHKWD_ODM_GET_FIRST);

    TRC("odm_get_first","Getting first object","","","");

    rc = (char *)  odm_get_obj(classp,crit,p,TRUE);

    TRC("odm_get_first","Get obj returned %x",rc,"odmerrno ",odmerrno);
    STOP_ROUTINE;
    return((void *)rc);

}
/*
 *   NAME:     ODM_GET_NEXT
 *   FUNCTION: Retrieve the next object which matched a criteria.
 *   RETURNS:  A pointer to the object if successful, -1 otherwise.
 *
 */
void *
odm_get_next(
struct Class *classp,   /* class symbol */
void *p)                /* area of memory to put memory object */
{
    char *rc;

    START_ROUTINE(ODMHKWD_ODM_GET_NEXT);
    TRC("odm_get_next","Getting next object","","","");

    rc = (char *)odm_get_obj(classp,"",p,FALSE);

    TRC("odm_get_next","Get obj returned %x",rc,"odmerrno ",odmerrno);
    STOP_ROUTINE;
    return((void *) rc);
}

/*
 *    NAME:     ODM_ADD_OBJ
 *    FUNCTION: Add an object to a class
 *              given the class symbol and the memory object.
 *              Set the id in the memory object to the id in the database.
 *    RETURNS:  The identifier number (id) of the object added if successful,
 *              -1 otherwise.
 */
int odm_add_obj(classp,cobj)
struct Class *classp;
void *cobj;
{
    int rc;
    int object_id;
    int was_open;
    int temp_error;
    struct Class *return_class;
    char *cobj_copy;
    struct Class *NEWclassp;


    START_ROUTINE(ODMHKWD_ODM_ADD_OBJ);
    TRC("odm_add_obj","Adding object","","","");

    if (verify_class_structure(classp) < 0)
    {
        TRC("odm_add_obj","Invalid class structure! ","","","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if ((cobj == NULL) || (cobj < 0))
    {
	TRC("odm_add_obj","Invalid object!","","","");
        odmerrno = ODMI_PARAMS;
        STOP_ROUTINE;
        return(-1);
    }  /* endif */

    cobj_copy = (char *)malloc(classp->structsize+1);
    bcopy(cobj,cobj_copy,classp->structsize);

    if (!classp->reserved)
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
           TRC("odm_add_obj","could not open class %s",classp->classname,"","");
           STOP_ROUTINE;
           return(-1);
       }

       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 

	copyinfo(NEWclassp, classp);             /* Defect 100497 */

    }

    if (reserved_type(classp))
    {
       if (add_convert_to_vchar(classp,cobj_copy) < 0)
       {
          TRC("odm_add_obj","add_convert_to_vchar failed! ","","","");
          STOP_ROUTINE;
          return(-1);
       } /* endif */
    }

    was_open = classp->open;
    return_class = raw_addr_class ( classp );
    if ( (int) return_class == -1 )
    {
        TRC("odm_add_obj","Could not addr class! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   
        STOP_ROUTINE;
        return((int) -1);
    }


    if (classp->open & OPENED_AS_READ_ONLY)
    {
       /*----------------------------------------------------------*/
       /* If the class has been opened as read only, it cannot be  */
       /* added to.                                                */
       /*----------------------------------------------------------*/
       TRC("odm_add_obj","Class is read only!","","","");
       if (reserved_type(classp))
          convert_to_char(classp,NULL);   
       raw_close_class(classp,was_open);
       odmerrno = ODMI_READ_ONLY;
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    object_id = raw_add_obj(classp,cobj_copy);
    if (object_id < 0)
    {
        TRC("odm_add_obj","Could not add obj! err %d",odmerrno,"","");
        if (reserved_type(classp))
           convert_to_char(classp,NULL);   

        temp_error = odmerrno;
        raw_close_class(classp,was_open);
        odmerrno = temp_error;

        STOP_ROUTINE;
        return(-1);
    } /* endif */

    TRC("odm_add_obj","Added object. Id %d",object_id,"","");

    rc = raw_close_class(classp,was_open);
    if (rc < 0)
    {
       TRC("odm_add_obj","Could not close class! err %d",odmerrno,"","");
       if (reserved_type(classp))
           convert_to_char(classp,NULL);   
       STOP_ROUTINE;
       return(-1);
    } /* endif */

    /*if_err_ret_err(object_id,,ODMI_ADD_ERR);*/

    *(long *) cobj = object_id;
    ++adds;

    TRC("odm_add_obj","Successful add. adds %d",adds,"","");

    if (reserved_type(classp))
        add_convert_to_char(classp);

    free(cobj_copy);
    STOP_ROUTINE;
    return(object_id);
}


/*
 *    NAME:     ODM_FREE_LIST
 *    FUNCTION: free the set of objects meeting a criterion into
 *              an array of C structures, recursing for LINKS and VLINKS
 *              if the info structs indicate sublists were allocated.
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */

odm_free_list(
void *cobj,               /* address of the memory object list */
struct listinfo *info) /* top level info structure */
{
    int i;          /* scratch integer */
    int nd;         /* number of descriptors */
    int rv;         /* return value from subroutines */
    int size;       /* size of each memory object, for stepping thru links */
    char *p,*pi,*pend;  /* pointer to list, index, final value */
    char *d;        /* to hold address of pointers to child lists */
    struct ClassElem *e;    /* pointer to area in ODM struct where clc saved info
                   about how to represent this particular descriptor
                       in a memory object */
    struct Class *classp;    /* class symbol */
    struct listinfo **inf;


    START_ROUTINE(ODMHKWD_ODM_FREE_LIST);
    TRC("odm_free_list","Freeing list %x",cobj,"","");

    if (cobj == NULL  || info == NULL )
    {
        TRC("odm_free_list","Null parameter(s)! cobj %x",cobj,
            "info %x",info);
        odmerrno = ODMI_PARAMS;
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if (verify_class_structure(info->class) < 0)
    {
        TRC("odm_free_list","Invalid class structure! err %d",odmerrno,
            "","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    /* if valid is false, there is no list */
    if(!info->valid)
    {
        TRC("odm_free_list","No list, returning","","","");
        STOP_ROUTINE;
        return(0);
    }

    classp = info->class;

    /* prepare pointers for stepping through the objects */
    p = cobj;
    size = classp->structsize;
    pend = p + info->num * size;

    nd = classp->nelem;
    TRC("odm_free_list","Number of  objects %d",info->num,"","");

    for(e = classp->elem,i=0;i<nd;i++,e++)
    {
       if(e->type == ODM_LINK)
       {
          TRC("odm_free_list","Freeing link info for %s",e->elemname,
              "","");

          for(pi = p; pi <pend; pi += size)
          {
             d = pi + e->offset;

             /* if pointer is null, nothing to free */
             inf = (struct listinfo **)(d + LINK_INFO_OFFSET);
             if( d != NULL && *(char **)d)
             {
                 rv = odm_free_list( *(char **)d, *inf);
                 if ( rv == -1 )
                 {
                     TRC("odm_free_list",
                         "Could not free link! err %d",odmerrno,"","");
                     STOP_ROUTINE;
                     return(-1);
                 }
                    /*if_err_ret_err(rv,(int),0);*/
             } /* endif */

             if( *inf)
               free((void *) *inf);
          } /* endfor */
      } /* end if */
      else if(e->type == ODM_VCHAR)
      {
          TRC("odm_free_list","Freeing vchar info %s",e->elemname,
              "","");
          for(pi = p; pi <pend; pi += size)
          {
              d = pi + e->offset;
              if( * (char **) d)
              {
                 free((void *)  * (char **) d);
                 * (char **) d = NULL;
              } /* endif */

          } /* endfor */
      } /* end else */

    } /* end for */

    TRC("odm_free_list","Freeing object list %x",cobj,"","");

    if(cobj)
        free((void *) cobj);

    TRC("odm_free_list","Successful free of list","","","");
    STOP_ROUTINE;
    return(0);
}

/*
 * NAME:     NOTE_CLASS
 * FUNCTION: Stores the CLASS_SYMBOL (classp) for an object class so that if
 *           an object class is mounted multiple times, the description will
 *           already be in memory.
 * RETURNS:  A 0 if successful, -1 otherwise.
 */
int note_class(classp)
struct Class *classp; /* Pointer to the CLASS_SYMBOL to save */
{
    int i;
    char full[MAX_ODM_PATH + MAX_ODMI_NAME + 1];
    struct stat statbuf;
    struct Class *NEWclassp; /* Pointer to the CLASS_SYMBOL from mount_class */

    START_ROUTINE(ODMHKWD_NOTE_CLASS);
    TRC("note_class","Noting class","","","");
    if (verify_class_structure(classp) < 0)
    {
        TRC("note_class","Invalid class! err %d",odmerrno,"","");
        STOP_ROUTINE;
        return(-1);
    } /* endif */

    if (!classp->reserved) 
    {
       if((NEWclassp=odm_mount_class(classp->classname))==NULL ||
           NEWclassp == (CLASS_SYMBOL) -1  )
       {
           TRC("note_class","could not open class %s",classp->classname,"","");
           STOP_ROUTINE;
           return(-1);
       }

       /* Defect 100497: classp->reserved is used to indicate the object
          class has already been mounted, classp->reserved is also used
          to indicate the object class has nchar type. classp->reserved
          and other data is set in odm_mount_class, the data needs to 
          be copy to the classp, so that the content can be returned to 
          the calling routine. */ 

       copyinfo(NEWclassp, classp);           /* Defect 100497 */
    }

    for(i=0;i<numClasses;i++)
      {
        if (Classlist[i] == NULL)
          {
            TRC("note_class","Available slot, i %d",i,"","");
            Classlist[i] = classp;

            if(repospath[0])
                sprintf(full,"%s/%s",repospath,classp->classname);
            else
                strcpy(full,classp->classname);

            if ( (stat(full,&statbuf)) == -1 )
                odm_searchpath(full,classp->classname);

            Namelist[i] = (char *)malloc(strlen(full)+1);

            if ( Namelist[i] == NULL ) {
                TRC("note_class","Malloc failed! size %d",strlen(full),"","");
                odmerrno = ODMI_MALLOC_ERR;
                STOP_ROUTINE;
                return(-1);
            }
            strcpy(Namelist[i],full);

            STOP_ROUTINE;
            return(0);
          } /* endif */

        if (0==strcmp(Classlist[i]->classname,classp->classname))
          {
            TRC("note_class","Found match. index %d",i,"","");
            STOP_ROUTINE;
            return(0);
          }

      }

    if(numClasses == MAX_CLASSES)
      {
        TRC("note_class","Too many classes!","","","");
        odmerrno = ODMI_TOOMANYCLASSES;
        STOP_ROUTINE;
        return(-1);
      }

    TRC("note_class","Setting class to index %d",numClasses,"","");

    if(repospath[0])
        sprintf(full,"%s/%s",repospath,classp->classname);
    else
        strcpy(full,classp->classname);

    if ( (stat(full,&statbuf)) == -1 )
        odm_searchpath(full,classp->classname);

    Namelist[numClasses] = (char *)malloc(strlen(full)+1);
    if ( Namelist[numClasses] == NULL ) {
        TRC("note_class","Malloc failed! size %d",strlen(full),"","");
        odmerrno = ODMI_MALLOC_ERR;
        STOP_ROUTINE;
        return(-1);
    }

    strcpy(Namelist[numClasses],full);

    Classlist[numClasses++] = classp;

    STOP_ROUTINE;
    return(0);
}

/*
 * NAME:     ODM_MOUNT_CLASS
 * FUNCTION: Takes the name of an object class and generates a
 *           CLASS_SYMBOL for the object class.
 * RETURNS:  The CLASS_SYMBOL for the class if successful, -1 otherwise.
 */
struct Class *
odm_mount_class(
char    *name)  /* Name of the object class */
{
    int     i,j;
    char    path[MAX_ODM_PATH + MAX_ODMI_NAME + 1];
    char    clxnname[MAX_ODMI_NAME + 3 + 1];
    char    full[MAX_ODM_PATH + MAX_ODMI_NAME + 1];
    int     fd;
    int     size;
    int     reloc;
    int     has_clxn;             /* boolean to indicate class has nchar 
                                     type, vchar type, or link type    */
 
    struct  ClassFileHdr  filehdr;
    struct  Class   *Classp;           /* class from disk */
    char *cp;       /* scratch character pointer */
    char *rv;       /* return val from subroutine - ptr */
    int offset;
    struct stat statbuf;


    START_ROUTINE(ODMHKWD_ODM_MOUNT_CLASS);
    TRC("odm_mount_class","Mouting class","","","");
    if (name == NULL || name[0] == '\0')
    {
       TRC("odm_mount_class","Null classname! ","","","");
       odmerrno = ODMI_PARAMS;
       STOP_ROUTINE;
       return((struct Class *) -1);
    } /* endif */

    TRC("odm_mount_class","Mounting class %s",name,"","");

    if(repospath[0])
        sprintf(full,"%s/%s",repospath,name);
    else
        strcpy(full,name);

    if ( (stat(full,&statbuf)) == -1 )
       odm_searchpath(full,name);

    for(i=0;i<numClasses && Classlist[i];i++)
    {
       if (0==strcmp(Namelist[i],full))      /* Defect 91574 */
       {
          TRC("odm_mount_class","Found match in list. index %d",i,
              "returning %x",Classlist[i]);
          STOP_ROUTINE;
          return(Classlist[i]);
       }
    } /* endfor */

    TRC("odm_mount_class","Could not find class in list, index %d",i,"","");
    if (i>=MAX_CLASSES) /* Defect 48906 */
    {
       TRC("odm_mount_classes","Too many classes!","","","");

       odmerrno = ODMI_TOOMANYCLASSES;
       return((struct Class *) -1);
    } 

    if(repospath[0])
        sprintf(path,"%s/%s",repospath,name);
    else
        strcpy(path,name);

    if ( (stat(path,&statbuf)) == -1 )
        odm_searchpath(path,name);

    TRC("odm_mount_class","Mounting class %s",path,"","");

    fd = open(path,O_RDONLY);
    if(fd == -1)
    {
       if (errno == ENOENT)
          odmerrno = ODMI_CLASS_DNE;
       else if (errno == EACCES)
          odmerrno = ODMI_CLASS_PERMS;
       else
          odmerrno = ODMI_OPEN_ERR;

       TRC("odm_mount_class","Could not open class. err %d",
           odmerrno,"errno %d",errno);

       STOP_ROUTINE;
       return((struct Class *)-1);
    }


    TRC("odm_mount_class","Reading file header","","","");

    if (sizeof(filehdr) != read(fd,(char *) &filehdr,sizeof(filehdr)) )
    {
       TRC("odm_mount_class","Could not read hdr!","","","");
       close(fd);
       odmerrno = ODMI_INVALID_CLASS;
       STOP_ROUTINE;
       return((struct Class *) -1);
       /*ret_err((struct Class *),ODMI_INVALID_CLASS);*/
    }
	
    if(filehdr.Hdr.magic != ODMI_MAGIC)
    {
       TRC("odm_mount_class","Invalid hdr magic number!","","","");
       close(fd);
       odmerrno = ODMI_MAGICNO_ERR;
       STOP_ROUTINE;
       return((struct Class *) -1);
       /*ret_err((struct Class *),ODMI_MAGICNO_ERR);*/
    }

    if(filehdr.Class.begin_magic != ODMI_MAGIC
       || filehdr.Class.end_magic != -ODMI_MAGIC)
    {
       TRC("odm_mount_class","Invalid class magic number!","","","");
       close(fd);
       odmerrno = ODMI_MAGICNO_ERR;
       STOP_ROUTINE;
       return((struct Class *) -1);
       /*ret_err((struct Class *),ODMI_MAGICNO_ERR);*/
    }

    size = (int)filehdr.Class.data-sizeof(struct ClassHdr);

    TRC("odm_mount_class","Class size %d",size,"","");
    Classp = (struct Class *) malloc(size);
    if (Classp == NULL)
    {
       TRC("odm_mount_class","Could not malloc Classp!, err %d",errno,
           "","");
       close(fd);
       odmerrno = ODMI_MALLOC_ERR;
       STOP_ROUTINE;
       return((struct Class *) -1);
    } /* endif */

    lseek(fd,(off_t) sizeof(struct ClassHdr),SEEK_SET);

    TRC("odm_mount_class","Reading class description","","","");

    if (size != read(fd,(char *) Classp,(unsigned) size))
    {
       TRC("odm_mount_class","Could not read class! err %d",errno,"","");
       close(fd);
       free((void *) Classp);
       odmerrno = ODMI_INVALID_CLASS;
       STOP_ROUTINE;
       return((struct Class *) -1);
       /*ret_err((struct Class *),ODMI_INVALID_CLASS);*/
    }
    close(fd);

    reloc = (int)Classp - sizeof( struct ClassHdr);
    Classp->classname += reloc;

    TRC("odm_mount_class","Relocating string pointers","","","");

    if (strcmp(Classp->classname,name))
    {
       TRC("odm_mount_class","Class names don't match! %s",Classp->classname,
           "name %s",name);

       odmerrno = ODMI_BAD_CLASSNAME;
       free((void *) Classp);
       STOP_ROUTINE;
       return((struct Class *) -1);
       /*ret_err((struct Class *),ODMI_BAD_CLASSNAME);*/
    }

    Classp->elem = (struct ClassElem *)((char *)(Classp->elem)+reloc);
    has_clxn = FALSE;
    for(j=0;j<Classp->nelem;j++)
    {
        (Classp->elem)[j].elemname += reloc;
        if((Classp->elem)[j].type == ODM_VCHAR)
        {
           TRC("odm_mount_class","Collection exists","","","");
           has_clxn++;
        }
	else if((Classp->elem)[j].reserved)
        {
           has_clxn++;
        }
        else if((Classp->elem)[j].type == ODM_LINK)
        {
           TRC("odm_mount_class","Class has link","","","");

           (Classp->elem)[j].col += reloc;
           /*copy name of link instead of struct ptr*/
           cp = (char *)(Classp->elem)[j].link;
           cp += reloc;
           rv = (char *) odm_mount_class(cp);
           if((int)rv != -1)
              (Classp->elem)[j].link = (struct Class *)rv;
           else
              (Classp->elem)[j].link = NULL;
        }
    } /* endfor */

    if( has_clxn)
    {
       clxnname[0] = '\0';

       strncat(clxnname,Classp->classname,MAX_ODMI_NAME);
       strcat(clxnname,".vc");
       TRC("odm_mount_class","Mounting collection %s",clxnname,"","");

       Classp->clxnp = (struct StringClxn *) mount_clxn (clxnname);
       if ( (int) Classp->clxnp == -1 )
       {
          TRC("odm_mount_class","Could not mount collection! err %d",
               odmerrno,"","");
          free((void *) Classp);
          STOP_ROUTINE;
          return((struct Class *) -1);
       }

       /*if_err_ret_err(
                  (Classp->clxnp = (struct StringClxn *)
                                mount_clxn(clxnname)),
                        (struct Class *),0);*/
    }

    TRC("odm_mount_class","Storing classp in list. index %d",numClasses,"","");

    Namelist[numClasses] = (char *)malloc(strlen(full)+1);
    if ( Namelist[numClasses] == NULL )
    {
        TRC("odm_mount_class","Malloc failed! size %d",strlen(full),"","");
        odmerrno = ODMI_MALLOC_ERR;
        STOP_ROUTINE;
        return((struct Class *) -1);
    }

    strcpy(Namelist[numClasses], full);

    Classlist[numClasses++] = Classp;

    STOP_ROUTINE;


/* If Class structure contains a nchar type then modify the class structure 
   to make it the nchar type look like a char type even though it is 
   actually stored as a vchar type */
    if (reserved_type(Classp))
    {
        offset = (Classp->elem)[0].offset;

        for (i=0;i<Classp->nelem;i++)
        {
           if ((Classp->elem)[i].reserved)
           {
              (Classp->elem)[i].type = ODM_CHAR;
           }

           /* all offsets appear to change */
           (Classp->elem)[i].offset = offset;
           if ((Classp->elem)[i].type == ODM_SHORT)
           {
	       offset = (offset + 1) & ~1;
               offset += 2;
           }

           else if ((Classp->elem)[i].type == ODM_DOUBLE)
               offset += sizeof(double);

           else if ((Classp->elem)[i].type == ODM_LINK)
               offset += (Classp->elem)[i].size + 8;

           else if (((Classp->elem)[i].type == ODM_CHAR) ||
                    ((Classp->elem)[i].type == ODM_METHOD))
               offset += (Classp->elem)[i].size;

           else if ((Classp->elem)[i].type == ODM_LONG)
           {
	       offset = (offset + 3) & ~3;
               offset += 4;
           }
 
           else if ((Classp->elem)[i].type == ODM_ULONG)
	   {
	       offset = (offset + 3) & ~3;
               offset += 4;
           }

           else
               offset += 4;
        }     
        offset = (offset + 3) & ~3;  
        Classp->structsize = offset;
        Classp->reserved = offset;
    }
    else
        Classp->reserved = 1;

    return(Classp);

} /* end of odm_mount_class */


/*      NAME            GET_OFFSETS
 *      FUNCTION        get offsets for each of the elements in the C structure,
 *                      each of which corresponds to a descriptor in the object
 *                      class we are preparing to define.  We also put the size
 *                      of the complete structure in each elem struct, since
 *                      the elem struct (to be copied to the user area) is the
 *                      only place clc has to record it.
 *      NOTES           All V3 machines use the same compiler.  The element
 *                      assignment rule is that you round off to the boundary
 *                      required by the current type.  This sometimes results
 *                      in pad bytes being generated.
 * RETURNS:             A 0 if successful, -1 otherwise.
 */
int get_offsets(classp)
struct Class *classp;   /* Pointer to the object class' CLASS_SYMBOL */
{
    int i,current,size,elems;
    struct ClassElem *elem;

    START_ROUTINE(ODMHKWD_GET_OFFSETS);
    TRC("get_offsets","Setting offsets for class","","","");

    if (verify_class_structure(classp) < 0)
      {
        TRC("get_offsets","Invalid class structure","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    elem = classp->elem;
    elems = classp->nelem;

    TRC("get_offsets","Number of elems %d",elems,"","");

    current = 3 * sizeof(long);
    /* first three longwords reserved for id and scratch */

    for(i=0;i<elems;i++)
      {
        switch(elem[i].type)
          {
        case ODM_VCHAR:
        case ODM_LONG:
            /* rounding the address up to the next
                                          longword boundary by adding three and
                                           masking off the last two bits */
            current = (current + 3) & ~3;
            elem[i].offset = current;
            current += 4;
            break;
        case ODM_ULONG:
            /* rounding the address up to the next
                                          longword boundary by adding three and
                                           masking off the last two bits */
            current = (current + 3) & ~3;
            elem[i].offset = current;
            current += 4;
            break;
        case ODM_DOUBLE:
            /* rounding the address up to the next
                                          longword boundary by adding three and
                                          masking off the last two bits */
            current = (current + 3) & ~3;
            elem[i].offset = current;
            current += sizeof(double);
            break;
        case ODM_SHORT:
            /* rounding the address up to the next
                                          short word boundary by adding one and
                                           masking off the last bit */
            current = (current + 1) & ~1;
            elem[i].offset = current;
            current += 2;
            break;
        case ODM_LINK:
            /* links begin with pointers, so we are
                                           rounding the address up to the next
                                          longword boundary by adding three and
                                           masking off the last two bits */
            current = (current + 3) & ~3;
            elem[i].offset = current;
            current += elem[i].size + 8; /* was 544 */
            break;
        case ODM_METHOD:
            /* method is a char array, no rounding */
            elem[i].offset = current;
            current += elem[i].size;
            break;
        case ODM_LONGCHAR:
        case ODM_BINARY:
        case ODM_CHAR:
            /* char arrays, no rounding */
            elem[i].offset = current;
            current += elem[i].size;
            break;
        default:
            TRC("get_offsets","Invalid type! %d",elem[i].type,
                "name %s",elem[i].elemname);

            odmerrno = ODMI_INTERNAL_ERR;
            STOP_ROUTINE;
            return((int) -1);
            /*ret_err((int),ODMI_INTERNAL_ERR);*/
          } /* endswitch */

        TRC("get_offsets","Set %s",elem[i].elemname,
            "to offset %x",elem[i].size);
      } /* endfor */

    size = (current + 3) & ~3;
    classp->structsize = size;

    TRC("get_offsets","Struct size %d",size,"","");
    STOP_ROUTINE;
    return(0);
}


/*
 *   NAME:     S_COPY
 *   FUNCTION: Copies string_ptr (len bytes) into string pp at offset
 *             
 *
 *   RETURNS: Modified string pp
 */

s_copy(string_ptr,pp,len,offset)
char *string_ptr;
char *pp;
int len;
int offset;
{
char *tmp;

tmp = pp;
tmp = tmp + offset;

bcopy(string_ptr,tmp,len);

}
/*
 *   NAME:     D_COPY
 *   FUNCTION: Routine copies len bytes into pp at offsetpp from p at ofsetp
 *             
 *
 *   RETURNS: Modified string pp
 */

d_copy(pp,p,offsetp,offsetpp,len)
char *pp;
char *p;
int offsetp;
int offsetpp;
int len;
{
int i;
char *tmp_p;
char *tmp_pp;
tmp_p = p + offsetp;
tmp_pp = pp + offsetpp;


bcopy(tmp_p,tmp_pp,len);
}

/*
 *   NAME:     RESERVED_TYPE
 *   FUNCTION: Routine deternimes if type is a new type
 *             
 *
 *   RETURNS: True/False
 */
reserved_type(classp)
struct Class *classp;   
{
int i;
struct Class *Classp;   

    
for (i=0;i<classp->nelem;i++)
	{ 
	if ((classp->elem)[i].reserved)
		{
		return(1);
		}
	}

return(0);
}

/*
 *   NAME:     CONVERT_TO_VCHAR
 *   FUNCTION: Routine converts class structure offsets from char format to 
 *	       vchar format.  
 *
 *
 *   RETURNS: classp
 */

convert_to_vchar(classp)
struct Class *classp;   /* class handle */
{
int offset;
int i;

offset = (classp->elem)[0].offset;

for (i=0;i<classp->nelem;i++)
    { 
    if ((classp->elem)[i].reserved)
	{
        (classp->elem)[i].type = ODM_VCHAR;
        (classp->elem)[i].offset = offset;
	offset += 4;
	}
	
    else if (((classp->elem)[i].type == ODM_CHAR) || ((classp->elem)[i].type == ODM_METHOD))
	{
        (classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size;
	}
    else if ((classp->elem)[i].type == ODM_LINK)
	{
        (classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size + 8;
	}
		
    else if ((classp->elem)[i].type == ODM_SHORT)
	{
       	(classp->elem)[i].offset = offset;
	offset = (offset + 1) & ~1;
	offset += 2;
	}
    else if ((classp->elem)[i].type == ODM_VCHAR)
	{
        (classp->elem)[i].offset = offset;
	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_LONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_ULONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

	offset += 4;
	}
   else if ((classp->elem)[i].type == ODM_DOUBLE)
	{
       	(classp->elem)[i].offset = offset;
	offset += sizeof(double);
	}
    }
offset = (offset + 3) & ~3;

classp->structsize = offset;
return(0);
}
	
/*
 *   NAME:     CONVERT_TO_CHAR
 *   FUNCTION: Routine converts class structure offsets and data from vchar 
 *             format to char format. 
 *
 *
 *   RETURNS: classp, p
 */


convert_to_char(classp,p)
struct Class *classp;   /* class handle */
void *p;                /* area of memory to put memory object */

{
if (p)      /* Defect 107645: if abnornal exit or no object found,     */
   convert_to_char_data(classp,p);   /* no need to convert data.       */
convert_to_char_struct(classp);
return(0);
}


/*
 *   NAME:     ADD_CONVERT_TO_VCHAR
 *   FUNCTION: Routine converts class structure offsets and data from char 
 *             format to vchar format. Routine is used in odm_add_obj
 *
 *
 *   RETURNS: classp,cobj
 */


add_convert_to_vchar(classp,cobj)
struct Class *classp;      /* class handle */
char *cobj;                /* area of memory to put memory object */
{
char *c_cobj;
int offset;
int i;
int fv;
char **vchar_location;          /* ptr to vchar location in the structure */
int descriptor_index;
char *descrip_value;
char *descriptor_offset;
char *first_err;
char *descrip_value_ptr;


c_cobj = (char *)malloc(classp->reserved+1);
bcopy(cobj,c_cobj,classp->reserved);

/* Adjust the offset field for each element of the object */
offset = (classp->elem)[0].offset;
for (i=0;i<classp->nelem;i++)
    {
    /* case the elem is a nchar type */
    if ((classp->elem)[i].reserved)
	{
        (classp->elem)[i].type = ODM_VCHAR;
        (classp->elem)[i].offset = offset;
	offset += 4;
	}
    /* case the elem is a vchar type */
    else if ((classp->elem)[i].type == ODM_VCHAR)
	{
        (classp->elem)[i].offset = offset;
	offset += 4;
	}
	
    else if (((classp->elem)[i].type == ODM_CHAR) || ((classp->elem)[i].type == ODM_METHOD))
	{
        (classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size;
	}
    else if ((classp->elem)[i].type == ODM_LINK)
	{
        (classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size + 8;
	}
		
    else if ((classp->elem)[i].type == ODM_SHORT)
	{
        (classp->elem)[i].offset = offset;
	offset = (offset + 1) & ~1;
	offset += 2; 
	}
    else if ((classp->elem)[i].type == ODM_LONG)
	{
       	(classp->elem)[i].offset = offset;
	offset = (offset + 3) & ~3;

	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_ULONG)
	{
       	(classp->elem)[i].offset = offset;
	offset = (offset + 3) & ~3;

	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_DOUBLE)
	{
       	(classp->elem)[i].offset = offset;
	offset += sizeof(double);
	}
    }
offset = (offset + 3) & ~3; 

classp->structsize = offset;
	

/* Now put data in in VCHAR format */
fv = (classp->elem)[0].offset;

for (descriptor_index = 0; descriptor_index < classp->nelem;
                                                        descriptor_index++)
    {
    if ((classp->elem)[descriptor_index].reserved)
	{
	descrip_value=(char *)(c_cobj+fv);
        descrip_value_ptr = (char *)malloc(strlen(descrip_value)+1); /* Defect 98312 */
        strcpy(descrip_value_ptr, descrip_value);

	fv+= (classp->elem)[descriptor_index].size ;
	descriptor_offset = cobj + classp->elem[descriptor_index].offset;
	/*------------------------------------------------*/
        /* Since the vchars are not put directly into the */
        /* structure, we need to save the string in the   */
        /* buffer pointed to by descriptor_offset.        */
        /*------------------------------------------------*/

        vchar_location =  (char **) descriptor_offset;
        *vchar_location = descrip_value_ptr;

	}
    else if  ((classp->elem)[descriptor_index].type == ODM_SHORT)
	{
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;
        *(short *)descriptor_offset = *(short *)(fv+ c_cobj);
	fv = (fv + 1) & ~1;
	fv+= 2;

	}

    else if  (((classp->elem)[descriptor_index].type == ODM_CHAR) || ((classp->elem)[descriptor_index].type == ODM_METHOD))
	{
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;
	descrip_value = (char *)malloc(strlen(fv + c_cobj)+1);
        sprintf(descrip_value,"%s",(fv + c_cobj));
        *descriptor_offset = '\0';
        strncat(descriptor_offset,descrip_value,
        (classp->elem)[descriptor_index].size - 1);
	fv+= (classp->elem)[descriptor_index].size ;
	free (descrip_value);
	}
    else if  ((classp->elem)[descriptor_index].type == ODM_LINK)
	{ 
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;

	descriptor_offset = descriptor_offset + 2 * sizeof (char *);
	descrip_value = (char *)malloc((classp->elem)[descriptor_index].size+1);
	sprintf(descrip_value,"%s",(char *)(fv + c_cobj + 2*sizeof(char *)));
        *descriptor_offset = '\0';
        strncat(descriptor_offset,descrip_value,
        (classp->elem)[descriptor_index].size - 1);
	fv+= (classp->elem)[descriptor_index].size + 8 ;
	free (descrip_value);
	}
    else if  ((classp->elem)[descriptor_index].type == ODM_LONG)
	{
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;
        *(long *)descriptor_offset = *(long *)(fv+ c_cobj);

        fv = (fv + 3) & ~3;

	fv+= (classp->elem)[descriptor_index].size ;

	}
    else if  ((classp->elem)[descriptor_index].type == ODM_ULONG)
	{
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;
        *(unsigned long *)descriptor_offset = *(unsigned long *)(fv+ c_cobj);

        fv = (fv + 3) & ~3;

	fv+= (classp->elem)[descriptor_index].size ;

	}
    else if  ((classp->elem)[descriptor_index].type == ODM_DOUBLE)
	{
        descriptor_offset = cobj + classp->elem[descriptor_index].offset;
        *(double *)descriptor_offset = *(double *)(fv+ c_cobj);

	fv+= sizeof(double);

	}
    else if  ((classp->elem)[descriptor_index].type == ODM_VCHAR)
	{

	descrip_value= *(char **)(c_cobj+fv);

        descrip_value_ptr = (char *)malloc(strlen(descrip_value)+1); /* Defect 98312 */
        strcpy(descrip_value_ptr, descrip_value);

	fv+=  4; 

	descriptor_offset = cobj +
        classp->elem[descriptor_index].offset;
	/*------------------------------------------------*/
        /* Since the vchars are not put directly into the */
        /* structure, we need to save the string in the   */
        /* buffer pointed to by descriptor_offset.        */
        /*------------------------------------------------*/

        vchar_location =  (char **) descriptor_offset;
	if (*vchar_location != NULL)
            {
            free(*vchar_location);
            *vchar_location = NULL;
            } 

	*vchar_location = (char *) malloc(strlen(descrip_value_ptr) + 1);
        if (*vchar_location == NULL)
            {
            TRC("add_convert_to_vchar","vchar malloc failed! %d",
                strlen(descrip_value) + 1,"","");

            odmerrno = ODMI_INTERNAL_ERR;
            STOP_ROUTINE;
            return((int) -1);
	    }
        strcpy(*vchar_location,descrip_value_ptr);
        free(descrip_value_ptr);

	}

    }

free(c_cobj);
return(0);
}

/*
 *   NAME:     ADD_CONVERT_TO_CHAR *   FUNCTION: Routine converts class structure offsets from vchar 
 *             format to char format. Routine is used in odm_add_obj
 *
 *
 *   RETURNS: classp
 */

add_convert_to_char(classp)
struct Class *classp;      /* class handle */
{
int offset; 
int i; 

offset = (classp->elem)[0].offset;

for (i=0;i<classp->nelem;i++)
    { 
    if (((classp->elem)[i].type == ODM_CHAR) || ((classp->elem)[i].type == ODM_METHOD))
	{
        (classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size;
	}
    else if ((classp->elem)[i].type == ODM_LINK)
	{
       	(classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size + 8;
	}
    else if ((classp->elem)[i].reserved)
	{
       	(classp->elem)[i].type = ODM_CHAR;
       	(classp->elem)[i].offset = offset;
	offset += (classp->elem)[i].size;
	}
    else if ((classp->elem)[i].type == ODM_VCHAR)
	{
       	(classp->elem)[i].offset = offset;
	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_SHORT)
	{
       	(classp->elem)[i].offset = offset;
	offset = (offset + 1) & ~1;
	offset += 2;
	}
    else if ((classp->elem)[i].type == ODM_LONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_ULONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

	offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_DOUBLE)
	{
       	(classp->elem)[i].offset = offset;
	offset += sizeof(double);
	}
    }
offset = (offset + 3) & ~3; 

classp->structsize = offset;
return(0);
}



convert_to_char_data(classp,p)
struct Class *classp;   /* class handle */
char *p;                /* area of memory to put memory object */

{
int offset;
char *pp;
int i;
char *string_ptr;
static int j=0;


pp = (char *)malloc(classp->reserved+1); /* expanded structure size */ 
offset = (classp->elem)[0].offset;

for (i=0;i<classp->nelem;i++)
    {
    if (((classp->elem)[i].type == ODM_CHAR) || ((classp->elem)[i].type == ODM_METHOD))
	{
	string_ptr=(char *)(p+(classp->elem)[i].offset );
	d_copy(pp,p,(classp->elem)[i].offset,offset,strlen(string_ptr)+1); 
        offset += (classp->elem)[i].size;

        }
    if ((classp->elem)[i].type == ODM_LINK)
	{
	string_ptr=(char *)(p+(classp->elem)[i].offset + 2*sizeof(char *));

	d_copy(pp,p,(classp->elem)[i].offset,offset,strlen(string_ptr)+8+1); 
        offset += (classp->elem)[i].size + 8;

        }
      else if ((classp->elem)[i].reserved)
	{
j++;

	string_ptr = *(char **)(p + (classp->elem)[i].offset);
	s_copy(string_ptr,pp,strlen(string_ptr)+1,offset);
        offset += (classp->elem)[i].size;
	free(string_ptr);
	} 
    else if ((classp->elem)[i].type == ODM_VCHAR)
	{
	string_ptr = *(char **)(p + (classp->elem)[i].offset);
	d_copy(pp,p,(classp->elem)[i].offset,offset,4); 
        offset += 4;
	} 


    else if ((classp->elem)[i].type == ODM_SHORT)
	{
	d_copy(pp,p,(classp->elem)[i].offset,offset,4); 
	offset = (offset + 1) & ~1;
        offset += 2;
	}
    else if ((classp->elem)[i].type == ODM_LONG)
	{
	d_copy(pp,p,(classp->elem)[i].offset,offset,4); 
        offset = (offset + 3) & ~3;

        offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_ULONG)
	{
	d_copy(pp,p,(classp->elem)[i].offset,offset,4); 
        offset = (offset + 3) & ~3;

        offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_DOUBLE)
	{

d_copy(pp,p,(classp->elem)[i].offset,offset,sizeof(double)); 
        offset += sizeof(double);
	}


    }
d_copy(pp,p,0,0,4);


bcopy(pp,p,classp->reserved);

free(pp); 
return(0);

}



convert_to_char_struct(classp)
struct Class *classp;   /* class handle */

{
int offset;
char *pp;
int i;
char *string_ptr;


offset = (classp->elem)[0].offset;

for (i=0;i<classp->nelem;i++)
    {
    if (((classp->elem)[i].type == ODM_CHAR) || ((classp->elem)[i].type == ODM_METHOD))
	{
        (classp->elem)[i].offset = offset;
        offset += (classp->elem)[i].size;

        }
    if ((classp->elem)[i].type == ODM_LINK)
	{
        (classp->elem)[i].offset = offset;
        offset += (classp->elem)[i].size + 8;

        }
    else if ((classp->elem)[i].reserved)
	{
	(classp->elem)[i].type = ODM_CHAR;
        (classp->elem)[i].offset = offset;
        offset += (classp->elem)[i].size;
	} 
    else if ((classp->elem)[i].type == ODM_VCHAR)
	{
        (classp->elem)[i].offset = offset;
        offset += 4;
	} 


    else if ((classp->elem)[i].type == ODM_SHORT)
	{
       	(classp->elem)[i].offset = offset;
	offset = (offset + 1) & ~1;
        offset += 2;
	}
    else if ((classp->elem)[i].type == ODM_LONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

        offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_ULONG)
	{
       	(classp->elem)[i].offset = offset;
        offset = (offset + 3) & ~3;

        offset += 4;
	}
    else if ((classp->elem)[i].type == ODM_DOUBLE)
	{
       	(classp->elem)[i].offset = offset;
        offset += sizeof(double);
	}


    }
offset = (offset + 3) & ~3; 
classp->structsize = offset;
return(0);

}
/*      NAME            ODM_SEARCHPATH
 *      FUNCTION        searchs the environmental variable ODMPATH for
 *                      a directory containing the class indicated by
 *                      the variable name.  Returns the full path to
 *                      the class (if it is found) with the variable
 *                      path.
 *      NOTES           odm code should look for class directories in
 *                      the following order: $ODMIR, the current directory,
 *                      $ODMPATH.
 *      RETURNS         void
 */
void odm_searchpath(path,name)
char *path, *name;
{
        struct stat statbuf;
        char *dirpath;
        char newpath[MAX_ODM_PATH + MAX_ODMI_NAME + 1];
        int dirp, newp, found;

        dirpath = getenv("ODMPATH");
        for (dirp=0,newp=0,found=0; found==0 && dirp!=strlen(dirpath)+1; dirp++)
        {
                if ( dirpath[dirp] == '\0' || dirpath[dirp] == ':' )
                {
                        newpath[newp]='/';
                        newpath[++newp]='\0';
                        strcat(newpath,name);
                        if ( (stat(newpath,&statbuf)) != -1 )
                        {
                                strcpy(path,newpath);
                                found=1;
                        }
                        else
                                newp=0;
                }
                else
                        newpath[newp++]=dirpath[dirp];
        }
} /* end of odm_searchpath */

/*      NAME            COPYINFO
 *      FUNCTION        copy NEWclassp to classp.
*/

copyinfo(NEWclassp, classp)
struct Class *NEWclassp;  /* source */
struct Class *classp;  /* destination */
{
   int i;

   classp->reserved = NEWclassp->reserved;

   for (i=0;i<classp->nelem;i++)
      (classp->elem)[i].reserved = (NEWclassp->elem)[i].reserved;


   /* NEWclassp->clxnp is not NULL if there is a vchar or nchar 
      data type in the object class.                                 */

   if (NEWclassp->clxnp != NULL)                  /* Defect 122460)  */
   {
      if (classp->clxnp) 
        free (classp->clxnp);

      classp->clxnp = (struct StringClxn *)malloc (sizeof(struct StringClxn)+1);
      bcopy(NEWclassp->clxnp, classp->clxnp, sizeof(struct StringClxn));
   } 

}
