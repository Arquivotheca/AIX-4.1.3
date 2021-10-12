static char sccsid[] = "@(#)04	1.3  src/bos/diag/tu/pcitok/getsky.c, tu_pcitok, bos41J, 9516A_all 4/18/95 09:59:52";
/****************************************************************************
 *   COMPONENT_NAME: tu_pcitok
 *
 * FUNCTIONS:	int getmps()
 *		int getatt()
 *		int dump_dds()
 *		int hexdump()
 *		int att_changed()
 *		int convert_att()
 *		int convert_seq()
 *		int err_exit()
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
***************************************************************************/


/**************************************************************************
* This file contains functions that Access the ODM database to get
* it's information on Skyline 
**************************************************************************/


/* header files needed for compilation */
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <cf.h>

#include "skytu_type.h"

/***************************************************************************
 * NAME: getmps
 *
 * FUNCTION: Builds the DDS (Defined Data Structure) for the
 *           MPS Adapter
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
***************************************************************************/
int getmps(
TUTYPE *tucb_ptr,
char *logical_name, 
mps_dds *dds)
{
  int     rc;                     /* used for return codes */
  int     lockid;                 /* used for odm_lock     */
  char    sstring[512];           /* working string                   */
  char    *lname;                 /* pointer to device name */
  char    *pname;                 /* pointer to parent name */
  char    *ut;                    /* pointer to device's uniquetype */
  char    *pt;                    /* pointer to parent's uniquetype */
  struct  PdAt   pdatobj;

  struct  cfg_dd cfg;             /* sysconfig command structure */

  struct Class *cusdev;           /* customized devices class ptr */
  struct CuDv cusobj;             /* customized device object storage */
  struct CuDv parobj;             /* customized device object storage */
  uchar  ring_speed[20];


  /***************************************************************
  The purpose of this function is to find all the proper values of the
  device specific data structrure, DDS.  

  char			adpt_name[16]
  char			par_name[16]
  unsigned int		slot_num           ** includes PCI device and function
  unsigned int		bus_intr_lvl 
  unsigned int		intr_priority 
  unsigned int 		dma_chan_id        ** returned by diag_open
  unsigned long		bus_io_addr
  unsigned long		bus_io_length
  unsigned long		dma_bus_length
  unsigned ing		dma_flags          ** not used for PCI 
  unsigned long		bus_id
  unsigned long		xmt_que_size
  unsigned long		ring_speed
  char			alt_addr[6]
  char			intr_func[80]      ** not used at this time
  char			attn_mac[4]
  char			beacon_mac[4]
  char			use_alt_addr[4]
  char			alias[10]
  unsigned long		rcv_que_size
  ***************************************************************/

  /*****                                                          */
  /***** Validate Parameters                                      */
  /*****                                                          */
  /* logical name must be specified */
  if (logical_name == NULL) {
	DEBUG_0("getmps: logical name must be specified\n");
  	return(E_LNAME);
  }


  /* start up odm */
  if (odm_initialize() == -1) {
  	/* initialization failed */
  	DEBUG_0("getmps: odm_initialize() failed\n") 
	diagex_initial_state(logical_name);
   	return(E_ODMINIT);
  }

  /* Change device state to DIAGNOSE */
    rc = diagex_cfg_state(logical_name);

    DEBUG_2("DIAGEX CFG STATE RC = %d,lname = %s\n", rc,logical_name);
    if (rc) {
	return (CHG_STATE_ERR);
    }

  /* lock the database */
  lockid = odm_lock("/etc/objrepos/config_lock",ODM_WAIT);
  if (lockid == -1) {
  	DEBUG_0("getmps: odm_lock() failed\n") 
	diagex_initial_state(logical_name);
  	return(err_exit(E_ODMLOCK));
  }

  DEBUG_0 ("ODM initialized and locked\n") 

  /* open customized devices object class */
  if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
  	DEBUG_0("getmps: open class CuDv failed\n"); 
	diagex_initial_state(logical_name);
  	return(err_exit(E_ODMOPEN));
  }
  /* search for customized object with this logical name */
  sprintf(sstring, "name = '%s'", logical_name);
  rc = (int)odm_get_first(cusdev,sstring,&cusobj);
  if (rc==0) {
  	/* No CuDv object with this name */
  	DEBUG_1("getmps: failed to find CuDv object for %s\n", logical_name);
	diagex_initial_state(logical_name);
  	return(err_exit(E_NOCuDv));
  } else if (rc==-1) {
  	/* ODM failure */
  	DEBUG_0("getmps: ODM failure getting CuDv object");
	diagex_initial_state(logical_name);
  	return(err_exit(E_ODMGET));
  }
                                                           /* get par_name */
  sprintf(sstring, "name = '%s'", cusobj.parent);
  rc = (int)odm_get_first(cusdev,sstring,&parobj);
  if (rc==0) {
  	/* Parent device not in CuDv */
  	DEBUG_0("getmps: no parent CuDv object\n")
  	return(err_exit(E_NOCuDvPARENT));
  } else if (rc==-1) {
  	/* ODM failure */
  	DEBUG_0("getmps: ODM failure getting parent CuDv object\n")
	diagex_initial_state(logical_name);
  	return(err_exit(E_ODMGET));
  }
  /* parent must be available to continue */
  if (parobj.status != AVAILABLE) {
  	DEBUG_0("getmps: parent is not AVAILABLE")
	diagex_initial_state(logical_name);
  	return(err_exit(E_PARENTSTATE));
  }

  /* save some strings for short hand */
  lname = cusobj.name;
  ut = cusobj.PdDvLn_Lvalue;
  pname = cusobj.parent;
  pt = parobj.PdDvLn_Lvalue;
  strcpy(dds->adpt_name,lname);				 /* assign adpt_name */
  strcpy(dds->par_name,pname);          	         /* assign par_name */

		/* get the bus_id of the parent and assign it to the child */
  DEBUG_2("getmps: pname = %s pt = %s \n",pname,pt)
      if((int)(odm_open_class(CuAt_CLASS)) == -1){
  	DEBUG_0("getmps: can not open CuAt\n")
	diagex_initial_state(logical_name);
  	return E_ODMOPEN;
  }
  if((int)(odm_open_class(PdAt_CLASS)) == -1){
  	DEBUG_0("bld_dds: can not open PdAt\n");
	diagex_initial_state(logical_name);
  	return E_ODMOPEN;
  }

  /* get bus attributes */
  DEBUG_0 ("Getting bus attributes\n")

                                                  	/* assign bus_id */

  if((rc=getatt(tucb_ptr, &dds->bus_id,'l',CuAt_CLASS,PdAt_CLASS,pname,pt,
	    "bus_id",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }
  dds->bus_id |= 0x800C0020;

  /* get device attributes */
  DEBUG_0 ("Getting device attributes\n")
   						    /* assign bus_io_length */
   
  dds->bus_io_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);


						  /* assign dma_bus_length */

  dds->dma_bus_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);
	
						  /* assign xmt_que_size */

  if((rc=getatt(tucb_ptr,&dds->xmt_que_size,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "xmt_que_size",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

						  /* assign ring_speed */

  if((rc=getatt(tucb_ptr, ring_speed,'s',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "ring_speed",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

  if (strcmp(ring_speed,"4") == 0) {
	dds->ring_speed=0;
  } else {
  	if (strcmp(ring_speed,"16") == 0) {
  		dds->ring_speed=1;
  	} else {
  		/* ring speed = autosense */
  		dds->ring_speed=2;
  	}
  }

DEBUG_1 ("ring speed = %d\n",dds->ring_speed);
 						  /* assign alt_addr */

  if((rc=getatt(tucb_ptr, &dds->alt_addr,'s',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "alt_addr",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

						  /* assign attn_mac */		

  if((rc=getatt(tucb_ptr, &dds->attn_mac,'s',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "attn_mac",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

						  /* assing beacon_mac */

#ifdef REAL_CONFIG
  if((rc=getatt(tucb_ptr, &dds->beacon_mac,'s',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "beacon_mac",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }
#endif

						  /* assign use_alt_addr */

  if((rc=getatt(tucb_ptr, &dds->use_alt_addr,'s',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "use_alt_addr",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

						  /* assign slot_num */

  dds->slot_num = atoi(cusobj.connwhere);

  					          /* assign bus_intr_lvl */
						  
  if((rc=getatt(tucb_ptr,&dds->bus_intr_lvl,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "busintr",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }
  
					       	      /* assign bus_io_addr */

  if((rc=getatt(tucb_ptr, &dds->bus_io_addr,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "busio",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }

  sprintf( sstring, "uniquetype = %s AND attribute = busio",ut);

  if( (rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdatobj) )==0 ) {
	diagex_initial_state(logical_name);
  	return( E_NOCuDv );
   } else if ( rc == -1 ) {
	diagex_initial_state(logical_name);
  	return( E_ODMGET );
  }
						/* assign alias */
/*
  if((rc=getatt(tucb_ptr, &dds->alias,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "alias",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }
  */
						  /* assign rcv_que_size */
/*
  if((rc=getatt(tucb_ptr,&dds->rcv_que_size,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
	    "rcv_que_size",NULL))>0) {
	diagex_initial_state(logical_name);
	return rc;
  }
  */
  
  odm_close_class(CuDv_CLASS);
  odm_close_class(CuAt_CLASS);
  odm_close_class(PdAt_CLASS);
  odm_unlock(lockid);

  dds->intr_priority = INTCLASS2;
  return (0);
}







/**************************************************************************
 * NAME: getatt
 *
 * FUNCTION: Reads an attribute from the customized database, predefined
 *      database, or change list.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is linked into the device specific sections of the
 *      various config, and change methods. No global variables are used.
 *
 * NOTES:
 *
 * int
 *   getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt )
 *
 *      dest_addr = pointer to the destination field.
 *      dest_type = The data type which the attribute is to be converted to
 *                    's' = string              rep=s
 *                    'b' = byte sequence       rep=s,  e.g. "0x56FFE67.."
 *                    'l' = long                rep=n
 *                    'i' = int                 rep=n
 *                    'h' = short (half)        rep=n
 *                    'c' = char                rep=n,or s
 *                    'a' = address             rep=n
 *      cuat_oc   = Customized Attribute Object Class.
 *      pdat_oc   = Predefined Attribute Object Class.
 *      lname     = Device logical name. ( or parent's logical name )
 *      utype     = Device uniquetype. ( or parent's uniquetype )
 *      att_name  = attribute name to retrieve from the Customized
 *                  Attribute Object Class.
 *      newatt    = New attributes to be scanned before reading database
 *
 *
 * RETURNS:
 *      0  = Successful
 *      <0 = Successful (for byte sequence only, = -ve no. of bytes)
 *      >0 = errno ( E_NOATTR = attribute not found )
 *
**************************************************************************/
int getatt(tucb_ptr, dest_addr, dest_type, cuat_oc, pdat_oc, lname, utype,
att_name, newatt)
TUTYPE *tucb_ptr;
void            *dest_addr;     /* Address of destination                   */
char            dest_type;      /* Destination type                         */
struct  Class   *cuat_oc;       /* handle for Customized Attribute OC       */
struct  Class   *pdat_oc;       /* handle for Predefined Attribute OC       */
char            *lname;         /* device logical name                      */
char            *utype;         /* device unique type                       */
char            *att_name;      /* attribute name                           */
struct  attr    *newatt;        /* List of new attributes                   */
{
  struct  CuAt    cuat_obj;
  struct  PdAt    pdat_obj;
  struct  attr    *att_changed();
  struct  attr    *att_ptr;
  int             convert_seq();
  int             rc;
  char            srchstr[100];
  char            *val_ptr;
  char            rep;

  /* Note: We need an entry from customized, or predefined even if */
  /* an entry from newatt is going to be used because there is no  */
  /* representation (rep) in newatt                                */
  
  DEBUG_2("getatt(): Attempting to get attribute %s for device %s\n",
	    att_name, lname)
 /* SEARCH FOR ENTRY IN CUSTOMIZED ATTRIBUTE CLASS */

  sprintf(srchstr, "name = '%s' AND attribute = '%s'", lname, att_name );

  if( cuat_oc == (struct Class *)NULL ) {
	rc = 0;
  } else {
	rc = (int)odm_get_obj( cuat_oc, srchstr, &cuat_obj, TRUE );
  }

  if( rc == 0 ) {
	/* OBJECT NOT FOUND, SEARCH IN PREDEFINED ATTRIBUTE CLASS */

	sprintf(srchstr, "uniquetype = '%s' AND attribute = '%s'",
		    utype, att_name );

	if((rc=(int)odm_get_obj(pdat_oc, srchstr, &pdat_obj, TRUE ))==0) {
		DEBUG_1("getatt(): Attribute %s not found ", att_name )
		DEBUG_2("in PdAt, or CuAt for %s, utype: %s\n", lname, utype )
	    	return(E_NOATTR);
	} else if ( rc == -1 ) {
		DEBUG_1("getatt(): error reading PdAt where %s\n", srchstr )
		return(E_ODMGET);
	}
	/* USE THE PREDEFINED ENTRY ( for now ) */

	val_ptr = pdat_obj.deflt;
	rep = pdat_obj.rep[strcspn(pdat_obj.rep,"sn")];

  } else if ( rc == -1 ) {
	DEBUG_1("getatt(): error reading CuAt where %s\n", srchstr )
	return(E_ODMGET);
  } else {
	/* USE THE CUSTOMIZED ENTRY ( for now ) */

	val_ptr = cuat_obj.value;
	rep = cuat_obj.rep[strcspn(cuat_obj.rep,"sn")];
  }

  /* CHECK TO SEE IF THIS ATTRIBUTE IS IN CHANGED LIST */

  if( ( att_ptr = att_changed(newatt,att_name))!=NULL) {
	val_ptr = att_ptr->value;
  }

  DEBUG_3("Attribute %s = '%s', rep = '%c'\n", att_name, val_ptr, rep )

  /* CONVERT THE DATA TYPE TO THE DESTINATION TYPE */

  return (convert_att(tucb_ptr, dest_addr, dest_type, val_ptr, rep ));
}





/*************************************************************************
 * NAME: dump_dds
 *
 * FUNCTION: Display a DDS for debugging purposes
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
*************************************************************************/

dump_dds( dds, dds_len )
char *dds;
int dds_len;
{
  #ifdef CFGDEBUG
  hexdump( dds, (long)dds_len);
  #endif
  ;
}






/*************************************************************************
 * NAME: hexdump
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
*************************************************************************/


hexdump(data,len)
char *data;
long len;
{

  #ifdef CFGDEBUG
  
  int     i,j,k;
  char    str[18];

  fprintf(stderr,"hexdump(): length=%ld\n",len);
  i=j=k=0;
  while(i<len) {
	j=(int) data[i++];
	if(j>=32 && j<=126) {
		str[k++]=(char) j;
	} else {
		str[k++]='.';
        }
	fprintf(stderr,"%02x ",j);
	if(!(i%8)) {
		fprintf(stderr,"  ");
		str[k++]=' ';
	} if(!(i%16)) {
		str[k]='\0';
		fprintf(stderr,"     %s\n\n",str);
		k=0;
	}
  }
  while(i%16) {
	if(!(i%8)) {
		fprintf(stderr,"  ");
	}
	fprintf(stderr,"   ");
	i++;
  }
  str[k]='\0';
  fprintf(stderr,"       %s\n\n",str);
  fflush(stderr);
  #endif
	;
}






/**************************************************************************
 * NAME: att_changed
 *
 * FUNCTION: Searches for an attribute in the new_attributes list
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Routines calling att_changed should include pparms.h for
 *      the definition of struct attr.
 *      This routine uses no global variables.
 *
 * NOTES:
 *
 *      if the list of changed attributes (at) is a NULL pointer, the
 *      routine accepts that there are no parameters in the list.
 *      Generally, the list consists of a sequence of attributes with
 *      the last attribute having a name of NULL.
**************************************************************************/

struct attr *att_changed(at,attname)
struct  attr *at;
char    *attname;
{
  struct  attr *p = at;

  if( at != NULL ) {
	while(p->attribute != NULL) {
		if(strcmp(p->attribute,attname) == 0) {
			return p;
		}
		p++;
	}
  }
  return (struct attr *)NULL;
}





/*************************************************************************
 * NAME: convert_att
 *
 * FUNCTION: This routine converts attributes into different data types
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Generally this routine is called by getatt(), but it is available
 *      to other procedures which need to convert data which may not also
 *      be represented in the database.
 *      No global variable are used, so this may be dynamically linked.
 *
 * RETURNS:
 *
 *       0 = Successful
 *      <0 = Successful (for byte sequence only, = -ve no. of bytes)
 *      >0 = errno
**************************************************************************/
int convert_att(tucb_ptr, dest_addr, dest_type, val_ptr, rep )
TUTYPE *tucb_ptr;
void    *dest_addr;             /* Address of destination                   */
char    dest_type;              /* Destination type                         */
char    *val_ptr;               /* Address of source                        */
char    rep;                    /* Representation of source ('s', or 'n')   */
{

  if( rep == 's' ) {
	switch( dest_type ) {
		case 's':
			strcpy( (char *)dest_addr, val_ptr );
			break;
		case 'c':
			*(char *)dest_addr = *val_ptr;
			break;
		case 'b':
			return ( convert_seq( val_ptr, (char *)dest_addr ) );
		default:
			DEBUG_1("dest_type is %c, should be s, c, or b\n",
			    dest_type )
			    return E_BADATTR;
	}
  } else if( rep == 'n' ) {
	switch( dest_type ) {
		case 'l':
			*(long *)dest_addr =
			    strtoul( val_ptr, (char **)NULL, 0);
			break;
		case 'i':
			*(int *)dest_addr =
			    (int)strtoul( val_ptr, (char **)NULL, 0);
			break;
		case 'h':
			*(short *)dest_addr =
			    (short)strtoul( val_ptr, (char **)NULL, 0);
			break;
		case 'c':
			*(char *)dest_addr =
			    (char)strtoul( val_ptr, (char **)NULL, 0);
			break;
		case 'a':
			*(void **)dest_addr =
			    (void *)strtoul( val_ptr, (char **)NULL, 0);
			break;
		default:
			DEBUG_1("dest_type is %c, should be l,i,h,c, or a\n",
			    dest_type )
			    return E_BADATTR;
	}
  } else {
	DEBUG_1("Rep field in attribute is %c, should be s, or n\n", rep)
	return E_BADATTR;
  }
  return 0;
}





/**************************************************************************
 * NAME: convert_seq
 *
 * FUNCTION: Converts a hex-style string to a sequence of bytes
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine uses no global variables
 *
 * NOTES:
 *
 *      The string to be converted is of the form
 *      "0xFFAAEE5A567456724650789789ABDEF678"  (for example)
 *      This would put the code FF into the first byte, AA into the second,
 *      etc.
 *
 * RETURNS: No of bytes, or -3 if error.
 *
***************************************************************************/

int convert_seq( source, dest )
char *source;
uchar *dest;
{
  char    byte_val[5];    /* e.g. "0x5F\0"        */
  int     byte_count = 0;
  uchar   tmp_val;
  char    *end_ptr;

  strcpy( byte_val, "0x00" );

  if( *source == '\0' ) {   /* Accept empty string as legal */
		return 0;
  }

  if( *source++ != '0' ) {
	return E_BADATTR;
  }

  if( tolower(*source++) != 'x' ) {
	return E_BADATTR;
  }

  while( ( byte_val[2] = *source ) && ( byte_val[3] = *(source+1) ) ) {
	source += 2;

	/* be careful not to store illegal bytes in case the
         * destination is of exact size, and the source has
         * trailing blanks
         */

	tmp_val = (uchar) strtoul( byte_val, &end_ptr, 0 );
	if( end_ptr != &byte_val[4] ) {
		break;
        }

	*dest++ = tmp_val;
	byte_count++;
  }

  return -byte_count;
}






/**************************************************************************
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * NOTES:
 *
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
**************************************************************************/
err_exit(exitcode)
char    exitcode;
{
  /* Close any open object class */
  odm_close_class(CuDv_CLASS);
  odm_close_class(PdDv_CLASS);
  odm_close_class(CuAt_CLASS);

  /* Terminate the ODM */
  odm_terminate();
  return(exitcode);
}





