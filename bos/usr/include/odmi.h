/* @(#)66       1.24  src/bos/usr/include/odmi.h, libodm, bos411, 9438C411a 9/22/94 17:15:38 */
/*
 *   COMPONENT_NAME: LIBODM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ODMI
#define _H_ODMI
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/limits.h>
#define MAX_ODMI_NAME MAXNAMLEN  
#define MAX_ODMI_CRIT 256
#define MAX_CLASSES 1024
#define MAX_ODM_PATH  PATH_MAX


/*
  Define the new interface names to the old names, so folks can
  start using the new interface.
*/


#define objlistinfo listinfo
#define odmcf_errno odmerrno

struct Class {
        int begin_magic;
        char *classname;
        int structsize;
        int nelem;
        struct ClassElem *elem;
        struct StringClxn *clxnp;
        int open;
        struct ClassHdr  *hdr;
        char *data;     /* new */
        int fd;         /* new */
        int current;    /* new */
        struct Crit *crit;
        int ncrit;
        char critstring[MAX_ODMI_CRIT];
        int reserved;
        int end_magic;
        };
struct ClassElem {
        char *elemname;
        int type;
        int offset;
        int size;
        struct Class *link;     /* if link or vlink */
        char *col;              /* if link or vlink */
        int linktype;           /* if link or vlink */
        char *holder;           /* temp storage during change */
        int ordinal;            /* future use for virtual database */
        int reserved;
        };

struct ClassHdr {
        int magic;
        int ndata;
        int version;            /* number of times modified */
        };

#define ODM_FIRST 1     /* Get the first object */
#define ODM_NEXT  0     /* Get the next object  */

#define ODMI_MAGIC 0xdcfac      /* hex chars from ODmCF_mAgiC */

/* ODM error numbers                                         */
/* If an error is added, the message files must be updated   */
/* (odmmsg.h, libodm.msg)                                    */

#define ODMI_OPEN_ERR                   5900
#define ODMI_MALLOC_ERR                 5901
#define ODMI_MAGICNO_ERR                5902   /* bad file magic num */
#define ODMI_NO_OBJECT                  5903
#define ODMI_BAD_CRIT                   5904
#define ODMI_INTERNAL_ERR               5905
#define ODMI_TOOMANYCLASSES             5906
#define ODMI_LINK_NOT_FOUND             5907
#define ODMI_INVALID_CLASS              5908
#define ODMI_CLASS_EXISTS               5909
#define ODMI_CLASS_DNE                  5910
#define ODMI_BAD_CLASSNAME              5911
#define ODMI_UNLINKCLASS_ERR            5912
#define ODMI_UNLINKCLXN_ERR             5913
#define ODMI_INVALID_CLXN               5914
#define ODMI_CLXNMAGICNO_ERR            5915
#define ODMI_BAD_CLXNNAME               5916
#define ODMI_CLASS_PERMS                5917 /* PERMISSIONS DON'T ALLOW OPEN */
#define ODMI_BAD_TIMEOUT                5918 /* INVALID TIMEOUT VALUE        */
#define ODMI_BAD_TOKEN                  5919 /* UNABLE TO OPEN/CREATE TOKEN  */
#define ODMI_LOCK_BLOCKED               5920 /* ANOTHER PROCESS HAS LOCK     */
#define ODMI_LOCK_ENV                   5921 /* CANNOT GET/SET ENV VARIABLE  */
#define ODMI_UNLOCK                     5922 /* CANNOT UNLOCK THE TOKEN      */
#define ODMI_BAD_LOCK                   5923 /* UNABLE TO SET LOCK           */
#define ODMI_LOCK_ID                    5924 /* INVALID LOCK ID              */
#define ODMI_PARAMS                     5925 /* INVALID PARAMETERS PASSED IN */
#define ODMI_OPEN_PIPE                  5926 /* COULD NOT OPEN CHILD PIPE    */
#define ODMI_READ_PIPE                  5927 /* COULD NOT READ FROM CHILD PIPE*/
#define ODMI_FORK                       5928 /* COULD NOT FORK CHILD PROCESS */
#define ODMI_INVALID_PATH               5929 /* PATH OR FILE IS INVALID      */
#define ODMI_READ_ONLY                  5930 /* CLASS IS OPENED AS READ-ONLY */
#define ODMI_NO_SPACE                   5931 /* FILESYSTEM FULL */


/* Object Class Collection Errors */

#define VCHAR_OPEN_ERR                  5800
#define VCHAR_MAGICNO_ERR               5801
#define VCHAR_CLASS_DNE                 5802
#define VCHAR_BADSTRINGADDR             5803
#define VCHAR_CLASS_PERMS               5804

#define LINK_INFO_OFFSET    4     /* offset from beginning of link ptr
                                  to beginning of info struct in mem obj
                                    value = sizeof struct pointer */
#define LINK_VAL_OFFSET     8     /* offset from beginning of link to beginning
                                   of the value array.  info_offset + sizeof
                                     an info structure */

#define ODM_WAIT   -1  /* Wait forevever                               */
#define ODM_NOWAIT  0  /* Return immediately if lock cannot be granted */

struct listinfo {
        char classname[MAX_ODMI_NAME];
        char crit[MAX_ODMI_CRIT];
        int  num;
        int valid;
        struct Class *class;
        };

#define MAX_CRITELEM_LEN 256
struct Crit {
        char value[MAX_CRITELEM_LEN];
        char name[MAX_CRITELEM_LEN];
        int relation;
        int offset;
        int type;
        int match;
        };

/* ODM TYPES AND OBJECT CLASS RELATIONSHIPS  */

#define ODM_CHAR                 0
#define ODM_LONGCHAR             1
#define ODM_BINARY               2
#define ODM_SHORT                3
#define ODM_LONG                 4
#define ODM_LINK                 5
#define ODM_METHOD               6
#define ODM_VCHAR                7
#define ODM_DOUBLE               8
#define ODM_ULONG                9


/* FILE FORMATS */

#define STANZA      1
#define COLON       2          /* assumes file with : separators */
/* #define DBM         3       remove constant - P42333  */
#define ODM         4
#define DMS         6
#define WHITESPACE  8
                         /* type 7 currently reserved */


struct StringClxn {
        char *clxnname;
        int open;
        struct ClassHdr  *hdr;
        char *data;
        int fd;
        long reserved[2];
        };

#define VCHAR_MAGIC 0xcaa1c     /* hex chars from vChAr_mAg1C */



typedef struct Class *CLASS_SYMBOL;     /* The class symbol for most  */
                                        /* applications will be:      */
                                        /*    <class_name>_CLASS      */

extern int odmerrno;
extern int odm_read_only;

int             odm_add_obj(CLASS_SYMBOL,void *);
int             odm_change_obj(CLASS_SYMBOL,void *);
int             odm_close_class(CLASS_SYMBOL);
int             odm_create_class(CLASS_SYMBOL);
int             odm_free_list(void *, struct listinfo *);
void           *odm_get_by_id(CLASS_SYMBOL, int, void *);
void           *odm_get_first(CLASS_SYMBOL, char *, void *);
void           *odm_get_list(CLASS_SYMBOL, char *, struct listinfo *, int, int);
void           *odm_get_next(CLASS_SYMBOL, void *);
void           *odm_get_obj(CLASS_SYMBOL, char *, void *, int);
int             odm_initialize();
int             odm_lock(char *, int);
CLASS_SYMBOL    odm_mount_class(char *);
int             odm_err_msg(int, char **);
CLASS_SYMBOL    odm_open_class(CLASS_SYMBOL);
int             odm_rm_by_id(CLASS_SYMBOL, long);
int             odm_rm_class(CLASS_SYMBOL);
int             odm_rm_obj(CLASS_SYMBOL,char *);
int             odm_run_method(char *, char*, char **, char **);
char           *odm_set_path(char *);
int             odm_set_perms(int);
int             odm_terminate();
int             odm_unlock(int);
void            odm_searchpath(char *, char *);
#endif /* _H_ODMI */                                /* Defect 91718 */
