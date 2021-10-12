static char sccsid[] = "@(#)73        1.23.3.13  src/bos/kernel/s/aud/auditobj.c, syssaud, bos41J, 9519A_all 5/2/95 09:59:50";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditobj() system call
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/errno.h>
#include	<sys/user.h>
#include	<sys/malloc.h>
#include	<sys/audit.h>
#include	<sys/auditk.h>
#include	<sys/systm.h>
#include	<sys/pathname.h>
#include	<sys/priv.h>
#include	<sys/vnode.h>
#include	<sys/vattr.h>
#include	<sys/vfs.h>
#include	<sys/uio.h>
#include	<sys/lockl.h>

/* global cred lock */

extern  Simple_lock cred_lock;

/* 
 * forward declarations 
 */

static int ObjectsStore(struct o_event *UsrObjects, int Onum);
static int ObjectsCopyIn(struct o_event *, struct o_event **, char **, char **, int);
static struct AObject_t * ObjectCreate(struct o_event *);
static int ObjectGetFid(char *, char *, struct AObject_t *);
static int ObjectsRemove();
static int ObjectsFetch(struct o_event *, int);
static int ObjectsCopyOut(struct o_event *, struct o_event *, int);
static int ObjectId(struct vnode *, struct AObject_t *);
static struct AObject_t * ObjectFind(struct AObject_t *, char *);
static int ObjectRegister(struct AObject_t *, struct AObject_t *);
int aud_vn_exec(struct vnode *);
int aud_vn_rdwr(struct vnode *, enum uio_rw, int, struct uio *, int, caddr_t);
int ObjectRdWr(struct vnode *, enum uio_rw);
int aud_vn_create(struct vnode *, struct vnode **, int, caddr_t, int, caddr_t *);
int ObjectMake(struct vnode *, struct vnode *, char *);
void ObjectClear(void);
static int ObjectInsert(struct AObject_t *);
static int ObjectHash(struct AObject_t *, int);
static unsigned int StringHash(char *Name);
static int CheckObjLock();

/* 
 * Internal struct for 
 * tracking objects 
 */

static struct AObject_t {
	char 	*path;
	char 	*filename;
	int	Class;
	long	Id;
	int	event[MAX_EVNTNUM];
	short   real;
	short	filenamehash;
	short   type;
	struct 	AObject_t *next;
	struct 	AObject_t *prev;
};

#define	AUDITOBJ(vp)	((struct AObject_t *) (vp)->v_audit)
#define AUDITOBJ_OFF	((struct AObject_t *) -1)
#define AUDITOBJ_NULL	((struct AObject_t *) 0)

/* 
 * Hash table
 */

static struct AObject_t *AObject[HASHLEN];	

/*                                                                              
 * NAME: auditobj
 *                                                                             
 * FUNCTION: Gets or sets the auditing modes of system data objects.
 *
 * INPUTS : 
 *	Cmd : Specifies whether the object audit event lists are to be 
 *	      read or written. The valid values are AUDIT_SET, AUDIT_GET
 *	      and AUDIT_LOCK
 *	Onum : Specifies the number of object audit events if Cmd=AUDIT_SET
 *	       Specifies the size of buffer if Cmd is AUDIT_GET or AUDIT_LOCK
 *
 * INPUT/OUTPUT:
 *	Oevents : Specifies the buffer that contains (AUDIT_SET) or will
 *		  contain (AUDIT_GET or AUDIT_LOCK) the list of object 
 *		  audit events. This buffer is an array of o_event 
 *		  structures.
 *
 * ERRORS:
 *	EPERM if the caller doesn't possess AUDIT_CONFIG privilege
 *	EINVAL if Cmd parameter is not one of AUDIT_SET  AUDIT_GET
 *	       and  AUDIT_LOCK
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	if Cmd=AUDIT_LOCK audobj_block is set to the PID of the process.
 *	It is reset when Cmd=AUDIT_SET
 *
 * RETURNS:
 *	-1 in case of error
 *	If the system call completes successfully, the number of object
 *	audit event definitions is returned if the Cmd parameter is 
 *	AUDIT_GET or AUDIT_LOCK; a value of 0 is returned if the Cmd
 *	parameter is AUDIT_SET. 
 *
 */                                                                            
int
auditobj(int Cmd, struct o_event *Oevents, int Onum){

	int rc = 0;

	/* 
	 * privilege check 
	 */

	if(privcheck (AUDIT_CONFIG)){
		u.u_error = EPERM;
		return (-1);
	}

	if (Cmd == AUDIT_SET) {

		/*
		 * Resetting the list of object, clear all the
		 * vnodes since tracked objects may change.  Do
		 * this before acquiring audit_lock since this
		 * function may get other locks.
		 */

		ObjectClear();
	}

	/* 
	 * lock audit for 
	 * setting up audit objects 
	 */

	simple_lock(&audit_lock);
	switch(Cmd){

		case AUDIT_SET:	

			if(CheckObjLock()){
				break;
			}

			ObjectsStore(Oevents, Onum);

			if(audobj_block){
				audobj_block = 0;
				e_wakeup(&audit_anchor.lock);   
			}

			break;

		case AUDIT_GET:	

			rc = ObjectsFetch(Oevents, Onum);

			break;

		case AUDIT_LOCK:	

			if(CheckObjLock()){
				break;
			}
                        audobj_block = thread_self();
			rc = ObjectsFetch(Oevents, Onum);

			break;

		default:
			u.u_error = EINVAL;
	}

	simple_unlock(&audit_lock);


	return (u.u_error ? -1 : rc);
}

/*                                                                              
 * NAME: ObjectsStore()
 *
 * FUNCTION: First clears up the objects table and then creates new objects
 *	     as specified in the input object audit events.
 *
 * INPUTS : 
 *	UsrObjects : List of object audit events
 *	Onum : Number of object audit events
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	NONE
 *
 * RETURNS:
 *	-1 if failed
 *	0 if Onum is <= 0
 *
 */
static
int 
ObjectsStore(struct o_event *UsrObjects, int Onum){

	struct o_event *Optr;
	struct o_event *KrnObjects = NULL;
	struct AObject_t *AObj;
	char *NameSpace = NULL;
	char *EventSpace = NULL;
	int i;

	/* 
	 * Clear out all old objects 
	 */

	ObjectsRemove();
	
	if(Onum <= 0){
		return(0);
	}

	if(ObjectsCopyIn(UsrObjects, &KrnObjects, &NameSpace,
	&EventSpace, Onum)){
		goto fail;
	}

	for(i = 0, Optr = KrnObjects; i < Onum; i++, Optr++){

		/* 
		 * Create Object 
		 */

		if((AObj = ObjectCreate(Optr)) == NULL){
			
			/* 
			 * Clean up hash table
			 */

			ObjectsRemove();
			goto fail;

		}
	}

fail:

	/* 
	 * Return memory
	 */

	if(KrnObjects){
		free(KrnObjects); 
	}
	
	if(NameSpace){
		free(NameSpace);
	}

	if(EventSpace){
		free(EventSpace);
	}

	return(-1);

}

/*
 * NAME: ObjectsCopyIn
 *
 * FUNCTION: Copies input object audit events from user to kernel space  
 *
 * INPUTS :
 *	UsrObjects : User space object audit events
 *	Onum : number of object audit events
 *
 * OUTPUTS :
 *	KrnObjects : kernel space object audit events
 *	Namespace : buffer which holds the pathnames of all object
 *		    audit definitions
 *	EventSpace : buffer which holds the event lists of all object
 *		     audit definitions
 *
 * ERRORS:
 *	ENOMEM if no kernel space could be allocated for objects
 *	EFAULT if sanity check fails
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *	NONE
 *
 * RETURNS:
 *	-1 on failure
 *	0 on success.
 *
 */
static
int 
ObjectsCopyIn(struct o_event *UsrObjects, 
struct o_event **KrnObjects, char **NameSpace, char **EventSpace, int Onum){

	struct o_event *Optr;
	int EventCount;
	int Siz; 
	int i; 
	int j; 
	char *Np; 
	char *Ep;

	/* 
	 * Bring in the o_event 
	 * structures 
	 */

	Siz = Onum * sizeof(struct o_event);
	if((*KrnObjects = (struct o_event *)malloc(Siz)) == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}
	bzero(*KrnObjects, Siz);
	if(copyin((caddr_t)UsrObjects, *KrnObjects, Siz)) {
		u.u_error = EFAULT;
		goto fail;
	}

	/* allocate maximum space for all o_name values
	 * this buffer will be deallocated after all AObjects's
	 * are created 
	 */

	if((*NameSpace = malloc(Onum * MAX_PATHSIZ)) == NULL) {
		u.u_error = ENOMEM;
		goto fail;
	}
	bzero(*NameSpace, Onum * MAX_PATHSIZ);

	/* 
	 * Now all memory is allocated.
	 * move in names 
	 */

	for(i = 0, Optr = *KrnObjects, Np = *NameSpace, 
	EventCount = 0; i < Onum; i++, Optr++){

		if(copyin(Optr->o_name, Np, MAX_PATHSIZ)){
			u.u_error = EFAULT; 
			goto fail; 
		} 

		*(Np + MAX_PATHSIZ - 1) = '\0';
		Optr->o_name = Np;
		Np += MAX_PATHSIZ;

		/* 
		 * count up all events through 
		 * this loop
		 */

		for(j = 0; j < MAX_EVNTNUM; j++){
			if(Optr->o_event[j]){
				EventCount++;
			}
		}
	}

	/* allocate space for all o_event values */
	if ((*EventSpace = malloc(EventCount * MAX_EVNTSIZ)) == NULL) {
		u.u_error = ENOMEM;
		goto fail;
	}
	bzero(*EventSpace, EventCount * MAX_EVNTSIZ);

	/* 
	 * Now get all event names that the 
	 * user is associated with this pathname 
	 */

	for(i = 0, Optr = *KrnObjects, Ep = *EventSpace; i < Onum; 
	i++, Optr++){

		for(j = 0; j < MAX_EVNTNUM; j++){

			if(Optr->o_event[j]){

				if(copyin(Optr->o_event[j], Ep, MAX_EVNTSIZ)){
					u.u_error = EFAULT; 
					goto fail;
				}

				*(Ep + MAX_EVNTSIZ - 1) = '\0';
				Optr->o_event[j] = Ep;
				Ep += MAX_EVNTSIZ;

			}
		}
	}

	return (0);

fail:

	return (-1);
}

/*                                                                              
 * NAME: ObjectCreate()
 *
 * FUNCTION: Allocates memory for the new object and inserts it in the 
 *	     object table. The events associated with the object are 
 *	     registered in the event symbol table if not already done.
 *
 * INPUTS : 
 *	Optr : events associated with the object
 *
 * OUTPUTS : NONE explicitly. Many globals are changed implicitly
 *
 * ERRORS:
 *	ENOMEM if malloc fails
 *	EINVAL if the type of object is not AUDIT_FILE or if the
 *	       path len of the object is NULL
 *
 * TYPE: static struct AObject_t *
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	oevent_total and object_total have to be incremented when an
 *	object is inserted in tne object table AObject_t.
 *
 * RETURNS:
 *	pointer to the object that has been inserted into the object table
 *	on success else NULL on failure
 *
 */
static struct AObject_t *
ObjectCreate(struct o_event *Optr){

	struct AObject_t *NewObject = NULL;
	char *Parent = NULL;		
	int Len;			
	int i;			

	/* 
	 * Do the allocation 
	 */

	if((NewObject = malloc(sizeof(struct AObject_t))) == NULL){
		u.u_error = ENOMEM;
		return(NULL);
	}
	bzero(NewObject, sizeof(struct AObject_t));

	/* 
	 * find length of path 
	 */

	Len = strlen(Optr->o_name) + 1;

	if((NewObject->path = malloc(Len)) == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}
	bzero(NewObject->path, Len);
	bcopy(Optr->o_name, NewObject->path, Len);
	
	if((NewObject->type = Optr->o_type) != AUDIT_FILE){
		u.u_error = EINVAL;
		goto fail;
	}

	/* 
	 * find last component in 
	 * path name for hashing by directory Id and filename
	 */

	if(!(Len = strlen(NewObject->path))){
		u.u_error = EINVAL;
		goto fail;
	}

	/* 
	 * Check for trailing slash
	 * Set to null if found
	 */

	if(NewObject->path[Len - 1] == '/'){
		NewObject->path[--Len] = '\0';
	}

	/* 
	 * Parse for just  the filename 
	 */

	for(Len--; Len > 0 && NewObject->path[Len] != '/'; Len--);

	if(Len != 0 || NewObject->path[Len] == '/'){
		Len++;
	}

	NewObject->filename = &(NewObject->path[Len]);

	/* 
	 * make a copy of the parent directory name for
	 * lookupname 
	 * This is if the object is imaginary we hash
	 * by its parent Id and filename
	 */

	if((Parent = malloc(Len + 1)) == NULL){
		u.u_error = ENOMEM;
		goto fail;
	}

	bzero(Parent, Len);
	bcopy(NewObject->path, Parent, Len);

	Parent[Len] = '\0';

	/* 
	 * Object could be real or imaginary
	 * Its Id will be tagged
	 * And its real or imaginary staus set by ObjectGetFid
	 */

	if(ObjectGetFid(Parent, NewObject->path, NewObject)){ 
		goto fail;
	}

	/* 
	 * Insert into hash table. The object is either
	 * real,  or imaginary with its parent directory
	 * and filename stored. Check redundancy.
	 */

	if(ObjectInsert(NewObject) < 0){
		goto fail;
	}

	/* 
	 * Hash all the associated event names 
	 * into the event hash table, so the associated events
	 * can be tracked
	 */

	for(i = 0; i < MAX_EVNTNUM; i++){

		if(Optr->o_event[i]){

			oevent_total++;

			NewObject->event[i] = AuditLookup(Optr->o_event[i]);
		}
		else {

			NewObject->event[i] = 0;

		}

	}

	object_total++;

	if(Parent){

		free(Parent);
		Parent = NULL;

	}

	return(NewObject);

fail:
	if(NewObject->path){

		free(NewObject->path);
		NewObject->path = NULL;

	}

	if(NewObject){

		free(NewObject);
		NewObject = NULL;

	}

	if(Parent){

		free(Parent);
		Parent = NULL;

	}

	return(NULL);
}

/*                                                                              
 * NAME: ObjectGetFid()
 *
 * FUNCTION: sets the Id of the new object after accessing the object's
 *	     vnode and sanity checks the input. Checks whether the object
 *	     is real or imaginary and sets the corresponding fields in the
 *	     AObject_t  structure appropriately.
 *
 * INPUTS : 
 *	Parent : Directory in which the object exists
 *	Path : Full path name of the object
 *	NewObject : Pointer to the object of type AObject_t
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS:
 *	EINVAL if the object is already present in the object table
 *	ENOENT if the object and its parent does nor exist
 *	EFAULT if the object type is none of VSOCK VREG and VDIR
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	Object's vnode is referenced to get the information regarding
 *	Id and to determine whether the object is real or imaginary.
 *
 * RETURNS:
 *	-1 in case of error
 *	0  in case of success.
 *
 */
static
int
ObjectGetFid(char *Parent, char *Path, struct AObject_t *NewObject){

	struct vnode *Vp = NULL;	
	struct vnode *VpPdir = NULL;	
	struct ucred *crp;
	int do_it;

	/* if we already hold the cred lock, for example, we came
	   through the setuid() call to cut an audit record, then we 
	   will crash if we call crref, which also needs that lock.  So
	   check first.  If we don't have the lock, then it's safe
	   to call crref; else if we do have the lock, then we
	   can examine the cred structure directly. */

	do_it = lock_mine(&cred_lock);
	if (!do_it)
		crp = crref();
	else
		crp = U.U_cred;

	/* 
	 * get component and 
	 * directory vnode's 
	 */

	if(!lookupname(Path, SYS, L_NOFOLLOW, &VpPdir, &Vp, crp)){ 

		/* 
		 * object is real 
		 */

		if(!ObjectId(Vp, NewObject)){

			u.u_error = EFAULT;
			goto fail;

		}

		/* 
		 * Sanity check on input
		 */

		if(ObjectFind(NewObject, NULL)){

			u.u_error = EINVAL;
			goto fail;

		}

		NewObject->real = 1;

	}
	else {
		if(lookupname(Parent, SYS, L_NOFOLLOW, &VpPdir, &Vp, crp)){ 
			u.u_error = ENOENT;
			goto fail;
		}

		if(!Vp){
			u.u_error = ENOENT;
			goto fail;
		}

		/* 
		 * we have a Id for only the parent 
		 * object is imaginary
		 */

		if(!ObjectId(Vp, NewObject)){

			u.u_error = EFAULT;
			goto fail;

		}

		/* 
		 * Sanity check on input
		 */

		if(ObjectFind(NewObject, NewObject->filename)){

			u.u_error = EINVAL;
			goto fail;

		}

		NewObject->real = 0;
	}

fail:

	/* 
	 * release these vnodes
	 */

	if(VpPdir){
		VNOP_RELE(VpPdir);
	}

	if(Vp){
		VNOP_RELE(Vp);
	}

	if (!do_it) crfree(crp);

	return(u.u_error ? -1 : 0);
}

/*
 * NAME: ObjectsRemove()
 *
 * FUNCTION: Free  the memory of the audit object table
 *
 * INPUTS : NONE
 *
 * OUTPUTS : NONE
 *
 * ERRORS: None
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	Since the object table is deallocated the variables associated
 * 	with it have to be reset. Thus oevent_total and object_total
 *	are made 0. The object table  AObject_t is freed.
 *
 * RETURNS: NOTHING
 *
 */
static int
ObjectsRemove(){

	struct	AObject_t *Current;
	struct	AObject_t *Saved;
	int 	i;

	oevent_total = 0;
	object_total = 0;

	/* 
	 * All objects are in AObject 
	 * lets walk the hash table 
	 * freeing memory
	 */

	for(i = 0; i < HASHLEN; i++){

                Current = Saved = AObject[i];
                AObject[i] = (struct AObject_t *)NULL;

                while(Saved){

                        if(Current->path){

                                free(Current->path);
				Current->path = NULL;

                        }

                        Saved = Current->next;
                        free(Current);
                        Current = Saved;

		}
	}

}

/*                                                                              
 * NAME: ObjectsFetch()
 *
 * FUNCTION: Fetches the object audit events
 *
 * INPUTS : 
 *	Onum : size of buffer in which the object events are to be held
 *
 * OUTPUTS :
 * 	UsrEvents : Object audit events fetched from kernel space.
 *
 * ERRORS:
 *	ENOSPC if the size of the buffer is less than the total 
 *	       size of all audit object events.
 *	EFAULT if sanity check fails
 *	ENOMEM if malloc fails
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures,
 *	NONE
 *
 * RETURNS:
 *	-1 on failure
 *	number of objects on success
 *
 */
static
int
ObjectsFetch(struct o_event *UsrEvents, int Onum){

	struct	o_event *KrnEvents = NULL;
	int	MemSize;
	int	rc = object_total;

	if(object_total == 0)return(rc);

  	/* 
	 * if user's buffer is too small then return the size
 	 * needed (in the first word of the user's buffer)
 	 * note:  Onum is size of user's buffer 
	 */

	MemSize = object_total * (sizeof(struct o_event) + MAX_PATHSIZ) 
        + oevent_total * MAX_EVNTSIZ;

	if(MemSize > Onum){

		u.u_error = ENOSPC;

		rc = -1;

		if(copyout(&MemSize, UsrEvents, 4)){
			u.u_error = EFAULT;
			rc = -1;
			goto fail;
		}

		goto fail;
	}

 	/* 
	 * allocate the buffer for 
	 * the objects
	 */

	if((KrnEvents = (struct o_event *)malloc(MemSize)) == NULL){
		u.u_error = ENOMEM;
		rc = -1;
		goto fail;
	}
	bzero(KrnEvents, MemSize);

 	/* 
	 * Copy the event symbol table into event buffer 
	 */

	if((ObjectsCopyOut(KrnEvents, UsrEvents, MemSize)) < 0) {
		u.u_error = EFAULT;
		rc = -1;
		goto fail;
	}

fail:

	if(KrnEvents){

		free(KrnEvents);

	}

	return(u.u_error ? -1 : rc);
}

/*
 * NAME: ObjectsCopyOut()
 *
 * FUNCTION: This function fetches the objects from the object table into
 *	     kernel space and then copies them into user space.
 *
 * INPUTS : 
 *	Memsize : Total kernel space allocated
 *
 * OUTPUTS : 
 *	UsrEvents : User space audit object events
 *	KrnEvents : Kernel space  audit object events
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	References AObject_t table and object_total and oevent_total
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */
static
int
ObjectsCopyOut(struct o_event *KrnEvents, struct o_event *UsrEvents,
int MemSize){

	char	*KrnPtr;
	char	*UsrPtr;
	int	chk_objcnt;
	int	chk_evcnt;
	struct	o_event *Op;
	struct	AObject_t *Ao;
	int	i, j;
	int	diff;
	
	chk_objcnt = 0;
	chk_evcnt = 0;

	/* 
	 * Cp points beyond the area for o_event structs 
	 * where names will be copied 
	 */

	KrnPtr = (char *)&(KrnEvents[object_total]);
	UsrPtr = (char *)&(UsrEvents[object_total]);

	/* 
	 * Find all objects by traversing the AObject table 
	 */

	for(i = 0, Op = KrnEvents; i < HASHLEN; i++) {
		for(Ao = AObject[i]; Ao; Op++, Ao = Ao->next){
			chk_objcnt++;

			if(object_total < chk_objcnt){
				return (-1);
			}

			strcpy(KrnPtr, Ao->path);

			Op->o_name = UsrPtr;
			Op->o_type = Ao->type;

			KrnPtr += MAX_PATHSIZ;
			UsrPtr += MAX_PATHSIZ;

			for(j = 0; j < MAX_EVNTNUM; j++){
				if(Ao->event[j]){
					chk_evcnt++;

					if(oevent_total < chk_evcnt){
						return (-1);
					}
					strcpy(KrnPtr, 
					audit_getname(Ao->event[j]));

					Op->o_event[j] = UsrPtr;

					KrnPtr += MAX_EVNTSIZ;
					UsrPtr += MAX_EVNTSIZ;

				}
				else Op->o_event[j] = 0;
			}
		}

	}

        if(copyout(KrnEvents, UsrEvents, MemSize)){
		return(-1);
	}

	return (0);
}

/*
 * NAME: ObjectId()
 *
 * FUNCTION: Sets the Id of the object. The information is obtained 
 *	     from the objects vnode.
 *
 * INPUTS :
 *	Vp : Object's vnode
 *
 * INPUT/OUTPUT :
 *	Object : Object of type AObject_t.Object Id and class is set
 *	 	 in this structure.
 *	
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * DATA STRUCTURES: 
 *	References the vnode fields of the object to get its 
 *	attributes like Id and class and set these values in the
 *	AObject_t structure. 
 *
 * RETURNS:
 *	0 on failure
 *	1 on success
 *
 */
static
int 
ObjectId(struct vnode *Vp, struct AObject_t *Object){
	
	struct vattr Vattr;
	struct ucred *crp;

	bzero(&Vattr, sizeof(Vattr));

	/* if we already hold the cred lock, for example, we came
	   through the setuid() call to cut an audit record, then we 
	   will crash if we call crref, which also needs that lock.  So
	   check first.  If we don't have the lock, then it's safe
	   to call crref; else if we do have the lock, then we
	   can examine the cred structure directly. */

	if (!lock_mine(&cred_lock))
	{
		crp = crref();
		VNOP_GETATTR(Vp, &Vattr, crp);
		crfree(crp);
	}
	else
	{
		crp = U.U_cred;
		VNOP_GETATTR(Vp, &Vattr, crp);
	}

	switch(Vattr.va_type){ 

		case VSOCK:

			Object->Class = 0;
			Object->Id = Vattr.va_serialno;

			return(1);

		case VREG:
		case VDIR:

			Object->Class = Vp->v_vfsp->vfs_number;
			Object->Id = Vattr.va_serialno;

			return(1);
	
		default:
			
			return(0);
	}
}

/*
 * NAME: ObjectFind()
 *
 * FUNCTION: Search for the Object whose name is given as input in the 
 *	     AObject object table and if it is found return the pointer
 *	     to it else return NULL.
 *
 * INPUTS :
 *	Object : Object of type AObject_t
 * 	Name : Name of the object
 *	Both of these inputs are needed to hash into the object table
 *	by object Id, class and name.
 *
 * OUTPUTS : return pointer to the object if found in the object table
 *	     else NULL
 *
 * ERRORS: NONE
 *
 * TYPE: struct AObject_t *
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	Hashes into the AObject object table to search for the object.
 *
 * RETURNS: Returns the pointer to the object in the hashing list on
 *	    success else NULL.
 *
 */
static
struct AObject_t *
ObjectFind(struct AObject_t *Object, char *Name){

	struct AObject_t *Ap;
	int filenamehash = 0;
	int value;

	/* if Name is not NULL
	 * then the object is imaginary
	 * and its hash value was determined
	 * by its parent's Id and it hashed Name
	 */

	if(Name){
		filenamehash = StringHash(Name);
	}
	value = ObjectHash(Object, filenamehash);

	for(Ap = AObject[value]; Ap; Ap = Ap->next){

		/* 
		 * Are we trying to look up an
		 * imaginary or a real object?
		 */

		if(Ap->real){

			if((Ap->Id == Object->Id) &&
			(Ap->Class == Object->Class)){

				return(Ap);

			}
		}
		else {

			if((Ap->Id == Object->Id) &&
			(Ap->Class == Object->Class) &&
			(Ap->filenamehash == filenamehash)){

				return(Ap);

			}
		}
	}

	return (NULL);
}
 
/*                                                                              
 * NAME: ObjectRegister
 *                                                                             
 * FUNCTION: Registers new objects on the object list   
 *
 * INPUT: 
	ImagObj : The imaginary object
	Container : The real object to register
 *
 * TYPE: int
 *                                                                             
 * RETURNS: 1                                                                   
 *
 */                                                                            
static
int
ObjectRegister(struct AObject_t *ImagObj, struct AObject_t *Container){

	struct AObject_t *Next;
	struct AObject_t *Head;
	struct AObject_t *RealObj;
	int value;

	/* 
	 * This is somewhat tricky
	 * We will "unregister" the 
	 * imaginary object by
	 * pulling him out of the hash table.
	 * We will then "register" the real object
	 */
	
	/* 
	 * Down on the linked list
	 * this is the easiest case
	 */

	if(ImagObj->prev){
		ImagObj->prev->next = ImagObj->next;

		if(ImagObj->next){
			ImagObj->next->prev = ImagObj->prev;
		}
	}
	
	/* 
	 * Top of the linked list
	 * We now have to update the hash table
	 * and remove node from linked list
	 */

	else {
		value = ObjectHash(ImagObj, ImagObj->filenamehash);

		AObject[value] = ImagObj->next;

		if(ImagObj->next){
			ImagObj->next->prev = NULL;
		}
	}

	/* 
	 * Hash real object into object table to top 
	 * of linked list
	 * Real now assumes Imaginary 
	 */

	RealObj = ImagObj;
	RealObj->filenamehash = 0;
	RealObj->real = 1;

	value = ObjectHash(Container, 0);

	Head = AObject[value];

	if(Head){
		Head->prev = RealObj;
	}

	/* insert into
	 * linked list at the hashed value
	 * always at the head 
	 */

	RealObj->Class = Container->Class;
	RealObj->Id = Container->Id;

	RealObj->next = Head;
	RealObj->prev = NULL;

	AObject[value] = RealObj;

	return(1);
}

/*                                                                              
 * NAME: aud_vn_exec
 *                                                                             
 * FUNCTION: finds audit object exec events and writes them 
 *             
 * INPUT: Vp : a pointer to the event list                   
 *                                                                             
 * TYPE: int                           
 *
 * RETURNS:                                                                    
 *
 */                                                                            
int
aud_vn_exec(struct vnode *Vp){

	struct AObject_t Object;
	struct AObject_t *Ao = AUDITOBJ_NULL;
	int Temp;
	int tlock;

	/*
	 * just return if auditing suspended
	 */
	if (U.U_auditstatus != AUDIT_RESUME) 
		return;

	/*
	 * If object auditing is disabled for this vnode, just return.
	 */

	if (AUDITOBJ (Vp) == AUDITOBJ_OFF) 
		return;

	/*
	   Now we do an ObjectId() to get the status of the
	   Object. We don't want to do the ObjectId() under the lock,
	   because if the object is an NFS-file, this operation can hang,
	   and we don't want to hang with the audit_lock in our hands...
	   (see defect 175662).
	 */

	if (AUDITOBJ (Vp) == AUDITOBJ_NULL)
		Temp = ObjectId(Vp, &Object);

	/*
	 * Don't know the status of this object, see if this object
	 * has been registered and cache the response from ObjectFind()
	 * if so.  This saves the effort of ObjectId, which can be
	 * quite expensive for NFS files.
	 */

	if (!(tlock=lock_mine(&audit_lock))) simple_lock(&audit_lock);

	/*
	 * Must be retested after locking: defect 165606
	 */
	if (AUDITOBJ (Vp) == AUDITOBJ_OFF) {
		if (!tlock)
			simple_unlock(&audit_lock);
		return;
  	}

	if (AUDITOBJ (Vp) == AUDITOBJ_NULL) {
		if(Temp){

			Ao = ObjectFind(&Object, NULL);

		}
		if (Ao == AUDITOBJ_NULL) {
			Vp->v_audit = (char *) AUDITOBJ_OFF;

			if (!tlock)
				simple_unlock(&audit_lock);

			return;
		}
		Vp->v_audit = (char *) Ao;

	/*
	 * ObjectFind() cached this the last time we looked.  Let's just
	 * return that pointer.
	 */

	} else
		Ao = (struct AObject_t *) Vp->v_audit;

	if(Ao){
		int	exec_event;
		char	local_path[PATH_MAX];

		exec_event = Ao->event[AUDIT_EXEC];
		strcpy (local_path, Ao->path);
		if (!tlock) simple_unlock(&audit_lock);
		if ( exec_event ) {
			audit_write(exec_event, 
			AUDIT_OK, "audit object exec event detected ",
			33, local_path, 
			strlen(local_path), 0);
		}
	} else
		if (!tlock) simple_unlock(&audit_lock);
	
		
}
 
/*                                                                              
 * NAME: aud_vn_rdwr , ObjectRdWr 
 *                                                                             
 * FUNCTION: finds audit object read-write (aud_vn_rdwr) and writes
 *		it (ObjectRdWr)                      
 *
 *      INPUT: Vp : a pointer to the event list
 *             op : the operation performed by the event ( READ, WRITE )
 *
 * TYPE: int (both)                                                                      
 *
 * RETURNS: 0
 *
 */                                                                            
int
aud_vn_rdwr(struct vnode *Vp, enum uio_rw op, int flags, struct uio *uiop, 
int ext, caddr_t vinfo){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, op);
	}

	return(0);
}

int
aud_vn_readdir(struct vnode *Vp, struct uio *uiop, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, UIO_READ);
	}

	return (0);
}

int
aud_vn_link(struct vnode *Vp, struct vnode *Dp, char *tnm, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Dp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_mkdir(struct vnode *Vp, char *nm, int mode, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_mknod(struct vnode *Vp, char *nm, int mode, dev_t dev, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_remove(struct vnode *Vp, struct vnode *Dp, char *nm, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_rename (struct vnode *vp, struct vnode *dp, caddr_t nm,
	       struct vnode *tp, struct vnode *tdp, caddr_t tnm,
	       struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(dp, UIO_WRITE);
		ObjectRdWr(tdp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_rmdir(struct vnode *Vp, struct vnode *Dp, char *nm, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Dp, UIO_WRITE);
	}

	return (0);
}

int
aud_vn_symlink(struct vnode *Vp, char *lnm, char *tnm, struct ucred *crp){

	/*
	 * only log if not suspended
	 */
	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr(Vp, UIO_WRITE);
	}

	return (0);
}

int
ObjectRdWr(struct vnode *Target, enum uio_rw Op){

	struct AObject_t *Ao = NULL;
	struct AObject_t Object;
	int Temp;
	int tlock;

	/*
	 * If object auditing is disabled for this vnode, just return.
	 */

	if (AUDITOBJ (Target) == AUDITOBJ_OFF) 
	    return;


	/*
	   Now we do an ObjectId() to get the status of the
	   Object. We don't want to do the ObjectId() under the lock,
	   because if the object is an NFS-file, this operation can hang,
	   and we don't want to hang with the audit_lock in our hands...
	   (see defect 175662).
	 */

	if (AUDITOBJ (Target) == AUDITOBJ_NULL)
		Temp = ObjectId(Target, &Object);
	/*
	 * Don't know the status of this object, see if this object
	 * has been registered and cache the response from ObjectFind()
	 * if so.  This saves the effort of ObjectId, which can be
	 * quite expensive for NFS files.
	 */

	if (!(tlock=lock_mine(&audit_lock))) simple_lock(&audit_lock);

	/*
	 * Must be retested after locking: defect 165606
	 */
	if (AUDITOBJ (Target) == AUDITOBJ_OFF) {
		if (!tlock)
			simple_unlock(&audit_lock);
		return;
  	}

	if (AUDITOBJ (Target) == AUDITOBJ_NULL) {
		if(Temp){

			Ao = ObjectFind(&Object, NULL);

		}
		if (Ao == AUDITOBJ_NULL) {
			Target->v_audit = (char *) AUDITOBJ_OFF;

			if (!tlock)
				simple_unlock(&audit_lock);

			return;
		}
		Target->v_audit = (char *) Ao;
	} else

	/*
	 * ObjectFind() cached this the last time we looked.  Let's just
	 * return that pointer.
	 */

		Ao = (struct AObject_t *) Target->v_audit;

	if(Ao){
		int	write_event = Ao->event[AUDIT_WRITE],
			read_event = Ao->event[AUDIT_READ];
		char	local_path[PATH_MAX];

		strcpy (local_path, Ao->path);
		if (!tlock) simple_unlock(&audit_lock);

		if((Op == UIO_WRITE) && write_event){
			audit_write(write_event, 
			AUDIT_OK, "audit object write event detected ",
			34, local_path, strlen(local_path), 0);

		}


		if((Op == UIO_READ) && read_event){

			audit_write(read_event, 
			AUDIT_OK, "audit object read event detected ",
			33, Ao->path, strlen(Ao->path), 0);

		}

	} else
		if (!tlock) simple_unlock(&audit_lock);

}

/*                                                                              
 * NAME:  aud_vn_create / ObjectMake
 *                                                                             
 * FUNCTION: Create an object for each event being tracked 
 *
 * INPUT: Dp: Parent of the event being tracked
 *        Vpp : Target object event list                         
 *        Name : name of the event
 * TYPE: int (both)                
 *
 * RETURNS: 0
 *
 */                                                                            
int
aud_vn_create(struct vnode *Dp, struct vnode **Vpp, int flags, 
caddr_t Name, int mode, caddr_t *vinfo){


	/* Now look this up by the hashed parent
	 * Class, Id and file name. If it is 
	 * being tracked then use Class, Id
	 * of the object and hash it into the table.
	 * Don't audit either of these if the process
	 * has suspended auditing.
	 */

	if (U.U_auditstatus == AUDIT_RESUME) {
		ObjectRdWr (Dp, UIO_WRITE);
		ObjectMake(Dp, *Vpp, Name);
	}

	return(0);
}

int
ObjectMake(struct vnode *Parent, struct vnode *Target, char *Name){

	struct AObject_t *Ao = NULL;
	struct AObject_t Object;
	int tlock;

	if (!(tlock=lock_mine(&audit_lock))) simple_lock(&audit_lock);

	if(ObjectId(Parent, &Object)){

		Ao = ObjectFind(&Object, Name);

	}

	if(Ao){

		if(ObjectId(Target, &Object)){

			if(ObjectRegister(Ao, &Object)){

				Target->v_audit = (char *)Ao;
			}
		}
	}
	if (!tlock) simple_unlock(&audit_lock);
}
 
/*                                                                              
 * NAME: ObjectInsert
 *                                                                             
 * FUNCTION: Inserts a new object to the object list                      
 *
 * INPUT: Ao : The object we want to insert                     
 *                                                                             
 * TYPE: int                           
 *
 * RETURNS: 0                                                                   
 *
 */                                                                            
static
int
ObjectInsert(struct AObject_t *Ao){

	struct	AObject_t *Head = NULL;
	int value;
	
	if(Ao->real){

		value = ObjectHash(Ao, 0);

	}
	else {

		Ao->filenamehash = StringHash(Ao->filename);
		value = ObjectHash(Ao, Ao->filenamehash);

	}

	/* 
	 * Always put new object at head of 
	 * link list 
	 */

	Head = AObject[value];

	if(Head){

		Head->prev = Ao;

	}

	Ao->next = Head;
	Ao->prev = NULL;

	AObject[value] = Ao;

	return(0);
}

/*                                                                              
 * NAME:  ObjectHash
 *                                                                             
 * FUNCTION: Hashes the object in the object table by Class and ID
 *
 * INPUT: Object : The object we put in the table
 *	  filenamehash :  the filename to hash with ( if wanted only )         
 *                                                                             
 * TYPE: int                           
 *
 * RETURNS: new place of object in list                       
 *
 */                                                                            
static
int
ObjectHash(struct AObject_t *Object, int filenamehash){

	int value;

	value = Object->Id + Object->Class;

	if(filenamehash){

		value |= filenamehash;

	}

	return(value & (HASHLEN - 1));
}

/*                                                                              
 * NAME: StringHash
 *                                                                             
 * FUNCTION: given a file name , returns the hash value                      
 *
 * INPUT: Name: The file name                   
 *                                                                             
 * RETURNS: The hashed value                       
 *
 */                                                                            
static
unsigned int
StringHash(char *Name){

	unsigned int hashval;

	for(hashval = 0; *Name != '\0'; *Name++){

		hashval = (unsigned int)*Name + 31 * hashval;

	}

	return(hashval & (HASHLEN - 1));
}
/*                                                                              
 * NAME: CheckObjLock
 *                                                                             
 * FUNCTION: Checks if audit object sub-subsystem is blocked ( being used )
 *
 * INPUT: NONE                                                               
 *                                                                             
 * ERRORS: EBUSY if blocked                        
 *
 * TYPE: int                           
 *
 * DATA STRUCTURES: audobj_block is read        
 *                                                                             
 * RETURNS: 0 if not blocked , 1 if blocked                       
 *
 */                                                                            
static
int
CheckObjLock(){

        struct thread   *tp;

	if((audobj_block == 0) || (audobj_block == curthread->t_tid)){

		return(0);
	}

 	tp = VALIDATE_TID(audobj_block);
	if(tp == NULL){
		audobj_block = 0;
		return(0);
	}
	if(tp->t_state != TSRUN){
		audobj_block = 0;
		return(0);
	}
	u.u_error = EBUSY;
	return(-1);
}

/*
 * Name: ObjectClear, ClearVFS, ClearVnode, ClearVaudit
 *
 * Function: Reset the v_audit field for all vnodes
 */

static int
ClearVnode (struct vnode * Vp)
{
	Vp->v_audit = (char *) AUDITOBJ_NULL;
	/*
	 * defect 167479: ClearVnode() must return 0 otherwise
	 *	- vn_search stops after the 1st vnode,
	 *	- vfs_search stops after the 1st vfs,
	 *     => ObjectClear() clears 1 vnode only ...
	 */
	return(0);
}

static void
ClearVFS (struct vfs * Vfs)
{
	vn_search (Vfs, (int (*)()) ClearVnode, (caddr_t) 0);
}

static void
ObjectClear ()
{
	vfs_search ((int (*)()) ClearVFS, (caddr_t) 0);
}
