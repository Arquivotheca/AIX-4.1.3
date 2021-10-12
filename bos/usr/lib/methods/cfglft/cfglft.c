static char sccsid[] = "@(#)00	1.12  src/bos/usr/lib/methods/cfglft/cfglft.c, lftdd, bos41J, 9516B_all 4/18/95 10:02:50";
/*
 * COMPONENT_NAME: (LFTDD)	LFT configuration routine
 *
 * FUNCTIONS:
 *
 *	main, build_dds, make_special_files,
 *	err_exit, err_undo1, err_undo2
 *
 * ORIGIN: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* Include files needed for this module follow */
#include <lft.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>


/* device configuration include files */
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"

#include "ttycfg.h"
#include "ldtty.h"
#include "stream_tioc.h"

#include <sys/stropts.h>
#include <sys/sad.h>


/* Set permissions for special file */
#define MODE	S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

/* define path for lft device */
#define DEV_PATH "/dev/"

/* modules info */
#define LDTERM_MODULE       "ldterm"
#define TIOC_MODULE         "tioc"
#define ADMINDEV            "/dev/sad"

/* Define strings for inittab file */
#define LFTID	"\"lft\""
#define LFTON	"\"lft:2:respawn:/usr/sbin/getty /dev/lft0\""

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *	build_dds will allocate memory for the dds structure, reporting any
 *	errors, then open the Customized Attribute Class to get the attribute
 *	objects needed for filling the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called from the device independent module used
 *	for configuring all devices.
 *
 * RETURNS:
 *	0 on success
 *	positive return code on failure
 *
 */
build_dds(lname, dds_out, size)
char *lname;			/* logical name of device */
char **dds_out;			/* pointer to dds structure for return */
int	*size;			/* pointer to dds structure size */
{
lft_dds_t	*dds;			/* pointer to dds structure */
struct CuAt	cuat;			/* customized attribute object */
struct CuAt	*cuat_ptr;		/* customized attribute object */
struct PdAt	pdat;			/* predefined attribute object */
struct PdAt	*pdat_ptr;		/* predefined attribute object */
struct PdAt	*pdat_ptr2;		/* predefined attribute object */
struct objlistinfo	pdat_info;	/* result of search stats */
struct CuDv	*cudv, *cudv_ptr2;	/* ptr to customized device object */
struct CuDv	cudv_obj;		/* custom device obj for odm_get_obj*/
struct objlistinfo	cudv_info;	/* result of search stats */
ulong		value;			/* temp variable to get value into */
char		temp[256];
char		crit[256];		/* search criteria string */
char		def_dsp[NAMESIZE];	/* holds name of default display */
dev_t		devno;			/* used to obtain device number */
struct CuAt	*get_attrval();		/* get_attribute value routine */
int		rc,x,i,index;		/* temporary variables */
int		nbr_displays;		/* number of displays found */
int		tmp_fd;			/* temp file descriptor */
int		font_id;
int		kbd_fnd;		/* Did we find a keyboard */
static char	font_files[LFTNUMFONTS][FILE_NAME_LEN];	/* font file names */
static char	swkbd_file[FILE_NAME_LEN];		/* swkbd file name */

	/*
	 * The first thing to do is get the total number of displays
	 * that are available to the lft. Once we have this number we can then
	 * allocate the memory for the dds structure.
	 */

	/* Get a list of adapter types belonging to the lft from the PdAt */
	pdat_ptr = pdat_ptr2 = (struct PdAt *)odm_get_list(PdAt_CLASS,
		 "deflt='graphics' AND attribute='belongs_to'",&pdat_info,1,1);
	if ( pdat_ptr == (struct PdAt *) -1 )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_list",-1,
				CFG_ODM_PDAT, UNIQUE_1);
		DEBUG_0("build_dds : error in retrieving displays from PdAt\n");
		return(E_ODMGET);
	}

	/* loop thru each adapter type to get total number of displays */
	for (i = 1,nbr_displays=0;pdat_ptr && i<=pdat_info.num; i++,pdat_ptr++) 
	{
		/* get number of available displays for each display type */
		sprintf(crit,"PdDvLn = '%s' AND status = '%d'",
				pdat_ptr->uniquetype, AVAILABLE);
		cudv = (struct CuDv *)
				odm_get_list(CuDv_CLASS,crit,&cudv_info,1,1);
		if ( cudv == (struct CuDv *) -1 ) 
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_list",-1,
					CFG_ODM_CUDV, UNIQUE_2);
			DEBUG_0("build_dds : error getting list from CuDv\n");
			if(pdat_ptr2)
				odm_free_list(pdat_ptr2, &pdat_info);
			return(E_ODMGET);
		}
		if(cudv)
		{
			/* add number of displays found to the total */
			nbr_displays += cudv_info.num;
			odm_free_list(cudv, &cudv_info); 
		}
	}

	/* Check if any displays were found, if not the LFT method fails. */
	if( nbr_displays == 0 )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,0, 
				CFG_NO_DISPS, UNIQUE_3);
		DEBUG_0("build_dds: no displays configured\n");
		if(pdat_ptr2)
			odm_free_list(pdat_ptr2, &pdat_info);
		return(E_NODEPENDENT);
	}

	/*
	 * Obtain size of device specific dds structure. The lft_dds structure
	 * already has an array of 1 display structures in it so we need
	 * to add the number of displays found minus one display structures.
	 */

	*size = sizeof(lft_dds_t) + ((nbr_displays -1) * sizeof(lft_disp_t));

	/* allocate the space required for the dds structure */
	if( (dds = (lft_dds_t *) malloc(*size)) == NULL )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","malloc",0, CFG_NO_MEM, 
				UNIQUE_4);
		DEBUG_0("build_dds : malloc failed for dds structure\n");
		if(pdat_ptr2)
			odm_free_list(pdat_ptr2, &pdat_info);
		return(E_MALLOC);
	}

	/* zero out the dds */
	bzero( dds, *size);

	/*
	 * Get all the font pathnames from the CuAt and PdAt. Copy each
	 * font filename into the dds.
	 * Store the total number of fonts found in the dds.
	 */
	dds->number_of_fonts = 0;
	for( i = 0; i < LFTNUMFONTS; i++ )
	{
		sprintf(crit, "font%d_path", i+1);
		if( get_attrval(lname, crit, temp, &value, &rc) == NULL )
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","getattr",0, 
					CFG_ODM_CUAT, UNIQUE_5);
			DEBUG_1("font%d_path not found\n", i + 1);
		}
		if( temp[0] != '\0' )
		{
			strcpy(font_files[dds->number_of_fonts], temp);
			++dds->number_of_fonts;
		}
	}

	if( dds->number_of_fonts == 0 )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,0, CFG_NO_FONTS,
				UNIQUE_6);
		DEBUG_1("No fonts for %s\n", lname);
		if(pdat_ptr2)
			odm_free_list(pdat_ptr2, &pdat_info);
		return(E_DDS);
	}

	dds->font_file_name = (char*) font_files[0];

	/*
	 * Search the ODM for all available displays for the lft and store
	 * all the corresponding information in the array of display structures.
	 * Get the default display name and set its corresponding display
	 * index in the dds. Check for a custom font associated with each
	 * display and store the results into the display structure.
	 * Store the total number of displays found.
	 */

	 /* Get the default display name */
	if( (cuat_ptr = getattr(lname,"default_disp",FALSE,&rc)) ==	NULL )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","getattr",0,
				CFG_ODM_CUAT,UNIQUE_7);
		DEBUG_0("build_dds : error getting default_disp from CuAt\n");
		if(pdat_ptr2)
			odm_free_list(pdat_ptr2, &pdat_info);
		return(E_ODMGET);
	}
	strcpy(def_dsp, cuat_ptr->value);


	/*
		loop thru each adapter type for a list of available adapters
		The index variable points to the current display in the array
		of display info. It gets incremented each time a new available
		display is found
	 */
	pdat_ptr = pdat_ptr2;
	dds->default_disp_index = -1;	/* set default index to -1 */
	for ( i = 1, index = 0; pdat_ptr && i <= pdat_info.num; i++,pdat_ptr++) 
	{
		/*
		 * Get a list of available adapter device names from the list 
		 * of adapter types. The names come from the 'name' field of 
		 * the CuDv class. (ie.. 'ppr0'). The 'status' field having a 
		 * value of 1 means the given adapter is available.
		 */
		sprintf(crit,"PdDvLn = '%s' AND status = '%d'",
				pdat_ptr->uniquetype, AVAILABLE);
		cudv = cudv_ptr2 = (struct CuDv *)
				odm_get_list(CuDv_CLASS,crit,&cudv_info,1,1);
		if ( cudv == (struct CuDv *) -1 ) 
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_list",-1,
					CFG_ODM_CUDV,UNIQUE_8);
			DEBUG_0("build_dds : error getting list from CuDv\n");
			if(pdat_ptr2)
				odm_free_list(pdat_ptr2, &pdat_info);
			return(E_ODMGET);
		}
	
		/*
		 * loop thru list of available adapters, storing the device
		 * name and devno for each available display into the dds 
		 * display structure. If a custom font exists for a display
		 * then store it in the associated display structure.
		 */
		for (x = 1; cudv && x <= cudv_info.num; x++, cudv++, index++) 
		{
			/* store devno and devname into display structure */
			if((rc=get_devno(cudv->name,
					&dds->displays[index].devno)) != E_OK)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft",
						"get_devno",rc,
						CFG_BAD_DEVNO,UNIQUE_9);
				DEBUG_1("build_dds : error getting devno \
						of display %s\n", cudv->name);
				if(pdat_ptr2)
					odm_free_list(pdat_ptr2, &pdat_info);
				return(rc);
			}
			strcpy(dds->displays[index].devname, cudv->name);

			/* if display is default, then store index into dds */
			if( ! strcmp(def_dsp, cudv->name) )
				dds->default_disp_index = index;

			/*
			 * Check if custom font exists for display. If legal
			 * font index found then store it otherwise store -1
			 * and remove it from CuAt if it exists.
			 */
			dds->displays[index].font_index = -1;
			sprintf(crit,"name='%s' AND attribute='custom_font'",
					cudv->name);

			if((rc = (int)odm_get_first(CuAt_CLASS,crit,&cuat)) > 0) 
			{
				font_id = atoi(cuat.value);
				if( font_id >= 0 &&
						font_id < dds->number_of_fonts)
				{
					dds->displays[index].font_index = 
							font_id;
				} 
				else 
				{
					/* invalid custom font so remove it */
					odm_rm_obj(CuAt_CLASS, crit);
				}
			}

		} /* for all available displays */
		if(cudv_ptr2)
			odm_free_list(cudv_ptr2, &cudv_info);

	} /* for all adapter types */
	if(pdat_ptr2)
		odm_free_list(pdat_ptr2, &pdat_info);
 
	/* store the total number of displays found. */
	dds->number_of_displays = nbr_displays;
			
	/*
	 * if a default display was found in the CuAt and that display was
	 * not available to the lft, or no default display was found in the 
	 * CuAt, then force the default_disp attribute in the ODM to the 
	 * first display found and set the default index to zero.
	 */
	if ( (def_dsp[0] != '\0' && dds->default_disp_index == -1) ||	
			( def_dsp[0] == '\0') )
	{
		strcpy(cuat_ptr->value, dds->displays[0].devname);
		if( putattr(cuat_ptr) == -1 )
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","putattr",-1,
					CFG_ODM_CUAT,UNIQUE_10);
			DEBUG_0("build_dds : error updating default_disp \
					in the CuAt\n");
			return(E_ODMGET);
		}
		dds->default_disp_index = 0;
	}


	/*
	 * All keyboards have the attribute 'sys_kbd' defined in their PdAt
	 * class.	The odm_get_list function is used to create a list of
	 * these.	The uniquetype from this list is used to look for the
	 * logical name in the CuDv class (PdDvLn = <uniquetype> and status =
	 * AVAILABLE). 
	 */

	pdat_ptr = pdat_ptr2 = (struct PdAt *) odm_get_list(PdAt_CLASS,
			"attribute = 'sys_kbd'", &pdat_info,1,1);
	if (pdat_ptr == (struct PdAt *) -1)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_list",-1,
				CFG_ODM_PDAT,UNIQUE_11);
		DEBUG_0("build_dds: error retrieving keyboard information \
				from PdAt\n");
		return(E_ODMGET);
	}

	/*
	 * We now have the list - walk this to find the keyboard that is
	 * in the available state and get its logical name.
	*/
	for (i = 1, kbd_fnd = 0; pdat_ptr && i <=pdat_info.num; i++,pdat_ptr++)
	{
		sprintf(crit, "PdDvLn = '%s' AND status = '%d'",
				pdat_ptr->uniquetype, AVAILABLE);
		rc = (int) odm_get_obj(CuDv_CLASS,crit,&cudv_obj, ODM_FIRST);
		if(rc == -1)			/* Failed */
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_obj",-1,
					CFG_ODM_CUDV,UNIQUE_12);
			DEBUG_0("build_dds: Could not retrieve keyboard \
					information from CuDv\n");
			if(pdat_ptr2)
				odm_free_list(pdat_ptr2, &pdat_info); 
			return(E_NOCuDv);
		}
		if(rc == 0)		/* no entry found */
			continue;	/* go get the next */

		if( (rc = get_devno(cudv_obj.name, &dds->kbd.devno)) != 0 )
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","get_devno",rc,
					CFG_BAD_DEVNO,UNIQUE_13);
			DEBUG_1("build_dds : error getting devno of \
					keyboard %s\n", cudv_obj.name);
			if(pdat_ptr2)
				odm_free_list(pdat_ptr2, &pdat_info);
			return(rc);
		}
		strcpy(dds->kbd.devname, cudv_obj.name);
		kbd_fnd++;
		break;
	} /* end of for */

	if(pdat_ptr2)
		odm_free_list(pdat_ptr2, &pdat_info);

	if(!kbd_fnd)	/* no keyboard ! */
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,-1,
				CFG_NO_KYBD,UNIQUE_14);
		DEBUG_0("build_dds : No available system keyboard found.\n");
#ifdef KBDRQD
		return(-1);
#else
		dds->kbd.devno = -1;
#endif
	}
		
	/*
	 * Search the PdAt class for a 'fkproc' attribute. If one is found
	 * then set the 'start_fkproc' member of the dds to true. This flag
	 * is used by the lft driver to indicate whether the font server
	 * process needs to be invoked.
	 */

	pdat_ptr = (struct PdAt *)odm_get_list(PdAt_CLASS,
			 "attribute = 'fkproc'",&pdat_info,1,1);
	if ( pdat_ptr == (struct PdAt *) -1 )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_list",-1,
				CFG_ODM_PDAT,UNIQUE_15);
		DEBUG_0("build_dds : error in retrieving 'fkproc' from PdAt\n");
		return(E_ODMGET);
	}
	if ( pdat_ptr && pdat_info.num > 0)
		dds->start_fkproc = TRUE;
	else
		dds->start_fkproc = FALSE;

	if(pdat_ptr)  /* done with the list so free it */
		odm_free_list(pdat_ptr, &pdat_info);

	/*
	 * Get the pathname to the software keyboard file and copy the
	 * the pathname into the dds. Before copying in the pathname
	 * be sure the file can be opened. If it can't then try the
	 * filename from the PdAt.
	 */
	if( get_attrval(lname, "swkb_path", temp, &value, &rc) == NULL )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","get_attrval",0,
				CFG_ODM_CUAT,UNIQUE_16);
		DEBUG_0("cfglft: found no swkb_path\n");
		return(rc);
	}
	if( (tmp_fd = open(temp, O_RDONLY)) != -1 )
	{
		/* Open ok so copy in the pathname */
		strcpy( swkbd_file, temp);
	}
	else 
	{
		/*
		 * The open failed probably from a path stored in the CuAt.
		 * Lets now try to open the default file stored in the PdAt.
		 */
		strcpy(crit, "attribute = 'swkb_path'");
		if ((rc = (int)odm_get_first(PdAt_CLASS, crit, &pdat)) <= 0)
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_first",
					rc, CFG_ODM_PDAT,UNIQUE_17);
			DEBUG_2("cfglft: crit=%s found no objects,rc = %d\n",
					crit,rc);
			return(E_NOATTR);
		}

		if( (tmp_fd = open(pdat.deflt, O_RDONLY)) == -1 )
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","open",-1,
					CFG_NO_SWKBD,UNIQUE_18);
			DEBUG_1("Can't open keyboard file : %s\n",pdat.deflt);
			return(E_DDS);
		}
		else
		{
			strcpy( swkbd_file, pdat.deflt);
		}
	}
	dds->swkbd_file = swkbd_file;
	close(tmp_fd); /* close the swkb file */

	/* Copy the lft logical device name and devno to dds	*/
	strcpy(dds->lft.devname, lname);

	/* store dds pointer to be returned */
	*dds_out = (char *)dds;

        /* Retrieve the 3 timeout values used by Display Power Manager */
        for( i = 0; i < 3; i++ )
        {
                sprintf(crit, "pwr_mgr_t%d", i+1);
                if( (cuat_ptr = getattr(lname,crit,FALSE,&rc)) == NULL )
                {
		 	/* There is a problem with non CCM graphics adapter during
                         * installation.  What happens is that on a system with more 
                         * one adapter, only one display is turned on.  Consequently,
                         * one only see one message "hit Fx to select this display as
                         * the console for installation".  This is confusing for users.
                         * Therefore, to fix this problem, we have to move the PdAts 
                         * for DPMS from lft.add into a separate file, so that we don't 
                         * find it during installation.  The lft packaging is modified to
			 * install this new .add so that we can enable DPMS during normal 
		         * boot.
			 */
                        dds->pwr_mgr_time[i] = 0;     /* zero disable DPMS during non normal boot */ 
                        continue;
                }
                dds->pwr_mgr_time[i] = atoi(cuat_ptr->value) * 60;   /* convert minutes into seconds */
        }


	/*
	 * Find the console 
         */
        if (( cuat_ptr = getattr("sys0", "syscons", FALSE, &rc )) == NULL )
	{
	   return(-1);    
	}

	/*
 	 *  If the console is not defined (i.e., users have not selected it), set this flag 
         *  to tell the LFT DD start DPMS.  Note this flag take precedence over the first
         *  zero time-out value
         */ 
	if (strcmp(cuat_ptr->value,"") == 0)
	{
	   dds->enable_dpms = 0;           /* don't start DPMS */
	}
	else
	{
	   dds->enable_dpms = 1;           /* enable DPMS */
	}	
	

	return(E_OK);
}


/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependent routine creating the devices special files
 *	in /dev
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called from the main module.
 *
 * RETURNS:
 *	0 - success
 *	positive return code on failure
 */
int
make_special_files(lname, devno)
char	*lname;	 /* logical device name */
dev_t devno;		/* major/minor number */
{
char devpath[256];	/* path to special file */
struct	stat buf;
extern int errno;

	/* form the full pathname for the lft special file */
	strcat( strcpy(devpath, DEV_PATH), lname);

	/* attempt to make the special file for lft */
	if (mknod(devpath,MODE,devno))
	{
		/* mknod failed so see what error was */
		if (errno != EEXIST)
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","mknode",
					errno, CFG_BAD_SP_FILE,UNIQUE_19);
			DEBUG_1("make_special_files: can't mknod %s\n",devpath);
			return(E_MKSPECIAL);
		}

		/* device already exists so get status on it */
		if (stat(devpath,&buf))
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","stat",
					errno, CFG_BAD_SP_FILE,UNIQUE_20);
			DEBUG_1("make_special_files: cannot stat %s\n",devpath);
			return(E_MKSPECIAL);
		}
		/* check if devno of new matches devno of existing file */
		/* and file format is correct                           */
		if ( major(buf.st_rdev)==major(devno) &&
		  minor(buf.st_rdev)==minor(devno) &&
		  ((buf.st_mode & (S_IFMT|S_ISVTX)) == S_IFCHR))
			return(E_OK);

		/* unlink existing special file name */
		if (unlink(devpath))
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","unlink",
					errno, CFG_BAD_SP_FILE,UNIQUE_21);
			DEBUG_1("make_special_files:can't unlink %s\n",devpath);
			return(E_MKSPECIAL);
		}

		/* try mknod again */
		if (mknod(devpath,MODE,devno))
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","mknode",
					errno, CFG_BAD_SP_FILE,UNIQUE_22);
			DEBUG_1("make_special_files: can't mknod %s\n",devpath);
			return(E_MKSPECIAL);
		}
	}

	chmod(devpath, MODE); /* give new file correct permissions */

	return(E_OK);
}



/*
 * NAME: main
 *
 * FUNCTION:
 *
 *	The purpose of cfglft is to configure the LFT pseudo device into
 *	the system and make it ready for use.	It is called with the name
 *	of the logical device representing the LFT and possibly a flag
 *	representing which phase the configuration is taking place in.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
main(argc, argv)
int	 argc;
char	*argv[];
{
struct cfg_dd cfg;		/* sysconfig command structure */

char	*logical_name;		/* logical name to configure */
char	sstring[256];		/* search criteria pointer */

struct CuDv cusobj;		/* customized device object storage */
struct PdDv preobj;		/* predefined device object storage */
struct CuAt *cuat_ptr;		/* customized attribute object */

mid_t	kmid;			/* module id from loader */
dev_t	devno;			/* device number for config_dd */

long	majorno, minorno;	/* major and minor numbers */
long	*minor_ptr;		/* pointer returned by getminor */
int	 how_many;		/* number of minors in list */
int	 ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
int	 rc;			/* return codes go here */
char     *outp, *errp;		/* stdout and stderr for odm_run_method * 

extern	int	 optind;	/* for getopt function */
extern	char	*optarg;	/* for getopt function */
int	 errflg,c;		/* used in parsing parameters	*/


	/* Parse the input parameters */

	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:12")) != EOF) 
	{
		switch (c) 
		{
		case 'l': /* logical name parameter */
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case '1': /* phase 1 of IPL */
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE1;
			break;
		case '2': /* phase 2 of IPL */
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) 
	{
		/* error parsing parameters */
		cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,0,CFG_BAD_PARMS,UNIQUE_23);
		DEBUG_0("cfglft: command line error\n");
		exit(E_ARGS);
	}

	/* Validate logical name was specified */
	if (strncmp(logical_name, "lft", 3) ) 
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,0,CFG_BAD_LNAME,UNIQUE_24);
		DEBUG_0("cfglft: logical name must be specified\n");
		exit(E_LNAME);
	}

	/* Start up odm. */
	if ((rc = odm_initialize()) < 0)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_initialize",-1, 
				CFG_ODM_INIT, UNIQUE_25);
		DEBUG_0("cfglft: odm_initialize() failed\n");
		exit(E_ODMINIT);
	}

	/* Search CuDv for customized object with the logical name passed in */

	sprintf(sstring, "name = '%s'", logical_name);
	if ((rc = (int)odm_get_first(CuDv_CLASS, sstring, &cusobj)) == 0)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_first",0,
				CFG_ODM_CUDV, UNIQUE_26);
		DEBUG_1("cfglft: No CuDv object found for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc == -1)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_first",-1,
				CFG_ODM_CUDV, UNIQUE_27);
		DEBUG_1("cfglft: ODM failure getting CuDv, crit=%s\n",sstring);
		err_exit(E_ODMGET);
	}

	/*
	 * Get the predefined object for this device. This object is located
	 * searching the predefined devices object class based on the unique
	 * type link descriptor found in the CuDv class.
	 */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	if ((rc = (int)odm_get_first(PdDv_CLASS, sstring, &preobj)) == 0)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_first",0,
				CFG_ODM_PDDV, UNIQUE_28);
		DEBUG_1("cfglft: found no PdDv object for crit=%s\n", sstring);
		err_exit(E_NOPdDv);
	}
	else if (rc == -1)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","odm_get_first",-1,
				CFG_ODM_PDDV, UNIQUE_29);
		DEBUG_1("cfglft: ODM failure getting PdDv, crit=%s\n",sstring);
		err_exit(E_ODMGET);
	}

	/*
	 * If this device is being configured during an ipl phase, then
	 * display this device's LED value on the system LEDs.
	 */
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/*
	 * Check to see if the device is already defined.
	 * We actually go about the business of configuring the device
	 * only if the device is defined. Configuring the device in this
	 * case refers to the process of checking for attribute consistency,
	 * building a DDS, loading the driver, etc...
	 */
	if (cusobj.status == DEFINED)
	{
		if (cusobj.chgstatus == MISSING)
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft",NULL,0,CFG_DEV_MISSING,
					UNIQUE_30);
			DEBUG_1("cfglft: device %s is MISSING\n",logical_name);
			err_exit(E_DEVSTATE);
		}

		/* create and initialize the dds structure for the lft */
		if ((rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen))
				!= E_OK)
		{ 
			cfg_lfterr(NULL,"CFGLFT","cfglft","build_dds",rc,
					CFG_BAD_DDS, UNIQUE_31);
			DEBUG_1("cfglft: error building dds, rc=%d\n", rc)
			err_exit(rc);
		}

		/*
		 * Check to see if the device has a device driver. If it
		 * does, then call loadext(). If needed, this function
		 * will load the driver and return the driver's kernel
		 * module id (kmid). If the device has a driver, then the
		 * device driver instance field in the device's customized
		 * object will be a pathname to the device driver in the
		 * directory /etc/drivers.
		 */
		if (strcmp(preobj.DvDr, "") != 0)
		{
			/* Call loadext() to load device driver.	*/
			if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE))
					== NULL)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft","loadext",0,
						CFG_BAD_LOAD, UNIQUE_32);
				DEBUG_1("cfglft: error loading driver %s\n",
					preobj.DvDr);
				err_exit(E_LOADEXT);
			}

			/*
			 * genmajor() will create a major number for this device
			 * if one has not been created yet. If one already 
			 * exists, it will return that one.
			 */
			if ((majorno = genmajor(preobj.DvDr)) == -1)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft","genmajor",-1,
						CFG_BAD_MAJOR, UNIQUE_33);
				DEBUG_0("cfglft:error creating major number\n");
				err_undo1(preobj.DvDr); /* unload driver */
				err_exit(E_MAJORNO);
			}

			/* Create minor number. */
			minor_ptr = getminor(majorno, &how_many, logical_name);
			if (minor_ptr == NULL || how_many == 0)
			{
				if((minor_ptr = genminor(logical_name, majorno,
						-1,1,1,1)) == NULL)
				{
					reldevno(logical_name, TRUE);
					err_undo1(preobj.DvDr);
					cfg_lfterr(NULL,"CFGLFT","cfglft",
							"genminor",0,
							CFG_BAD_MINOR,
							UNIQUE_34);
					DEBUG_0("cfglft: error creating minor \
							number\n")
					err_exit(E_MINORNO);
				}
			}
			minorno = *minor_ptr;

			/* Create devno for this device */
			cfg.devno = makedev(majorno, minorno);
			((lft_dds_t*)cfg.ddsptr)->lft.devno = cfg.devno;

			/* Now call sysconfig() to configure the driver. */
			cfg.cmd = CFG_INIT;
			if (sysconfig(SYS_CFGDD, &cfg, 
					sizeof(struct cfg_dd )) == -1)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft","sysconfig",-1,
						CFG_BAD_INIT, UNIQUE_35);
				DEBUG_1("cfglft: error configuring driver %s\n",
						logical_name);
				err_undo1(preobj.DvDr);
				err_exit(E_CFGINIT);
			}

			/*
			 * Now make the special files that this device will
			 * be accessed through.
			 */
			if ((rc = make_special_files(logical_name, cfg.devno)) 
					!= E_OK)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft",
						"make_special_files",rc,
						CFG_BAD_SP_FILE, UNIQUE_36);
				DEBUG_2("cfglft: error making special file \
						for %s, devno=0x%x\n",
						logical_name, cfg.devno);
				err_undo2(cfg.devno);
				err_undo1(preobj.DvDr);
				err_exit(rc);
			}
	
			/* Push on the ldterm and tioc streams modules */
			if ((rc = lft_autopush(cfg.devno)) != SUCCESS )
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft","lft_autopush",
						rc, CFG_BAD_STREAM, UNIQUE_37);
				DEBUG_1("cfglft: error pushing streams modules \
						for %s\n", logical_name);
				err_undo2(cfg.devno);
				err_undo1(preobj.DvDr);
				err_exit(rc);
			}

		} /* end if (device has a driver) then ... */

		/*
		 * Update the customized object to reflect the device's
		 * current status. The device status field should be
		 * changed to AVAILABLE, and, if they were generated, the
		 * device's major & minor numbers should be updated.
		 */
		cusobj.status = AVAILABLE;
		if ((rc = odm_change_obj(CuDv_CLASS, &cusobj)) < 0)
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft","odm_change_obj",-1,
						CFG_ODM_CUDV, UNIQUE_38);
			DEBUG_0("cfglft: ODM failure updating CuDv object\n");
			err_undo2(cfg.devno);
			err_undo1(preobj.DvDr);
			unlink(DEV_PATH);
			err_exit(E_ODMUPDATE);
		}

	}

	 /*
	    The following checks for the pathname of the console - lft or tty
	    The console device pathname is a customizes attribute belonging
	    to the system object - sys0.  We use getattr to search the CuAt
	    first followed by PdAt if nothing is found in CuAt
	*/
	if( (cuat_ptr = getattr("sys0","syscons",FALSE,&rc)) ==	NULL )
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","getattr",0,
				CFG_ODM_CUAT,UNIQUE_39);
		DEBUG_0("cfglft : error getting syscons from CuAt\n");
		return(E_ODMGET);
	}
	/*
	    If the value for attribute syscons is not lft0, we will make
	    an entry in the /etc/inittab file using the mkitab command.
	    If the value for attribute syscons is lft0, it implies that
	    the system console is set to lft therefore we should delete any
	    lft entry in the inittab file so that multiple gettys do not
	    occur on the lft.
	*/

	rc = odm_run_method("/usr/sbin/lsitab", LFTID, &outp, &errp);
	if ( strncmp("lft0", cuat_ptr->value + 5, 4) != 0 )
	{
		if(rc == 1 && cuat_ptr->value[0] != '\0')
		{
		    /* LFT is not console and not in inittab, so create it */
		    rc = odm_run_method("/usr/sbin/mkitab", LFTON, &outp, &errp);
		    if(rc == -1)
		    {
			cfg_lfterr(NULL,"CFGLFT","cfglft", "odm_run_method",0,
				 CFG_BAD_INIT, UNIQUE_40);
			DEBUG_0("cfglft: error updating inittab using mkitab.\n");
		    }
		}
		else if(rc == 0 && cuat_ptr->value[0] == '\0')
		{
			/* Console is not defined and has LFT in inittab, so remove it */
			rc = odm_run_method("/usr/sbin/rmitab", LFTID, &outp, &errp);
			if(rc == -1)
			{
				cfg_lfterr(NULL,"CFGLFT","cfglft", "odm_run_method",0,
					 CFG_BAD_INIT, UNIQUE_46);
				DEBUG_0("cfglft: error updating inittab using rmitab.\n");
			}
		}
	}
	else if(rc == 0) 
	{
		/* LFT is the console and has entry in inittab, so remove it */
		rc = odm_run_method("/usr/sbin/rmitab", LFTID, &outp, &errp);
		if(rc == -1)
		{
			cfg_lfterr(NULL,"CFGLFT","cfglft", "odm_run_method",0,
				 CFG_BAD_INIT, UNIQUE_47);
			DEBUG_0("cfglft: error updating inittab using rmitab.\n");
		}
	}


	/*
	 * Terminate the ODM and exit with success
	 */
	odm_terminate();
	exit(E_OK);
}




/*
 * NAME: lft_autopush
 *
 * FUNCTION: Loads the ldterm and tioc streams modules and then makes
 *           a ioctl call to SAD to autopush them on the lft stream.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 * 	This function make all stuff concerning lft stream part cfg.
 *	Mainly :
 *		- Loads the necessary modules
 *		- Passes them the dds (if they have one)
 *		- invokes the sad driver to auto push them
 *
 * RETURNS:
 *         0 = SUCCESS
 */

int
lft_autopush(dev_t devno)
{
struct strapush modules_list;    /* list of modules to push */
struct cfg_kmod cfg;             /* structure for module configuration */
struct ldterm_dds l_dds;
struct tioc_dds  t_dds;

int file_desc;
int cr;

	/*
	   The ldterm and tioc dds structures are needed to allow the 
	   strload counter to be set properly.
	*/
	bzero((char * )&l_dds, sizeof(struct ldterm_dds));
	l_dds.which_dds = LDTERM_DDS;

	bzero((char * )&t_dds, sizeof(struct tioc_dds));
	t_dds.which_dds = TIOC_DDS;

	/*
	   Zero the module_list structure.  The information in this 
	   structure is retured by the streams_ioctl in ucfglft.
	*/
	bzero((char *)&modules_list, sizeof(struct strapush));

	/*
	   Then We've to loadext the two modules (LDTERM and TIOC) got from the
           list in ODM and call their sysconfig entry:
       	   - LDTERM
	   - TIOC
 	*/
	cfg.cmd = CFG_INIT;
	cfg.mdiptr = (caddr_t)&l_dds;
	cfg.mdilen = sizeof (struct ldterm_dds);
	cfg.kmid = loadext(LDTERM_MODULE, TRUE, FALSE);
	cr = sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg));
	if(cr)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","loadext",cr,
				CFG_BAD_LOAD, UNIQUE_41);
		DEBUG_1("cfglft: error loading ldterm. rc = %d\n",cr);
		return (cr);
	}
	cfg.cmd = CFG_INIT;
	cfg.mdiptr = (caddr_t)&t_dds;
	cfg.mdilen = sizeof (struct tioc_dds);
	cfg.kmid = loadext(TIOC_MODULE, TRUE, FALSE);
	cr = sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg));
	if(cr)
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","loadext",cr,
				CFG_BAD_LOAD, UNIQUE_42);
		DEBUG_1("cfglft: error loading tioc. rc = %d\n",cr);
		return (cr);
	}

	/* Opens sad device */
	file_desc = stream_open(ADMINDEV, O_RDWR);

	
	/*
	   And this is the interface with SAD driver to register the auto-push.
	*/
	strncpy(modules_list.sap_list[0],LDTERM_MODULE,sizeof(LDTERM_MODULE));
	strncpy(modules_list.sap_list[1],TIOC_MODULE,sizeof(TIOC_MODULE));
	modules_list.sap_cmd = SAP_ONE;
    	modules_list.sap_major = major(devno);
	modules_list.sap_minor = minor(devno);
	modules_list.sap_lastminor = 0;
	modules_list.sap_npush = 2;


	/* Calls sad to push these modules at next open of the line */
	cr = stream_ioctl(file_desc, SAD_SAP, &modules_list);
	stream_close(file_desc);
	if(cr != 0)
	{
	 	cfg_lfterr(NULL,"CFGLFT","cfglft","loadext",cr,
                                CFG_BAD_LOAD, UNIQUE_43);
                DEBUG_1("cfglft: error in SAD_SAP: rc =  %d\n",cr);
	}	
	return (cr);
}




/*
 * NAME: err_exit
 *
 * FUNCTION: Terminates ODM.	Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This routine is to be used only within this file.
 *
 * NOTES:
 *
 *	err_exit( exitcode )
 *		exitcode = The error exit code.
 *
 * RETURNS:
 *			None
 */

err_exit(exitcode)
char	exitcode;
{
	/*
	 * Terminate the ODM 
	 */
	odm_terminate();

	exit(exitcode);
}


/*
 * NAME: err_undo1
 *
 * FUNCTION: Unloads the device's device driver. Used to back out on an
 *		error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This routine is to be used only within this file.
 *
 * NOTES:
 *
 * void
 *	err_undo1( DvDr )
 *		DvDr = pointer to device driver name.
 *
 * RETURNS:
 *			None
 */

err_undo1(DvDr)
char	*DvDr;			/* pointer to device driver name */
{
	/* unload driver */
	if (loadext(DvDr,FALSE,FALSE) == NULL) 
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","loadext",0,
				CFG_BAD_UNLOAD, UNIQUE_44);
		DEBUG_0("cfglft: error unloading driver\n");
	}
	return;
}

/*
 * NAME: err_undo2
 *
 * FUNCTION: Terminates the device. Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *		This routine is to be used only within this file.
 *
 * NOTES:
 *
 * void
 *	err_undo2( devno )
 *		devno = The device's devno.
 *
 * RETURNS:
 *			None
 */

err_undo2(devno)
dev_t	devno;			/* The device's devno */
{
	struct	cfg_dd cfg;		 /* sysconfig command structure */

	/* terminate device */
	cfg.devno = devno;
	cfg.kmid = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;

	if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) 
	{
		cfg_lfterr(NULL,"CFGLFT","cfglft","sysconfig",-1,
				CFG_BAD_TERM, UNIQUE_45);
		DEBUG_0("cfglft: error unconfiguring device\n");
	}
	return;
}
