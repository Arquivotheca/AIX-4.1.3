static char sccsid[] = "@(#)56  1.19  src/bos/usr/lib/methods/defsys/defsys.c, cfgmethods, bos41J, 9511A_all 3/8/95 14:31:10";
/*
 * COMPONENT_NAME: (CFGMETHODS) SYSTEM DEFINE PROGRAM
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include "cfgdebug.h"
#include <sys/iplcb.h>
#include <sys/mdio.h>
#include <fcntl.h>
#define _KERNSYS
#define _RS6K_SMP_MCA
#include <sys/systemcfg.h>
#undef _RS6K_SMP_MCA
#undef _KERNSYS

#define SYS0_NAME	"sys0"
#define SYS0_UTYPE	"sys/node/sys"
#define SYS0_UTYPE1	"sys/node/sys1"
#define SYS0_UTYPE_P	"sys/node/sys_p"
#define	DEVPKG_PREFIX	"devices"		/* device package prefix */
#define STD_PKG		"rs6k"
#define SMP_PKG		"rs6ksmp"
#define MP_MODEL 		0x080000A0
#define RS1_MODEL		0x00000001
#define RS1_XIO_MODEL		0x01000000
#define RSC_MODEL		0x02000000
#define RS2_MODEL		0x04000000
#define PowerPC_MODEL		0x08000000 

IPL_DIRECTORY	iplcb_dir;	/* IPL control block directory */
IPL_INFO	iplcb_info;	/* IPL control block info section */
SYSTEM_INFO	iplcb_system;	/* IPL control block system info */

void err_exit();


/*
 * NAME: main
 *
 * FUNCTION: Create a system object (if it is not already present), and
 *		return it's name to the Config-Manager
 *
 * EXECUTION ENVIRONMENT: Called directly by Config-Manager, with no parameters
 *
 * RETURNS: "sys0"
 */

int allpkg = 0;

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];
{
	struct	CuDv	cudv;		/* structure for CuDv object */
	struct	PdDv	pddv;		/* structure for PdDv object */
	struct	PdDv	pddv2;		/* structure for PdDv object */
	int	rc;			/* ODM return code */
	char	sstr[256];		/* ODM search string */
	char	*outp;			/* stdout from define method */
   	char	sys_ut[UNIQUESIZE];	/* sys0  uniquetype 	*/
	uint	model;			/* model type of machine */
	uint 	need_diag = 0;		/* to indicate diag pkg needed */
	char	package[16];		/* package name */
	char 	pkg_desc[16] = "";

	if (!strcmp(getenv("DEV_PKGNAME"), "ALL"))
		allpkg = 1;

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		exit(E_ODMINIT);
	}
	DEBUG_0( "ODM initialized\n")

	/* determine which type of sys object we need to define */

	/* Read in the IPL Control Block directory */
	rc = mdd_get(&iplcb_dir,128,sizeof(iplcb_dir),MIOIPLCB);
	if (rc) 
	{
		err_exit(rc);
	}

   	DEBUG_0( "Got the iplcb directory\n")

	/* Read in the IPL info section of the Control Block */
	rc = mdd_get(&iplcb_info,iplcb_dir.ipl_info_offset,
					sizeof(iplcb_info),MIOIPLCB);
	if (rc) 
	{
		err_exit(rc);
	}

	DEBUG_0( "Got the iplcb_info section \n")


	/* look for package descriptor field in iplcb */
	if ((&(iplcb_dir.system_info_offset) < ((int)(&iplcb_dir) + iplcb_dir.ipl_info_offset-128))
		&& (iplcb_dir.system_info_offset != 0) && (iplcb_dir.system_info_size != 0))
	{
	        DEBUG_0( "The system info information is present - use it\n")

		/* Read in the IPL system section of the Control Block */
		rc = mdd_get(&iplcb_system,iplcb_dir.system_info_offset,
					sizeof(iplcb_system),MIOIPLCB);
		if (rc) 
		{
			err_exit(rc);
		}

               /* Check to see if the system info has a pkg_desc field.  */
                if (((uint)&iplcb_system.pkg_descriptor - (uint)&iplcb_system) 
				< iplcb_dir.system_info_size) 
                {
                    /* set up package descriptor variable */

		    DEBUG_2("pkg descriptor in iplcb is %s  \n and as hex %x \n",
		    iplcb_system.pkg_descriptor, iplcb_system.pkg_descriptor[0])

                    /* get package descriptor from iplcb if not \0 or FF */
        	    if (iplcb_system.pkg_descriptor[0] != '\0' &&
                	iplcb_system.pkg_descriptor[0] != 0xFF)
                        	strcpy(pkg_desc, iplcb_system.pkg_descriptor);

                }
	}

	/* package descriptor field is not there or was null - use macro */
	if (pkg_desc[0] == '\0')
	{
		/* need to use _rs6k_smp_mca macro */
		if (__rs6k_smp_mca())
			strcpy(pkg_desc, SMP_PKG);
		else
			strcpy(pkg_desc, STD_PKG);
	}

	DEBUG_1( "package descriptor set to %s ",
                                pkg_desc)

	/* develop search criteria based on package descriptor */
	if (strcmp(pkg_desc, STD_PKG)==0)
	{

		DEBUG_0("We have a UP model \n")

                /* Determine which UP model we are working with */
		/* Also, set need_diag for any rack mounted system */
		/* so that the diagnostic package will be printed out */
		/* below.						*/
                model = iplcb_info.model & 0xFF000000;

                /* If RS1 model, set a bit so we can use it later */
                if (model == 0)
		{
                    model = RS1_MODEL;

		    /* Check for obsolete model code of 0x20 which */
		    /*	is actually a rack system.		   */
		    if (iplcb_info.model & 0x000000FF == 0x00000020)	
			need_diag = 1;

		    /* For most RS1 models, 0x00000002 means rack */
		    else need_diag = iplcb_info.model & 0x00000002;
		}

		else
		    /* For non RS1 models, 0x00020000 is a rack system */
		    need_diag = iplcb_info.model & 0x00020000;
	}	

	else
	{
		/* If this is an SMP model, load the diagnostic package. */
		if (strcmp(pkg_desc, SMP_PKG)==0)
			need_diag = 1;
	}


	if ((need_diag != 0) && allpkg)
		fprintf(stdout, ":bos.diag ");
	
	sprintf(sstr, "uniquetype like sys/node/* AND devid=%s", pkg_desc);

	DEBUG_1("search criteria for PdDv is %s \n", sstr)

	rc = (int)odm_get_first(PdDv_CLASS,sstr,&pddv);
	if (rc==-1) 
	{
		/* ODM failure */
		DEBUG_0("defsys: ODM failure getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	if (rc==0 || allpkg)
	{
		/* print out package name based on package descriptor */
		if (strcmp(pkg_desc, STD_PKG)==0)
			fprintf(stdout, ":%s.base ", DEVPKG_PREFIX);
		else
			fprintf(stdout, ":%s.%s.base ", DEVPKG_PREFIX, pkg_desc);
	
		/* If we couldn't find a PdDv, then return */	
		if (rc==0)
			/* this case could only occur if PdDv is deleted*/
			exit(0);

	}

	/* At this point, we have a PdDv for the system object */
	/* See if CuDv object exists for system object */
	sprintf(sstr, "name=%s",SYS0_NAME);
	rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudv);
	if (rc==-1) {
		/* ODM failure */
		DEBUG_0("defsys: ODM failure getting CuDv object\n")
		err_exit(E_ODMGET);
	}
	if (rc==0) {
		/* No CuDv object found so need to define one */

		sprintf(sstr,"-c %s -s %s -t %s",
				pddv.class,pddv.subclass,pddv.type);

		DEBUG_2( "Invoking %s method with params:\n%s\n",
						pddv.Define, sstr)

		rc = odm_run_method(pddv.Define,sstr,&outp,NULL);
		if (rc) {
			DEBUG_1( "Returned string: '%s'\n", outp )
			err_exit(E_ODMRUNMETHOD);
		}

		fprintf(stdout,"%s\n",outp);

	} 
	else 
	{
		/* The CuDv exists for the sys object */
		DEBUG_0("CuDv for sys object already exists\n")

		/* Make sure CuDv has the correct uniquetype and 
		   change it if necessary. */

		/* if cudv ut not equal to the pddv ut */
		if (strcmp(cudv.PdDvLn_Lvalue, pddv.uniquetype)) {
			
			/* CuDv could still be correct due to the
			   existence of multiple PdDv's with this devid */

			sprintf(sstr, "uniquetype=%s and devid=%s",
				cudv.PdDvLn_Lvalue, pkg_desc);
	
        		rc = (int)odm_get_first(PdDv_CLASS,sstr,&pddv2);
        		if (rc==-1) {
                		DEBUG_0("defsys: ODM failure getting PdDv object\n")
                		err_exit(E_ODMGET);
        		}

			/* the PdDv for this CuDv was not found, delete
			   the customized information and redefine sys0 
			   because it has the wrong uniquetype       */

        		if (rc==0) {
				sprintf(sstr,"name=%s",SYS0_NAME);
			        rc = (int)odm_rm_obj(CuDv_CLASS,sstr);
        			if (rc == -1)
                			return(E_ODMDELETE);
			        rc = (int)odm_rm_obj(CuAt_CLASS,sstr);
        			if (rc == -1)
                			return(E_ODMDELETE);
				/* define sys0 with correct uniquetype */
		                sprintf(sstr,"-c %s -s %s -t %s",
 	                               pddv.class,pddv.subclass,pddv.type);

 		                DEBUG_2( "Invoking %s method with parms:\n%s\n",
                                                pddv.Define, sstr)

		                rc=odm_run_method(pddv.Define,sstr,&outp,NULL);
                		if (rc) {
                        		DEBUG_1( "Returned string: '%s'\n", 
							outp )
                        		err_exit(E_ODMRUNMETHOD);
               		 	}

			}
		}
                fprintf(stdout,"%s\n",cudv.name);
	}

	(void)odm_terminate();
	exit(0);
}


/*
 * NAME: mdd_get
 *
 * FUNCTION: Reads "num_bytes" bytes from nvram or IPL control block.
 *	     Bytes are read from the offset into nvram or the IPL 
 *           control block and stored at address "dest". 
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
mdd_get(dest, offset, num_bytes, ioctl_type)
char	*dest;
int	offset;
int	num_bytes;
int	ioctl_type;
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram", O_RDONLY)) == -1) { 
                DEBUG_0("Unable to open /dev/nvram")
	        return(E_DEVACCESS);
        }

        DEBUG_2("mdd_get: offset=%08x, size=%08x\n",offset,num_bytes)
        DEBUG_1("mdd_get: ioctl type = %d\n",ioctl_type)

        mdd.md_addr = offset;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	DEBUG_0("Calling mdd ioctl\n")

	if (ioctl(fd, ioctl_type, &mdd) == -1) { 
                DEBUG_0("Error reading IPL-Ctrl block")
                close(fd);
                return(E_DEVACCESS);
        }


	close(fd);
	return(0);
}




/*
 * NAME: err_exit
 *
 * FUNCTION: Terminates ODM on fatal errors.
 *
 * RETURNS: none
 *
 */

void
err_exit(error_code)
int	error_code;

{
	(void)odm_terminate();
	exit(error_code);
}
