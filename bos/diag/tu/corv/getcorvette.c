static char sccsid[] = "@(#)29  1.3  src/bos/diag/tu/corv/getcorvette.c, tu_corv, bos411, 9428A410j 1/26/94 13:22:05";
/*
 *   COMPONENT_NAME: TU_CORV
 *
 *   FUNCTIONS: att_changed
 *              convert_att
 *              convert_seq
 *              err_exit
 *              getatt
 *              get_scsi_id
 *              getcorvette
 *              hexdump
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* header files needed for compilation */
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <cf.h>

#include "cfgad.h"
#include "getcorvette.h"
#include "ScsiBld.h"

/*
 *======================================================================
 * NAME: Get_Parent_Bus
 *
 * FUNCTION: Searches up the device heirarchy until it finds a device
 *           with class bus and returns its CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used within config methods.
 *
 *      THERE IS A DUPLICATE OF THIS ROUTINE IN CFGTOOLSX.C
 *
 * NOTES:
 *
 * int
 *   Get_Parent_Bus(Cus_Dev, Dev_Name, Bus_Obj)
 *      Cus_Dev  - INPUT: Open Customized device object class.
 *      Dev_Name - INPUT: Pointer to name of device from which
 *                        to start the search.
 *      Bus_Obj  - OUTPUT:pointer to space to put the bus devices CuDv
 *                        object.
 *
 * RETURNS:
 *      0        : Found bus device and returned CuDv object in parameter.
 *      E_ODMGET : Could not access ODM successfully.
 *      E_PARENT : Could not find a bus device in the heirarchy above
 *                 the input device.
 *======================================================================
 */

int
Get_Parent_Bus(Cus_Dev, Dev_Name, Bus_Obj)
    struct  Class   *Cus_Dev  ;
    char            *Dev_Name ;
    struct  CuDv    *Bus_Obj  ;
{
    char     Bus_Class[] = "bus/" ;   /* Bus class search string.       */
    int      rc ;
    int      myrc ;
    char     sstr[32] ;
/*----------------------------------------------------------------------*/
/* BEGIN Get_Parent_Bus */

    myrc = E_PARENT ;
    sprintf(sstr,"name = '%s'",Dev_Name);
    DEBUG_1("Get_Parent_Bus: getting CuDv for %s\n", sstr)
    rc = (int)odm_get_first(Cus_Dev, sstr, Bus_Obj) ;

    while ((rc != 0) && (myrc == E_PARENT))
    {
        if (rc == -1)
        {
            myrc = E_ODMGET ;   /* ODM error occurred; abort out.   */
        }
        else if (strncmp(Bus_Obj->PdDvLn_Lvalue, Bus_Class,
                 strlen(Bus_Class)) == 0)
        {
            myrc = 0 ;          /* Got bus object.                 */
        }
        else
        {
            sprintf(sstr,"name = '%s'", Bus_Obj->parent);
            DEBUG_1("Get_Parent_Bus: getting CuDv for %s\n", sstr)
            rc = (int)odm_get_first(Cus_Dev, sstr, Bus_Obj) ;
        }
    }
    return(myrc) ;
} /* END Get_Bus_Parent */

/*
 * NAME: get_scsi_id
 *
 * FUNCTION: Acquires the external scsi id address from nvram.
 *
 * RETURNS: >= 0 = external scsi id address
 *           < 0 = failure
 *
 */
 int get_scsi_id(slot_num)
  int slot_num;
 {
  FILE *in_file;
  char nvram_file[20];
  char nv_buf[100] ;
  char sstr[80] ;
  struct mdio mdio_struct;
  int ext_scsi_id;
  int rc;

  strcpy(nvram_file,"/dev/nvram");
  in_file = openx(nvram_file,O_RDONLY,0,1);
  if (in_file == NULL) {
   DEBUG_0("get_scsi_id: nvram file open error.\n");
   return(-1);
  }

  mdio_struct.md_addr = 0x00;        /* specified address */
  mdio_struct.md_size = 40;          /* size of md_data */
  mdio_struct.md_incr = MV_BYTE;     /* increment type MV_BYTE or MV_WORD */
  mdio_struct.md_data = &nv_buf[0];  /* pointer to space of size md_size */
  mdio_struct.md_sla = 0;            /* entry buid value, exit error code */

  rc = ioctl(in_file,MIONVGET,&mdio_struct);
  if (rc == -1) {
   sprintf(sstr,"%d",rc);
   DEBUG_1("get_scsi_id: ioctl error (%s).\n",sstr);
   return(-2);
  }

  rc = close(in_file);

  /*- scsi ids start at 0x10 offset in nvram -*/
  ext_scsi_id = (nv_buf[0x10 + slot_num] & 0x0F);  /*- low nibble BUID20 -*/
  sprintf(sstr,"%d",ext_scsi_id);
  DEBUG_1("get_scsi_id: external scsi id = %s.\n",sstr);

  if (rc == 0)
   return(ext_scsi_id);
  else
   return(rc);
}

/*
 * NAME: getad
 *
 * FUNCTION: Builds the DDS (Defined Data Structure) for the
 *           Lace Adapter
 *
 * RETURNS: 0 - success
 *         >0 - failure
 *
 */
corvette_dds *getcorvette(char *logical_name, corvette_dds **temp_dds)
{
   int     rc;                     /* used for return codes */
   char    sstring[512];           /* working string                   */
   char    *lname;                 /* pointer to device name */
   char    *pname;                 /* pointer to parent name */
   char    *ut;                    /* pointer to device's uniquetype */
   char    *pt;                    /* pointer to parent's uniquetype */
   struct  PdAt   pdatobj;

   struct  cfg_dd cfg;             /* sysconfig command structure */

   struct Class *cusdev;           /* customized devices class ptr */
   struct CuDv cusobj;             /* customized device object storage */
   struct CuDv busobj;             /* customized device object storage */
   struct CuDv parobj;             /* customized device object storage */
   corvette_dds *dds;

   int lock_id;
   char     sstr[32] ;


   /*****                                                          */
   /***** Validate Parameters                                      */
   /*****                                                          */
   /* logical name must be specified */
   if (logical_name == NULL) {
      DEBUG_0("cfgad: logical name must be specified\n");
      return(E_LNAME);
   }

   /* start up odm */
   if (odm_initialize() == -1) {
      /* initialization failed */
      DEBUG_0("cfgad: odm_initialize() failed\n")
      return(E_ODMINIT);
   }

   /* lock the database */
   if ((lock_id = odm_lock("/etc/objrepos/config_lock",0)) == -1) {
      DEBUG_0("cfgad: odm_lock() failed\n")
      return(err_exit(E_ODMLOCK));
   }

   DEBUG_0 ("ODM initialized and locked\n")

   /* open customized devices object class */
   if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
      DEBUG_0("cfgad: open class CuDv failed\n");
      return(err_exit(E_ODMOPEN));
   }
   /* search for customized object with this logical name */
   sprintf(sstring, "name = '%s'", logical_name);
   rc = (int)odm_get_first(cusdev,sstring,&cusobj);
   if (rc==0) {
      /* No CuDv object with this name */
      DEBUG_1("cfgad: failed to find CuDv object for %s\n", logical_name);
      return(err_exit(E_NOCuDv));
   } else if (rc==-1) {
     /* ODM failure */
     DEBUG_0("cfgad: ODM failure getting CuDv object");
     return(err_exit(E_ODMGET));
   }

   sprintf(sstring, "name = '%s'", cusobj.parent);
   rc = (int)odm_get_first(cusdev,sstring,&parobj);
   if (rc==0) {
      /* Parent device not in CuDv */
      DEBUG_0("cfgad: no parent CuDv object\n")
      return(err_exit(E_NOCuDvPARENT));
   } else if (rc==-1) {
       /* ODM failure */
       DEBUG_0("cfgad: ODM failure getting parent CuDv object\n")
       return(err_exit(E_ODMGET));
    }
    /* parent must be available to continue */
    if (parobj.status != AVAILABLE) {
       DEBUG_0("cfgad: parent is not AVAILABLE")
       return(err_exit(E_PARENTSTATE));
    }


   dds = (corvette_dds *) malloc( sizeof(corvette_dds) );
   if (dds == NULL)
           return(E_MALLOC);       /* report allocation error */
   /* Driver requires dds be cleared: */
   memset( dds, 0, sizeof(corvette_dds) );

   /* save some strings for short hand */
   lname = cusobj.name;
   ut = cusobj.PdDvLn_Lvalue;
   pname = cusobj.parent;
   pt = parobj.PdDvLn_Lvalue;
   strcpy(dds->par_name,pname);
   strcpy(dds->adpt_name,lname);

   DEBUG_2("getad: pname = %s pt = %s \n",pname,pt)
   if((int)(odm_open_class(CuAt_CLASS)) == -1){
       DEBUG_0("bld_dds: can not open CuAt\n")
       return E_ODMOPEN;
   }
   if((int)(odm_open_class(PdAt_CLASS)) == -1){
       DEBUG_0("bld_dds: can not open PdAt\n")
       return E_ODMOPEN;
   }

   if((rc=Get_Parent_Bus(cusdev, lname, &busobj))>0)
        return rc;

   /* get bus attributes */
   if((rc=getatt(&dds->bus_id,'l',CuAt_CLASS,PdAt_CLASS,busobj.name,busobj.PdDvLn_Lvalue,
       "bus_id",NULL))>0)return rc;
   if((rc=getatt(&dds->bus_type,'i',CuAt_CLASS,PdAt_CLASS,busobj.name,busobj.PdDvLn_Lvalue,
       "bus_type",NULL))>0)return rc;

   /* get device attributes */
   if((rc=getatt(&dds->bus_io_addr,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "bus_io_addr",NULL))>0)return rc;
   sprintf( sstring, "uniquetype = %s AND attribute = bus_io_addr",ut);
   if( (rc = odm_get_first(PdAt_CLASS, sstring, &pdatobj) )==0 )
           return( E_NOCuDv );
   else if ( rc == -1 )
           return( E_ODMGET );
   dds->bus_io_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);
   if((rc=getatt(&dds->bus_intr_lvl,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "bus_intr_lvl",NULL))>0)return rc;
   if((rc=getatt(&dds->intr_priority,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "intr_priority",NULL))>0)return rc;
   if((rc=getatt(&dds->dma_lvl,'i',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "dma_lvl",NULL))>0)return rc;
   if((rc=getatt(&dds->dma_bus_mem,'l',CuAt_CLASS,PdAt_CLASS,lname,ut,
       "dma_bus_mem",NULL))>0)return rc;
   dds->slot_num = atoi(cusobj.connwhere)-1;
   DEBUG_1("getcorvette: raw slot num = %d.\n",dds->slot_num);

   sprintf( sstring, "uniquetype = %s AND attribute = dma_bus_mem",ut);
   if( (rc = odm_get_first(PdAt_CLASS, sstring, &pdatobj) )==0 )
           return( E_NOCuDv );
   else if ( rc == -1 )
           return( E_ODMGET );
   dds->dma_bus_length = (ulong)strtol(pdatobj.width,(char **) NULL,0);
   dds->intr_flags =0;

   /* lock the database */
   if (odm_unlock(lock_id) == -1) {
      DEBUG_0("cfgad: odm_unlock() failed\n")
      return(err_exit(E_ODMLOCK));
   }

   odm_close_class(CuAt_CLASS);
   odm_close_class(PdAt_CLASS);

   /*--------------------------------------------*/
   /*- acquire the internal/external scsi ids   -*/
   /*-  internal is forced to id '7' by ipl ros -*/
   /*-  external is from nvram area             -*/
   /*-      nibble = --HI--/--LOW-              -*/
   /*-  nvram byte = BUID21/BUID20              -*/
   /*--------------------------------------------*/
   dds->int_scsi_id = _SCSI_ID_7;
   rc = get_scsi_id(dds->slot_num);
   if ( rc >= 0 )
    dds->ext_scsi_id = rc;
   else
    return( E_ODMGET );

   *temp_dds = dds;
   return 0;
}
/*
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
 */

int getatt(dest_addr,dest_type,cuat_oc,pdat_oc,lname,utype,att_name,newatt)
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

        if( cuat_oc == (struct Class *)NULL )
                rc = 0;
        else
                rc = odm_get_obj( cuat_oc, srchstr, &cuat_obj, TRUE );

        if( rc == 0 )
        {
                /* OBJECT NOT FOUND, SEARCH IN PREDEFINED ATTRIBUTE CLASS */

                sprintf(srchstr, "uniquetype = '%s' AND attribute = '%s'",
                        utype, att_name );

                if((rc=odm_get_obj( pdat_oc, srchstr, &pdat_obj, TRUE ))==0)
                {
                        DEBUG_1("getatt(): Attribute %s not found ", att_name )
                        DEBUG_2("in PdAt, or CuAt for %s, utype: %s\n", lname,
                                utype )
                        return(E_NOATTR);
                }
                else if ( rc == -1 )
                {
                        DEBUG_1("getatt(): error reading PdAt where %s\n",
                                srchstr )
                        return(E_ODMGET);
                }
                /* USE THE PREDEFINED ENTRY ( for now ) */

                val_ptr = pdat_obj.deflt;
                rep = pdat_obj.rep[strcspn(pdat_obj.rep,"sn")];

        }
        else if ( rc == -1 )
        {
                DEBUG_1("getatt(): error reading CuAt where %s\n",
                        srchstr )
                return(E_ODMGET);
        }
        else
        {
                /* USE THE CUSTOMIZED ENTRY ( for now ) */

                val_ptr = cuat_obj.value;
                rep = cuat_obj.rep[strcspn(cuat_obj.rep,"sn")];
        }

        /* CHECK TO SEE IF THIS ATTRIBUTE IS IN CHANGED LIST */

        if( ( att_ptr = att_changed(newatt,att_name))!=NULL)
                val_ptr = att_ptr->value;

        DEBUG_3("Attribute %s = '%s', rep = '%c'\n", att_name, val_ptr, rep )

        /* CONVERT THE DATA TYPE TO THE DESTINATION TYPE */

        return (convert_att( dest_addr, dest_type, val_ptr, rep ));
}

/*
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
 */

struct attr *att_changed(at,attname)
struct  attr *at;
char    *attname;
{
        struct  attr *p = at;

        if( at != NULL )
                while(p->attribute != NULL)
                {
                        if(strcmp(p->attribute,attname) == 0)
                                return p;
                        p++;
                }
        return (struct attr *)NULL;
}
/*
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
 */

int convert_att( dest_addr, dest_type, val_ptr, rep )
void    *dest_addr;             /* Address of destination                   */
char    dest_type;              /* Destination type                         */
char    *val_ptr;               /* Address of source                        */
char    rep;                    /* Representation of source ('s', or 'n')   */
{

        if( rep == 's' )
        {
                switch( dest_type )
                {
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
        }
        else if( rep == 'n' )
        {
                switch( dest_type )
                {
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
        }
        else
        {
                DEBUG_1("Rep field in attribute is %c, should be s, or n\n",
                        rep)
                return E_BADATTR;
        }
        return 0;
}
/*
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
 */

int convert_seq( source, dest )
char *source;
uchar *dest;
{
        char    byte_val[5];    /* e.g. "0x5F\0"        */
        int     byte_count = 0;
        uchar   tmp_val;
        char    *end_ptr;

        strcpy( byte_val, "0x00" );

        if( *source == '\0' )   /* Accept empty string as legal */
                return 0;

        if( *source++ != '0' )
                return E_BADATTR;
        if( tolower(*source++) != 'x' )
                return E_BADATTR;

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

        return -byte_count;
}

/*
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
 */

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
