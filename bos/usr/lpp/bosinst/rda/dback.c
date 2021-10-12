static char sccsid[] = "@(#)371.13 src/bos/usr/lpp/bosinst/rda/dback.c, bosinst, bos411, 9428A410j 94/03/23 09:33:50";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: dback, get_real_name, output_dev_cmds, output_attr_cmds
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*********************************************************************
 * NAME:	dback.c                                              *
 *                                                                   *
 * FUNCTION: 	This routine uses the information kept about the old *
 *              odm to determine the proper define, config, and      *
 *              change methods required to configure the current     *
 *              odm as closely to the old odm as possible.  It then  *
 *              outputs these methods to the named file which will   *
 *              in korn shell script form.                           *
 *                                                                   *
 * EXECUTION ENVIRONMENT:                                            *
 *                                                                   *
 *               dback must be run after the routine which extracts  *
 *               the information from the old odm.  dback takes its  *
 *               input from stanza files created in that extract     *
 *               process.                                            *
 *                                                                   *
 *	Parameters: -- dback(stanza file containing device info,     *
 *                           stanza file containing attribute info,  *
 *                           output file name)                       *
 *                                                                   *
 *********************************************************************/

#include <sys/stat.h>
#include   <sys/cfgdb.h>
#include   <sys/cfgodm.h>
#include  "dback.h"

int cmd_num = 0;
static struct Class *PdDv_class;	/* class ptr for odm stuff	*/
static struct Class *CuDv_class;	/* class ptr for odm stuff	*/

dback(char *dv_fname, char *at_fname, char *scriptname, int relax)

	{
	FILE   		*fd, *fa;	/* File pointers for dev and attr */
	FILE		*fout;		/* Output file pointer		  */
	int		num_cfg;
	/*
	 * Define names of stanza files.
	 */
					/* array of pointers to device stanza */
	struct dev_struct *devices[MAX_CLASSES];
	/*
	 * array of pointers to device stanza which will be configured by the
	 * device define/cfg section.
	 */
	struct dev_struct *willcfg[MAX_CLASSES];
				/* array of pointers to attribute stanza */
	struct attr_stanza *attributes[MAX_CLASSES];
	int num_of_devs, num_of_attrs, i;
					/* Index structure pointers.	  */
	struct PdDv pddv;             	/* Storage area for Predefined Class */
	struct CuDv cudv;             	/* Storage area for Customized Class */
	char    *new_obj_path ;		/* Path of ODMDIR.		*/


/* BEGIN */
	/*
	 * Open the stanza files.
	 */
	DEBUG_0("dback: fopen dv_fname\n")
	if((fd = fopen(dv_fname,"r")) == NULL)
	   {
	   fprintf(stderr,"Unable to open Config file -->%s<--\n",dv_fname);
	   return(NULL);
	   }

	DEBUG_0("dback: fopen at_fname\n")
	if((fa = fopen(at_fname,"r")) == NULL)
	   {
	   fprintf(stderr,"Unable to open Config file -->%s<--\n",at_fname);
	   return(NULL);
	   }

	/*
	 * Open the output file.
	 */
	if ( scriptname == NULL )
		{
		DEBUG_0("dback: scriptname is null\n")
		fout = stdout;
		}
	else
		{
		DEBUG_1("dback: fopen %s\n",scriptname)
		if((fout = fopen(scriptname,"w")) == NULL)
		    {
		    fprintf(stderr,"Unable to open script file -->%s<-- for writing\n",
			    scriptname);
		    return(NULL);
		    }
		if(chmod(scriptname,S_IRUSR|S_IXUSR) == -1)
		    {
		    fprintf(stderr,"Unable to change permissions for script file \
-->%s<-- to executable\n",
			    scriptname);
		    return(NULL);
		    }
		}

	/*
	 * Set database path.
	 */
	if ((new_obj_path = getenv("ODMDIR")) == (char *)NULL)
	{
	    new_obj_path = "/etc/objrepos" ;
	}
	odm_set_path(new_obj_path) ;
	/* mount the class */
	PdDv_class = odm_mount_class("PdDv");
	CuDv_class = odm_mount_class("CuDv");

	/*
	 * Initialize the devices array with the name, parent, connwhere,
	 * and PdDvLn information out of the stanza created from the old ODM.
	 */
	num_of_devs = make_dev_index(fd, devices);

	/*
	 * Get the real names of devises listed in the index.  It is possible
	 * that the current device name has changed or that the device does not
	 * exist at all.
	 */
	get_real_names(devices, num_of_devs, relax);

	/*
	 * Before writing commands to fout, output the shell to be exec-ed
	 * and any traps to be set.
	 */
	fprintf(fout, PRESCRIPT );

	/*
	 * Process the devices and output define and configure commands for any
	 * applicable devices.
	 */
	num_cfg = output_dev_cmds(fout, devices, num_of_devs, willcfg);

	/*
	 * Initialize the attributes array with the name and attribute
	 * information from the stanza file.
	 */
	num_of_attrs = make_attr_index(fa, attributes);

	/*
	 * Process the attributes for each device and output change methods.
	 */
	output_attr_cmds(fout, attributes, num_of_attrs,
			 devices, num_of_devs, willcfg, num_cfg );

	/*
	 * Complete the shell script with post processing.
	 */
	fprintf(fout, POSTSCRIPT ,cmd_num);

	fclose(fa);
	fclose(fd);
	if (fout != stdout)
		{
		fclose(fout);
		}
	return(E_OK);
	}

/*
 * get_real_names():
 *	Determine if the name of the device has been reassinged during dynamic
 *	device configuration.
 */
get_real_names(struct dev_struct *devices[], int num_of_devs, int relax)
	{
	int i;				/* array index.			*/
	int locmatches = TRUE;
	char   sstring[MAX_CRITELEM_LEN];/* search string 		*/
	struct PdDv pddv;             	/* Storage area for Predefined Class */
	struct CuDv cudv;             	/* Storage area for Customized Class */



	for ( i = 0; i <= num_of_devs; i++ )
	    {
	    /*
	     * If there is no predefined for this device then it cannot be
	     * configured and it will be skipped in the step which defines and
	     * configures the devices.
	     */
	    if ( strcmp(devices[i]->PdDvLn, "") == 0 )
		{
		strcpy (devices[i]->PdDvLn, "null");
		}
	    sprintf(sstring,"uniquetype LIKE %s",devices[i]->PdDvLn);
	    if((int) odm_get_obj(PdDv_class, sstring, &pddv,ODM_FIRST) <= 0)
		{
		continue;
		}

	    /*
	     * If the device is not dectable then use the name from the stanza
	     * file as the real name since the config manager will not name the
	     * device so there is no chance of the device having a different
	     * name in the new CuDv.
	     */
	    if(pddv.detectable != 1)
		{
		continue;
		}

	    /*
	     * If we got this far, there is a predefined entry for the device
	     * and it is detectable.
	     */

	    /*
	     * See if the CuDv has a device with the same name as the device
	     * listed in the stanza file.  If so, compare the uniquetype
	     * (PdDvLn) and location codes of the device in the CuDv with those
	     * of the device listed in the stanza file.  If all of that matches,
	     * then the name from the stanza file is the real name of the device
	     * listed in the CuDv.
	     */
	    if ( strcmp(devices[i]->name, "") == 0 )
		{
		strcpy (devices[i]->name, "null");
		}
	    sprintf(sstring,"name = %s",devices[i]->name);
	    if((int) odm_get_obj(CuDv_class, sstring, &cudv,ODM_FIRST) > 0)
		{
		/*
		 * If this is not the relaxed case, check the location codes.
		 */
		if ( ! relax )
		    {
		    /*
		     * If location code from CuDv does not match the location
		     * code from the stanza file, then set the locmatches
		     * variable to false.
		     */
		    if(strcmp(devices[i]->location,cudv.location) != 0)
			{
			locmatches = FALSE;
			}
		    }
		/*
		 * If the device type is the same and the location code match,
		 * then we have a match and we use the name from the stanza
		 * file as the real name.
		 */
		if ( strcmp(devices[i]->PdDvLn,cudv.PdDvLn_Lvalue) == 0
			&& locmatches )
		    {
		    continue;
		    }
		}

	/*
	 * If we got this far, either the device name from the stanza file did
	 * not exist in the CuDv, the uniquetype (PdDvLn) of the device
	 * retrieved from CuDv did not match that of the device listed in the
	 * stanza file, or the location of the device retrieved from the CuDv
	 * is different than that of the device listed in the stanza file.
	 */

	    /*
	     * Do a lookup in the CuDv based on the location code of the
	     * device listed in the stanza file.  If it is not found,
	     * then the device does not exist as it did previously, and
	     * the configureation for that device cannot be performed.
	     */
	    if ( strcmp(devices[i]->location, "") == 0 )
		{
		strcpy (devices[i]->location, "null");
		}
	    sprintf(sstring,"location = %s",devices[i]->location);
	    if((int) odm_get_obj(CuDv_class, sstring, &cudv, ODM_FIRST) <= 0)
		{
		fprintf(stderr, "The %s device cannot be configured exactly as \
it was previously.\nAttempting to continue.\n", devices[i]->name);
		continue;
		}

	    /*
	     * If the device is found by location, check the uniquetype (PdDvLn)
	     * of the device retrieved from the CuDv to see if it is the same
	     * as that of the device listed in the stanza file.  If they match,
	     * then use the name for the device in that location as listed in
	     * the CuDv.
	     * If they do not match, then the device does not exist as it did
	     * previously, and the configureation for that device cannot be
	     * performed.
	     */
	    if(strcmp(devices[i]->PdDvLn,cudv.PdDvLn_Lvalue) == 0 )
		{
		strcpy(devices[i]->real_name , cudv.name);
		}
	    else
		{
		fprintf(stderr, "The %s device cannot be configured exactly as \
it was previously.\nAttempting to continue.\n", devices[i]->name);
		}
	    }
	}

/*
 * Output the device definition and configure methods for appropriate devices.
 */
output_dev_cmds (
FILE *fout,
struct dev_struct *devices[],
int num_of_devs,
struct dev_struct *willcfg[]
	)
	{
	int i,j;			/* array index.			*/
	char   sstring[MAX_CRITELEM_LEN];/* search string 		*/
	struct PdDv pddv;             	/* Storage area for Predefined Class */
	struct CuDv cudv;             	/* Storage area for Customized Class */
	char *parent_real_name;		/* real name of parent */
	int    found;			/* Output flag			*/
	int    rc, num;			/* Return Code 			*/
	char *define_name;	     /* name of define method without path */

	num = -1;

	for( i = 0; i <= num_of_devs; i++)
		{
		/*
		 * Set output flag.
		 */
		found = FALSE;

		DEBUG_1("dback: Checking for uniquetype LIKE %s in PdDv\n",
				devices[i]->PdDvLn)
		if ( strcmp(devices[i]->PdDvLn, "") == 0 )
		    {
		    strcpy (devices[i]->PdDvLn, "null");
		    }
		sprintf(sstring,"uniquetype LIKE %s",devices[i]->PdDvLn);
		if((int) odm_get_obj(PdDv_class, sstring, &pddv,ODM_FIRST) <= 0)
			{
			continue;
			}

		/*
		 * Check for device already there.
		 */
		DEBUG_1("dback: Checking for name = %s in CuDv\n",
				devices[i]->name)
		if ( strcmp(devices[i]->name, "") == 0 )
		    {
		    strcpy (devices[i]->name, "null");
		    }
		sprintf(sstring,"name = %s",devices[i]->name);
		if((rc = (int) odm_get_obj(CuDv_class,
					   sstring,
					   &cudv,ODM_FIRST)) > 0)
			{
			DEBUG_1("dback: Device name = %s was found\n",
				 devices[i]->name)
			DEBUG_1("dback: cudv.name = %s\n",cudv.name)
			DEBUG_1("dback: cudv.status = %d\n",cudv.status)
			DEBUG_1("dback: cudv.chgstatus = %d\n",cudv.chgstatus)

		/*** If device is found in CuDv then already defined and
		     we don't want to create another one
		     ---> Doesn't account for failure to read
			  odm_get_obj returns >0 if found
					      =0 if not found
					      <0 if error
		*********************************************************/

			found = TRUE;
			}

		DEBUG_0("dback: Checking for exceptions\n")
		/*
		 * Check for exceptions.
		 */
		if(strncmp(devices[i]->name,"inet",4) == 0)
			found = TRUE;

		if(strcmp(pddv.prefix,"vg") == 0 ||
					strcmp(pddv.prefix,"lv") == 0 )
			found = TRUE;
		/*
		 * If not already configured and not detectable, output define
		 * and configure methods.
		 */
		DEBUG_2("dback: found = %d pddv.detectable = %d\n", found,
					pddv.detectable)
		if( ! found && pddv.detectable != 1 )
		    {	
		    /*
		     * Get parent's real name.
		     */
		    DEBUG_0("dback: Get parent's real name\n")
		    parent_real_name = NULL;
		    for ( j = 0; j <= num_of_devs; j++)
			{
			if (strcmp(devices[i]->parent,devices[j]->name) == 0)
				{
				parent_real_name = devices[j]->real_name;
				j = num_of_devs;
				}
			}

		    /*
		     * Get name of define method without the full path prefix.
		     */
		    if ( (define_name = strrchr ( pddv.Define, '/' )) == NULL )
			{
			define_name = pddv.Define;
			}
		    else	/* skip past the last / */
			{
			define_name = define_name + (1 * sizeof(char));
			}

		    /*
		     * Process the exceptions.
		     */
		    /*
		     * For devices with no parent (dlc, netbios, ...) and
		     * devices configured by the "defif" method, use the
		     * mkdev command and only specify class, subclass, and type.
		     */
 		    if ( strcmp (parent_real_name,"") == 0 ||
			 strcmp (define_name, "defif") == 0     )
			{
			    fprintf(fout,
				"cmd_%d='%s -c %s -s %s -t %s '\n",
				cmd_num++, "mkdev", pddv.class, pddv.subclass,
				pddv.type);
			}
		     else
			{
			    DEBUG_0("dback: Output define method.\n")
			    /*
			     * Define the device.
			     */
			    fprintf(fout,
			    "cmd_%d='%s -l %s -c %s -s %s -t %s -p %s -w %s '\n",
			    cmd_num++, pddv.Define, devices[i]->real_name,
			    pddv.class, pddv.subclass, pddv.type,
			    parent_real_name, devices[i]->connwhere);
			    /*
			     * Configure the device.
			     */
			    DEBUG_0("dback: Output config method.\n")
			    fprintf(fout,
				"cmd_%d='%s -l %s '\n", cmd_num++,
				pddv.Configure,devices[i]->real_name);
			}

		    /*
		     * fill in the array which lists the devices to be
		     * configured by this process.
		     */
		    DEBUG_0("dback: Assign willcfg a pointer.\n")
		    willcfg[++num] = devices[i];
		    }
		    DEBUG_0("dback: Go to top of loop.\n")
		}
	return (num);
	}

/*
 * Output the device change methods for changing user-settable device
 * attributes.
 */
output_attr_cmds(
FILE *fout,
struct attr_stanza *attributes[],
int num_of_attrs,
struct dev_struct *devices[],
int num_of_devs,
struct dev_struct *willcfg[],
int num_cfg
	)
    {

    char   sstring[MAX_CRITELEM_LEN];/* search string 		*/
    int	   i,j;		/* array indecies */
    int    rc;			/* Return Code 			*/
    char   real_name[NAMESIZE];
    struct attr_struct	*curattr;
    struct PdDv pddv;             	/* Storage area for Predefined Class */
    struct CuDv cudv;             	/* Storage area for Customized Class */

    for ( i = 0; i <= num_of_attrs; i++)
	{
	/*
	 * Try to find a match between the name in the attributes list and the
	 * name in the devices list.
	 */
	for (j = 0;
	    j <= num_of_devs &&
		strcmp(attributes[i]->name,devices[j]->name) != 0 ;
	     j++) ;

	/*
	 * If device was not found in devices array, assign real name to null.
	 * otherwise, assign real_name to real name of device as listed in
	 * devices array.
	 */
	if ( j > num_of_devs)
	    {
	    real_name[0] = '\0';
	    }
	else
	    {
	    strcpy (real_name, devices[j]->real_name);
	    }

	/*
	 * Get customized information to determine if the device is
	 * already configured.  If it is, we can set its attributes.
	 */
	if ( strcmp(real_name, "") == 0 )
	    {
	    strcpy (real_name, "null");
	    }
	sprintf(sstring,"name = %s",real_name);
	if((rc = (int) odm_get_obj(CuDv_class, sstring, &cudv,ODM_FIRST)) <= 0)
	    {
	    /*
	     * If the device was not found in the customized database, see if
	     * it is in the list of devices about to be configured by this
	     * process.
	     */
	    for (j = 0;
	    j <= num_cfg && strcmp(attributes[i]->name,willcfg[j]->name) != 0 ;
		 j++) ;
	    /*
	     * If device is not in CuDv and not in list of devices we are about
	     * to define, we cannot set its attributes.
	     */
	    if ( j > num_cfg )
		{
		fprintf(stderr,"Could not find entry for %s\n", sstring);
		continue;
		}
	    /*
	     * Otherwise, the device was found, so get link information from
	     * the devices array.
	     */
	    else
		{
		strcpy(cudv.PdDvLn_Lvalue,willcfg[j]->PdDvLn);
		}
	    }

	/*
	 * Get predefined info in order to tell which change methods to use.
	 */
	    if ( strcmp(cudv.PdDvLn_Lvalue, "") == 0 )
		{
		strcpy (cudv.PdDvLn_Lvalue, "null");
		}
	sprintf(sstring,"uniquetype LIKE %s",cudv.PdDvLn_Lvalue);
	if((rc = (int) odm_get_obj(PdDv_class,sstring,&pddv,ODM_FIRST)) <= 0)
	    {
	    fprintf(stderr,"Could not find pddv entry for %s\n",sstring);
	    continue;
	    }
	
	    /*
	     * Do not change attributes if logical volume manager
	     * or the floppy drive is involved.  The reason the floppy
	     * is to be left alone is for restores of the mksysb image
	     * onto different density floppy drives.
	     */
	if(strcmp(pddv.prefix,"vg") &&
			strcmp(pddv.prefix,"lv") &&
			strcmp(pddv.prefix,"fd"))
	    {
	    DEBUG_2("output_attr_cmds: name = %s pddv.prefix = %s\n",
			attributes[i]->name, pddv.prefix)
	    /*
	     * Some devices (i.e. x25) are extremely slow at executing the
	     * 'chdev' command.  Its much faster to have just one 'chdev'
	     * per device with all the attribute value pairs than many 'chdev's
	     * per device with one attribute value pair per command for such
	     * devices.
	     */
	    fprintf(fout,"cmd_%d='%s -l %s ",cmd_num++, "chdev", real_name);
	    for(curattr = attributes[i]->fields;
	        curattr != NULL;
	        curattr = curattr->next)
		{
	        DEBUG_2("	attr:%s	val:%s\n",
				curattr->attribute, curattr->value);
		fprintf(fout,"\n\t-a %s=%s",curattr->attribute, curattr->value);
	        }
	    fprintf(fout,"'\n");
	    }
	} /* end of for each stanza in device attribute list. */
    } /* end of output_attr_cmds() */
