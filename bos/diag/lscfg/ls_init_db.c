static char sccsid[] = "@(#)21	1.10.1.11  src/bos/diag/lscfg/ls_init_db.c, lscfg, bos41J, 9521B_all 5/26/95 10:04:28";
/*
 * COMPONENT_NAME: DUTIL - Diagnostic Utility 
 *
 * FUNCTIONS: 	init_db
 *		find_CuVPD
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <nl_types.h>
#include <sys/cfgdb.h>                  /* cfg db literals */
#include <odmi.h>
#include "diag/class_def.h"             /* object class data structures */
#include "diag/tmdefs.h"
#include "diag/diag.h"

#define DEPTH 1
#define	DIAG_CONVERSION_KEY	"DIAGS_VERSION_4"

/* GLOBAL VARIABLES */
struct PDiagDev *T_PDiagDev;
struct CDiagDev *T_CDiagDev;
struct CuDv 	*T_CuDv;
struct CuDv **Parent_CuDv;
int num_CuDv;
int num_PDiag;
int num_CDiag; 

struct CuVPD	*CuVPD_HW;
struct CuVPD	*CuVPD_USER;

int			All_ls_count;     	/* count of devices in All    */
short	old_CDiagDev;	/* flag to see if the CDiagDev is old, i.e  */
			/* have device type instead of device name  */
			/* in the DType field.			    */

extern diag_dev_info_ptr_t     *All;          	/* all supported devices      */

/* CALLED FUNCTIONS */
char *substrg(int, char *);
struct CDiagDev 	*find_CDiag( );
struct PDiagDev 	*find_PDiag( );
struct CuVPD    	*find_CuVPD( );
struct CuDv 		**hash_CuDv( );
diag_dev_info_ptr_t     *generate_All(); /* generate stacked devices   */
extern char *strpbrk( );
extern nl_catd catopen(char *, int);
extern char *catgets(nl_catd, int, int, char *);
void do_error(void);

/*  */

/* NAME: init_db 
 *
 * FUNCTION: This function creates a master array containing entries 
 * 	for each device identified in the current machine. 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	diag_dev_info_t *    : pointer to series of linked structures
 *	diag_dev_info_t * -1 : error
 *
 */

diag_dev_info_ptr_t *
init_db (num_devices)
int *num_devices;
{

	int 			i, j, len, device_added;
	char			*base_text;
	char			*temp_ptr;
	char			criteria[48];
	char			*odmemsg;
	char			*filename;
	char 			new_path[256];

	struct listinfo	c_info;
	struct listinfo	p_info;
	struct listinfo	d_info;
	struct listinfo	cdiag_info;
	diag_dev_info_t 	*Tmp;
	
	struct	PdDv	*pddv;
	struct	CDiagDev *cdiagdev;
	char	*ptype,*psclass,*pclass;
	nl_catd fdes_dev, dflt_fdes_dev;

	*num_devices = 0;	

	/* get all device types in the Customized Device Class */
	T_CuDv = get_CuDv_list(CuDv_CLASS, "chgstatus != 3", 
				&c_info, MAX_EXPECT, DEPTH);

	if ( T_CuDv == (struct CuDv *) -1 ) {
		do_error();
		return( (diag_dev_info_ptr_t *) -1);
	}
	num_CuDv = c_info.num;

	/* create hash on parent index in T_CuDv */
	Parent_CuDv = hash_CuDv ( num_CuDv );
	if ( Parent_CuDv == (struct CuDv **) -1) {
		do_error();
		return( (diag_dev_info_ptr_t *) -1);
	}

	/* get the predefined diag devices -> devices supported by diag */
	T_PDiagDev = get_PDiagDev_list( PDiagDev_CLASS, "", &p_info, 
						MAX_EXPECT, 1);
	if ( T_PDiagDev == (struct PDiagDev *) -1 ) {
		do_error();
		return( (diag_dev_info_ptr_t *) -1);
	}
	num_PDiag = p_info.num;

	/* get customized diag data: test status, deletion flag */
	T_CDiagDev = get_CDiagDev_list(CDiagDev_CLASS, "", &c_info, 
						MAX_EXPECT, 1);
	if ( T_CDiagDev == (struct CDiagDev *) -1 ) {
		do_error();
		return( (diag_dev_info_ptr_t *) -1);
	}
	num_CDiag = c_info.num;

        /* Determine here if we are dealing with an old CDiagDev  */
	/* or a newly converted CDiagDev.                         */

	sprintf(criteria, "DType = %s", DIAG_CONVERSION_KEY);
	cdiagdev = (struct CDiagDev *)get_CDiagDev_list(CDiagDev_CLASS,
			criteria, &cdiag_info, 1, 1);
	if(cdiag_info.num == 0)
		old_CDiagDev=1;
	else
		odm_free_list(cdiagdev, &cdiag_info);


	/* allocate space for TOPMaster */
	Tmp = (struct diag_dev_info_s *) 
		calloc( num_CuDv, sizeof( struct diag_dev_info_s ) );

	for (i=0; i < num_CuDv; i++){

		/* set customized device pointer */
		Tmp[i].T_CuDv = &T_CuDv[i];

		ptype = (char *)substrg(PTYPE,&Tmp[i].T_CuDv->PdDvLn_Lvalue);
		psclass = (char *)substrg(PSCLASS,
				&Tmp[i].T_CuDv->PdDvLn_Lvalue);
		pclass = (char *)substrg(PCLASS,
				&Tmp[i].T_CuDv->PdDvLn_Lvalue);

		sprintf(criteria,"uniquetype = %s",T_CuDv[i].PdDvLn_Lvalue);
		pddv = get_PdDv_list(PdDv_CLASS,criteria,&d_info,1,1);
		if(pddv == (struct PdDv *) -1) {
			do_error( );
			return( (diag_dev_info_ptr_t *) -1);
		}

		Tmp[i].T_Pdv = (struct Pdv *) calloc(1,sizeof(struct Pdv));
		if(Tmp[i].T_Pdv == (struct Pdv *) 0)
			return( (diag_dev_info_ptr_t *) -1);
		Tmp[i].T_Pdv->led = pddv->led;
		Tmp[i].T_Pdv->detectable = pddv->detectable;
		Tmp[i].T_Pdv->fru = pddv->fru;
		Tmp[i].T_Pdv->setno = pddv->setno;
		Tmp[i].T_Pdv->msgno = pddv->msgno;
		strcpy(Tmp[i].T_Pdv->catalog,pddv->catalog);
						
		odm_free_list(pddv,&d_info);
		/* set predefined diagnostic pointer */ 
		Tmp[i].T_PDiagDev = find_PDiag( ptype ,psclass, pclass);

		/* look for any hardware vpd */ 
		Tmp[i].CuVPD_HW = find_CuVPD( T_CuDv[i].name, HW_VPD );
		/* look for any user vpd */ 
		Tmp[i].CuVPD_USER = find_CuVPD( T_CuDv[i].name, USER_VPD );

		/* set customized diagnostic pointer, allocate if necessary */
		if (Tmp[i].T_PDiagDev)
			if(old_CDiagDev)
				Tmp[i].T_CDiagDev = 
					find_CDiag( ptype, T_CuDv[i].location, pclass, (struct PDiagAtt *)NULL );
			else
				Tmp[i].T_CDiagDev =
					find_CDiag( T_CuDv[i].name, T_CuDv[i].location, pclass, (struct PDiagAtt *)NULL );
		else Tmp[i].T_CDiagDev = (struct CDiagDev *) NULL;
	}

	/* get text for devices */
	for (i=0; i < num_CuDv; i++) {
		if ( Tmp[i].T_Pdv->msgno != 0 ) {
			filename = Tmp[i].T_Pdv->catalog;
        		sprintf(new_path, "%s/%s", CFGMETHODSDIR, filename);
			fdes_dev = catopen(filename, NL_CAT_LOCALE);
			if ((-1 == (int )fdes_dev) && (*filename != '/'))
          			fdes_dev = catopen(new_path, NL_CAT_LOCALE);

			base_text = catgets(fdes_dev, Tmp[i].T_Pdv->setno,
		  		            Tmp[i].T_Pdv->msgno, "n/a" );
			/* If cannot find message in catalog, then look in   */
			/* /usr/lib/methods. This will take care of the case */
			/* where only the device's default message catalog   */
			/* is updated 					     */
			if(!strcmp(base_text, "n/a"))
			{
          			dflt_fdes_dev=catopen(new_path,NL_CAT_LOCALE);
				temp_ptr=catgets(dflt_fdes_dev,
						Tmp[i].T_Pdv->setno,
		  		                Tmp[i].T_Pdv->msgno, "n/a" );
				/* copy the msg to a new memory block  */
				/* to close the default catalog.       */
			        len = strlen(temp_ptr);
	    			base_text = (char *)malloc(len + 1);
				strcpy(base_text, temp_ptr);
				if ((int )dflt_fdes_dev > 0)
					catclose(dflt_fdes_dev);
			}
			Tmp[i].Text = (char *)malloc(strlen(base_text) + 1);
			strcpy(Tmp[i].Text, base_text);
			if ((int )fdes_dev > 0) catclose(fdes_dev);
		}
		else
			get_device_text(Tmp[i].T_CuDv,Tmp[i].T_Pdv, 
				     &Tmp[i].Text );
	}
	*num_devices = num_CuDv;
	if ( (All = generate_All(num_CuDv, Tmp, num_devices)) == 
				(diag_dev_info_ptr_t *)-1 )
		return( (diag_dev_info_ptr_t *) -1);
	return(All);
}
			
/* NAME: find_CuVPD 
 *
 * FUNCTION: This function searches the CuVPD object class for any
 * 	VPD data. 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	 struct CuVPD *       : pointer to an entry in CuVPD 
 *	(struct CuVPD *)NULL  : no entries
 *
 */

struct CuVPD *
find_CuVPD( name, type )
char		*name;
int		type;		/* HW_VPD or USER_VPD */
{
	int 		i;
	struct CuVPD	*CuVPD;
	struct listinfo v_info;
	char	criteria[128];
	
	sprintf(criteria, "name = %s and vpd_type = %d", name, type);

	/* search CuVPD for an entry */
	CuVPD = get_CuVPD_list(CuVPD_CLASS, criteria, &v_info, 1, 1); 
	
	if ( CuVPD == (struct CuVPD *) -1 ) {
		do_error();
		return( (struct CuVPD *) NULL);
	}
	return( &CuVPD[0] );

}

void do_error( void)
{
char *emsg;
int err = odmerrno;
int rc;

	rc = odm_err_msg(err,&emsg);
	fprintf(stdout,"%s\n",emsg);
}


/* ^L */

/* NAME: hash_CuDv
 *
 * FUNCTION: This function creates an array, Parent_CuDv, that supplements
 *      T_CuDv. Each element has a counterpart in T_CuDv. For example, the
 *      element T_CuDv[i] corresponds to Parent_CuDv[i].  The array
 *      Parent_CuDv contains a pointer to the T_CuDv[i] entry that describes
 *      the parent.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      struct CuDv **           : pointer to Parent_CuDv
 *      struct CuDv ** -1        : error
 *
 */

struct CuDv **hash_CuDv(int num)
{
        int i, j;
        struct CuDv **Tmp;

        Tmp = (struct CuDv **) calloc(num, sizeof(Tmp[0]));
        if (Tmp == (struct CuDv **) NULL)
                return( (struct CuDv **) -1 );
        for (i = 0; i < num; i++){
                j = i;
                if (T_CuDv[i].parent[0] != '\0'){
                        for ( j = i-1; j >= 0; j--)
                                if (!strcmp(T_CuDv[i].parent, T_CuDv[j].name))
                                        break;
                        if (j < 0)
                                for (j = num-1; j > i; j--)
                                        if (!strcmp(T_CuDv[i].parent,
                                                    T_CuDv[j].name))
                                                break;
                }
                if ( j != i )
                        Tmp[i] = &T_CuDv[j];
                else
                        Tmp[i] = (struct CuDv *) -1;
        }
        return( Tmp );
}

/* ^L */

/* NAME: find_CDiag
 *
 * FUNCTION: This function returns a pointer to a previous entry
 *      in the Customized Diagnostic Device Object Class if present
 *      or a pointer to a new initialized structure.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *       struct CDiagDev *    : pointer to new structure
 *       struct CDiagDev * -1 : error
 *
 */

struct CDiagDev *find_CDiag(
        char *type,
        char *loc,
        char *class,
        struct PDiagAtt *pdiagatt)
{
        int             i, j, k, l;
        int     flag;
        int parentfound =0;
        struct CDiagDev *new;
        char    typepath[48];
        char    newsparent[16];

        char    *tokn;

        /* find the device in the CDiagDev table */
	/* type can be device type or device name*/

        for (i = 0; i < num_CDiag; i++){
                if ( !strncmp(T_CDiagDev[i].Location, loc, LOCSIZE) &&
                     !strncmp(T_CDiagDev[i].DType, type, TYPESIZE) )
                        break;
        }

        /* If found, just set pointer              */
        if (i != num_CDiag)
                new = &T_CDiagDev[i];

        /* else allocate space for new one         */
        else {
                new = (struct CDiagDev *) calloc( 1, sizeof(struct CDiagDev) );
                if (new == (struct CDiagDev *) NULL )
                        return( (struct CDiagDev *) -1 );
                strncpy(new->DType, type, TYPESIZE);
                strncpy(new->Location, loc, LOCSIZE);
                new->SysxTime = 0;
                /* set default - in the diagnostic test list */
                new->RtMenu = RTMENU_DEF;

		if(old_CDiagDev)
		{
        	        /* search for parent and set child test list bit    */
			/* the same. Device Type is used in search criteria */
	                for(j=0;j<num_CuDv && parentfound < 1; j++)
       	         	{
                	        strcpy(typepath,T_CuDv[j].PdDvLn_Lvalue);
                       	 	tokn = (char *)strtok(typepath, "/");
                        	tokn = (char *)strtok(NULL, "/");
                        	tokn = (char *)strtok(NULL, "/");

                        	if((!strcmp(T_CuDv[j].location, loc)) &&
                                                (!strcmp(tokn, type)))
                        	{
                                	strcpy(newsparent, T_CuDv[j].parent);
                                	for(k=0; k< num_CuDv && parentfound <1; k++)
                                	{
                                        	if(!strcmp(T_CuDv[k].name, newsparent))
                                        	{
                                                	strcpy(typepath,T_CuDv[k].PdDvLn_Lvalue);
                                                	tokn = (char *)strtok(typepath, "/");
                                                	tokn = (char *)strtok(NULL, "/");
                                                	tokn = (char *)strtok(NULL, "/");
                                                	for(l=0;l<num_CDiag&&parentfound<1; l++)
                                                	{
                                                        	if((!strcmp(tokn,T_CDiagDev[l].DType)) &&
                                                           	   (!strcmp(T_CuDv[k].location, T_CDiagDev[l].Location))
)
                                                        	{
                                                                	new->RtMenu = T_CDiagDev[l].RtMenu;
                                                                	parentfound = 1;
                                                        	} /* if strcmp tokn */
	                                                } /* for l */
                                        	} /* if newsparent */
                                	} /* for k */
                        	} /* if T_CuDv[j].location */
                	}/* for j */
		} else /* New CDiagDev use device name in search criteria */
		{
	                for(j=0;j<num_CuDv && parentfound < 1; j++)
       	         	{

                        	if((!strcmp(T_CuDv[j].location, loc)) &&
                                              (!strcmp(T_CuDv[j].name, type)))
                        	{
                                	strcpy(newsparent, T_CuDv[j].parent);
                                	for(k=0; k< num_CuDv && parentfound <1; k++)
                                	{
                                        	if(!strcmp(T_CuDv[k].name, newsparent))
                                        	{
                                                	for(l=0;l<num_CDiag&&parentfound<1; l++)
                                                	{
                                                        	if((!strcmp(T_CuDv[k].name,T_CDiagDev[l].DType)) &&
                                                           	   (!strcmp(T_CuDv[k].location, T_CDiagDev[l].Location))
)
                                                        	{
                                                                	new->RtMenu = T_CDiagDev[l].RtMenu;
                                                                	parentfound = 1;
                                                        	} /* if strcmp name */
	                                                } /* for l */
                                        	} /* if newsparent */
                                	} /* for k */
                        	} /* if T_CuDv[j].location */
                	}/* for j */

		}
                if( (pdiagatt == (struct PDiagAtt *)NULL) ||
                    (pdiagatt == (struct PDiagAtt *)-1) )
                        new->Periodic = DEFAULT_TESTTIME;
                else {
                        if(flag & SUPTESTS_PERIODIC_MODE)
                                if(!strcmp(class, CLASS_IOPLANAR))
                                        new->Periodic = DEFAULT_IOP_TESTTIME;
                                else if(!strcmp(class, CLASS_DISK))
                                        new->Periodic = DEFAULT_DISK_TESTTIME;
                                else
                                        new->Periodic = DEFAULT_TESTTIME;
                }
                new->Frequency = 0;
        }
        new->State    = STATE_NOTEST;
        new->TstLvl   = TSTLVL_NOTEST;
        new->More     = DIAG_FALSE;
        new->NewEntry = DIAG_TRUE;


        return ( new );

}
/* ^L  */
/* NAME: find_PDiag
 *
 * FUNCTION: This function searches the Diagnostic Predefined Obj
 *      Class for the specified device type.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *       struct PDiagDev *    : pointer to an entry in PDiagDev
 *      (struct PDiagDev *)-1 : could not find the specified type
 *
 */

struct PDiagDev *find_PDiag(
        char *type,
        char *subclass,
	char *class)
{
        int             i,j;

        for (i = 0; i < num_PDiag; i++){
                if ( ( !strncmp(T_PDiagDev[i].DType, type, TYPESIZE) ) &&
		     ( !strncmp(T_PDiagDev[i].DClass, class, CLASSIZE) ) &&
                     ( !strncmp(T_PDiagDev[i].DSClass, subclass, CLASSIZE) ) )
                                break;
        }

	/* if not found serach one more time for supplemental device not */
	/* having DClass filled in.					 */

        if ( i == num_PDiag ){

		for(j=0; j< num_PDiag; j++)
                	if ( ( !strncmp(T_PDiagDev[j].DType, type, TYPESIZE) ) &&
                     	     ( !strncmp(T_PDiagDev[j].DSClass, subclass, CLASSIZE) ) )
                                break;
		if(j == num_PDiag) /* now return null pointer if not found */			
	                return( (struct PDiagDev *) NULL );
	}
        return( &T_PDiagDev[i] );

}

/* ^L */

/* NAME: get_device_text
 *
 * FUNCTION: This function searches the CuAt data base for attributes for
 *              a device. If not found, use standard device text.
 *              Else build text string from attributes.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *
 */
int get_device_text(
        struct CuDv *cudv,
        struct Pdv *pddv,
        char **textptr)
{
        int             msgno=0;
	int		len;
        struct CuAt     *cuat = (struct CuAt *) NULL;
        struct PdAt     *pdat = (struct PdAt *) NULL;
        struct listinfo c_info;
        char            *tmp, *ptype;
	char		*temp_ptr=(char *) NULL;
        char            crit[100];
        nl_catd         fdes_desc,dflt_fdes_dev;
	char		*filename;
	char 		new_path[256];

        /* check for type 'T' attribute in customized attributes */
        sprintf( crit, "name = %s AND type = T", cudv->name);
        cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit, &c_info, 1,1 );
        if ( cuat == (struct CuAt *) -1 )
                return(-1);

        /* if no customized attribute, then get default from PdAt */
        if ( c_info.num == 0 ) {
                sprintf( crit, "uniquetype = %s AND type = T",
                                                cudv->PdDvLn_Lvalue);
                pdat = (struct PdAt *)get_PdAt_list(PdAt_CLASS, crit,
                                                        &c_info, 1,1 );
                if ( pdat == (struct PdAt *) -1 )
                        return(-1);
                else if (c_info.num == 1 )
                        msgno = atoi(pdat->deflt);
        }
        else
                msgno = atoi(cuat->value);

        /* use attributes value for message index */
	filename = pddv->catalog;
       	sprintf(new_path, "%s/%s", CFGMETHODSDIR, filename);
        fdes_desc = catopen(filename, NL_CAT_LOCALE);

	if ((-1 == (int )fdes_desc) && (*filename != '/'))
                fdes_desc = catopen(new_path, NL_CAT_LOCALE);

        tmp = catgets(fdes_desc, pddv->setno, msgno, "n/a");
	/* If cannot find message in catalog, then look in   */
	/* /usr/lib/methods/devices.cat. This will take care */
	/* of the case where only the devices.cat is updated */

	if(!strcmp(tmp, "n/a"))
	{
          	dflt_fdes_dev = catopen(new_path, NL_CAT_LOCALE);
		tmp = catgets(dflt_fdes_dev, pddv->setno, msgno, "n/a" );
		/* copy the msg to a new memory block  */
		/* to close the default catalog.       */

  		temp_ptr = (char *)malloc(strlen(tmp) + 1);
		strcpy(temp_ptr, tmp);
		tmp = temp_ptr;
		if ((int )dflt_fdes_dev > 0) catclose(dflt_fdes_dev);
	}				
        if (cuat != (struct CuAt *)NULL) free(cuat);

 	/* Now for Self Configuring device, obtain the size_in_mb */
 	/* value and append it to the beginning of the text.      */
	ptype = (char *)substrg(PTYPE,cudv->PdDvLn_Lvalue);
 	if(!strcmp(ptype, "scsd")){
 		sprintf(crit, "name=%s AND attribute=size_in_mb", cudv->name);
 	        cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit,
 				&c_info, 1,1 );
 		/* If found the attribute and message is not n/a */
 		if ( (cuat != (struct CuAt *) -1) &&
 		     (strcmp(tmp, "n/a")) )
 		{
 			*textptr = (char *)malloc(strlen(tmp)+
 				strlen(cuat->value)+7);
			sprintf(*textptr, "%s (%s MB)", tmp, cuat->value);
 			free(cuat);
        		if ((int )fdes_desc > 0) catclose(fdes_desc);
 			return(0);
 		}
 	}
 	*textptr = (char *)malloc( strlen(tmp) + 1);
 	strcpy( *textptr, tmp );
      	if ((int )fdes_desc > 0) catclose(fdes_desc);
	if ( temp_ptr ) free (temp_ptr);
        return(0);
}


/* ^L */
/* NAME: substrg
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: A pointer to a string containing
 *          a PdDv object's type, class, or subclass.
 *
 */

char *substrg(int sel, char *in)
{
        register char *tptr;
        static char type[16],class[16],subclass[16];
        char strg[48];

        strcpy(strg,in);
        tptr = (char *)strrchr(strg,'/');
        ++tptr;
        strcpy(type,tptr);
        tptr = (char *)strtok(strg,"/");
        strcpy(class,tptr);
        tptr = (char *)strtok('\0',"/");
        strcpy(subclass,tptr);

        switch(sel) {
                case PTYPE  : return(type);
                case PCLASS : return(class);
                case PSCLASS : return(subclass);
        }
}

