static char sccsid[] = "@(#)26        1.25.1.2  src/bos/kernel/s/aud/auditsym.c, syssaud, bos411, 9428A410j 2/24/94 17:04:46";

/*
 * COMPONENT_NAME: (SYSSAUD) Auditing Management
 *
 * FUNCTIONS: auditsym() system call
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
#include	<sys/proc.h>
#include	<sys/systm.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/malloc.h>
#include	<sys/audit.h>
#include	<sys/auditk.h>
#include        <sys/lock_alloc.h>
#include        <sys/lockname.h>

 
/*
 * Forward declarations 
 */

static int AuditAlloc(struct base_events **, int *);
static int AuditGrow();
static int hash(unsigned char *);
static int hashins(struct base_events *);
int AuditLookup(char *);
int audinit();
int AuditReset();
int AuditOn();
int AuditGet(struct audit_class *, int);
int AuditRetrieve(struct audit_class *, struct audit_class *);
int AuditClassAdd(struct audit_class *);
char *audit_getname(int);

#define	round(n,m)	((((n)+((m)-1))/(m))*(m))

/*
 * Reset for audit shut downs
 */

static int Reset = 1;


/*
 * NAME: AuditAlloc()
 *
 * FUNCTION: Allocates memory for the new events symbol table rounded
 *	     upto the pagesize.
 *
 * INPUTS/OUTPUTS :
 * 	table : pointer to the allocated memory
 *	n     : number of base_event structures allocated
 *
 * ERRORS:
 *	ENOSPC if size of memory to be allocated exceeds 48k
 *	ENOMEM if malloc fails
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	NONE
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */
static
int
AuditAlloc(struct base_events **table, int *n){

	register int	siz;
	char	*p;

	/*
	 * Round the requested space up to a page multiple 
	 */

	siz = *n;
	siz *= sizeof(struct base_events);
	if(siz > MAX_TABSIZ){
		u.u_error = ENOSPC;
		return (-1);
	}
	siz = round(siz, PAGESIZE);

	/*
	 * Allocate full pages 
	 */

	p = palloc(siz, PGSHIFT);
	if(p == NULL){
		u.u_error = ENOMEM;
		return (-1);
	}

	bzero(p, siz);

	/*
	 * Return is by reference 
	 */

	*table = (struct base_events *)p;
	*n = siz / sizeof(struct base_events);
	return(0);
}


/*
 * NAME: AuditGrow()
 *
 * FUNCTION: This function increases the size of the event symbol table
 * 	     by allocating memory for the new symbol table and  copying 
 *	     old symbol contents into the new one. Finally it frees the 
 *	     old symbol table
 *	     
 * INPUTS : NONE
 *	
 * OUTPUTS : NONE
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	The size of the event symbol table be_symtab is raised.
 *	nevents now equals new size which is one more than the number
 *	of events that existed previously
 *
 * RETURNS:
 *	-1 on failure
 *	0 on success
 *
 */
static
int
AuditGrow(){

	struct	base_events	*newtab;
	int	newsiz;
	int	rc;

	newsiz = nevents + 1;

	/*
	 * Newsiz is updated by AuditAlloc
	 */

	rc = AuditAlloc(&newtab, &newsiz);
	if (rc < 0) 
		return (-1);

	/*
	 * Copy events to new table 
	 */

	bcopy(be_symtab, newtab, cevent*sizeof(struct base_events));

	/*
	 * Release the old table 
	 */

	free((char *)be_symtab);

	/*
	 * Install the new table 
	 */

	be_symtab = newtab;
	nevents = newsiz;

	return(0);
}


/*
 * NAME: hash()
 *
 * FUNCTION: returns the hash index of the string
 * Hashing funtion of the string = ((SUM of all chars in the string EXOR 
 *				   (EXOR of all chars)) AND
 *				   (HASHLEN))
 *	EXOR stands for exclusive OR.
 *	AND stands for bit-wise AND (&)
 *
 * INPUTS :
 *	s : string to which hashing function is to be applied.
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures
 *	NONE
 *
 * RETURNS:
 *	hash index of the string
 *
 */
static
int
hash(unsigned char *s){

	int	h = 0;
	int	x = 0;

	while(1){
		unsigned char	c;

		c = *s++;
		if(c == '\0'){
			return((h^x) & (HASHLEN-1));
		}
		h += c;
		x ^= c;
	}
}


/*
 * NAME: hashins()
 *
 * FUNCTION: This function introduces the event given as input into
 *	     the hashing table.
 *
 * INPUTS :
 *	sp : event of type base_events
 * OUTPUTS :
 *	NONE explicitly. The hash table is bumped.
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	The event is introduced into the hashing table which is global
 *
 * RETURNS: NOTHING explicitly
 *
 */
static
int
hashins(struct base_events *sp){

	struct	base_events	*next;
	int	h;

	h = hash(sp->be_name);

	next = hashtab[h];
	if(next)next->be_prev = sp;

	sp->be_next = next;
	sp->be_prev = NULL;
	hashtab[h] = sp;
}


/*
 * NAME: AuditLookup()
 *
 * FUNCTION: Looks for the given event in the base events symbol table.
 *	     If found, returns the index of the event in the symbol table
 *	     else inserts the event in the symbol table by increasing the
 *	     symbol table size if needed. The inserted event is also
 *	     included in the hashing table.
 *
 * INPUTS : 
 *	s : event name
 *
 * OUTPUTS : NONE explicitly. Index of base event is returned.
 *
 * ERRORS: NONE
 *
 * TYPE: static int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	In case the event name is not already present in the symbol table,
 *	it is inserted into it and the has table.
 *
 * RETURNS:
 *	-1 in case of error.
 *	event index in case of success.
 *
 */
int
AuditLookup(char *s){

	int	h;
	struct	base_events	*sp;

	h = hash(s);
	sp = hashtab[h];

	while(sp){

		if(strncmp(sp->be_name, s, 15) == 0){

			/*
			 * Gets added to the ALL class
			 * be_total_len gets bumped
			 */
			
			sp->be_bitmap |= (ulong)(1 << ALL_CLASS);
			be_total_len += strlen(sp->be_name) + 1;

			return(sp - be_symtab);
		}

		sp = sp->be_next;
	}

	/* 	
	 * Event not found so insert 
	 */

	if(cevent == nevents){

		if(AuditGrow() < 0){

			return(-1);
		}

	}

	sp = &(be_symtab[cevent++]);
	bzero(sp->be_name, 16);
	bcopy(s, sp->be_name, MIN(strlen(s)+1, 16));
	sp->be_name[15] = '\0';

	/*
	 * Add new event to ALL class and bump total length 
	 */

	sp->be_bitmap |= (ulong)(1 << ALL_CLASS);
	be_total_len += MIN(strlen(s) + 1, 16);

	hashins(sp);
	return(cevent - 1);
}


/*
 * NAME: audinit()
 *
 * FUNCTION: Allocates memory for the events symbol table, initializes
 *	     class_names array and sets of exception handler. It also
 *	     registers first two events as audit_void0 and audit_void1.
 *	     It also initializes the 31st element in the class_names 
 *	     array to "ALL"( ALL class)
 *
 * INPUTS : NONE
 *
 * OUTPUTS : NONE explicitly.
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 * 	Initializes be_symtab and class_names.
 *
 * RETURNS:
 *	-1 incase of error.
 *
 */
int
audinit(){

	nevents = 1024;
	cevent = 0;

	/*
	 * Initialize audit system locks 
	 */
	AllocAudLocks();

	if((AuditAlloc(&be_symtab, &nevents)) < 0){

		return (-1);
	}

	/*
	 * Initialize bit 31 to ALL_CLASS 
	 */

	bzero(class_names[MAX_ANAMES-1], 16);
	bcopy("ALL",class_names[MAX_ANAMES-1], 16);
	be_total_len = ALL_SIZE;

	/*
	 * Void out the first two fields svcnum = 0 or 1 confict 
	 */

	AuditLookup("audit_void0");
	AuditLookup("audit_void1");

	be_symtab[0].be_bitmap = 0;
	be_symtab[1].be_bitmap = 0;

}


/*
 * NAME: AuditReset()
 *
 * FUNCTION: resets auditing by zeroing the class_names array and 
 *	     registering only "ALL" as a class and by setting the
 *	     the be_bitmap field of all events in the symbol table
 *	     to the ALL class. It also disables fork and exit handler.
 *
 * INPUTS: NONE
 *
 * OUTPUTS: NONE explicitly
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	class_names and be_symtab 
 *
 * RETURNS: NONE explicitly
 *
 */
int
AuditReset(){

	int	i;
	struct	base_events	*bep;

	be_total_len = 0;
	Reset = 1;

	/*
	 * Zero out array of class names 
	 */

	bzero((char *)class_names, sizeof(class_names));

	/*
	 * Initialize bit 31 to ALL_CLASS 
	 */

	bzero(class_names[MAX_ANAMES-1], 16);
	bcopy("ALL",class_names[MAX_ANAMES-1], 16);
	be_total_len += ALL_SIZE;

	/*
	 * Disable auditing of kernel events, but keep name, next, prev 
	 * keep event in ALL_CLASS.
	 */

	for(i = 0, bep = be_symtab; i < cevent; i++, bep++){

		bep->be_bitmap = (ulong)(1 << ALL_CLASS);

	}
}

/*
 * NAME: AuditOn()
 *
 * FUNCTION: It turns the ALL_CLASS bit on all events and bumps
 *	     be_total_len
 *
 * INPUT: NONE
 *
 * OUTPUTS : NONE explicitly
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	be_symtab event table.
 *
 * RETURNS: NONE explicitly
 *
 */
int
AuditOn(){

	struct	base_events	*bep;
	int	i;

	/*
	 * Coming from an "audit start"
	 * turn on ALL_CLASS bit on all 
	 * events and bump be_total_len 
	 */

	if(Reset){

		for(i = 0, bep = be_symtab; i < cevent; i++, bep++){

			bep->be_bitmap |= (ulong)(1 << ALL_CLASS);
			be_total_len += strlen(bep->be_name) + 1;

		}

		Reset = 0;

	}
}


/*
 * NAME: AuditGet()
 *
 * FUNCTION: get all events from kernel space to user space. Do sanity
 *	     checking
 *
 * INPUTS : 
 *	numclasses : number of audit classes
 *
 * OUTPUT:
 *	uclasses : user space buffer which holds the audit_class
 *		   structures.
 *
 * ERRORS: 
 *	ENOSPC if be_total_len > numclasses
 *	EFAULT if sanity checking fails
 *	ENOMEM if malloc fails
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	NONE
 *
 * RETURNS:
 *	-1 on error
 *	number of events on success
 *	0 if auditting is reset.
 *
 */
int
AuditGet(struct audit_class *uclasses, int numclasses){

	struct	audit_class *classes = NULL;
	struct	audit_class *ep;
	int	ne = -1;	
	int	addr_diff;
	int	i;

	/*
	 * ALL_SIZE is the initial size given to the implicit 
	 * ALL_CLASS 
	 */

	if(Reset || (numclasses == 0)){

		if(suword(uclasses, 0) < 0){
			u.u_error = EFAULT;
			return(-1);
		}
		return (0);

	}
	else if(numclasses < sizeof(int)){

		u.u_error = EFAULT;
		return (-1);

	}
	else if(be_total_len > numclasses){

		u.u_error = ENOSPC;
		if(suword(uclasses, be_total_len) < 0){
			u.u_error = EFAULT;
		}
		return (-1);

	}

	/*
	 * Bad address 
	 */

	if(suword(uclasses, be_total_len) < 0){
		u.u_error = EFAULT;
		return (-1);
	}

	if((classes = (struct audit_class *)malloc(be_total_len))==NULL){
		u.u_error = ENOMEM;
		goto fail;
	}

	bzero((char *)classes, be_total_len);

	if((ne = AuditRetrieve(classes, uclasses)) < 0){
		u.u_error = EFAULT;
		goto fail;
	}

	if(copyout(classes, uclasses, be_total_len)){
		u.u_error = EFAULT;
		goto fail;
	}

fail:
	if(classes)free(classes);

	return(ne);

}


/*
 * NAME: AuditRetrieve()
 *
 * FUNCTION: For every class name found in the class_names array check
 *	     the be_symtab symbol table for all events under this class
 *	     and retrieve them into the kernel space buffer. The pointers
 *	     are set to point to approriate locations in user space.
 *
 * INPUTS : NONE
 *	
 * OUTPUTS :
 *	classes : kernel space buffer which holds audit_class structures
 *	uclasses: User space buffer.
 *	The class names and event lists are copied into the kernel space
 *	buffer but the pointers from the fields ae_name, ae_list are 
 *	made to point to appropriate locations in the user space buffer.
 *
 * ERRORS: NONE
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *
 * RETURNS: number of classes 
 *
 */
int
AuditRetrieve(struct audit_class *classes, struct audit_class *uclasses){

	int	numclasses;
	char 	*cp;
	char 	*usrptr;
	char	*tcp;
	struct	base_events	*sp;
	struct	base_events	*ep;
	struct	audit_class	*ap;
	ulong	bitpos;
	int	i;
	int 	namelength;

	numclasses = 0;

	for(i = 0; i < MAX_ANAMES; i++){

		if(class_names[i][0]){

			numclasses++;
		}
	}

	/*
	 * Set cp to point to the beginning of the class area 
	 */

	cp = (char *)&(classes[numclasses]);
	usrptr = (char *)&(uclasses[numclasses]);

	for(i = 0, ap = classes; i < MAX_ANAMES; i++){

		if(class_names[i][0] == '\0'){

			continue;
		}

		ap->ae_name = usrptr;

		strcpy(cp, class_names[i]);
		namelength = strlen(cp) + 1;
		cp += namelength;
		usrptr += namelength;

		ap->ae_list = usrptr;
		tcp = cp;
		bitpos = (1<<i);

		/*
		 * Get all events for this class 
		 * Skip first two (audit_void0, audit_void1)
		 */

		sp = &be_symtab[2];
		ep= &(be_symtab[cevent]);

		for( ; sp < ep; sp++){

			if(sp->be_bitmap & bitpos){
				strncpy(cp, sp->be_name, 16);
				namelength = strlen(cp) + 1;
				cp += namelength;
				usrptr += namelength;
			}

		}

		cp++;
		usrptr++;
		*cp = '\0';
		ap->ae_len = cp - tcp;
		ap++;
	}
			
	return(numclasses);
}


/*
 * NAME: AuditClassAdd()
 *
 * FUNCTION: Index into the class_names array for the ae_name field of the 
 *	     audit_class given as input and add this class index to all 
 *	     events of this class (ae_list) found or inserted in
 *	     events symbol table. This is done by ORing the be_bitmap
 *	     field of each event with the class index.
 *
 * INPUTS :
 *	class : audit_class structure whose events be_bitmap field in the
		be_symtab is to get updated.
 *	
 * OUTPUTS : NONE
 *
 * ERRORS:
 *	EFAULT if the class given as input is not registered in the 
 *	       class_names array.
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *	References class_names array and be_symtab. Updates be_total_len
 *	and be_bitmap field of each event in ae_list
 *
 * RETURNS:
 *	-1 on error
 *	0 on success
 *
 */
int
AuditClassAdd(struct audit_class *class){

	int	an_index;
	int	i;
	char	*bep;
	struct	base_events	*sym;

	/*
	 * Look for the given class name in the table classes.
	 * If it's already there then don't add this "new" class
	 * (simply return...) otherwise, copy in this class name.
	 */

	for(an_index = 0; an_index < MAX_ANAMES - 1; an_index++){

		if(strncmp(class_names[an_index], class->ae_name, 16) == 0){

			break;
		}
	}

	/*
	 * Didn't find it?  this can't happen...
	 */

	if(an_index == MAX_ANAMES-1){
		u.u_error = EFAULT;
		return(-1);
	}

 	/*
	 * We are keeping a running total of the 
	 * size of the user buffer
 	 * needed to hold all audit_class structures 
	 * (see auditevent(AUDIT_GET))
 	 * class structure + length of class name + the 2 ending nulls 
	 */

	be_total_len += sizeof(struct audit_class) + strlen(class->ae_name) + 2;

	/*
	 * Add events to this class 
	 */

	for(i = 0, bep = class->ae_list; i < class->ae_len; i++){

		int 	rc;

		rc = AuditLookup(bep);

		if(rc < 0){

			return(-1);
	
		}

		/*
		 * Set the bit 
		 * for this class
		 */

		sym = &(be_symtab[rc]);
		sym->be_bitmap |= (ulong) (1 << an_index);

		/*
	  	 * bump total space for event in this class 
		 */

		be_total_len += (strlen(bep) + 1);

		/*
		 * Move to next event 
		 */

		bep += strlen(bep) + 1;
	}

	return(0);
}


/*
 * NAME: audit_getname()
 *
 * FUNCTION: Given the index, it returns the event name present at that
 *	     index in the event symbol table.
 *
 * INPUTS : 
 *	index: index into the be_symtab symbol table
 *
 * OUTPUTS : NONE
 *
 * ERRORS: NONE
 *
 * TYPE: char *
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.
 *
 * DATA STRUCTURES: Effects on global data structures.
 *
 * RETURNS: event name
 *
 */
char *
audit_getname(int index){

	struct base_events *sp;

	sp = &(be_symtab[index]);

	return (sp->be_name);
}


AllocAudLocks()
{
	lock_alloc(&audit_lock,LOCK_ALLOC_PAGED,AUDIT_LOCK_CLASS,-1);
	lock_alloc(&audit_obj_lock,LOCK_ALLOC_PAGED,AUDITOBJ_LOCK_CLASS,-1);
	simple_lock_init(&audit_lock);
	simple_lock_init(&audit_obj_lock);
}
