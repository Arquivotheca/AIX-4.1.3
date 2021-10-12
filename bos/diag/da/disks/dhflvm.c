static char sccsid[] = "@(#)51	1.12  src/bos/diag/da/disks/dhflvm.c, dadisks, bos411, 9428A410j 2/15/93 10:36:11";

/*
 * COMPONENT_NAME: (CMDDIAG)  Frees disk for testing
 *
 * FUNCTIONS: 	freedisk
 *		get_vg_data
 *		list_pvmounts
 *		get_stat
 *		ckfs
 *		checkin
 *		beginswith
 *		unmount
 *		remount
 *		on_off
 *		restoredisk
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <locale.h>
#include <nl_types.h>
#include "diag/diago.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag.h"
#include "dhflvm_msg.h"
#include <sys/vmount.h>

/* GLOBAL VARIABLES	*/

#define	SPVRC_CHPV_R	51
#define	SPVRC_NOQUORUM1	52
#define	SPVRC_NOQUORUM2	53
#define	SPVRC_VARYOFF	54
#define	SPVRC_VARYON	55
#define	SPVRC_CHPV_A	56

static struct  msglist msgstat[] = {
	{ SET_MOUNT, MSG_MNT_T, },
	{ SET_STATUS1, MSG_STATUS1_A, },
	NULL
};
static struct  msglist restorestat[] = {
	{ SET_MOUNT, MSG_MNT_T, },
	{ SET_STATUS7, MSG_STATUS7_A, },
	NULL
};
static struct  msglist lvbusy[] = {
	{ SET_LVSBUSY, MSG_LVSBUSY_T, },
	{ SET_LVSBUSY, MSG_LVSBUSY_2, },
	{ SET_LVSBUSY, MSG_LVSBUSY_3, },
	{ SET_LVSBUSY, MSG_LVSBUSY_E, },
	NULL
};
static struct  msglist cpvelist[] = {
	{ SET_CPVERR, MSG_CPVERR_T, },
	{ SET_CPVERR, MSG_CPVERR_2, },
	{ SET_CPVERR, MSG_CPVERR_3, },	/* place holder */
	{ SET_CPVERR, MSG_CPVERR_E, },
	NULL
};
static struct  msglist unmntlist[] = {
	{ SET_UNMOUNT, MSG_UNMNT_T, },
	{ SET_UNMOUNT, MSG_UNMNT_2, },
	{ SET_UNMOUNT, MSG_UNMNT_E, },
	NULL
};
static struct  msglist mntlist[] = {
	{ SET_MOUNT, MSG_MNT_T, },
	{ SET_MOUNT, MSG_MNT_2, },
	{ SET_MOUNT, MSG_MNT_E, },
	NULL
};
static ASL_SCR_INFO	lvbusyinfo[ DIAG_NUM_ENTRIES(lvbusy) ];
static ASL_SCR_INFO	statinfo[ DIAG_NUM_ENTRIES(msgstat) ];
static ASL_SCR_INFO	restoreinfo[ DIAG_NUM_ENTRIES(restorestat) ];
static ASL_SCR_INFO	cpveinfo[ DIAG_NUM_ENTRIES(cpvelist) ];
static ASL_SCR_INFO	unmntinfo[ DIAG_NUM_ENTRIES(unmntlist) ];
static ASL_SCR_INFO	mntinfo[ DIAG_NUM_ENTRIES(mntlist) ];
static ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;

static	int		checkm_n;
static	int		umount_n;
static	int		umount_idx = -1;
static	char		*checkm_lst[1024];
static	char		*umount_lst[1024];
static	char	 	*mountd_lst[1024];
static  struct vmount	*vmtp_lst[1024];
static  struct vmount	*vm;
static  char		pvname[32];
static  char		pdisk[32];
static  char		*volgroup[52];
int			restoremode=0;
static  nl_catd		fdes;
extern	struct tm_input tm_input;
extern nl_catd diag_catopen(char *, int);
char    *devname;                            /* device name used in openx cmd */

/*  */
/*
 * NAME:  freedisk
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * NOTES:
 *
 * RETURNS:
 *      -5:  Cancel was entered at the unmount filesystems menu (DA_USER_QUIT )
 *      -4:  Exit was entered at the unmount filesystems menu ( DA_USER_EXIT )
 *      -3:  Command failure ( DA_ERROR_OTHER ) 
 *      -2:  disk cannot be tested, except from diskette ( DA_ERROR_OPEN )
 *      -1:  user indicated that he did not want to vary off.
 *       0:  success 
 */

freedisk(disk)
char	*disk;
{
	int rc, mode, num_pvmount;
	char  subsbuf[10240], *tmp;
	char  msgstr[512];

	/* For now return -2 to indicate an Open error. */
	/* There is no safe way to varyoff the volume   */
	/* group, since it may cause some data to be    */
	/* inaccessible. For instance DAVars cannot be  */
	/* accessed.					*/

	return(-2);
	strcpy( pvname, disk );
	strcpy( pdisk, disk );

	/* this code has its own catalog file */  
	setlocale( LC_ALL, "" );
	fdes = diag_catopen( MF_DHFLVM, 0 );

	/* 
	Determine if disk is the only disk in volume group, and thus
	needs to be varied off line.  Or if the disk is one of multiple 
	disks in the volume group and can be simply removed from the
	volume group temporarily.  Also check to determine if the disk
	can be removed from the volume group without violating the 
	quorum.
	*/

	switch(rc = get_vg_data()){
		case SPVRC_VARYOFF:			/* only disk in vg */ 
		case SPVRC_CHPV_R:			/* can be removed */
			mode = rc;			
			break;
		case SPVRC_NOQUORUM1:			/* can't test */
		case SPVRC_NOQUORUM2: 			
			return(-2);
			break;
		default:
			/* command failed */
			return(-3);
			break;
	}

	/* Display text "Checking for mounted filesystems". */
	rc = diag_display(NULL, fdes, msgstat, DIAG_MSGONLY,
		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, statinfo);
	sprintf( msgstr, statinfo[0].text, tm_input.dname,
		tm_input.dnameloc);
	free(statinfo[0].text);
	statinfo[0].text = (char *)malloc(strlen(msgstr + 1));
	strcpy(statinfo[0].text, msgstr);
	rc = diag_display(0x803000, fdes, NULL, DIAG_IO,
		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, statinfo);
	
	/* if disk is mounted */
	num_pvmount = list_pvmounts(pvname);
	if (num_pvmount > 0){
		/* 
		Display Menu "Do you want to unmount the following
		filesystems and attempt to free the disk for testing.
		*/
		rc = diag_display(NULL, fdes, lvbusy, DIAG_MSGONLY,
			ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, lvbusyinfo);
		sprintf( msgstr, lvbusyinfo[0].text, tm_input.dname,
			tm_input.dnameloc);
		free(lvbusyinfo[0].text);
		lvbusyinfo[0].text = (char *)malloc(strlen(msgstr + 1));
		strcpy(lvbusyinfo[0].text, msgstr);
		/* stick in mount points */
		strcpy(subsbuf,"  ");
		for(umount_idx = umount_n; umount_idx--;  ){
			strcat(subsbuf, umount_lst[umount_idx]);
			strcat(subsbuf, "\n");
			strcat(subsbuf,"  ");
		}
		/* remove last '\n' */
		subsbuf[strlen(subsbuf)-3] = NULL;
		tmp = (char *)malloc( strlen(subsbuf) + 
				      strlen(lvbusyinfo[2].text) + 1);
		sprintf(tmp, lvbusyinfo[2].text, subsbuf);
		free(lvbusyinfo[2].text);
		lvbusyinfo[2].text = tmp;
		rc = diag_display(0x803001, fdes, NULL, DIAG_IO, 
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutype, lvbusyinfo);
		switch(rc){
			case DIAG_ASL_COMMIT:
				switch( DIAG_ITEM_SELECTED(menutype) ){
					/* do not unmount */
					case 1:
						return(-1);
						break;
					/* unmount */
					case 2:
						if( unmount() )
							return(rc);
						break;
				}
				break;
			case DIAG_ASL_EXIT:
				return(-4);
				break;	
			case DIAG_ASL_CANCEL:
				return(-5);
				break;	
			default:
				return(-3);
				break;
		}    
	}
	else 
		if(num_pvmount < 0)
			return( -3 );
	return( on_off(mode) );
}
/*  */
/*
 * NAME: get_vg_data 
 *                                                                    
 * FUNCTION: 
 *      Runs ./etc/lpp/diagnostics/da/dgvars to determine info about 
 *	the volume group. 
 *
 * RETURN VALUE:  
 *    	-1: 			command failed
 *    	SPVRC_VARYOFF:		disk needs to be varied off
 *    	SPVRC_CHPV_R:		disk needs to be removed from vg
 *    	SPVRC_NOQUORUM1:	disk cannot be removed without violating the 
 *    	SPVRC_NOQUORUM2
 */  

static int 
get_vg_data()
{
	int rc;
	int pvdesc;			/* number desc on pv */
	int totalpvs;			/* total pvs in volume group */
	int activepvs;			/* active pvs in volume group */
	int totaldesc;			/* total desc in volume group */
	char *out_ptr, *err_ptr;
	char *diagdir, command[256];

	/* get strings from catalog for command string search */

	/* get directory path to use for command execution of diag's */
	if((diagdir = (char *)getenv("DIAGNOSTICS")) == NULL )
		diagdir = DIAGNOSTICS;
	strcpy(command, diagdir);
	strcat(command, "/da/dgvars");

	if (rc = odm_run_method( command, pvname, &out_ptr, &err_ptr) )
		return(-1);

	sscanf( out_ptr, "%s%d%d%d%d", volgroup, &pvdesc, 
		&totalpvs, &activepvs, &totaldesc );

	/* if total number of pvs = 1 */
	if( totalpvs == 1 ){
		rc = SPVRC_VARYOFF; 
	}
	/* else if number of descriptors on pvname = 1 */
	else 
		/* if two disk in volume group - one disk has two desc */
		if (totalpvs == 2)
			rc = SPVRC_CHPV_R;
		else if ( (activepvs-1) > (totaldesc/2) )
			rc = SPVRC_CHPV_R;
		else 
			/* 1 desc, but quorum violated */
			rc = SPVRC_NOQUORUM1; 
	return( rc );

}
/*  */
/*
 * NAME: list_pvmounts
 *                                                                    
 * FUNCTION: 
 * 	Invokes the "lspv" command to determine the logical volumes 
 *	on the physical volume to test, "pvname".  Puts these names
 *      in the checkm_lst array. 
 *                                                                    
 * RETURN VALUE: 
 *     -1: failure
 *	0: if not mounted
 *    n>0: the number of mounted filesystems
 *                                                                    
 */  

static int
list_pvmounts( pvname )
char	*pvname;
{
	int		nmounts, rc;
	struct vmount	*vmountp;
	char		lspvoption[260];
	char		devname[100], devnam[100];
	char		*devn, *out_ptr, *err_ptr;

	if ((nmounts = get_stat(&vmountp)) <= 0)
		return(-1);

	strcpy(devname, "/dev/");

	sprintf(lspvoption, "-l %s", pvname);
	if (odm_run_method(LSPV, lspvoption, &out_ptr, &err_ptr))
		return(-1);

	/* skip first two lines of output */
	devn = strtok(out_ptr, "\n");
	devn = strtok(NULL, "\n");

	/* put logical volume names in checkm_lst */ 
	while( 1 ){
		if( NULL != ( devn = strtok(NULL, "\n") )  ){
			sscanf(devn, "%s", devnam);
			sprintf(devname, "/dev/%s", devnam);
			checkm_lst[checkm_n]=(char *)malloc(strlen(devname)+1);
			strcpy(checkm_lst[checkm_n++], devname);
		}
		else
			break;
	}

	return(  ckfs(nmounts, vmountp, (char*) NULL)  );
}
/*  */
/*
 * NAME: get_stat
 *                                                                    
 * FUNCTION: 
 * 	get_stat gathers the mount status for this local
 *	machine	using mntctl.
 *                                                                    
 * RETURN VALUE: 
 *	n > 0: the number of struct vmounts in the buffer
 *	       which is pointed to by pointer left at *vmountpp.
 *      -1:    failure 
 */  

static int
get_stat(vmountpp)
struct vmount	**vmountpp;	/* place to tell where buffer is */
{
	int size, nmounts;

	/* set initial size of mntctl buffer to a MAGIC NUMBER */
	size = BUFSIZ;

	/* try the operation until ok or a fatal error */

	while (1){
		if ((vm = (struct vmount *)malloc(size)) == NULL)
			return(-1);

		/*
		 * perform the QUERY mntctl - if it returns > 0, that is the
		 * number of vmount structures in the buffer.  If it returns
		 * -1, an error occured.  If it returned 0, then look in
		 * first word of buffer for needed size.
		 */
		if ((nmounts = mntctl(MCTL_QUERY, size, (caddr_t)vm)) > 0){
			/* OK, got it, now return */
			*vmountpp = vm;
			return(nmounts);

		} else if (nmounts == 0){
			/* the buffer wasn't big enough .... */
			/* .... get required buffer size */
			size = *(int *)vm;
			free(vm);

		} else {
			/* some other kind of error occurred */
			free(vm);
			return(-1);
		}
	}
}
/*  */
/*
 * NAME: ckfs
 *                                                                    
 * FUNCTION: 
 *      Looks up directories mounted on the logical volumes on the physical 
 *	volume to be tested.
 *
 * NOTES:
 *      VMT_OBJECT: the mounted over object --> the first directory in "mount"
 *      VMT_STUB:   the mount point         --> the second directory in "mount"
 *
 * RETURN VALUE:  
 *	The number of mounted filesystems.	
 *
 */  

static int
ckfs(nmounts, vmtp, fsname)
int		nmounts;
struct vmount	*vmtp;
char		*fsname;
{
	int	save_checkm_n = checkm_n;

	umount_n = 0;

	while (nmounts--){
		vmtp = (struct vmount *)((char *)vmtp + vmtp->vmt_length);
		/* if mounted over object is in checkm_lst, list of lvs */
		if( checkin( vmt2dataptr(vmtp, VMT_OBJECT)) ){
			vmtp_lst[umount_n]     = vmtp;
			mountd_lst[umount_n]   = vmt2dataptr(vmtp, VMT_OBJECT);
			umount_lst[umount_n++] = vmt2dataptr(vmtp, VMT_STUB);
			/* add the mount point if it is not in the list */
			if( !checkin( vmt2dataptr(vmtp, VMT_STUB)) )
				checkm_lst[checkm_n++] = 
					vmt2dataptr(vmtp, VMT_STUB);
		}
		/* else if the mount point is in the list */
		else if (checkin( vmt2dataptr(vmtp, VMT_STUB)) ){
			vmtp_lst[umount_n]     = vmtp;
			mountd_lst[umount_n]   = vmt2dataptr(vmtp, VMT_OBJECT);
			umount_lst[umount_n++] = vmt2dataptr(vmtp, VMT_STUB);
		}
	}
	checkm_n = save_checkm_n;
	return( umount_n );
}
/*  */
/*
 * NAME: checkin
 *                                                                    
 * FUNCTION: 
 *      Checks if the directory path is in the checkm_lst, the list of   
 *	logical volumes and/or mount points on the pv to be tested.
 *
 * RETURN VALUE:  
 *	0:  It is not in list.
 *      1:  It is in the list.
 */  

static int
checkin(fsname)
char	*fsname;
{
	int	j;

	for( j=0; j < checkm_n; j++ ){
		if( beginswith(checkm_lst[j], fsname) ){
			return(1);
		}
	}
	return(0);
}

static int
beginswith(a, b)
char	*a, *b;
{
	char	last_char;
	int	name_len;

	name_len = strlen(a);

	if( !strncmp(a, b, name_len) ){
		last_char = b[ name_len ];
		if( '\0' == last_char || '/' == last_char )
			return(1);
	}
	return(0);
}
/*  */
/*
 * NAME:  unmount
 *
 * FUNCTION: 
 *	Unmounts filesystems.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * NOTES:
 *
 * RETURNS:
 *      -3:  unmount failed and could not remount.
 *      -2:  unmount failed and remounted.
 *       0:  success 
 */

static int
unmount()
{
	int rc, display_rc;
	char msgstr[512];
	char *tmp;

	for (umount_idx = umount_n-1; umount_idx >= 0; umount_idx-- ){
		rc = uvmount( vmtp_lst[umount_idx]->vmt_vfsnumber, 0 );
		if (rc != 0){
			/* Display "Cannot unmount %s from %s" */
			memset(&menutype, 0, sizeof(menutype));
			display_rc = diag_display(NULL, fdes, unmntlist,
				 DIAG_MSGONLY, ASL_DIAG_OUTPUT_LEAVE_SC,
				 &menutype, unmntinfo);
			sprintf( msgstr, unmntinfo[0].text, tm_input.dname,
				tm_input.dnameloc);
			free(unmntinfo[0].text);
			unmntinfo[0].text = (char *)malloc(strlen(msgstr + 1));
			strcpy(unmntinfo[0].text, msgstr);
			tmp = (char *)malloc( strlen(umount_lst[umount_idx]) +
					      strlen(mountd_lst[umount_idx]) +
				  	      strlen(umount_lst[umount_idx]) +
					      strlen(unmntinfo[1].text) + 1 );	
			sprintf(tmp, unmntinfo[1].text, umount_lst[umount_idx],
				mountd_lst[umount_idx], umount_lst[umount_idx]);
			free(unmntinfo[1].text);
			unmntinfo[1].text = tmp;
			display_rc = diag_display( 0x803002, fdes, NULL, DIAG_IO,	
				ASL_DIAG_KEYS_ENTER_SC, &menutype, unmntinfo );
			break;
		}
	}

	if (rc != 0){
		if ( remount(++umount_idx) == -1 )
			return(-3);
		else
			return(-2);
	}
	
	return(0);

}

static int
remount(umount_idx)
int umount_idx;
{
	int rc;
	char msgstr[512];
	char *tmp;

	for ( ; umount_idx < umount_n; umount_idx++){
		rc = vmount( vmtp_lst[umount_idx],
				vmtp_lst[umount_idx]->vmt_length );
		if (rc != 0){
			/* Display "Cannot restore mount %s on %s" */
			memset(&menutype, 0, sizeof(menutype));
			rc = diag_display(NULL, fdes, mntlist,
				 DIAG_MSGONLY, ASL_DIAG_OUTPUT_LEAVE_SC,
				 &menutype, mntinfo);
			sprintf( msgstr, mntinfo[0].text, tm_input.dname,
				tm_input.dnameloc);
			free(mntinfo[0].text);
			mntinfo[0].text = (char *)malloc(strlen(msgstr + 1));
			strcpy(mntinfo[0].text, msgstr);
			tmp = (char *)malloc( strlen(umount_lst[umount_idx]) +
					      strlen(mountd_lst[umount_idx]) +
					      strlen(mntinfo[1].text) + 1 );	
			sprintf(tmp, mntinfo[1].text, umount_lst[umount_idx],
				mountd_lst[umount_idx] );
			free(mntinfo[1].text);
			mntinfo[1].text = tmp;
			rc = diag_display( 0x803003, fdes, NULL, DIAG_IO,	
				ASL_DIAG_KEYS_ENTER_SC, &menutype, mntinfo );
			return(-1);
		}
	}
	
	return(0);

}

/*  */
/*
 * NAME:  on_off
 *
 * FUNCTION: 
 *	Adds/Deletes the LVM layer, employing varyonvg, varyoffvg,
 *	or chpv.  A menu is presented indicating what is happening.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * NOTES:
 *	Sets the global variable restoremode to the case which
 *	should be executed to restore the system.
 *
 * RETURNS:
 *      -2:  disk was never unmounted. 
 *      -1:  command failure
 *       0:  success 
 */

static int
on_off(mode)
int mode;	
{
	int rc;
	char  msgstr[512];
	char *out_ptr, *err_ptr, chpvargs[32];

	switch(mode){
		case SPVRC_VARYOFF:			
			rc = odm_run_method( VARYOFFVG, volgroup, 
					      &out_ptr, &err_ptr );
			restoremode = SPVRC_VARYON;
			break;
		case SPVRC_VARYON :
			rc = odm_run_method( VARYONVG, volgroup, 
					      &out_ptr, &err_ptr );
			break;
		case SPVRC_CHPV_A :
			sprintf(chpvargs, "-v a %s", pvname);
			rc = odm_run_method( CHPV, chpvargs, 
					      &out_ptr, &err_ptr );
			break;
		case SPVRC_CHPV_R:			
			sprintf(chpvargs, "-v r %s", pvname);
			rc = odm_run_method( CHPV, chpvargs, 
					      &out_ptr, &err_ptr );
			restoremode = SPVRC_CHPV_A;
			break;
		default :
			return(-2);
	}
	if(tm_input.loopmode != LOOPMODE_INLM)
		putdavar(tm_input.dname, "restoremode", DIAG_INT,
			&restoremode);
	if( rc != 0 || (err_ptr != NULL && *err_ptr != NULL) ){
		memset(&menutype, 0, sizeof(menutype));
		memset(cpveinfo,  0, sizeof(cpveinfo));
		/* Display "Please make a note of the information below." */
		rc = diag_display( NULL, fdes, cpvelist, DIAG_MSGONLY, NULL,
					&menutype, cpveinfo );
		sprintf( msgstr, cpveinfo[0].text, tm_input.dname,
			tm_input.dnameloc);
		free(cpveinfo[0].text);
		cpveinfo[0].text = (char *)malloc(strlen(msgstr + 1));
		strcpy(cpveinfo[0].text, msgstr);
		cpveinfo[2].text = err_ptr;
		rc = diag_display( 0x803004, fdes, NULL, DIAG_IO, 
					ASL_DIAG_NO_KEYS_ENTER_SC,
						&menutype, cpveinfo );
		return(-1);
	};

	return(0);

}

/*  */
/*
 * NAME:  restoredisk
 *
 * FUNCTION: 
 *	Bring the disk on line using the appropriate lvm command
 *	either varyonvg or chpv.  Then mount the file systems.
 *
 * RETURNS:
 *      -1:  failure 
 *       0:  success 
 */

restoredisk()
{
	int rc;
	char msgstr[512];

	return(0); /* Always return 0 since freedisk always return -2 */
	rc = diag_display(NULL, fdes, restorestat, DIAG_MSGONLY,
		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, restoreinfo);
	sprintf( msgstr, restoreinfo[0].text, tm_input.dname,
		tm_input.dnameloc);
	free(restoreinfo[0].text);
	restoreinfo[0].text = (char *)malloc(strlen(msgstr + 1));
	strcpy(restoreinfo[0].text, msgstr);
	sprintf( msgstr, restoreinfo[1].text, tm_input.dname);
	free(restoreinfo[1].text);
	restoreinfo[1].text = (char *)malloc(strlen(msgstr + 1));
	strcpy(restoreinfo[1].text, msgstr);
	rc = diag_display(0x803000, fdes, NULL, DIAG_IO,
		ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, restoreinfo);
	getdavar(tm_input.dname, "restoremode", DIAG_INT, &restoremode);	
	switch(rc = on_off(restoremode)){
		case 0:				/* disk is on line */
			rc = remount(0);	/* files remounted */
			break;
		case -2:                	/* disk was never unmounted */
			rc = 0;
			break;
		default :
			break;
	}

	free(vm);
	catclose(fdes);
	return( rc );

}
