static char sccsid[] = "@(#)70	1.2  src/bos/usr/lib/methods/cfgmpqd/cfgmpqd.c, cfgmethods, bos411, 9428A410j 8/6/93 12:09:04";
/*
 * COMPONENT_NAME: (CFGMETH) cfgmpqd
 *
 * FUNCTIONS: build_dds, generate_minor, make_special_files
 * FUNCTIONS: download_microcode, query_vpd, define_children
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
#include <sys/artic.h>

/*
 *
 * NAME: build_dds
 *
 * FUNCTION: Builds a DDS (Defined Data Structure) describing a device's
 *	characteristics to it's device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	Operates as a device dependent subroutine called by the generic
 *	configure method for all devices.
 *
 * NOTES: There is no DDS for the MPQP adapter (DDS's are built for the child
 *        MPQP ports) this is then a NULL function.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  build_dds( lognam, addr, len )
char	*lognam;		/* logical name of device */
uchar	*addr;			/* receiving pointer for DDS address */
long	*len;			/* receiving variable for DDS length */
{ 

	return 0; 
}
/*
 *
 * NAME: generate_minor
 *
 * FUNCTION: To provide a unique minor number for the current device instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: There is no minor number required for the MPQP adapter,
 *        this function is NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int generate_minor( lognam, majorno, minordest )
char	*lognam;
long	majorno;
long	*minordest;
{ 
	return 0; 
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: To create the special character file on /dev for the current device
 *           instance.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, there is no special character file on /dev, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int make_special_files( lognam, devno )
char	*lognam;
dev_t	devno;
{ 
	return 0; 
}

/*
 *
 * NAME: download_microcode
 *
 * FUNCTION: To download the micro code for the MPQP adapter.
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, micro code must be downloaded through a port,
 *        this function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  download_microcode( lognam )
char  *lognam;
{ 
	return 0; 
}

/*
 *
 * NAME: query_vpd
 *
 * FUNCTION: To query the MPQP device for VPD information (Vital Product Data)
 *
 * EXECUTION ENVIRONMENT:
 *     Operates as a device dependent subroutine called by the generic
 *     configure method for all devices.
 *
 * NOTES: For the MPQP adapter, VPD information is not currently supported, this
 *        function is then NULL.
 *
 * RETURNS: 0 - success, in all cases
 *
 */

int  query_vpd( newobj, kmid, devno, vpd_dest )
char   *newobj;
mid_t   kmid;
dev_t	devno;
char	*vpd_dest;
{ 
	return 0; 
}

/*
 *
 * NAME: define_children
 *
 * FUNCTION: To invoke the generic define method for each child device not
 *           already in the customized database. This will result in all
 *           children that are not AVAILABLE being created in the customized
 *           data base with status DEFINED. To then output on stdout the name
 *           of each DEFINED child device in order to cause that child's
 *           configuration.
 *
 * EXECUTION ENVIRONMENT:
 *    Operates as a device dependent subroutine called by the generic configure
 *    method for all devices.
 *
 * NOTES: For the MPQP adapter, there are four children (ports 0 through 3)
 *        which are specifically looped over.
 *
 * RETURNS: 0 - Success
 *         <0 - Failure
 *
 */

int  define_children( lognam, iplphs )
char  *lognam;				/* logical name of device instance */
int    iplphs;				/* phase if ipl */
{
	long    rc, port;		/* return code, port counter */
	char    string[512];		/* working string */
	char    *out_p;
	long    objtest = 0;		/* existance test, 1-test, 0-don't */
	struct  CuDv   cudvport;	/* Object structures */
	struct  PdDv   pddvport;
        struct  CuDv    cudvdriv;       /* object record for parent dev drvr */
	struct  CuAt    *cusatt;
	struct  Class   *cusdev ;       /* customized devices class ptr */
	ulong   ulong_val;


        DEBUG_1("cfgmpqd: driver lname=%s\n", lognam)
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ open customized device object class ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
                /* error opening class */
                DEBUG_0("cfgmpqd: open of CuDv failed\n")
                return(E_ODMOPEN);
        }

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ get the object record for the driver ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        sprintf(string,"name = '%s'",lognam);
        rc =(int)odm_get_first(cusdev,string,&cudvdriv);
        if (rc==0) {
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ failed to get an object ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                DEBUG_0("cfgmpqd: No device CuDv object\n")
                return(E_NOCuDv);
        }
        else if (rc==-1) { /* ODM error */
                DEBUG_0("cfgmpqd: ODM error getting CuDv object\n")
                return(E_ODMGET);
	}

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ read parent attribute from CuAt, and PdAt tables ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        sprintf( string, "name = '%s'", cudvdriv.parent );
        if ( (( cusatt = get_attrval( cudvdriv.parent, "_subtype",
               string, &ulong_val, &rc ) ) != NULL ) &&
                ( rc == E_OK ) ) {

                rc = strcmp( cusatt->value, PM_4PORT_MPQP_MSG );
                if( rc != 0 ) { /* this is not a MPQP adapter */
                        DEBUG_2("cfgmpqd: wrong parent adapter lname=%s _subtype =%s\n", cudvdriv.parent,cusatt->value)
			/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
       			³ Update customized device object.  Set current ³
       			³ status to DEFINED and reset change status to  ³
       			³ SAME only if it was MISSING                   ³
       			ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        		cudvdriv.status = DEFINED;
        		if (cudvdriv.chgstatus == MISSING)
        		{
            		cudvdriv.chgstatus = SAME ;
        		}
        		if (odm_change_obj(cusdev, &cudvdriv) == -1)
        		{
            			/* ODM failure */
            			DEBUG_1("cfgmpqd: ODM failure, %d, updating CuDv object\n", odmerrno);
            			return(E_ODMUPDATE);
        		}
                        return(E_WRONGDEVICE);
                }
        }
        else { /* failed to get the _subtype attribute of the parent adapter */
                DEBUG_0("cfgmpqd: Failed to get parent _subtype\n")
                return(rc);
        }
        DEBUG_2("cfgmpqd: parent adapter lname=%s _subtype =%s\n", cudvdriv.parent,cusatt->value)

	/* read Predef object for MPQP ports */
	if(( rc = odm_get_obj( PdDv_CLASS,"uniquetype = 'port/mpqp/mpqport'",
		&pddvport, ODM_FIRST )) == 0 ) {
		DEBUG_2("def_chil: get failed lname=%s rc=%d\n",
			lognam,rc)
		return E_NOPdDv;
	} else if ( rc == -1 ) {
		DEBUG_1("def_chil: odmget error lname=%s\n",lognam)
		return E_ODMGET;
	}

	DEBUG_0("def_chil: got pddv object\n")

	for (port=0; port<4; ++port)	/* for each port on the adapter */
	{  
		objtest = 1;

		/* retrieve current CuDv port object */
		sprintf( string, "parent = '%s' AND connwhere = '%d'", lognam,
			port );
		DEBUG_1("def_chil: str=*%s*\n",string)
		rc =(long)odm_get_obj(CuDv_CLASS,string,&cudvport,ODM_FIRST);
		DEBUG_1("def_chil: after get rc=%d\n",rc)

		if (rc == 0)	/* if current port is not defined ... */
		/* invoke the generic define method */
		/* retrieve current CuDv port object */
		{  
			sprintf( string,
				"-c port -s mpqp -t %s -p %s -w %d",
				"mpqport", lognam, port );
			DEBUG_2("def_chil: calling %s %s\n",pddvport.Define,
				string)
			if(odm_run_method(pddvport.Define,string,&out_p,NULL)){
				fprintf(stderr,"cfgmpqd: can't run %s\n",
					pddvport.Define);
				return E_ODMRUNMETHOD;
			}
			fprintf( stdout, "%s\n", out_p );
			objtest = 0;
		}
		else if (rc > 0) {
			cudvport.chgstatus = SAME;

			/*
	 		* Change Customized Device Object Class
	 		*/
			if ((rc = odm_change_obj(CuDv_CLASS,&cudvport)) < 0) {
				fprintf(stderr,"build_dds: change failed\n");
	   			return E_ODMUPDATE;
			}
			fprintf (stdout, "%s\n", cudvport.name);

		}
	}
	return 0;
}

