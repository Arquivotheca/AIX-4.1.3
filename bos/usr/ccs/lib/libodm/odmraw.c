static char sccsid[] =  "@(#)91	1.21	src/bos/usr/ccs/lib/libodm/odmraw.c, libodm, bos41B, 9504A	12/16/94	11:44:38"; 
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: catch_faults, raw_addr_class, raw_close_class, raw_find_byid,
              raw_rm_obj, raw_add_obj,verify_class_structure
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
#include <sys/errno.h>
#include <sys/shm.h>
#include <sys/signal.h>
#include <setjmp.h>
#include <unistd.h>
#include "odmtrace.h"
#include "odmhkids.h"

#include <odmi.h>
#include "odmlib.h"


extern int odmerrno;
extern char *odmcf_errstr;
extern char repospath[];
extern int odmcf_perms_op;
extern int odm_read_only;

char *raw_find_obj();
extern struct Class *class_cur_shmated;

/*
 *      NAME:   mem_fault
 *      FUNCTION: signal handler for segmentation voiolations.
 *		  prints a message indicating out of disk space and dies.
 *      RETURNS: never.
 */

static jmp_buf busv_jmp;
static int catch_count;

static void
sig_action()
{
    signal(SIGBUS, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    _longjmp(busv_jmp, 1);
}


/*
 *      NAME:   catch_faults
 *      FUNCTION: Turns on or off the segmentation violation signal handling,
 *		  and bus error signal handling. Nested calls are ok, the trap 
		  will occur to the last call.
 *		  This routine must be called to turn off signal handling
 *		  after a fault occurs.
 *		  The routine preserves the callers signal hander.
 *      RETURNS: 0 if ok, -1 if bus error or segmentation violation occurred.
 */

int
catch_faults(turn_on)
int turn_on;
{
    static void (*old_hndlr)() = SIG_DFL; 

    if (turn_on)
    {
	if (_setjmp(busv_jmp))
	    return -1;
	if (catch_count++ == 0)
	{
	    old_hndlr = signal(SIGBUS, sig_action);
	    old_hndlr = signal(SIGSEGV, sig_action);
	}
    }
    else
    {
	if (--catch_count == 0)
	{
	    signal(SIGBUS, old_hndlr);
	    signal(SIGSEGV, old_hndlr);
	    old_hndlr = SIG_DFL;
	}
	else if (catch_count < 0)
	    catch_count = 0;
    }
    return 0;
}



/*
 *      NAME:   raw_addr_class
 *      FUNCTION: open the class if necessary,
 *              then shmat and fill in classp->hdr.
 *      RETURNS: A pointer to the object class if successful, -1 otherwise.
 */

struct Class *
raw_addr_class(classp)
struct Class *classp;   /* Pointer to the object class */
{
    int size;
    char *mem;
    struct ClassHdr *chdr;
    struct Class    *fileclassp;
    struct ClassFileHdr *cur;
    struct Class *return_class;
    struct StringClxn *return_clxn;
    int opened_class;
    int temp_error;
    int shmat_mode;


    START_ROUTINE(ODMHKWD_RAW_ADDR_CLASS);
    TRC("raw_addr_class","Making class addressable","","","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_addr_class","Invalid class structure","","","");
        STOP_ROUTINE;
        return((struct Class *) -1);
      } /* endif */

    opened_class = FALSE;

    if(!classp->open)
      {
        TRC("raw_add_class","opening class","","","");

        return_class = odm_open_class ( classp );
        if ( (int) return_class == -1 )
          {
            TRC("raw_addr_class","Bad return from open! err %d",
                odmerrno,"","");
            STOP_ROUTINE;
            return((struct Class *) -1);
          }
        opened_class = TRUE;
        /*if_err_ret_err(odm_open_class(classp),(struct Class *),0);*/
      }

    if(classp->hdr != NULL)
      {
        TRC("raw_addr_class","Class already shmat'd %x",classp->hdr,"","");
      }
    else
      {
         if (class_cur_shmated)
           {

             /*--------------------------------------------------------------*/
             /* We must detach the class which is currently in shared memory */
             /* since only one class is allowed to be shmated.               */
             /*--------------------------------------------------------------*/
             TRC("raw_addr_class","Detaching class currently shated %s",
                 class_cur_shmated->classname,"","");

             /*--We do not need to call ftruncate to------------------*/
             /*--set the file size------------------------------------*/
             /* cur = (struct ClassFileHdr *) class_cur_shmated->hdr; */
             /* size =  (int)(cur->Class.data                         */
             /*     + cur->Hdr.ndata * cur->Class.structsize);        */
             /* size = (size+4096)&(-4096);                           */
             /* ftruncate(class_cur_shmated->fd, size);               */
             /*-------------------------------------------------------*/

             shmdt((char *) class_cur_shmated->hdr);
             class_cur_shmated->hdr = NULL;
             class_cur_shmated = NULL;
           } /* endif */


         shmat_mode = SHM_MAP;
         if ( odm_read_only == 1 ) classp->open = classp->open | OPENED_AS_READ_ONLY;
         /* If the file was opened as read-only, set the RDONLY flag */
         if (classp->open & OPENED_AS_READ_ONLY)
           {
             shmat_mode = shmat_mode | SHM_RDONLY;
           } /* endif */

         mem = shmat(  classp->fd  ,(char *)  0 , shmat_mode );

         if((int)mem == -1)
         {
	 /*the print statement here was removed by defect 63797*/
             odmerrno = ODMI_OPEN_ERR;
             return((struct Class *)-1);
           }

         TRC("raw_addr_class","Shmat'd class at %x",mem,"","");

         /* make sure its really an obj class */
         chdr = (struct ClassHdr *) mem;
         if(chdr->magic != ODMI_MAGIC)
           {
             TRC("raw_addr_class","Invalid magic!","","","");

             shmdt(mem);              /* Detach the shared memory */

             if (opened_class)
               {
                 /*---------------------------------------------------------------*/
                 /* Since raw_close_class calls raw_addr_class, we need to close  */
                 /* the object class without calling raw_close_class.  This is to */
                 /* prevent an infinite loop.                                     */
                 /*---------------------------------------------------------------*/
                 TRC("raw_addr_class","Closing the class without calling raw_close",
                            "","","");
                 close(classp->fd);
                 classp->open = FALSE;
                 if (classp->clxnp && classp->clxnp->open)
                   {
                     TRC("raw_addr_class","Closing collection","","","");
                     close(classp->clxnp->fd);
                     classp->clxnp->open = FALSE;
                   } /* endif */
               } /* endif */

             odmerrno = ODMI_MAGICNO_ERR;

             STOP_ROUTINE;
             return((struct Class *)-1);
           } /* endif chdr->magic != ODMI_MAGIC */

        classp->hdr = chdr;
        fileclassp = (struct Class *)(mem+sizeof(struct ClassHdr));
        classp->data = (char *)(mem + (ulong)fileclassp->data);
      } /* endif classp->hdr == NULL */



    if(classp->clxnp)
      {
        TRC("raw_addr_class"," Addr the collection",
            "","","");

        return_clxn = addr_clxn ( classp );
        if ( (int) return_clxn == -1 )
          {
            TRC("raw_addr_class","Could not addr clxn! err %d",odmerrno,
                "","");
            shmdt(mem);              /* Detach the shared memory */
            if (opened_class)
              {
             /*-----------------------------------------------------------*/
             /* Since raw_close_class calls raw_addr_class, we need to close */
             /* the object class without calling raw_close_class.  This is   */
             /* to prevent an infinite loop.                                 */
             /*-----------------------------------------------------------*/
                TRC("raw_addr_class",
                           "Closing the class without calling raw_close",
                           "","","");
                close(classp->fd);
                classp->open = FALSE;
                TRC("raw_addr_class","Closing collection","","","");
                close(classp->clxnp->fd);
                classp->clxnp->open = FALSE;
              } /* endif */

            STOP_ROUTINE;
            return((struct Class *) -1);
          }
      }

    class_cur_shmated = classp;  /* Save the class pointer */

    TRC("raw_addr_class","Returning classp %x",classp,"","");
    STOP_ROUTINE;
    return(classp);
}


/*
 *    NAME:    raw_close_class
 *    FUNCTION: close a odmcf odm object class
 *              Closes the class completely if wasopen is false.
 *    RETURNS:  A 0 if successful, -1 otherwise
 */
int raw_close_class(classp,was_open)
struct Class *classp;    /* Pointer to the object class */
int     was_open;        /* TRUE or FALSE to close or not */
{
    int rc;
    int i, ndata;
    int changed = FALSE;
    unsigned long size;
    int *p;

    START_ROUTINE(ODMHKWD_RAW_CLOSE_CLASS);
    TRC("raw_close_class","Closing class. was_open %d",was_open,"","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_close_class","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    if (!classp->open || was_open)
      {
        /*-------------------------------*/
        /* Don't really close the class. */
        /*-------------------------------*/
        TRC("raw_close_class","Don't need to close class","","","");
        STOP_ROUTINE;
        return(0);
      }



     /*------------------------------------------------------------*/
     /* Make class addressable and see if we can shorten the file. */
     /*------------------------------------------------------------*/

     if ( (int) raw_addr_class(classp) == -1 )
       {
         TRC("raw_close_class","Could not addr class! err %d",odmerrno,
             "","");
       }
     else
       {

          /*--------------------------------------------*/
          /* Check for the read-only status on the file */
          /*--------------------------------------------*/
          if ((classp->open & OPENED_AS_READ_ONLY) == 0)
            {

              /*-------------------------------------------------------------*/
              /* Shorten the file if we can and if the file is not read-only */
              /*-------------------------------------------------------------*/
              ndata = classp->hdr->ndata;

              i = ndata - 1;
              p = (int *)(classp->data + (i  * classp->structsize));
              while (  i != -1 && *p == -1 )   {
                  i--;
                  p = (int *)((char *)p - classp->structsize);
                };

	      if (i < (ndata - 1))
		  changed = TRUE;	

              classp->hdr->ndata = i+1;

              size = (ulong)p+classp->structsize - (ulong)(classp->hdr);
              size = (size+4096)&(-4096);
              TRC("raw_close_class","Making file size %d",size,"","");

	      /*------------------------------------------------------------*/
	      /* ftruncate if the size has changed or there are no objects  */
	      /* in the object class                                        */
	      /*------------------------------------------------------------*/
	      if (changed || (ndata == 0))
                  ftruncate(classp->fd, size);

            } /* endif */

       } /* endif */


    /*
           If this class is currently in shared memory, detach it.
        */

    if ( class_cur_shmated == classp )
      {
        TRC("raw_close_class","Detaching class %x",classp->hdr,"","");
        shmdt((char *) class_cur_shmated->hdr);
        class_cur_shmated->hdr = NULL;
        class_cur_shmated = NULL;
      }

    close(classp->fd);
    classp->open = FALSE;

    if(classp->clxnp)
      {
        TRC("raw_close_class","Closing collection","","","");
        rc = raw_close_clxn ( classp->clxnp, was_open );
        if ( rc == -1 )
          {
            TRC("raw_close_class","Could not close clxn %d",odmerrno,
                "","");
            STOP_ROUTINE;
            return(-1);
          }
        /*if_err_ret_err(raw_close_clxn(classp->clxnp,was_open),,0);*/
      }

    TRC("raw_close_class","Class is closed","","","");
    STOP_ROUTINE;
    return(0);
}

/*
 * NAME:     raw_find_byid
 * FUNCTION: Finds an object in the database based on an id.
 * RETURNS:  A pointer to the object if successful, -1 otherwise.
 */
char *
raw_find_byid(classp,id)
struct Class *classp;  /* Pointer to the object class */
long id;                /* ID of the object to get.    */
{
    char *offset;

    START_ROUTINE(ODMHKWD_RAW_FIND_BYID);
    TRC("raw_find_by_id","Looking for id %d",id,"","");

    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_find_by_id","Invalid class structure!","","","");
        STOP_ROUTINE;
        return((char *) -1);
      } /* endif */

    /*------------------------------------------------------------*/
    /* Calculate offset then check to see if the object is valid. */
    /*------------------------------------------------------------*/
    offset = classp->data + id * classp->structsize;

    if( id >= 0 && id <= classp->hdr->ndata && *(long *) offset == id)
      {
        offset = classp->data + id * classp->structsize;
        TRC("raw_find_by_id","Returning offset %d",offset,"","");
        STOP_ROUTINE;
        return((char *)offset);
      }
    else
      {
        TRC("raw_find_by_id","Invalid id","","","");

        odmerrno = ODMI_NO_OBJECT;
        STOP_ROUTINE;
        return((char *)-1);
      }
}

/*
 * NAME:     raw_rm_obj
 * FUNCTION: Removes objects from the database based on a criteria.
 * RETURNS:  The number of objects deleted if successful, -1 otherwise.
 */
int raw_rm_obj(classp,criteria)
struct Class *classp;   /* Pointer to the object class */
char *criteria;         /* Criteria determining which objects to delete */
{
    int ntries,ngot;
    char *pov;

    START_ROUTINE(ODMHKWD_RAW_RM_OBJ);
    TRC("raw_rm_obj","Removing object, crit %s",criteria,"","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_rm_obj","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    /* special case - no objects */
    if(classp->hdr->ndata == 0)
      {
        TRC("raw_rm_obj","No objects to delete","","","");
        STOP_ROUTINE;
        return(0);
      }

    ntries = 0;
    ngot = 0;
    while(1)  {
        if(ntries++ == 0)
            pov = raw_find_obj(classp,criteria,TRUE);
        else
            pov = raw_find_obj(classp,criteria,FALSE);

        if(pov == NULL ) break;


        if ( (int) pov == -1 )
          {
            TRC("raw_rm_obj","Could not find obj! err %d",odmerrno,
                "","");
            STOP_ROUTINE;
            return((int) -1);
          }
        /*if_err_ret_err(pov,(int),0);*/
        TRC("raw_rm_obj","Deleting at offset %x",pov,"","");

        *(long *)pov = -1;
        ngot++;
      }
    /* special case "" : side effect truncate the
                           working area of the file */

    if(!criteria || !*criteria)
      {
        TRC("raw_rm_obj","Setting ndata to 0","","","");

        classp->hdr->ndata = 0;
      }

    TRC("raw_rm_obj","Deleted %d",ngot,"","");
    STOP_ROUTINE;
    return(ngot);
}

/*
 * NAME:     raw_add_obj
 * FUNCTION: Add an object to the database.
 * RETURNS:  The object identifier (ID) if successful, -1 otherwise.
 */
int raw_add_obj(classp,cobj)
struct Class *classp;    /* Pointer to the object class */
char *cobj;                     /* address of object in memory */
{
    struct ClassElem *e;
    int id,rv;
    register int i,ndata,ne,size;
    char *dbobj;                    /* address of object in mapped database */


    START_ROUTINE(ODMHKWD_RAW_ADD_OBJ);
    TRC("raw_add_obj","Adding object %x",cobj,"","");
    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_add_obj","Invalid class structure!","","","");
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    if (cobj == NULL)
      {
        TRC("raw_add_obj","Null class structure","","","");
        odmerrno = ODMI_PARAMS;
        STOP_ROUTINE;
        return(-1);
      } /* endif */

    /* if find first empty struct */
    ndata = classp->hdr->ndata;

    size = classp->structsize;

    /* If ODMAPPEND is set, then append rather than insert into the
     * first available slot.
     */
       
    if (getenv("ODMAPPEND") != NULL)
      {
        dbobj = classp->data + size*ndata;
        i = ndata;
      }
    else
      {
        for(i=0,dbobj = classp->data;i<ndata;i++,dbobj += size)
          {
            id = *(long *)dbobj;
            if(id == -1)
              {
                TRC("raw_add_obj","Found avail slot at %d",i,"","");
                id = i;
                break;
              } /* endif */
            /* copy in struct */
          } /* endfor */
      }

    if(i == ndata)
      {
        TRC("raw_add_obj","Putting obj at end %d",i,"","");
        /* copy it to the end */
        id = ndata;
      }

    /* catch segmentation violations */
    if (catch_faults(TRUE) < 0) {
	/* A segmentation violation or bus error occured somewhere in 
	 * the code below.
	 *
	 * This is almost positively because the filesystem is out of
	 * space and a shared memory assignment could not page to the file.
	 *
	 * The code has been written such that the slot id is not filled
	 * in until all other work has been done.  This makes sure that
	 * the database is left in a consistent state.
	 */
	(void) catch_faults(FALSE);

	odmerrno = ODMI_NO_SPACE;
	TRC("raw_add_obj","Filesystem full! err %d", odmerrno,"","");
	STOP_ROUTINE;
	return((int) -1);
    }

    *(long *)dbobj = -1;
    bcopy(cobj, dbobj, classp->structsize);

    /* zero out link ptr & listinfo struct*/
    ne = classp->nelem;
    for(e = classp->elem,i=0;i<ne;i++,e++)
      {
        if(e->type == ODM_LINK)
          {
            TRC("raw_add_obj","Adding link info %s",e->elemname,"","");
            *(char **)(dbobj + e->offset ) = NULL;
            *(char **)(dbobj + e->offset + LINK_INFO_OFFSET) = NULL;
          } /* end if */
        else if(e->type == ODM_VCHAR)
          {
            TRC("raw_add_obj","Adding vlink info %s",e->elemname,"","");
            rv = add_vchar(classp, e, cobj, dbobj);
            if ( rv == -1 )
              {
                TRC("raw_add_obj","Could not add to vchar! err %d",
                    odmerrno,"","");
                STOP_ROUTINE;
		(void) catch_faults(FALSE);
                return((int) -1);
              }

            /*if_err_ret_err(rv,(int),0);*/
          } /* end else */
      } /* endfor */

    (void) catch_faults(FALSE);

    /* only now to we set the id, and possibly bump ndata,
     * this keeps database consistency if the filesystem fills
     * and we die above.
     */
    *(long *)cobj = *(long *)dbobj = id;
    if (id == ndata)
        classp->hdr->ndata++;

    TRC("raw_add_obj","Added object. Id %d",id,"","");
    STOP_ROUTINE;
    return((int) id);
}

/*
 * NAME:      verify_class_structure
 * FUNCTION:  Verifies that the class structure passed in by the user
 *            is a valid 'struct Class *' by checking the magic numbers
 *            in the Class structure.
 * RETURNS:   A 0 if the structure is good, -1 otherwise.
 */
int verify_class_structure(classp)
struct Class *classp;
{
    START_ROUTINE(ODMHKWD_VERIFY_CLASS_STRUCTURE);

    TRC("verify_class_structure","Checking structure %x (hex) ",classp,
        "","");


    if ((int) classp == -1 )
      {
        TRC("verify_class_structure","Invalid Class structure! (classp = -1)",
            "","","");
        odmerrno = ODMI_MAGICNO_ERR;
        STOP_ROUTINE;
        return(-1);
      }

    if ( classp == NULL || classp->begin_magic != ODMI_MAGIC )
      {
        TRC("verify_class_structure","Invalid Class structure! ","","","");
        odmerrno = ODMI_MAGICNO_ERR;
        STOP_ROUTINE;
        return(-1);
      }
    /*if_null_ret_err((classp->begin_magic == ODMI_MAGIC),
                (struct Class *), ODMI_MAGICNO_ERR);*/

    if ( classp->end_magic != -ODMI_MAGIC )
      {
        TRC("verify_class_structure","Invalid -Class structure!","","","");
        odmerrno = ODMI_MAGICNO_ERR;
        STOP_ROUTINE;
        return(-1);
      }
    /*if_null_ret_err((classp->end_magic == -ODMI_MAGIC),
                (struct Class *), ODMI_MAGICNO_ERR);*/

    TRC("verify_class_structure","Valid class structure %s",classp->classname,
        "","");
    STOP_ROUTINE;
    return(0);

} /* end verify_class_structure */
