/* @(#)55       1.1.1.3  src/bos/usr/bin/tcbck/tcbdbio.h, cmdsadm, bos411, 9435D411a 9/2/94 11:00:31 */
/*
 *
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
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

#define	MAXTCBSIZ	8192
#define         SYSCK_FILE    "/tmp/sysck."
#define         TEMP_SIZE       sizeof (SYSCK_FILE) + 6
#define         XXXXXX          "XXXXXX"

/*
 * Enumeration of possible types of files.
 */

typedef	enum {
	TCB_FILE,	/* Ordinary files */
	TCB_DIR,	/* Directories */
	TCB_FIFO,	/* Named pipes */
	TCB_BLK,	/* Block special */
	TCB_CHAR,	/* Character special */
	TCB_MPX,	/* Multiplexed character special */
        TCB_SYMLINK,    /* Symbolic link */
        TCB_LINK        /* Hark link */
} tcb_type_t;

struct	tcbent {
	char	*tcb_name;	/* Name of the filesystem object */
	char	**tcb_class;	/* Classes object is a member of */
	tcb_type_t tcb_type;	/* Type of object */
	uid_t	tcb_owner;	/* ID of object owner */
	gid_t	tcb_group;	/* ID of object group */
	char	*tcb_acl;	/* Text string of object ACL */
	char	*tcb_pcl;	/* Text string of object PCL */
	char	*tcb_checksum;	/* Text string of object checksum */
	char	**tcb_links;	/* List of hard links to object */
	char	**tcb_symlinks;	/* List of soft links to object */
	char	**tcb_program;	/* Check program and list of args */
	char	*tcb_source;	/* Source to create object from */
        char    *tcb_target;    /* Target of symbolic or hard link */
	int	tcb_mode;	/* Permission bits for object */
	long	tcb_size;	/* Size of object [ FILEs only ] */
	char	tcb_vsize;	/* Do not update size in database */
	char	tcb_vchecksum;	/* Do not update checksum in database */
	char	tcb_vsum;	/* tcb_sum is a valid value */
	unsigned tcb_sum;	/* Checksum value using builtin sum */
	int	tcb_valid;	/* Contents of this structure are OK */
	int	tcb_changed;	/* Contents of this structure changed */
	int	tcb_test;	/* Record has been selected to be tested */
};


/*
** The following struct was added to enable building
** a linked list of tcbent structs.
*/
struct symlink_rec {
	struct	symlink_rec	*next; 		/* Pointer to next struct */
	struct	tcbent		*tcbent;	/* Pointer to table ent   */
};
