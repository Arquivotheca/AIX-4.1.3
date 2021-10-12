/* @(#)80	1.26.1.1  src/bos/kernel/sys/ldr.h, sysldr, bos41J, 9507C 2/8/95 08:10:12 */
/*
 *   COMPONENT_NAME: SYSLDR
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef	_H_LDR
#define	_H_LDR	

struct file;
struct ld_info{
	uint		ldinfo_next;		/* offset of next entry from this or 0 if last */
	union {
		int	_ldinfo_fd;		/* fd returned by ptrace to debugger */
		struct	file	*_ldinfo_fp;	/* fp returned by loader to ptrace */
		int	_core_offset;		/* offset in core file of object */
	} _file;
#define	ldinfo_fd	_file._ldinfo_fd
#define	ldinfo_fp	_file._ldinfo_fp
#define ldinfo_core	_file._core_offset
	void		*ldinfo_textorg;	/* start of loaded program image (xcoffhdr)*/
	unsigned int	ldinfo_textsize;        /* length of loaded program image */
	void		*ldinfo_dataorg;        /* start of data instance */
	unsigned int	ldinfo_datasize;        /* size of data instance */
	char		ldinfo_filename[2];     /* null terminated path name followed by
						 * null terminated member name.
						 * (if not an archive, member name is just a
						 * null).
						 */
};

/* kernel services*/

int ld_getinfo(pid_t pid,unsigned int length,struct ld_info *ld_info);
/* returns 0 if good or ENOMEM if table is not large enough.
 * In later case, contents of ld_info are undefined.  Assumes ld_info
 * is safe to store into - caller is trusted.
 * ldinfo_next is the offset from this ldinfo entry to the next, or zero
 * for the last entry.  There will always be at least one entry.
 * flags should be zero unless otherwise specified in all calls.
 */

int (*ld_execload(struct file *fp,char *libpath,char *dompath))();		
				/*exec entry point to loader*/
void ld_usecount(int dir); 	/* inc/dec process image loader usecounts */
int ld_ptrace(pid_t pid);	/* put a process into PTRACE state */
int ld_ptracefork(pid_t	pid);	/* fix up child during fork */

int
kmod_load(caddr_t path,uint flags,caddr_t libpath,mid_t *kmid);
#define    LD_KERNELEX      0x00000001	/*flags - load as a kernel extension */
#define	   LD_USRPATH	    0x00000002  /*pathname passed is in user space  */
#define    LD_SINGLELOAD    0x00000004	/*if something is already loaded with
					 *this path name use that copy
					 *instead of a new one */
#define    LD_QUERY         0x00000008  /*just look up pathname-dont load*/

int kmod_unload(mid_t kmid,uint flags);		/* kernel unload*/

void (*(
kmod_entrypt(mid_t kmid,uint flags)))();	/* find the entrypoint from
						 * an mid */
int ld_pin(int(*func)());			/* pin a module */
int ld_unpin(int(*func)());


/* syscall's */
int
(*load(char *filenameparm,uint flags,char *libpathparm))();

#define L_SHARED_DATA	0x00000010	/* readwrite in libarary
					 * (not supported)*/
#define L_SHARED_ALL	0x00000020      /*      " and for libaries used */
#define	L_NOAUTODEFER	0x00000040      /* future loads don't see these
					 * defered symbols */
/* loader error codes - each error string will consist of the
 * code (in characters) followed by zero or more data strings
 * seperated by blanks.  For each message, the data strings are
 * listed */
#define	L_ERROR_TOOMANY	         1	/* too many errors, rest skipped */
					/* no data */
#define	L_ERROR_NOLIB	         2	/* can't load library */
					/* library-name optional-member-name */
#define	L_ERROR_UNDEF	         3	/* can't find symbol in library */
					/* symbol using-program-name */
#define L_ERROR_RLDBAD	         4	/* rld data offset or symbol index
					 * out of range or bad relocation
					 * type */
					/* rld-index-number program-name */
#define L_ERROR_FORMAT		 5	/* file not valid, executable xcoff */
					/* program-name */
#define L_ERROR_ERRNO		 6	/* the errno associated with the failure
					 * if not ENOEXEC, it indicates the
					 * underlying error, such as no memory*/
					/* error-number */
#define	L_ERROR_MEMBER		 7	/* member requested from a file which
					 * is not an archive or does not
					 * contain the member*/
					/* member-name */
#define L_ERROR_TYPE		 8	/* symbol type mismatch */
					/* symbol using-program-name */
#define L_ERROR_ALIGN		 9	/* text alignment in file is wrong */
#define L_ERROR_DOMCREAT	 10	/* Insufficient permission to create
					 * a loader domain */
#define L_ERROR_DOMADD		 11	/* Insufficient permission to add
					 * entries to a loader domain */
#define	L_ERROR_SYSTEM	        -1	/* system error - text explains */
					/* error text */


int
loadbind(int lflags,void *exporter,void *importer);

int
unload(void *function);
#define	L_PURGE		(void*)1		/* special parameter to unload to remove
						 * any currently unused modules in
						 * kernel or library - sysconfig	
						 * authority required*/
int
loadquery(int lflags,void *buffer,unsigned int length);
#define L_GETMESSAGES	0x00000001		/* retrieve error messages
						 */
#define L_GETINFO	0x00000002    		/* return same information ptrace gets
						 * for the current process*/
#endif	/* _H_LDR */
