static char sccsid[] = "@(#)44	1.17.2.13  src/bos/usr/ccs/lib/libdiag/init_diag.c, libdiag, bos41J, 9516B_all 4/20/95 16:00:45";
/*
 * COMPONENT_NAME: (LIBDIAG) Diagnostic Library
 *
 * FUNCTIONS: 		init_diag_db
 *			hash_CuDv
 *			Def_Missing_Ports
 *			find_CDiag
 *			find_PDiag
 *			find_PDiagAtt
 *			get_device_text
 *			get_dev_desc
 *			get_diag_att
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
#include <math.h>
#include <nl_types.h>
#include <sys/cfgdb.h>                  /* cfg db literals */
#include "diag/class_def.h"             /* object class data structures */
#include "diag/tmdefs.h"
#include "diag/diag.h"

#define DEPTH 1
#define	DIAG_CONVERSION_KEY	"DIAGS_VERSION_4"

/* GLOBAL VARIABLES */
struct CuDv *T_CuDv;
struct PDiagDev *T_PDiagDev;
struct PDiagAtt *T_PDiagAtt;
struct CDiagDev *T_CDiagDev;
struct CuDv **Parent_CuDv;

int num_CuDv;
int num_PDiag;
int num_PDiagAtt;
int num_CDiag; 

extern nl_catd diag_catopen(char *, int);
extern nl_catd diag_device_catopen(char *, int);
extern char *diag_cat_gets(nl_catd, int, int);
extern char *diag_device_gets(nl_catd, int, int, char *);

/* FUNCTION PROTOTYPES */
char *substrg(int, char *);
diag_dev_info_t *init_diag_db(int *);
struct CuDv **hash_CuDv(int);
int Def_Missing_Ports(int, int, int *);
struct CDiagDev *find_CDiag(char *, char *, char *, struct PDiagAtt *);
struct PDiagDev *find_PDiag(char *, char *, char *);
struct PDiagAtt *find_PDiagAtt(char *, char *, char *);
int get_device_text(struct CuDv *, struct Pdv *, char **);
char * get_dev_desc(char *);
int get_diag_att(char *, char *, char, int *, void *value);
int convert_sequence(char *, char *);
void convert_CDiagDev();
int  not_in_list(char *);


/*  */
/* NAME: substrg 
 *
 * FUNCTION:
 * 
 * NOTES: 
 *
 * RETURNS: A pointer to a string containing
 *	    a PdDv object's type, class, or subclass.
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

/*  */
/* NAME: init_diag_db 
 *
 * FUNCTION: This function creates a master array containing entries 
 * 	for each device identified in the current machine. 
 * 
 * NOTES: 
 *
 * RETURNS:
 *	diag_dev_info_t *    : pointer to series of linked structures
 *	diag_dev_info_t * -1 : error
 *
 */
diag_dev_info_t *init_diag_db(int *num_Top)
{
	char			*ptype,*psclass,*pclass;
 	char			criteria[256];
	int 			i, j, k, device_added;
	int			rc;
	int			bc;
	char			*tmp;
	char			catalog[CATSIZE+1];
	struct listinfo	c_info;
	struct listinfo	p_info;
	struct listinfo	d_info;
	struct listinfo	cdiag_info;
	struct listinfo	cuat_info;
	diag_dev_info_t 	*Top = NULL;
	struct 	PdDv		*pddv;
	struct	CDiagDev	*cdiagdev;
	struct	CuAt		*cuat_obj;
	struct	CuAt		*cuat_ptr;
	nl_catd			fdes_port, fdes_desc=0;

	/* get all device types in the Customized Device Class */
	T_CuDv = (struct CuDv *)diag_get_list(CuDv_CLASS, "", &c_info,
			MAX_EXPECT, DEPTH);
	if ( T_CuDv == (struct CuDv *) -1 )
		return( (diag_dev_info_t *) -1);
	*num_Top = num_CuDv = c_info.num;

	/* Serial devices like dials and lpfkeys that can attach to any         */
        /* serial port have an CuAt entry with attribute=ttydevice. CuAt->value */
        /* is set to the ttyx instance that the dials or keys device accesses.  */
        /* Search CuAt for any instance of this type. If any found, substitute  */
        /* the dials or keys device in for the ttyx instance it is representing */
        cuat_obj = (struct CuAt *)diag_get_list(CuAt_CLASS, "attribute=ttydevice",
                        &cuat_info, 2, 1 );

	/* For any device found with attribute=ttydevice, get the ttyx instance */
        /* from CuDv, and copy its parent info to the device. Set the ttyx      */
        /* parent info to null.                                                 */
        for ( k=0; k < cuat_info.num; k++ ) {
		/* first find the device emulating the tty */
                for (j=0; j < num_CuDv; j++) {
                        if (!strcmp( T_CuDv[j].name, cuat_obj[k].name ))
                                break;
                }
		/* now find the tty device */
                for (i=0; i < num_CuDv; i++) {
                        if (!strcmp( T_CuDv[i].name, cuat_obj[k].value )) {
				/* found - set the device parent the same as the tty */
                                strcpy(T_CuDv[j].parent,T_CuDv[i].parent);
                                strcpy(T_CuDv[j].location,T_CuDv[i].location);
                                strcpy(T_CuDv[j].connwhere,T_CuDv[i].connwhere);
				T_CuDv[i].parent[0] = '\0';
                                break;
                        }
                }
        }

	/* create hash on parent index in T_CuDv */
	Parent_CuDv = hash_CuDv ( num_CuDv );
	if ( Parent_CuDv == (struct CuDv **) -1)
		return( (diag_dev_info_t *) -1);

	/* get the predefined diag devices -> devices supported by diag */
	T_PDiagDev = (struct PDiagDev *)diag_get_list( PDiagDev_CLASS,
			"", &p_info, MAX_EXPECT, 1);
	if ( T_PDiagDev == (struct PDiagDev *) -1 )
		return( (diag_dev_info_t *) -1);
	num_PDiag = p_info.num;

	/* get the predefined attribute test mode -> devices that can be */
	/* periodically tested.						 */

	T_PDiagAtt = (struct PDiagAtt *)diag_get_list( PDiagAtt_CLASS,
			"attribute=test_mode", &p_info, MAX_EXPECT, 1);
	num_PDiagAtt = p_info.num;

	/* 
	Some devices are not in a usable state after IPL.  For example, 
	the device driver may be associated with an attached device, which
	is not automatically defined.  Therefore, define them.
	*/

	for (i=0; i < num_PDiag; i++){

	 	if ( T_PDiagDev[i].Ports ) {
			/* find a device in customized with specified type */
			for (j = 0; j < num_CuDv; j++){
				ptype =   substrg(PTYPE,  T_CuDv[j].PdDvLn_Lvalue);
				psclass = substrg(PSCLASS,T_CuDv[j].PdDvLn_Lvalue);
				pclass =  substrg(PCLASS, T_CuDv[j].PdDvLn_Lvalue);
				/* If type,subclass, and class (if given) match */
				if ( (!strcmp(ptype,T_PDiagDev[i].DType) ) &&
				     (!strcmp(psclass,T_PDiagDev[i].DSClass) ) ) {
					if ( ((strlen(T_PDiagDev[i].DClass)) &&
					      (!strcmp(pclass,T_PDiagDev[i].DClass))) ||
					      (!strlen(T_PDiagDev[i].DClass)) ) {
						rc=Def_Missing_Ports(j,i,&device_added);
						if ( rc !=0 )
							return( (diag_dev_info_t *) -1);
						break;
					}
				}
			}
		}
	}

	if (device_added){
		/* Free CuDv.  We may have just defined some devices */
		diag_free_list( T_CuDv, &c_info );
		free(Parent_CuDv);

		/* get all device types in the Customized Device Class */
		T_CuDv = (struct CuDv *)diag_get_list(CuDv_CLASS, "",
			&c_info, MAX_EXPECT, DEPTH);
		if ( T_CuDv == (struct CuDv *) -1 )
			return( (diag_dev_info_t *) -1);
		*num_Top = num_CuDv = c_info.num;

       		/* For any device found with attribute=ttydevice, get the ttyx instance */
        	/* from CuDv, and copy its parent info to the device. Set the ttyx      */
        	/* parent info to null.                                                 */
        	for ( k=0; k < cuat_info.num; k++ ) {
                	/* first find the device emulating the tty */
                	for (j=0; j < num_CuDv; j++) {
                        	if (!strcmp( T_CuDv[j].name, cuat_obj[k].name ))
                                	break;
                	}
                	/* now find the tty device */
                	for (i=0; i < num_CuDv; i++) {
                        	if (!strcmp( T_CuDv[i].name, cuat_obj[k].value )) {
                                	/* found - set the device parent same as the tty */
                                	strcpy(T_CuDv[j].parent,T_CuDv[i].parent);
                                	strcpy(T_CuDv[j].location,T_CuDv[i].location);
                                	strcpy(T_CuDv[j].connwhere,T_CuDv[i].connwhere);
                                	T_CuDv[i].parent[0] = '\0';
                                	break;
                        	}
                	}
        	}

		/* create hash on parent index in T_CuDv */
		Parent_CuDv = hash_CuDv ( num_CuDv );
		if ( Parent_CuDv == (struct CuDv **) -1)
			return( (diag_dev_info_t *) -1);
	}

        /* Now free the CuAt list, don't need it anymore */
        if(cuat_obj)
                diag_free_list(cuat_obj, &cuat_info);

	/* get customized diag data: test status, deletion flag */
	T_CDiagDev = (struct CDiagDev *)diag_get_list(CDiagDev_CLASS,
			"", &c_info, MAX_EXPECT, 1);
	num_CDiag = c_info.num;

        /* First find out if conversion to new CDiagDev is needed */
	/* CDiagDev used to have device type and location code as */
	/* keys to uniquely identify devices. This is no longer   */
	/* true, therefore, DType will now be used to store the   */
	/* logical device name, which for now is guaranteed to be */
	/* unique. Older CDiagDev objects need to be converted to */
	/* have the logical name in the DType field.              */


	sprintf(criteria, "DType = %s", DIAG_CONVERSION_KEY);
	cdiagdev = (struct CDiagDev *) diag_get_list(CDiagDev_CLASS,criteria,
			 &cdiag_info, 1, 1);
	if (cdiag_info.num == 0)
		convert_CDiagDev();
	else
		diag_free_list(cdiagdev, &cdiag_info);

	/* allocate space for TOPMaster */
	Top = (struct diag_dev_info_s *) 
		calloc( *num_Top, sizeof( struct diag_dev_info_s ) );

	for (i=0; i < *num_Top; i++){

		ptype = substrg(PTYPE, T_CuDv[i].PdDvLn_Lvalue);
		psclass =  substrg(PSCLASS, T_CuDv[i].PdDvLn_Lvalue);
		pclass =  substrg(PCLASS, T_CuDv[i].PdDvLn_Lvalue);

		/* set customized device pointer */
		Top[i].T_CuDv = &T_CuDv[i];

		sprintf(criteria,"uniquetype = %s",T_CuDv[i].PdDvLn_Lvalue);
		pddv = (struct PdDv *)diag_get_list(PdDv_CLASS,
			criteria,&d_info,1,1);
		if(pddv == (struct PdDv *) -1) 
			return((diag_dev_info_t *)-1);

		Top[i].T_Pdv = (struct Pdv *) calloc(1,sizeof(struct Pdv));
		if(Top[i].T_Pdv == (struct Pdv *) 0) return((diag_dev_info_t *)-1);
		Top[i].T_Pdv->led = pddv->led;
		if( pddv->led == 0 ) {
                	/* No LED number in PdDv so check for type Z */
                	/* attribute                                 */
                	if ((cuat_ptr = (struct CuAt *)getattr(Top[i].T_CuDv->name,
                       	                         "led",0,&bc)) != (struct CuAt *)NULL)
                           Top[i].T_Pdv->led = (short)strtoul(cuat_ptr->value,NULL,0);
		}

		Top[i].T_Pdv->detectable = pddv->detectable;
		Top[i].T_Pdv->fru = pddv->fru;
		Top[i].T_Pdv->setno = pddv->setno;
		Top[i].T_Pdv->msgno = pddv->msgno;
		strcpy(Top[i].T_Pdv->catalog,pddv->catalog);
						
		diag_free_list(pddv,&d_info);

		/* set predefined diagnostic pointer */ 
		Top[i].T_PDiagAtt = find_PDiagAtt(ptype, psclass, pclass);

		Top[i].T_PDiagDev = find_PDiag(ptype,psclass, pclass); 

		/* set customized diagnostic pointer, allocate if necessary */
		if (Top[i].T_PDiagDev)
			Top[i].T_CDiagDev=find_CDiag(T_CuDv[i].name,T_CuDv[i].location,pclass,
				Top[i].T_PDiagAtt);
		else Top[i].T_CDiagDev = (struct CDiagDev *) NULL;
	}

	/* get text for devices */
	fdes_port = diag_catopen(PORT_CAT, 0);
	for (i=0; i < *num_Top; i++){

		/* if device is not supported by diagnostics */
		/* use the adapter's description text        */
	 	if ( (Top[i].T_PDiagDev != NULL)  &&
		    !(Top[i].T_PDiagDev->SupTests & 7) ) {
			for (j=0; j < *num_Top; j++)
				if (!strcmp(Top[j].T_CuDv->name,
					            Top[i].T_CuDv->parent) )
					tmp = (char *)diag_cat_gets(fdes_port,
						Top[j].T_PDiagDev->PSet, 1);
					Top[i].Text = 
					       (char *)malloc( strlen(tmp) + 1);
					strcpy( Top[i].Text, tmp );
		}
		/*  else standard device text */
		else {
			if ( strncmp(catalog,Top[i].T_Pdv->catalog,
				      strlen(Top[i].T_Pdv->catalog))) {
				if ( fdes_desc  > (nl_catd)0 )
					catclose(fdes_desc);
				fdes_desc = diag_device_catopen(
						Top[i].T_Pdv->catalog,
						0),
				strcpy(catalog,Top[i].T_Pdv->catalog);
				catalog[strlen(Top[i].T_Pdv->catalog)]
						= '\0';
			}
			/* if device has one description */
			if ( Top[i].T_Pdv->msgno != 0 ) {
				tmp = diag_device_gets(fdes_desc,
		  			Top[i].T_Pdv->setno,
		  			Top[i].T_Pdv->msgno,
					"n/a" );
				Top[i].Text = (char *)malloc( strlen(tmp) + 1);
				strcpy( Top[i].Text, tmp );
			}
			else
				get_device_text(Top[i].T_CuDv,Top[i].T_Pdv,
						&Top[i].Text);
		}
	}

	if ( fdes_port > (nl_catd)0 )
		catclose(fdes_port);
	if ( fdes_desc > (nl_catd)0 )
		catclose(fdes_desc);

	return(Top);
}
			
/*  */

/* NAME: hash_CuDv 
 *
 * FUNCTION: This function creates an array, Parent_CuDv, that supplements  
 * 	T_CuDv. Each element has a counterpart in T_CuDv. For example, the 
 * 	element T_CuDv[i] corresponds to Parent_CuDv[i].  The array 
 *	Parent_CuDv contains a pointer to the T_CuDv[i] entry that describes 
 *	the parent. 
 *
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	struct CuDv **    	 : pointer to Parent_CuDv 
 *	struct CuDv ** -1 	 : error
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
				
/*  */
 
/* NAME: Def_Missing_Ports  
 *
 * FUNCTION: Searches CuDv for a child of Curr_CuDv attached to   
 * 	the designate port.
 * 
 * RETURNS:
 *
 */

int Def_Missing_Ports(
	int Curr_CuDv,		/* Customized Multi-port Adapter*/	
	int Curr_PDiag,		/* PreDefined Diag Adapter	*/
	int *device_added)
{
	int 		i, j;
	int		rc; 
	int		num_pdcn;
	char		buffer[128];
	char		new_device[NAMESIZE];
	char		mkdev_args[512];
	char		*outbuf, *errbuf;
	struct listinfo	p_info, c_info;
	struct PdDv	*T_PdDv;
	struct PdCn	*pdcn;

	/* search PdCn for all available 'connwhere' locations */
	sprintf(buffer, "uniquetype = %s and connkey = %s", 
			T_CuDv[Curr_CuDv].PdDvLn_Lvalue,
			T_PDiagDev[Curr_PDiag].AttSClass);
	pdcn = (struct PdCn *)diag_get_list(PdCn_CLASS, buffer,
			&c_info,MAX_EXPECT,1);
	if ( pdcn == (struct PdCn *) -1 )
		return(-1);
	num_pdcn = c_info.num;

	/* for each attached child , invalidate the connwhere key in pdcn */
	for ( i = 0; i < num_CuDv; i++ )
		/* find attached child */
		if(!strcmp(T_CuDv[Curr_CuDv].name, T_CuDv[i].parent) ){
			for ( j = 0; j < num_pdcn; j++ )
				if ( !strcmp( T_CuDv[i].connwhere, 
					        pdcn[j].connwhere) ) {
					pdcn[j].connwhere[0] = (char) NULL;
					break;
				}
		}

	/*
		query predefined for class, subclass corresponding
		to attdtype. 
		mkdev -c class -s subclass -t attdtype -p parent -w port
	*/
	if ( strlen(T_PDiagDev[Curr_PDiag].AttSClass) ) 
		sprintf(buffer, "type = %s and subclass = %s", 
				T_PDiagDev[Curr_PDiag].AttDType,
				T_PDiagDev[Curr_PDiag].AttSClass);
	else
		sprintf(buffer, "type = %s", T_PDiagDev[Curr_PDiag].AttDType);
	T_PdDv = (struct PdDv *)diag_get_list(PdDv_CLASS, buffer,
			&p_info, 1,1);
	if ( T_PdDv == (struct PdDv *) -1 )
		return(-1);

	/* for each valid connwhere field left in pdcn, issue a mkdev */
	for ( j = 0; j < num_pdcn; j++ )
		if (pdcn[j].connwhere[0] != (char) NULL){ 
			*device_added = 1;
			sprintf(mkdev_args," -c %s -s %s -t %s -p %s -w %s",
				T_PdDv->class,		/* class   	*/
				T_PdDv->subclass,	/* subclass 	*/
				T_PdDv->type,		/* type 	*/
				T_CuDv[Curr_CuDv].name,	/* parent	*/
				pdcn[j].connwhere);	/* connwhere	*/
			rc = invoke_method(T_PdDv->Define, mkdev_args, 
			  		&outbuf, &errbuf);
			if ( rc == 0 ) {
				sscanf ( outbuf, "%s", new_device );
				stack_device_name( new_device );
			}
			free ( outbuf );
			free ( errbuf );
		}

	diag_free_list( pdcn, &c_info );
	diag_free_list( T_PdDv, &p_info );
	return(0);
}

/*  */

/* NAME: find_CDiag 
 *
 * FUNCTION: This function returns a pointer to a previous entry
 * 	in the Customized Diagnostic Device Object Class if present
 *	or a pointer to a new initialized structure. 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	 struct CDiagDev *    : pointer to new structure
 *	 struct CDiagDev * -1 : error
 *
 */

struct CDiagDev *find_CDiag(
	char *dname,
	char *loc,
	char *class,
	struct PDiagAtt *pdiagatt)
{
	int 		i, j, k, l;
	int	flag;
	int parentfound =0;
	struct CDiagDev *new;
	char	newsparent[16];


	/* find the device in the CDiagDev table */
	for (i = 0; i < num_CDiag; i++){
		if ( !strncmp(T_CDiagDev[i].Location, loc, LOCSIZE) &&
		     !strncmp(T_CDiagDev[i].DType, dname, NAMESIZE) ) 
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
		strncpy(new->DType, dname, NAMESIZE); 
		strncpy(new->Location, loc, LOCSIZE);
		new->SysxTime = 0;
		/* set default - in the diagnostic test list */
		new->RtMenu = RTMENU_DEF;

		/* search for parent and set child test list bit the same */
		for(j=0;j<num_CuDv && parentfound < 1; j++)
		{

			if((!strcmp(T_CuDv[j].location, loc)) &&
					(!strcmp(T_CuDv[j].name, dname)))
			{
 				strcpy(newsparent, T_CuDv[j].parent);
				for(k=0; k< num_CuDv && parentfound <1; k++)
				{
					if(!strcmp(T_CuDv[k].name, newsparent))
					{
						for(l=0;l<num_CDiag&&parentfound<1; l++)
						{
							if((!strcmp(T_CuDv[k].name,T_CDiagDev[l].DType)) &&
							   (!strcmp(T_CuDv[k].location, T_CDiagDev[l].Location)))
							{
								new->RtMenu = T_CDiagDev[l].RtMenu;
								parentfound = 1;
							} /* if strcmp dname */
						} /* for l */
					} /* if newsparent */
				} /* for k */
			} /* if T_CuDv[j].location */
		}/* for j */
		if( (pdiagatt == (struct PDiagAtt *)NULL) ||
		    (pdiagatt == (struct PDiagAtt *)-1) )
			new->Periodic = DEFAULT_TESTTIME;
		else {
			flag=atoi(pdiagatt->value);
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
/*   */
/* NAME: find_PDiag 
 *
 * FUNCTION: This function searches the Diagnostic Predefined Obj
 * 	Class for the specified device type. 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	 struct PDiagDev *    : pointer to an entry in PDiagDev
 *	(struct PDiagDev *)-1 : could not find the specified type
 *
 */

struct PDiagDev *find_PDiag(
	char *type,
	char *subclass,
	char *class)
{
	int 		i,j;

	for (i = 0; i < num_PDiag; i++){
		if ( ( !strncmp(T_PDiagDev[i].DType, type, TYPESIZE) ) &&
		     ( !strncmp(T_PDiagDev[i].DClass, class, CLASSIZE) ) &&
		     ( !strncmp(T_PDiagDev[i].DSClass, subclass, CLASSIZE) ) )
				break;
	}

	if ( i == num_PDiag ){
		/* Need to take care of older PDiagDev on supplemental */
		/* diskette that does not have DClass setup.           */
		for(j = 0; j< num_PDiag; j++)
			if ( ( !strncmp(T_PDiagDev[j].DType,
					type, TYPESIZE) ) &&
		     	     ( !strncmp(T_PDiagDev[j].DSClass,
					subclass, CLASSIZE) ) )
				break;

		/* Now return null if not found */

		if(j == num_PDiag)
		 	return( (struct PDiagDev *) NULL );

		return( &T_PDiagDev[j] );
	}

	return( &T_PDiagDev[i] );

}

/*   */
/* NAME: find_PDiagAtt 
 *
 * FUNCTION: This function searches the Diagnostic Predefined attribute Obj
 * 	Class for the specified device type. 
 * 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	 struct PDiagAtt *    : pointer to an entry in PDiagAtt
 *	(struct PDiagAtt *)-1 : could not find the specified type
 *
 */

struct PDiagAtt *find_PDiagAtt(
	char *type,
	char *subclass,
	char *class)
{
	int 		i,j;

	if(num_PDiagAtt == 0)
	 	return( (struct PDiagAtt *) NULL );

	for (i = 0; i < num_PDiagAtt; i++){
		if ( ( !strncmp(T_PDiagAtt[i].DType, type, TYPESIZE) ) &&
		     ( !strncmp(T_PDiagAtt[i].DClass, class, CLASSIZE) ) &&
		     ( !strncmp(T_PDiagAtt[i].DSClass, subclass, CLASSIZE) ) )
				break;
	}

	if ( i == num_PDiagAtt ){	
		/* Need to take care of older PDiagAtt on supplemental */
		/* diskette that does not have DClass setup.           */
		for(j = 0; j< num_PDiagAtt; j++)
			if ( ( !strncmp(T_PDiagAtt[j].DType,
					type, TYPESIZE) ) &&
		     	     ( !strncmp(T_PDiagAtt[j].DSClass,
					subclass, CLASSIZE) ) )
				break;
		if(j == num_PDiagAtt)
		/* Now return null if not found */
		 	return( (struct PDiagAtt *) NULL );

		return( &T_PDiagAtt[j] );
	}

	return( &T_PDiagAtt[i] );

}
/*  */

/* NAME: get_device_text
 *
 * FUNCTION: This function searches the CuAt data base for attributes for
 *		a device. If not found, use standard device text.
 * 		Else build text string from attributes.
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
	int		msgno=0;
	struct CuAt	*cuat = (struct CuAt *) NULL;
	struct PdAt	*pdat = (struct PdAt *) NULL;
	struct listinfo	c_info;
	char		*tmp, *ptype;
	char		crit[100];
	nl_catd		fdes_desc;

	/* check for type 'T' attribute in customized attributes */
	sprintf( crit, "name = %s AND type = T", cudv->name);
	cuat = (struct CuAt *)diag_get_list(CuAt_CLASS, crit, &c_info,1,1);
	if ( cuat == (struct CuAt *) -1 )
		return(-1);

	/* if no customized attribute, then get default from PdAt */
	if ( c_info.num == 0 ) {
		sprintf( crit, "uniquetype = %s AND type = T", 
						cudv->PdDvLn_Lvalue);
		pdat = (struct PdAt *)diag_get_list(PdAt_CLASS, crit, 
							&c_info, 1,1 );
		if ( pdat == (struct PdAt *) -1 )
			return(-1);
		else if (c_info.num == 1 ) 
			msgno = atoi(pdat->deflt);
	}
	else
		msgno = atoi(cuat->value);

	/* use attributes value for message index */
	fdes_desc = diag_device_catopen(pddv->catalog, 0);
	tmp = diag_device_gets(fdes_desc, pddv->setno, msgno, "n/a");
	if (cuat != (struct CuAt *)NULL) free(cuat);

 	/* Now for Self Configuring device, obtain the size_in_mb */
 	/* value and append it to the beginning of the text.      */
	ptype = substrg(PTYPE,cudv->PdDvLn_Lvalue);
 	if(!strcmp(ptype, "scsd")){
 		sprintf(crit, "name=%s AND attribute=size_in_mb", cudv->name);
 	        cuat = (struct CuAt *)diag_get_list(CuAt_CLASS, crit,
 				&c_info, 1,1 );
 		/* If found the attribute and message found so far is not n/a */
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
	return(0);
}
/*  */
/*
 * NAME: get_diag_att
 *
 * FUNCTION: Reads an attribute from the predefined database.
 *
 *
 * NOTES:
 *
 *   int get_diag_att(type, attribute, conversion, byte_count, value) 
 *
 *      type = Device type
 *      attribute  = attribute name to retrieve from the Predefined
 *                  Attribute Object Class.
 *      conversion = The data type which the attribute is to be converted to
 *                    's' = string              rep=s
 *                    'b' = byte sequence       rep=s,  e.g. "0x56FFE67.."
 *                    'l' = long                rep=n
 *                    'i' = int                 rep=n
 *                    'h' = short (half)        rep=n
 *		      'f' = float	 	rep=n
 *                    'c' = char                rep=n,or s
 *                    'a' = address             rep=n
 *	byte_count = number of bytes (for byte sequence only)
 *      value    = pointer to where the converted attribute value is returned.
 *
 * RETURNS:
 *		0 = successful
 *	       <0 = unsuccessful
 */

int get_diag_att( char	*type, char *attribute,
char	conversion, int	*byte_count, void *value)
{
	struct	PDiagAtt	*pdiagatt;
	struct	listinfo	obj_info;
	char	criteria[128];
	char	*ptype, *psclass, *pclass;
	char	rep;
	char	*attribute_value;

	*byte_count=0;
	if(strchr(type, '/')) {
	/* type, class and subclass specified */
		
		ptype = substrg(PTYPE, type);
		psclass =  substrg(PSCLASS, type);
		pclass =  substrg(PCLASS, type);
		sprintf(criteria, 
			"DType = %s AND DSClass = %s AND DClass = %s AND attribute = %s",
			ptype, psclass, pclass, attribute);
	} else
		sprintf(criteria, "DType = %s AND attribute = %s", type, attribute);

	pdiagatt = (struct PDiagAtt *)diag_get_list(PDiagAtt_CLASS,
		 criteria, &obj_info, 1, 1);
	if( (pdiagatt == (struct PDiagAtt *)NULL) || (pdiagatt == 
			(struct PDiagAtt *)-1) )
		return(-1);
	attribute_value = pdiagatt->value;
	rep = pdiagatt->rep[strcspn(pdiagatt->rep,"sn")];
	if( rep == 's' ) {
		switch( conversion ){
		case 's':
			strcpy( (char *)value, attribute_value);
			break;
		case 'c':
			*(char *)value = *attribute_value;
			break;
		case 'b':
			*byte_count = convert_sequence(attribute_value,
					(char *)value);
			break;
		default:
			return( -1 );
		}
	}
	else if ( rep == 'n' ) {
		switch( conversion ) {
		case 'l':
			*(long *)value = strtoul(attribute_value, 
				(char **)NULL, 0);
			break;
		case 'f':
			*(float *)value = (float)atof(attribute_value);
			break;
		case 'h':
			*(short *)value = (short)strtoul(attribute_value,
				(char **)NULL, 0);
			break;
		case 'i':	
			*(int *)value = (int)strtoul(attribute_value,
				(char **)NULL, 0);
			break;
		case 'c':
			*(char *)value = (char)strtoul(attribute_value,
				(char **)NULL, 0);
			break;
		case 'a':
			*(void **)value = (void *)strtoul(attribute_value,
				(char **)NULL, 0);
			break;
		default:
			return( -1 );
		}
	} else {
		return (-1);
	}
	return ( 0 );
}
 
/*  */

/* NAME: get_dev_desc
 *
 * FUNCTION: This function searches the CuAt data base for attributes for
 *		a device. If not found, use standard device text.
 * 		Else build text string from attributes.
 *		Used by applications to retrieve device description text 
 * NOTES: 
 *
 * DATA STRUCTURES:
 *
 * RETURNS: character pointer to device description
 *
 */

char * get_dev_desc(
	char *device_name)
{
	int		msgno=0;
	struct CuDv	*cudv = (struct CuDv *) NULL;
	struct CuAt	*cuat = (struct CuAt *) NULL;
	struct PdAt	*pdat = (struct PdAt *) NULL;
	struct listinfo	c_info;
	char		*tmp, *textptr;
	char		crit[64];
	nl_catd		fdes_desc;

	/* get device CuDv and PdDv data */
	sprintf( crit, "name = %s", device_name);
	cudv = (struct CuDv *)diag_get_list(CuDv_CLASS,crit,&c_info,1,2);
	if ( cudv == (struct CuDv *) -1 )
		return((char *)-1);

	if ( cudv->PdDvLn->msgno != 0 ) {
		fdes_desc = diag_device_catopen(cudv->PdDvLn->catalog, 0);
		tmp = diag_device_gets(fdes_desc, cudv->PdDvLn->setno, 
						  cudv->PdDvLn->msgno, "n/a");
		textptr = (char *)malloc( strlen(tmp) + 1);
		strcpy( textptr, tmp );
		catclose(fdes_desc);
		return( textptr );
	}

	/* check for type 'T' attribute in customized attributes */
	sprintf( crit, "name = %s AND type = T", cudv->name);
	cuat = (struct CuAt *)diag_get_list(CuAt_CLASS,crit, &c_info, 1,1);
	if ( cuat == (struct CuAt *) -1 )
		return((char *)-1);

	/* if no customized attribute, then get default from PdAt */
	if ( c_info.num == 0 ) {
		sprintf( crit, "uniquetype = %s AND type = T", 
						cudv->PdDvLn_Lvalue);
		pdat = (struct PdAt *)diag_get_list(PdAt_CLASS, crit, 
							&c_info, 1,1 );
		if ( pdat == (struct PdAt *) -1 )
			return((char *)-1);
		else if (c_info.num == 1 ) 
			msgno = atoi(pdat->deflt);
	}
	else
		msgno = atoi(cuat->value);

	/* use attributes value for message index */
	if (cuat != (struct CuAt *)NULL) free(cuat);
	if (pdat != (struct PdAt *)NULL) free(pdat);
	fdes_desc = diag_device_catopen(cudv->PdDvLn->catalog, 0);
	tmp = diag_device_gets(fdes_desc, cudv->PdDvLn->setno, msgno, "n/a");
	textptr = (char *)malloc( strlen(tmp) + 1);
	strcpy( textptr, tmp );
	if ((int )fdes_desc > 0) catclose(fdes_desc);

	return( textptr );
}
/*  */
/*
 * NAME: convert_sequence
 *                                                                    
 * FUNCTION: Converts a hex-style string to a sequence of bytes
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine uses no global variables
 *                                                                   
 * NOTES:
 *	
 *	The string to be converted is of the form
 *	"0xFFAAEE5A567456724650789789ABDEF678"	(for example)
 *	This would put the code FF into the first byte, AA into the second,
 *	etc.
 *
 * RETURNS: No of bytes, or -3 if error.
 *
 */

int convert_sequence( char *source, uchar *dest )
{
	char	byte_val[5];	/* e.g. "0x5F\0"	*/
	int	byte_count = 0;
	uchar	tmp_val;
	char	*end_ptr;

	strcpy( byte_val, "0x00" );

	if( *source == '\0' )	/* Accept empty string as legal */
		return 0;

	if( *source++ != '0' )
		return (-1);
	if( tolower(*source++) != 'x' )
		return (-1);

	while( ( byte_val[2] = *source ) && ( byte_val[3] = *(source+1) ) )
	{
		source += 2;

		/* be careful not to store illegal bytes in case the
		 * destination is of exact size, and the source has
		 * trailing blanks
		 */

		tmp_val = (uchar) strtoul( byte_val, &end_ptr, 0 );
		if( end_ptr != &byte_val[4] )
			break;
		*dest++ = tmp_val;
		byte_count++;
	}

	return byte_count;
}

/* NAME: convert_CDiagDev
 *
 * FUNCTION: This function converts the existing CDiagDev object class
 *      to contain logical device name in its DType field.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */
void convert_CDiagDev()
{
	int     i,j;


	/* A CDiagDev list exists. Needs to modify its DType field to */
	/* contain the name from the CuDv.                            */

	for(i=0; i<num_CDiag; i++)
		for(j=0; j<num_CuDv; j++)
			if(!strncmp(T_CDiagDev[i].DType,
					T_CuDv[j].PdDvLn->type,TYPESIZE) &&
					!strncmp(T_CDiagDev[i].Location,
					T_CuDv[j].location,LOCSIZE))
				if(not_in_list(T_CuDv[j].name))
				{
					strncpy(T_CDiagDev[i].DType,
						   T_CuDv[j].name, NAMESIZE);
				        break;
				 }

}

/* NAME: not_in_list
 *
 * FUNCTION: This function searches the CDiagDev list for the given device
 *      name. This is used to prevent duplicate names from being merged
 *      into the CDiagDev DType field, during convert_CDiagDev().
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      1 device not in the list
 *      0 device is in the list
 */

int not_in_list(char *dname)
{
     int     i;

     for(i=0; i<num_CDiag; i++)
	     if(!strcmp(dname, T_CDiagDev[i].DType))
		     return(0);

     return(1);
}

