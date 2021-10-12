static char sccsid[] = "@(#)64	1.1  src/bos/usr/lib/methods/cfgkma/cfgkma.c, inputdd, bos41J, 9509A_all 2/14/95 12:56:44";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		device_specific
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		setlocation
 *		wrt_iocc
 *		wrt_mem
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cfgktsm.h"
#include <sys/mdio.h>
#include <fcntl.h>

extern int defchild(char *, char *, char *);
extern char *gettok(char **);

/*
 * NAME     : build_dds 
 *
 * FUNCTION : This function builds the DDS for device driver
 *
 * NOTE     : There is no device driver specified by PdDv so this
 *            routine is never called
 *
 * RETURNS : 0
 *
 */

int build_dds(lname,ddsptr,dds_len)
char	*lname;				/* logical name of the device	*/
char	**ddsptr;			/* pointer to dds structure	*/
int 	*dds_len;			/* size of dds structure	*/
{
	return(0);
}

/*
 * NAME     : generate_minor
 *
 * FUNCTION : This function generates a minor number
 *
 * NOTE     : There is no device driver specified by PdDv so this
 *            routine is never called
 *
 * RETURNS : 0
 */

int generate_minor(lname,majorno,minordest)
char    *lname;         /* logical name of device       */
long    majorno;        /* major number of the device       */
long    *minordest;
{
    return 0;
}

/*
 * NAME     : make_special_files
 *
 * FUNCTION : This function creates a special file for the adapter
 *
 * NOTES    : There is no special file for this device instance
 *            so this routine does nothing
 *
 * RETURNS : 0
 */

int make_special_files(lname,devno)
char    *lname;         /* logical name of the device       */
dev_t   devno;          /* device number            */
{
	return 0;
}

/*
 * NAME     : download_microcode
 *
 * FUNCTION : This function download microcode if applicable
 *
 * NOTES    : There is no microcode associated with this device
 *            so this routine does nothing
 *
 * RETURNS : 0
 */

int download_microcode(lname)
char    *lname;         /* logical name of the device       */
{
	return 0;
}

/*
 * NAME     : query_vpd
 *
 * FUNCTION : This function is used to get device specific VPD.
 *
 * NOTES    : There is no VPD associated with this device
 *            so this routine is never called
 *
 * RETURNS : 0
 */

int query_vpd(newobj,kmid,devno,vpd_dest)
struct  CuDv    *newobj;        /* vpd info will be put in that         */
mid_t   kmid;                   /* kernel module id                     */
dev_t   devno;                  /* device number                        */
char    *vpd_dest;              /* Destination for vpd                  */

{
	return 0;
}

/*
 * NAME     : setlocation
 *
 * FUNCTION : This function sets child's location code
 *
 * RETURNS : 0 = success
 */

int setlocation(char *lname, char *uniquetype, char *connection, char location)
{
	int i, rc = 0;
	char sstring[128];
	struct CuDv  cdobj;

	sprintf( sstring,
	  "PdDvLn = %s AND parent = %s AND connwhere = %s",
	  uniquetype, lname, connection );
	i = (int) odm_get_obj( CuDv_CLASS, sstring, &cdobj, ODM_FIRST);
	if( i < 0 ) {
	  DEBUG_0("odm_get_obj failed\n")
	  rc = E_ODMGET;
	}
	else {
	  if (i > 0) {
	    sprintf(cdobj.location, "%s-0%c", cudv.location, location) ;
	    if (odm_change_obj(CuDv_CLASS,&cdobj) == -1) {
	      DEBUG_0("Error updating location\n");
	      rc = E_ODMUPDATE;
	    }
	  }
	}
    return(rc);
}

/*
 * NAME     : define_children 
 *
 * FUNCTION : This function defines children attached to the adapter
 *
 * RETURNS : 0
 * 
 */

int define_children(lname,ipl_phase)
char	*lname;			/* logical name of the device 		*/
int	ipl_phase;		/* ipl phase				*/
{
	int rc;
	char sstring[128];
	struct PdAt pdat;
	char *p, *uniquetype, *connection, *tp;

	DEBUG_0( "\ndefine children\n" );

	/* get name of children from PdAt     */
	sprintf(sstring, "uniquetype='%s' AND attribute='children'",
	        pddv.uniquetype);
	rc = (int)odm_get_first(PdAt_CLASS, sstring, &pdat) ;
	if(rc  <= 0) {
	  DEBUG_1("Error during ODM get searching for %s\n", sstring)
	  return(E_ODMGET) ;
	}

	/* define the children              */
	else {
	  DEBUG_1("children attribute = %s\n", pdat.values)
	  p = pdat.values;
	  do {
	    /* define kid   */
	    uniquetype = gettok(&p);
	    connection = gettok(&p);
	    if (!(rc = defchild(lname, uniquetype, connection))) {

	    /* fixup location code   */
	      tp = strrchr(uniquetype, '/') + 1;
	      if (strcmp(tp, KEYBOARD_TYPE) == 0)
	        rc = setlocation(lname, uniquetype, connection, KEYBOARD_LOCATION);
          else {
	        if (strcmp(tp, MOUSE_TYPE) == 0)
	          rc = setlocation(lname, uniquetype, connection, MOUSE_LOCATION);
	      } 
	    } 
	  } while (*p && !rc);
    }

	return(rc);
}

/*
 * NAME     : wrt_mem
 *
 * FUNCTION : write one byte to memory using machine device driver
 * 
 * RETURNS : 0 if successful
 */

int wrt_mem(int fd, char data,  ulong addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = &data;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(char);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
  return(rc);
}

/*
 * NAME     : wrt_iocc
 *
 * FUNCTION : write one byte to IOCC using machine device driver
 * 
 * RETURNS : 0 if successful
 */

int wrt_iocc(int fd, char data, ulong addr)
{
  MACH_DD_IO iob;
  int rc;

  DEBUG_2("wrt_iocc: addr = %x, data = %x\n", addr, data)

  iob.md_data = &data;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof (char);
  iob.md_addr = addr;


  rc = ioctl(fd, MIOCCPUT, &iob);
  return(rc);
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : Perform special adapter intialization
 *
 * RETURNS :
 * 	0 for success, >0 = errno.
 *
 */

int device_specific()
{
	struct  CuDv    busobj ;    /* Customized object for bus    */
	int rc, fd;
	char name[64];
	ulong pos_base;

	/* get name of bus  */
	if(rc=Get_Parent_Bus(CuDv_CLASS, cudv.parent, &busobj)) {
	  DEBUG_0("error getting bus object \n");
	  return(rc);
	}

	/* open machine device driver   */
	sprintf(name, "/dev/%s", busobj.name);
	fd = open(name, O_RDWR, 0);
	if (fd < 0) {
	  DEBUG_0("error openning machine device driver");
	  return(E_OPEN);
	}

	/* initialize card                        */
   	pos_base = (((atoi(cudv.connwhere)-1) & 0x0f) << 16) | 0x00400000;
    DEBUG_1("pos_base = %x\n", pos_base);

	rc = wrt_iocc(fd, 0, (pos_base + 7));  /* clear internal Olympus reset   */
    if(rc) {
	  DEBUG_0("error clearing internal Olympus reset\n");
	}
	else {                       /* enable adapter and parity checking */
                                 /* force all devices into reset       */
	  rc = wrt_iocc(fd, 0xff, (pos_base + 2));
      if(rc) {
	    DEBUG_0("error enabling adapter\n");
  	  }
  	}

	close(fd);

	return(rc);
}
