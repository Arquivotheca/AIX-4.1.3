static char sccsid[] = "@(#)96        1.15  src/bos/usr/ccs/lib/libodm/odmvchar.c, libodm, bos411, 9428A410j 10/1/93 12:04:24";
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS:
 *         open_clxn, create_clxn, init_clxn, destroy_clxn, addr_clxn,
 *         raw_close_clxn, close_clxn, change_vchar, get_vchar
 *         add_vchar, get_string_dboff, raw_add_str, mount_clxn
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>

#include <odmi.h>
#include "odmlib.h"
#include "odmtrace.h"
#include "odmhkids.h"

char *malloc(),*realloc();
extern char *odmcf_errstr;
extern char repospath[];
extern int odmcf_perms_op;
extern int errno;

/* The following variables (adds,fetches,deletes, and
                           changes) are used for getting a summary with a
                           debugger of how many of each operation have been
                           done by the library in this process for performance
                           monitoring */
static int adds = 0;
static int fetches = 0;
static int deletes = 0;
static int changes = 0;

char *raw_find_str();
char *raw_add_str();
char *raw_add_str();

int catch_faults();

static struct ClxnFileHdr * clxn_file_hdr;
extern struct StringClxn *clxn_cur_shmated;

/*
 *    NAME:    open_clxn
 *    FUNCTION: open an odm string clxn in the mode used by
 *              cfg_odm, returning its symbol.
 *
 *    RETURNS:  Pointer to a string collection  if successful, -1
 *              otherwise.
 */
struct StringClxn *
open_clxn(classp)
struct Class *classp;  /* Pointer to the object class with the collection */
{
    int fd;
    char pathname[MAX_ODM_PATH + MAX_ODMI_NAME + 2];
    struct StringClxn *clxnp;   /* Pointer to the string collection */
    int open_mode;
    struct stat statbuf;

    START_ROUTINE(ODMHKWD_OPEN_CLXNP);

    TRC("open_clxn","Opening collection","","","");
    clxnp = classp->clxnp;

    if (clxnp == NULL)
      {
        TRC("open_clxn","Invalid collection!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((struct StringClxn *) -1);
      } /* endif */

    TRC("open_clxn","Collection name %s ",clxnp->clxnname,"","");
    if (!clxnp->open)
      {
        TRC("open_clxn","Need to open clxn","","","");

        if(repospath[0])
            sprintf(pathname,"%s/%s",repospath,clxnp->clxnname);
        else
            strcpy(pathname,clxnp->clxnname);

        if ( (stat(pathname,&statbuf)) == -1 )
            odm_searchpath(pathname,clxnp->clxnname);

        TRC("open_clxn","Collection path %s",pathname,"","");
        open_mode = O_RDWR;

        fd = open(pathname,open_mode ,odmcf_perms_op);
        if ((fd == -1) && ((errno == EACCES) || (errno == EROFS)))
          {
            /*----------------------------------------------------------*/
            /* The file could not be opened because of the permissions. */
            /* Attempt to open the file as READ-ONLY                    */
            /*----------------------------------------------------------*/
            TRC("open_clxn","Read-write open failed! Attempting read-only",
                         "","","");

            open_mode = O_RDONLY;
            fd = open(pathname,open_mode ,odmcf_perms_op);

          } /* endif */

        if(fd == -1)
          {
            if (errno == ENOENT)
              {
                odmerrno = VCHAR_CLASS_DNE;
              }
            else if (errno == EACCES)
              {
                odmerrno = VCHAR_CLASS_PERMS;
              }
            else
              {
                odmerrno = VCHAR_OPEN_ERR;
              } /* endif */
            TRC("open_clxn","Could not open clxn. err %d",
                odmerrno,"errno %d",errno);

            STOP_ROUTINE;
            return((struct StringClxn *)-1);
          }
        clxnp->fd = fd;
        clxnp->hdr = NULL;
        clxnp->open = TRUE;

        if (open_mode == O_RDONLY)
          {
            /* Set the read-only bit as well */
            TRC("open_clxn","Class is read-only %s",classp->classname,
                                    "","");
            classp->open = classp->open | OPENED_AS_READ_ONLY;
          } /* endif */
      }

    TRC("open_clxn","Opened collection at %x",clxnp,"","");
    STOP_ROUTINE;
    return(clxnp);
}

/*
 *    NAME:    create_clxn
 *    FUNCTION: create an empty string clxn
 *    RETURNS:  A 0 if successful, -1 otherwise.
 *
 */

int create_clxn(clxnp)
struct StringClxn *clxnp;  /* Pointer to the string collection */
{
    char *mem;
    int fd;
    int size;
    char pathname[MAX_ODM_PATH + MAX_ODMI_NAME + 2];

    START_ROUTINE(ODMHKWD_CREATE_CLXN);
    TRC("create_clxn","Creating collection","","","");
    if (clxnp == NULL)
      {
        TRC("create_clxn","NULL clxnp pointer!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return( -1);
      } /* endif */

    if(repospath[0])
        sprintf(pathname,"%s/%s",repospath,clxnp->clxnname);
    else
        strcpy(pathname,clxnp->clxnname);

    TRC("create_clxn","Clxn path is %s",pathname,"","");

    fd = open(pathname,O_RDWR | O_CREAT,odmcf_perms_op);

    if(fd == -1)
      {
        if (errno == ENOENT)
          {
            odmerrno = ODMI_INVALID_PATH;
          }
        else if (errno == EACCES)
          {
            odmerrno = VCHAR_CLASS_PERMS;
          }
        else
          {
            odmerrno = VCHAR_OPEN_ERR;
          } /* endif */
        TRC("create_clxn","Could not open clxn. err %d",
            odmerrno,"errno %d",errno);

        STOP_ROUTINE;
        return(-1);
      }


    mem = shmat(  fd  ,(char *) 0 , SHM_MAP );
    if((int)mem == -1)
      {
        TRC("create_clxn","Clxn shmat failed!!","","","");
        close(fd);
        odmerrno = VCHAR_OPEN_ERR;
        STOP_ROUTINE;
        return(-1);
      }

    size = init_clxn(clxnp,mem);
    if (size <= 0)
      {
        TRC("create_clxn","Could not init clxn! size %d",size,"error %d",
            odmerrno);
        shmdt ( mem );    /* detach the shared memory */
        close(fd);
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("create_clxn","Collection size is %d",size,"","");

    size = (size+4096)&(-4096);

    ftruncate(fd,size);

    shmdt ( mem );    /* detach the shared memory */

    close(fd);

    TRC("create_clxn","Successful create","","","");
    STOP_ROUTINE;

    return(0);
}

/*
 *    NAME:    init_clxn
 *    FUNCTION: write headers, etc into empty object clxn file
 *    RETURNS:  The length of the string collection header space if successful,
 *              -1 otherwise.
 */
int init_clxn(clxnparm,mem)
struct  StringClxn      *clxnparm; /* Pointer to the string collection */
char    *mem;                      /* Memory location of string collection */
{
    struct  StringClxn              *clxnp;
    char    *strings;

    START_ROUTINE(ODMHKWD_INIT_CLXN);
    TRC("init_clxn","Initializing the collection file. mem %x",mem,
        "clxnparm %x",clxnparm);
    if (clxnparm == NULL || mem == NULL)
      {
        TRC("init_clxn","NULL parameter(s)!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    ((struct ClassHdr *)mem)->magic = VCHAR_MAGIC;
    ((struct ClassHdr *)mem)->ndata = sizeof (long); /*zero reserved- NULL*/
    ((struct ClassHdr *)mem)->version = 0;

    clxnp = (struct StringClxn *)(mem + sizeof(struct ClassHdr));
    bcopy(clxnparm,clxnp,sizeof(struct StringClxn));

    TRC("init_clxn","Clxn name %s",clxnparm->clxnname,"","");

    strings = (char *)clxnp + sizeof(struct StringClxn);

    /* clean up clxn structure incase someone reads it in
        and tries to use it */

    clxnp->open = FALSE;
    clxnp->hdr = NULL;

    /* fix up StringClxn structure so all addresses are relative to file */
    clxnp->clxnname = strings-(ulong)mem;
    strcpy(strings,clxnparm->clxnname);
    strings+=strlen(strings)+1;

    /*round up to aligned boundary*/
    strings = (char*)(((ulong)strings + sizeof(ulong))&(-sizeof(ulong)));

    clxnp->data = (char*)(strings - mem);

    TRC("init_clxn","Clxn init successful. size %d",clxnp->data,"","");
    STOP_ROUTINE;
    return((int)clxnp->data);

}

/*
 *    NAME:    destroy_clxn
 *    FUNCTION: eliminate an string clxn
 *    RETURNS:  A 0 if successful, -1 otherwise.
 *
 */
int destroy_clxn(clxnp)
struct StringClxn *clxnp;  /* Pointer to the string collection */
{
    int rc;
    int size;
    char pathname[MAX_ODMI_NAME + MAX_ODM_PATH + 2];
    struct ClxnFileHdr *cur;
    struct stat statbuf;

    START_ROUTINE(ODMHKWD_DESTROY_CLXN);
    TRC("destroy_clxn","Removing collection","","","");
    if (clxnp == NULL)
      {
        TRC("destroy_clxn","NULL clxnp!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("destroy_clxn","Clxn name is %s",clxnp->clxnname,"","");

    if(repospath[0])
        sprintf(pathname,"%s/%s",repospath,clxnp->clxnname);
    else
        strcpy(pathname,clxnp->clxnname);

    /* Defect 109208: When remove .vc files, don't check the ODMPATH  */
    /* if ( (stat(pathname,&statbuf)) == -1 )
        odm_searchpath(pathname,clxnp->clxnname);                     */

    TRC("destroy_clxn","Path is %s",pathname,"","");
    /*
           Need to detach the vchar file from shared memory if it is
           currently attached.
        */

    if ( clxn_cur_shmated == clxnp )
      {
        /*--We do not have to ftruncate to-----------------------*/
        /*--set the file size------------------------------------*/
        /* cur = (struct ClxnFileHdr *) clxn_cur_shmated->hdr;   */
        /* size =  (int)(cur->StringClxn.data + cur->Hdr.ndata); */
        /* size = (size+4096)&(-4096);                           */
        /* ftruncate(clxn_cur_shmated->fd, size);                */
        /*-------------------------------------------------------*/
        shmdt((char *) clxn_cur_shmated->hdr);
        clxn_cur_shmated->hdr = NULL;
        clxn_cur_shmated = NULL;
      }

    rc = unlink ( pathname );
    if ( rc == -1 )
      {
        if (errno == ENOENT)
          {
            odmerrno = VCHAR_CLASS_DNE;
          }
        else if (errno == EACCES)
          {
            odmerrno = VCHAR_CLASS_PERMS;
          }
        else
          {
            odmerrno = ODMI_UNLINKCLXN_ERR;
          } /* endif */
        TRC("destroy_clxn","Could not unlink class! err %d",errno,"","");

        STOP_ROUTINE;
        return(-1);
      }
    /*if_err_ret_err(unlink(pathname),,ODMI_UNLINKCLXN_ERR);*/
    TRC("destroy_clxn","Collection destroyed","","","");
    STOP_ROUTINE;
    return(0);
}


/*
 *      NAME:   addr_clxn
 *      FUNCTION: open the clxn if necessary, then shmat and fill in clxnp->hdr
 *              called by internal functions before operating
 *      RETURNS: A pointer to a string collection if successful, -1 otherwise
 *
 */

struct StringClxn *
addr_clxn(classp)
struct Class *classp; /* Pointer to the class with the collection */
{

    int rc;
    int size;
    char *mem;
    struct ClassHdr *chdr;
    struct StringClxn    *fileclxnp;
    struct ClxnFileHdr *cur;
    int was_open;
    struct StringClxn *clxnp;  /* The pointer to the string collection */
    int shmat_mode;

    START_ROUTINE(ODMHKWD_ADDR_CLXN);

    clxnp = classp->clxnp;

    TRC("addr_clxn","Making collection addressable %x",clxnp,"","");


    if (clxnp == NULL)
      {
        TRC("addr_clxn","Clxn is NULL!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((struct StringClxn *) -1);
      } /* endif */

    TRC("addr_clxn","Collection is %s",clxnp->clxnname,"","");

    /* see if its already addressable - if so we are done */
    if (clxnp->hdr )
      {
        TRC("addr_clxn","Collection is already addressable","","","");
        STOP_ROUTINE;
        return(clxnp);
      }

    was_open = clxnp->open;

    if(!clxnp->open)
      {

        rc = (int) open_clxn(classp );
        if ( rc == -1 )
          {
            TRC("addr_clxn","Could not open collection! err %d",odmerrno,
                "","");
            STOP_ROUTINE;
            return((struct StringClxn *) -1 );
          }
      }


    if (clxn_cur_shmated)
      {
        /*--------------------------------------------------------------*/
        /* We must detach the clxn which is currently in shared memory */
        /* since only one clxn is allowed to be shmated.               */
        /*--------------------------------------------------------------*/
        TRC("addr_clxn","Detaching collection %s",
            clxn_cur_shmated->clxnname,"","");

        /*--We do not have to use ftruncate to-------------------*/
        /*--set the file size------------------------------------*/
        /* cur = (struct ClxnFileHdr *) clxn_cur_shmated->hdr;   */
        /* size =  (int)(cur->StringClxn.data + cur->Hdr.ndata); */
        /* size = (size+4096)&(-4096);                           */
        /* ftruncate(clxn_cur_shmated->fd, size);                */
        /*-------------------------------------------------------*/
        shmdt((char *) clxn_cur_shmated->hdr);
        clxn_cur_shmated->hdr = NULL;
        clxn_cur_shmated = NULL;
      } /* endif */


     shmat_mode = SHM_MAP;
     /* If the file was opened as read-only, set the RDONLY flag */
     if (classp->open & OPENED_AS_READ_ONLY)
       {
         shmat_mode = shmat_mode | SHM_RDONLY;
       } /* endif */

    mem = shmat(  clxnp->fd  , (char *) 0 , shmat_mode );

    if((int)mem == -1)
      {
	/*
        printf("libodmi.a:  Fatal Error! Shmat failed for clxn %s\n",
            clxnp->clxnname);
        exit(100);
	*/
	TRC("addr_clxn","Fatal Error! Shmat failed for clxn %s",                            clxnp->clxnname,"","");
        odmerrno = VCHAR_OPEN_ERR;
	STOP_ROUTINE;
        return((struct StringClxn *)-1);
      }

    /* make sure its really an obj clxn */
    chdr = (struct ClassHdr *) mem;
    if(chdr->magic != VCHAR_MAGIC)
      {
        TRC("addr_clxn","Bad magic info!","","","");
        raw_close_class(clxnp,was_open);
        shmdt(mem);              /* Detach the shared memory */
        odmerrno = VCHAR_MAGICNO_ERR;
        STOP_ROUTINE;
        return((struct StringClxn *)-1);
      }

    clxn_cur_shmated = clxnp;  /* Save the clxn pointer */

    clxnp->hdr = chdr;
    fileclxnp = (struct StringClxn *)(mem+sizeof(struct ClassHdr));
    clxnp->data = (char *)(mem + (ulong)fileclxnp->data);

    TRC("addr_clxn","Collection is now addressable","","","");
    STOP_ROUTINE;
    return(clxnp);
}

/*
 *    NAME:    raw_close_clxn
 *    FUNCTION: conditionally close a odmcf odm string clxn
 *
 *    raw_close_clxn closes the clxn completely if wasopen is false.
 *    if wasopen is true, it just releases the addressability but leaves it open
 *
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
raw_close_clxn(clxnp,was_open)
struct StringClxn *clxnp;    /* Pointer to the string collection */
int     was_open;            /* TRUE or FALSE if close or not */
{
    unsigned long size;

    START_ROUTINE(ODMHKWD_RAW_CLOSE_CLXN);
    TRC("raw_close_clxn","Closing collection %x",clxnp,
        "was open %d",was_open);
    if (clxnp == NULL)
      {
        TRC("raw_close_clxn","NULL clxnp!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    if (!clxnp->open || was_open)
      {
        TRC("raw_close_clxn","Do not need to close","","","");
        STOP_ROUTINE;
        return(0);
      }



    /*
           If this class is currently in shared memory, detach it.
        */

    if ( clxn_cur_shmated == clxnp )
      {
        shmdt((char *) clxn_cur_shmated->hdr);
        clxn_cur_shmated->hdr = NULL;
        clxn_cur_shmated = NULL;
      }

    close(clxnp->fd);
    clxnp->open = FALSE;

    /*if_err_ret_err(status,,ODMI_CLOSECLXN_ERR);*/
    TRC("raw_close_clxn","Class successfully closed","","","");
    STOP_ROUTINE;
    return(0);
}

/*
 *    NAME:     close_clxn
 *    FUNCTION: unconditionally close a odmcf odm string clxn
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */

int close_clxn(clxnp)
struct StringClxn *clxnp;  /* Pointer to the string collection */
{
    int rc;
    START_ROUTINE(ODMHKWD_CLOSE_CLXN);
    TRC("close_clxn","Closing clxn %x",clxnp,"","");

    rc = raw_close_clxn(clxnp,0);

    TRC("close_clxn","raw close returned %d",rc,"","");
    STOP_ROUTINE;
    return(rc);
}


/*
 *    NAME:     change_vchar
 *    FUNCTION: changes a particular strings in a clxn by its id
 *              given the clxn ptr and the memory string
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
int change_vchar(classp,elemp,cobj,dobj)
struct Class *classp;    /* Pointer to the object class */
struct ClassElem *elemp; /* Pointer to the element being changed */
char *cobj;             /* object in memory */
char *dobj;             /* object in database */
{
    int rc;
    struct StringClxn *clxnp;
    register int prev_len,new_len;
    long id;
    char *caddr,*dbaddr;
    int dboff;
    int was_open;
    int temp_error;

    START_ROUTINE(ODMHKWD_CHANGE_VCHAR);
    TRC("change_vchar","Changing vchar %x",classp,"Element %x",elemp);
    if (verify_class_structure(classp) < 0)
      {
        TRC("change_vchar","Invalid class!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("change_vchar","cobj %x",cobj,"dobj %x",dobj);
    if (cobj == NULL || dobj == NULL)
      {
        TRC("change_vchar","NULL parameters!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    clxnp = classp->clxnp;
    was_open = clxnp->open;

    /*--Clxn should already be addressable-----------------------------------*/
    /* rc = (int) addr_clxn(classp );                                        */
    /* if ( rc == -1 )                                                       */
    /*   {                                                                   */
    /*     TRC("change_vchar","Could not addr clxn! err %d",odmerrno,"",""); */
    /*     STOP_ROUTINE;                                                     */
    /*     return(-1);                                                       */
    /*   }                                                                   */
    /*-----------------------------------------------------------------------*/



    caddr = *(char **)(cobj + elemp->offset);
    id = *(long *)cobj;

    dboff = get_string_dboff ( classp, id, elemp->offset );
    if ( dboff == -1 )
      {
        TRC("change_vchar","Could not get db offset! err %d",odmerrno,
            "","");
        temp_error = odmerrno;
        raw_close_clxn(clxnp,was_open);
        odmerrno = temp_error;
        STOP_ROUTINE;
        return(-1);
      }
    /*if_err_ret_err((dboff = (char *)
                get_string_dboff(classp,id,elemp->offset))
                ,,0);*/
    TRC("change_vchar","db offset is %d",dboff,"","");

    dbaddr = clxnp->data + dboff;
    if(!dboff)
        prev_len = 0;
    else
        prev_len = strlen(dbaddr);

    TRC("change_vchar","Previous length %d",prev_len,"Caddr %x",caddr);

    if(!caddr)
        new_len = 0;
    else
        new_len = strlen(caddr);

    TRC("change_vchar","new length %d",new_len,"","");

    if(!new_len)
        dboff = 0;
    else if(prev_len >= new_len)
        strcpy(dbaddr,caddr);
    else
        dboff = (int) raw_add_str(clxnp,(char **) (cobj+elemp->offset) );

    elemp->holder = (char *)dboff;

    ++changes;

    rc = raw_close_clxn ( clxnp, was_open );
    if ( rc == -1 )
      {
        TRC("change_vchar","Could not close clxn! err %d",odmerrno,"","");
        STOP_ROUTINE;

        return((int) -1);
      }
    /*if_err_ret_err(raw_close_clxn(clxnp,was_open),(int),0);*/

    TRC("change_vchar","Successful change","","","");
    STOP_ROUTINE;
    return(0);
}

/*
 *    NAME:     get_vchar
 *    FUNCTION: gets a particular strings in a clxn by its id
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
int get_vchar(classp,elemp,cobj)
struct Class *classp;     /* Pointer to the object class */
struct ClassElem *elemp;  /* Pointer to an element in the object class */
char *cobj;               /* Pointer to the object itself. */
{
    int rc;
    long id;         /* id of str to be deleted */
    int offset;
    struct StringClxn *clxnp;
    int size;               /* size of the string */
    int was_open;
    int dboff;      /* validated string address in database */
    int ptroff;     /* dboff, unvalidated, just used to see if its a null pointer */
    char *p,*pov;   /* malloc'd to receive string */
    int temp_error;


    START_ROUTINE(ODMHKWD_GET_VCHAR);
    TRC("get_vchar","Getting a vchar","","","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("get_vchar","Invalid classp!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("get_vchar","elemp %x",elemp,"cobj %x",cobj);
    if (elemp == NULL || cobj == NULL)
      {
        TRC("get_vchar","Null parameter(s)!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    id = *(long *)cobj;
    offset = elemp->offset;
    TRC("get_vchar","Id %d",id,"Offset %d",offset);

    ptroff = *(int *)(classp->data + id * classp->structsize + offset);

    TRC("get_vchar","Vchar offset %d",ptroff,"","");

    if(ptroff)
      {
        TRC("get_vchar","Getting value from clxn","","","");

        clxnp = classp->clxnp;
        was_open = clxnp->open;

        /*-Clxn should already be addressable--------------------------*/
        /* rc = (int) addr_clxn(classp  );                             */
        /* if ( rc == -1 )                                             */
        /*   {                                                         */
        /*     TRC("get_vchar","Could not addr clxn! err %d",odmerrno, */
        /*         "","");                                             */
        /*     STOP_ROUTINE;                                           */
        /*     return(-1 );                                            */
        /*   }                                                         */
        /*-------------------------------------------------------------*/

        dboff = get_string_dboff ( classp, id, offset );
        if ( dboff == -1 )
          {
            TRC("get_vchar","Could not get database offset! err %d",
                odmerrno,"","");
            temp_error = odmerrno;
            raw_close_clxn(clxnp,was_open);
            odmerrno = temp_error;
            STOP_ROUTINE;
            return(-1 );
          }
        /*if_err_ret_err(
                        dboff = get_string_dboff(classp,id,offset),,0);*/


        pov = clxnp->data + dboff;
        TRC("get_vchar","database offset is %d",dboff,"string %s",pov);

        size = strlen(pov);

        p = malloc ( size+1 );
        if (  p == NULL )
          {
            TRC("get_vchar","Could not malloc size %d!",size,"","");
            raw_close_clxn(clxnp,was_open);
            odmerrno = ODMI_MALLOC_ERR;
            STOP_ROUTINE;
            return ( -1 );
          }
        /*if_null_ret_err(p = malloc(size+1),,ODMI_MALLOC_ERR);*/

        strcpy(p,pov);
        *(char **)(cobj + offset) = p;

        rc = raw_close_clxn ( clxnp, was_open );
        if ( rc == -1 )
          {
            TRC("get_vchar","Could not close clxn! err %d",odmerrno,
                "","");
            free((void *) p);
            STOP_ROUTINE;
            return(-1 );
          }
        /*if_err_ret_err(raw_close_clxn(clxnp,was_open),,0);*/
      }
    else
      {
        TRC("get_vchar","Null vchar","","","");
        /*------------------------------------------------------------*/
        /* Just so that the user will get a pointer back, even if the */
        /* string is NULL, malloc some space and put it in the vchar. */
        /*------------------------------------------------------------*/
        p = malloc ( 1 );
        if (  p == NULL )
          {
            TRC("get_vchar","Could not malloc 1  !","","","");
            odmerrno = ODMI_MALLOC_ERR;
            STOP_ROUTINE;
            return(-1 );
          }

        *p = '\0';   /* Set to NULL */
        *(char **)(cobj + offset) = p;
      }

    TRC("get_vchar","Successful get","","","");
    STOP_ROUTINE;
    return(0);
}
/*
 *    NAME:     add_vchar
 *    FUNCTION: Add an string to a clxn
 *              given the clxn symbol and the memory string.
 *    RETURNS:  A 0 if successful, -1 otherwise.
 */
int add_vchar(classp,elemp,cobj,dobj)
struct Class *classp;    /* Pointer to the object class */
struct ClassElem *elemp; /* Pointer to an element in the object class */
char *cobj;             /* object in memory */
char *dobj;             /* object in database */
{
    struct StringClxn *clxnp;
    char *stroff,*dboff;
    char **coff;
    int rv;
    int was_open;
    int temp_error;

    START_ROUTINE(ODMHKWD_ADD_VCHAR);
    TRC("add_vchar","Adding vchar","","","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("add_vchar","Invalid class structure","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("add_vchar","cobj %x",cobj,"dobj %x",dobj);

    if (cobj == NULL || dobj == NULL)
      {
        TRC("add_vchar","NULL parameter(s)!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    coff = (char **) (cobj + elemp->offset);
    if( !*coff || ! **coff )
      {
        /*---------------------------------------------------------------*/
        /* To save space in the collection file, if the pointer is NULL  */
        /* or it points to a NULL string, set the database offset to     */
        /* NULL instead of putting a NULL string in the collection.      */
        /*---------------------------------------------------------------*/
        TRC("add_vchar","Null string or NULL ptr. Setting to NULL","",
            "","");
        dboff = dobj + elemp->offset;  /* put NULL to indicate no str */
        *(char **)dboff = NULL; /* no string to add */

        STOP_ROUTINE;
        return(0);
      }

    TRC("add_vchar","String is %s",*coff,"","");

    clxnp = classp->clxnp;
    was_open = clxnp->open;

    /*--Clxn should already be addressble ---------------------------------*/
    /* rv = (int) addr_clxn(classp );                   make addressable  */
    /* if ( rv == -1 )                                                     */
    /*   {                                                                 */
    /*     TRC("add_vchar","Could addr clxn! err %d",odmerrno,"","");      */
    /*     STOP_ROUTINE;                                                   */
    /*     return(-1);                                                     */
    /*   }                                                                 */
    /*---------------------------------------------------------------------*/

    stroff = raw_add_str(clxnp,coff);       /* add string to collection */
    if ( (int) stroff == -1 )
      {
        TRC("add_vchar","Could not add string! err %d",odmerrno,"","");
        temp_error = odmerrno;
        raw_close_clxn(clxnp,was_open);
        odmerrno = temp_error;
        STOP_ROUTINE;

        return(-1 );
      }

    /*if_err_ret_err(stroff,,0);*/

    TRC("add_vchar","String offset in database %d",stroff,"","");

    dboff = dobj + elemp->offset;   /* put offset in object class */
    *(char **)dboff = (char *)stroff;

    rv = raw_close_clxn(clxnp,was_open);
    if ( rv == -1 )
      {
        TRC("add_vchar","Could not close clxn! err %d",odmerrno,"","");
        STOP_ROUTINE;
        return(-1 );
      }
    /*if_err_ret_err(rv,,VCHAR_ADD_ERR);*/

    ++adds;
    TRC("add_vchar","Successful add","","","");
    STOP_ROUTINE;
    return(0);
}


/*
 * NAME:      get_string_dboff
 * FUNCTION:  Determines the offset of an object's string in the string
 *            collection.
 * RETURNS:   The offset of the string in the string collection if successful,
 *            -1 otherwise.
 */
int get_string_dboff(classp,id,offset)
struct Class *classp;  /* Pointer to the object class */
long id;               /* ID of the object            */
int  offset;           /* Element offset in the object */
{
    char *coff;
    int dboff;

    START_ROUTINE(ODMHKWD_GET_STRING_DBOFF);
    TRC("get_string_dboff","Getting offset","","","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("get_string_dboff","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    TRC("get_string_dboff","Id %d",id,"offset %d",offset);
    if (id < 0 || offset < 0)
      {
        TRC("get_string_dboff","Invalid id or offset!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    coff = classp->data + id * classp->structsize + offset;
    dboff = *(int *)coff;
    TRC("get_string_dboff","coff %x",coff,"Database offset %x",dboff);

    if( dboff >= 0 && dboff <= classp->clxnp->hdr->ndata )
      {
        TRC("get_string_dboff","Dboff is valid","","","");
        STOP_ROUTINE;
        return(dboff);
      }
    else
      {
        TRC("get_string_dboff","offset is outside range! top %d",
            classp->clxnp->hdr->ndata,"","");

        odmerrno = VCHAR_BADSTRINGADDR;
        STOP_ROUTINE;
        return(-1);
      }
}

/*
 * NAME:     raw_add_str
 * FUNCTION: Adds a string to an object class's string collection.
 * RETURNS:  The offset for this string in the collection if successful,
 *           -1 otherwise.
 */
char *
raw_add_str(clxnp,values)
struct StringClxn *clxnp;  /* Pointer to the string collection */
char **values;             /* Pointer to a pointer to the string */
{
    register int ndata,len;
    char *offset;

    START_ROUTINE(ODMHKWD_RAW_ADD_STR);
    TRC("raw_add_str","Adding to collection %x",clxnp,"values %x",values);
    if (clxnp == NULL || values == NULL)
      {
        TRC("raw_add_str","NULL parameter(s)!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((char *)-1);
      } /* endif */

    TRC("raw_add_str","Collection %s",clxnp->clxnname,"string %s",*values);

    /* catch segmentation violations */
    if (catch_faults(TRUE) < 0) {
	(void) catch_faults(FALSE);

	TRC("raw_add_str","Filesystem full! err %d", odmerrno,"","");
	odmerrno = ODMI_NO_SPACE;
	STOP_ROUTINE;
	return((char *)-1);
    }
    ndata = clxnp->hdr->ndata;
    offset = clxnp->data + ndata;
    len = strlen(*values);

    strcpy(offset,*values);
    clxnp->hdr->ndata += len + 1;

    (void) catch_faults(FALSE);
    TRC("raw_add_str","Returning %x",ndata,"","");
    STOP_ROUTINE;
    return((char *)ndata);
}

/*
 * NAME:       mount_clxn
 * FUNCTION:   Creates a string collection pointer for a given string
 *             collection.
 * RETURNS:    A string collection pointer if successful, -1 otherwise.
 */
struct StringClxn *
mount_clxn(name)
char    *name;   /* The name of the collection to mount */
{
    char    path[MAX_ODMI_NAME + MAX_ODM_PATH  + 2];
    int     fd;
    int     size;
    int     reloc;
    struct ClxnFileHdr filehdr;
    struct  StringClxn      *StringClxnp;
    struct stat statbuf;

    START_ROUTINE(ODMHKWD_MOUNT_CLXN);
    TRC("mount_clxn","Mounting collection %s",name,"","");
    if (!name || !*name )
      {
        TRC("mount_clxn","NULL name!","","","");
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((struct StringClxn *) -1);
      } /* endif */

    if(repospath[0])
        sprintf(path,"%s/%s",repospath,name);
    else
        strcpy(path,name);

    if ( (stat(path,&statbuf)) == -1 )
        odm_searchpath(path,name);

    TRC("mount_clxn","Clxn path is %s",path,"","");

    fd = open(path,O_RDONLY);
    if(fd == -1)
      {
        if (errno == ENOENT)
          {
            odmerrno = VCHAR_CLASS_DNE;
          }
        else if (errno == EACCES)
          {
            odmerrno = VCHAR_CLASS_PERMS;
          }
        else
          {
            odmerrno = VCHAR_OPEN_ERR;
          } /* endif */
        TRC("mount_clxn","Could not open clxn. err %d",
            odmerrno,"errno %d",errno);

        STOP_ROUTINE;
        return((struct StringClxn *)-1);
      }

    /*if_err_ret_err(fd,(struct StringClxn *),ODMI_OPENCLXN_ERR);*/

    if (sizeof(filehdr) != read(fd,(char *) &filehdr,sizeof(filehdr)) )
      {
        TRC("mount_clxn","Could not read header!","","","");
        close(fd);
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((struct StringClxn *) -1 );
        /*ret_err((struct StringClxn *),ODMI_INVALID_CLXN);*/
      }

    if (filehdr.Hdr.magic != VCHAR_MAGIC)
      {
        TRC("mount_clxn","Invalid magic value!","","","");
        close(fd);
        odmerrno = ODMI_CLXNMAGICNO_ERR;
        STOP_ROUTINE;
        return((struct StringClxn *) -1 );
        /*ret_err((struct StringClxn *),ODMI_CLXNMAGICNO_ERR);*/
      }

    size = (int)filehdr.StringClxn.data-sizeof(struct ClassHdr);

    TRC("mount_clxn","Size %d",size,"","");

    StringClxnp  =  (struct StringClxn *) malloc(size);
    if (StringClxnp == NULL)
      {
        TRC("mount_clxn","Collection malloc failed! err %d",
            errno,"","");
        close(fd);
        odmerrno = ODMI_MALLOC_ERR;
        STOP_ROUTINE;
        return((struct StringClxn *) -1);
      } /* endif */

    lseek(fd,(off_t) sizeof(struct ClassHdr),SEEK_SET);

    if (size != read(fd,(char *) StringClxnp,(unsigned) size))
      {
        TRC("mount_clxn","Could not read strings! err %d",errno,"","");
        close(fd);
        odmerrno = ODMI_INVALID_CLXN;
        STOP_ROUTINE;
        return((struct StringClxn *) -1 );
        /*ret_err((struct StringClxn *),ODMI_INVALID_CLXN);*/
      }
    close(fd);

    reloc = (int)StringClxnp - sizeof( struct ClassHdr);
    StringClxnp->clxnname += reloc;

    TRC("mount_clxn","Collection name is %s",StringClxnp->clxnname,"","");

    if (strcmp(StringClxnp->clxnname,name))
      {
        TRC("mount_clxn","Names do not match!","","","");

        odmerrno = ODMI_BAD_CLXNNAME;
        STOP_ROUTINE;
        return((struct StringClxn *) -1 );
        /*ret_err((struct StringClxn *),ODMI_BAD_CLXNNAME);*/
      }

    TRC("mount_clxn","Successful mount. Clxnp %x",StringClxnp,"","");
    STOP_ROUTINE;
    return(StringClxnp);
}
