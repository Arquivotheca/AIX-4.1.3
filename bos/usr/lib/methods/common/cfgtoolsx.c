static char sccsid[] = "@(#)48        1.7  src/bos/usr/lib/methods/common/cfgtoolsx.c, cfgmethods, bos41J, 9516A_all 4/14/95 15:02:42";
/*
 * COMPONENT_NAME: (CFGMETH) Routines required by Config, & Change methods.
 *
 * FUNCTIONS: Get_Parent_Bus, getatt, convert_att, convert_seq, hexdump, 
 * FUNCTIONS: mk_sp_file, read_descriptor, add_descriptor, put_vpd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "cfgdebug.h"
#include "pparms.h"
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <ctype.h>
#include <cf.h>
#include "cfgtoolsx.h"

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
 *      THERE IS A DUPLICATE OF THIS ROUTINE IN CFGTOOLS.C
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
    int	     rc ;
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
	    myrc = E_ODMGET ;	/* ODM error occurred; abort out.   */
	}
        else if (strncmp(Bus_Obj->PdDvLn_Lvalue, Bus_Class, 
		 strlen(Bus_Class)) == 0)
        {
	    myrc = 0 ;		/* Got bus object.		   */
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
 * NAME: get_attr_list
 *                                                                    
 * FUNCTION: Reads in all of the attributes for a device from the customized
 *	     database and predefined database and merges into one list of
 *	     attributes.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is linked into the device specific sections of the 
 *      various config, and change methods.
 *                                                                   
 * NOTES:
 *
 *   struct attr_list *
 *   get_attr_list(lname,utype,how_many,guess_num)
 *                                                                          
 *      lname = device name
 *	utype = device's uniquetype
 *      how_many = number of attributes being returned
 *      guess_num = expected number of attributes to be returned
 *                                                                          
 *                                                                          
 * RETURNS:
 *	(struct attr_list *)NULL = error, how_many indicates error code
 *	!=NULL = successful, how_many indicates how many attributes
 *		 were returned
 */  

struct attr_list *
get_attr_list(lname,utype,how_many,guess_num)
char    	*lname;         /* device logical name */
char		*utype;		/* device unique type */
int		*how_many;      /* number of attributes obtained */
int		guess_num;	/* expected number of attributes */
{
	struct  CuAt    *cuat;
	struct  PdAt    *pdat;
	struct listinfo pdat_info;
	struct listinfo cuat_info;
	struct	attr_list	*alist;
	int		i,j;
	char		criteria[256];


	sprintf(criteria, "uniquetype = %s",utype);
	pdat = (struct PdAt *)
		odm_get_list(PdAt_CLASS, criteria, &pdat_info, guess_num, 1);
	if ((int)pdat == -1) {
		*how_many = E_ODMGET;
		return(NULL);
	}
	if (!pdat_info.num) {
		*how_many = E_NOATTR;
		return(NULL);
	}

	/* malloc enough space for same number of CuAt attributes */
	alist = (struct attr_list *)
		malloc(sizeof(int) + (sizeof(struct CuAt) * pdat_info.num));
	if (alist == NULL) {
		*how_many = E_MALLOC;
		return(NULL);
	}

	/* copy the pdat info into the cuat structures */
	for(i=0; i<pdat_info.num; i++) {
		strcpy(alist->cuat[i].name, lname);
		strcpy(alist->cuat[i].attribute, pdat[i].attribute);
		strcpy(alist->cuat[i].value, pdat[i].deflt);
		strcpy(alist->cuat[i].type, pdat[i].type);
		strcpy(alist->cuat[i].generic, pdat[i].generic);
		strcpy(alist->cuat[i].rep, pdat[i].rep);
		alist->cuat[i].nls_index = pdat[i].nls_index;
	}
	if (pdat) free(pdat);
	*how_many = pdat_info.num;
	alist->attr_cnt = pdat_info.num;

	sprintf(criteria, "name = %s",lname);
	cuat = odm_get_list(CuAt_CLASS, criteria, &cuat_info, pdat_info.num, 1);
	if ((int)cuat == -1) {
		*how_many = E_ODMGET;
		return(NULL);
	}
	if (!cuat_info.num) {
		return(alist);
	}

	/* overlay the pdat data with the cuat data */
	for(j=0; j<cuat_info.num; j++) {
		for(i=0; i<*how_many; i++) {
			if (!strcmp(cuat[j].attribute,alist->cuat[i].attribute)) {
				strcpy(alist->cuat[i].value, cuat[j].value);
			}
		}
	}
	if (cuat) free(cuat);

	return(alist);
}


/*
 * NAME: getatt
 *                                                                    
 * FUNCTION: Obtains attribute values from a list of attributes created
 *	     by get_attr_list.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is linked into the device specific sections of the 
 *      various config, and change methods.
 *                                                                   
 * NOTES:
 *
 *   int                                                                        
 *   getatt(alist,aname,dest_addr,dest_type,bc)
 *
 *      alist = pointer returned by get_attr_list
 *      dest_addr = pointer to the destination field.                       
 *	dest_type = The data type which the attribute is to be converted to 
 *                    's' = string		rep=s			    
 *                    'b' = byte sequence	rep=s,  e.g. "0x56FFE67.."  
 *                    'l' = long		rep=n			    
 *                    'i' = int			rep=n			    
 *                    'h' = short (half)	rep=n			    
 *                    'c' = char		rep=n,or s		    
 *                    'a' = address		rep=n			    
 *      aname = attribute name to retrieve from the list          
 *                                                                          
 *                                                                          
 * RETURNS:
 *	 0 = Successful                                             
 *	<0 = Successful (for byte sequence only, = -(no. of bytes))    
 *	>0 = errno ( E_NOATTR = attribute not found )
 *
 */  

int
getatt(alist,aname,dest_addr,dest_type,bc)

struct attr_list	*alist;		/* list of attributes */
char		*aname;		/* attribute name */
void		*dest_addr;	/* Address of destination */
char		dest_type;	/* Destination type */
int		*bc;
{
	struct CuAt	*cuat;
	char		*val;
	char		rep;
	int		i;
	int		rc;
	int		attr_cnt;


	attr_cnt = alist->attr_cnt;
	cuat = (struct CuAt *)NULL;

	/* look through attribute list for desired attribute */
	for(i=0; i<attr_cnt; i++) {
		if (!strcmp(alist->cuat[i].attribute,aname)) {
			cuat = &alist->cuat[i];
			break;
		}
	}
	if (cuat == NULL)
		return(E_NOATTR);


	val = cuat->value;
	rep = cuat->rep[strcspn(cuat->rep,"sn")];

	return (convert_att( dest_addr, dest_type, val, rep, bc ));
}

/*
 * NAME: convert_att
 *                                                                    
 * FUNCTION: This routine converts attributes into different data types
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Generally this routine is called by getatt(), but it is available
 *	to other procedures which need to convert data which may not also
 *	be represented in the database.
 *
 * RETURNS:
 *
 *       0 = Successful                                             
 *      <0 = Successful (for byte sequence only, = -ve no. of bytes)    
 *      >0 = errno
 */  

int convert_att( dest_addr, dest_type, val_ptr, rep, bc )
void	*dest_addr;		/* Address of destination		    */
char	dest_type;		/* Destination type			    */
char	*val_ptr;		/* Address of source			    */
char	rep;			/* Representation of source ('s', or 'n')   */
int	*bc;
{
	int	rc;

	if( rep == 's' ) {
		switch( dest_type ) {
		case 's':
			strcpy( (char *)dest_addr, val_ptr );
			break;
		case 'c':
			*(char *)dest_addr = *val_ptr;
			break;
		case 'b':
			rc = convert_seq( val_ptr, (char *)dest_addr);
			if (rc>0)
				return(rc);
			*bc = -rc;
			break;
		default:
			DEBUG_1("dest_type is %c, should be s, c, or b\n",
				dest_type )
			return E_BADATTR;
		}
	}
	else if( rep == 'n' ) {
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
		case 's':
			strcpy((char *) dest_addr, val_ptr );
			break;
		default:
			DEBUG_1("dest_type is %c, should be l,i,h,c,s, or a\n",
				dest_type )
			return E_BADATTR;
		}
	}
	else {
		DEBUG_1("Rep field in attribute is %c, should be s, or n\n",
			rep)
		return(E_BADATTR);
	}
	return(0);
}

/*
 * NAME: convert_seq
 *                                                                    
 * FUNCTION: Converts a hex-style string to a sequence of bytes
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
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

int convert_seq( source, dest )
char *source;
uchar *dest;
{
	char	byte_val[5];	/* e.g. "0x5F\0"	*/
	int	byte_count = 0;
	uchar	tmp_val;
	char	*end_ptr;

	strcpy( byte_val, "0x00" );

	if( *source == '\0' )	/* Accept empty string as legal */
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
 * NAME: att_changed
 *                                                                    
 * FUNCTION: Searches for an attribute in the new_attributes list
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	Routines calling att_changed should include pparms.h for 
 *	the definition of struct attr.
 *                                                                   
 * NOTES:
 *	
 *	if the list of changed attributes (at) is a NULL pointer, the
 *	routine accepts that there are no parameters in the list.
 *	Generally, the list consists of a sequence of attributes with
 *	the last attribute having a name of NULL.
 */

struct attr *att_changed(at,attname)
struct	attr *at;
char	*attname;
{
	struct	attr *p = at;

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
 * NAME: hexdump
 *                                                                    
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *                                                                   
 * RETURNS: NONE
 */  

hexdump(data,len)
char *data;
long len;
{

#ifdef CFGDEBUG

	int	i,j,k;
	char	str[18];

	fprintf(stderr,"hexdump(): length=%ld\n",len);
	i=j=k=0;
	while(i<len)
	{
		j=(int) data[i++];
		if(j>=32 && j<=126)
			str[k++]=(char) j;
		else
			str[k++]='.';
		fprintf(stderr,"%02x ",j);
		if(!(i%8))
		{
			fprintf(stderr,"  ");
			str[k++]=' ';
		}
		if(!(i%16))
		{
			str[k]='\0';
			fprintf(stderr,"     %s\n\n",str);
			k=0;
		}
	}
	while(i%16)
	{
		if(!(i%8))
			fprintf(stderr,"  ");
		fprintf(stderr,"   ");
		i++;
	}
	str[k]='\0';
	fprintf(stderr,"       %s\n\n",str);
	fflush(stderr);
#endif
	;
}


/*
 * NAME: mk_sp_file
 *
 * FUNCTION: Creates, or alters a special file as required
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The device-specific portions of the config methods call this
 *	routine to generate the special files the device requires.
 *
 * NOTES:
 *	mk_sp_file(devno,suffix,cflags)
 *
 *	suffix	= suffix part of the special file name.  For most cases
 *		this will be the device logical name.  For devices with
 *		more than one special file, this routine will be called
 *		one time for each special file needed, passing the file
 *		name required for the special file.
 *	cflags	= create flags for determining the type and mode of the
 *		special file.
 *
 *	If the special file already exists, then the major/minor numbers
 *	are checked. If they are incorrect, the old file is deleted, and
 *	a new one created. If the numbers were correct, no action is
 *	taken, and 0 is returned.
 *
 * RETURNS: 0 For success, errno for failure.
 */
extern	int	errno;

int mk_sp_file(devno,suffix,cflags)
dev_t	devno;			/* major and minor numbers */
char	*suffix;		/* suffix for special file name */
long	cflags;			/* create flag / mode & type indicator */
{
	struct	stat	buf;
	char	spfilename[128];
	int	rc;
	long	filetype;	/* character or block device */

	filetype=cflags&(S_IFBLK|S_IFCHR);

	if(devno<0 | *suffix=='\0' | !filetype)
		return(E_MKSPECIAL);	/* error in parameters */

	sprintf(spfilename,"/dev/%s",suffix);	/* file name =/dev/[suffix] */


	if(stat(spfilename,&buf)) {

		/* stat failed, check that reason is ok */

		if( errno != ENOENT ) {
			DEBUG_0("stat failed\n")
			return(E_MKSPECIAL);
		}

		/* file does not exist, so make it */

		if(mknod(spfilename,filetype,devno)) {
			DEBUG_0("mknod failed\n")
			return(E_MKSPECIAL);
		}

	} else {

		/* stat succeeded, so file already exists */

		if(buf.st_rdev==devno)		/* major/minor #s are same, */
			return(0);		/* leave special file alone */
		if(unlink(spfilename)) {	/* unlink special file name */
			DEBUG_0("unlink failed\n")
			return(E_MKSPECIAL);
		}
		if(mknod(spfilename,filetype,devno)) {
			/* create special file */
			DEBUG_0("mknod failed\n")
			return(E_MKSPECIAL);
		}
	}

	/* change mode of special file.  This is not in the same step as   */
	/* creating the special file because of an error in mknod().	   */
	if(chmod(spfilename, (cflags&(~filetype)))) {
		DEBUG_0("chmod failed\n")
		return (E_MKSPECIAL);
	}

	return (0);
}

/*
 * NAME: read_descriptor
 *                                                                    
 * FUNCTION: Reads a descriptor from the VPD for a device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This function is used to decode VPD which is stored in the format
 *	used by the system devices ( CPU's, Planars etc. )
 *                                                                   
 * NOTES:
 *	VPD is stored as a series of descriptors, each of which
 * is encoded as follows:
 *
 * Byte 0 = '*'
 * Byte 1,2 = mnemonic		( E.g. "TM", "Z1", etc )
 * Byte 3 = Total length / 2
 * Byte 4.. = data
 *
 *  E.g.:  Byte#     0    1    2    3    4    5    6    7    8    9
 *         Ascii    '*'  'Z'  '1'       '0'  '1'  '2'  '0'  '0'  '1'
 *         Hex       2A   5A   31   05   30   31   32   30   30   31
 *         Oct      052  132  061  005  060  061  062  060  060  061
 *
 * RETURNS:
 *
 *	A pointer to the static char array "result" is returned, with
 *	the array being empty if the descriptor was not present in the
 *	VPD passed in.
 */

char *read_descriptor( vpd, name )
register char *vpd;
char *name;
{
static		char	result[256];
register	char	*res_ptr;
register	int	bytecount;

	res_ptr = result;
	*res_ptr = '\0';

	while( *vpd == '*' )
	{
		if( ( vpd[1] == name[0] ) && ( vpd[2] == name[1] ) )
		{
			/* This is the correct descriptor */
			bytecount = ((int)vpd[3] << 1 ) - 4;

			vpd += 4;
			
			while( bytecount-- )
				*res_ptr++ = *vpd++;

			*res_ptr = '\0';
		}
		else
			/* Skip to next descriptor */
			vpd += ( (int)vpd[3] << 1 );
	}

	return result;
}

/*
 * NAME: add_descriptor
 *                                                                    
 * FUNCTION: Adds a descriptor to the VPD for a device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This function is used to encode VPD in the format used by the
 *	system devices ( CPU's, Planars etc. )
 *                                                                   
 * NOTES:
 *	VPD is stored as a series of descriptors, each of which
 *	is encoded as follows:
 *
 *	Byte 0 = '*'
 *	Byte 1,2 = mnemonic		( E.g. "TM", "Z1", etc )
 *	Byte 3 = Total length / 2
 *	Byte 4.. = data
 *
 *	 E.g.:  Byte#     0    1    2    3    4    5    6    7    8    9
 *	        Ascii    '*'  'Z'  '1'       '0'  '1'  '2'  '0'  '0'  '1'
 *	        Hex       2A   5A   31   05   30   31   32   30   30   31
 *	        Oct      052  132  061  005  060  061  062  060  060  061
 *
 *	It is up to the calling method to verify that there is enough space
 *	in the destination VPD.
 *
 * RETURNS: NONE
 */

add_descriptor( vpd, name, value )
register char *vpd;
char *name;
register char *value;
{
register int len;

#ifdef CFGDEBUG
	char	*vpd_start = vpd;

	DEBUG_3("adding descriptor '%s', length %d, value '%s'\n",
		name, strlen(value), value )
#endif

	len = strlen( value );


	/* Skip past descriptors already there */
	while( *vpd == '*' )
		vpd += ( (int)vpd[3] << 1 );

	/* Byte #0 = '*' */
	*vpd++ = '*';

	/* Byte #1, and #2 contain mnemonic */
	*vpd++ = *name++;
	*vpd++ = *name;

	if( len & 1 )
	{
		/* Value has an odd length, so a space must be padded */

		/* Byte #3 contains TOTAL number of bytes divided by 2 */
		*vpd++ = (char)((len>>1)+3);
		
		/* Byte #4.. contains data */
		while( len-- )
			*vpd++ = *value++;
		*vpd++ = ' ';
		*vpd = '\0';
	}
	else
	{
		/* Value has an even length */

		/* Byte #3 contains TOTAL number of bytes divided by 2 */
		*vpd++ = (char)((len>>1)+2);

		/* Byte #4.. contains data */
		while( *vpd++ = *value++ );
	}

	DEBUG_1("TOTAL VPD LENGTH NOW %d\n", strlen( vpd_start ) )

}	


/*
 * NAME: put_vpd
 *                                                                    
 * FUNCTION:
 *
 *	copies the VPD from src to dest for len number of bytes.  This
 *	does any conversion of data nesseccary for correct storage in the data
 *	base.  A null byte is added to the end to terminate the VPD.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This function is use to transfer the VPD to the VPD database field.
 *	This is used by device methods which add or change VPD.
 *                                                                   
 * NOTES:
 *                                                                   
 *	Translation done: null bytes changed to spaces.
 *
 * RETURNS: NONE
 */

put_vpd(dest,src,len)
char	*dest,*src;
int	len;
{
	int i;

#ifdef CFGDEBUG
	DEBUG_0("put_vpd(): Dump of source\n");
	hexdump(src,(long) len);
#endif

	/* If the VPD is from an adapter card, there will be a 'VPD' at		*/
	/* the start, followed by a crc. This is not copied to the database	*/
	if( len > 8 )
		if( ( strncmp( src, "VPD", 3 ) == 0 ) && src[7] == '*' )
		{
			src += 7;
			len -= 7;
		}

	for(i=0; i<VPDSIZE; i++ )
	{
		if( len )
		{
			len--;
			*dest++ = *src++;
		}
		else
			*dest++ = '\0';
	}

#ifdef CFGDEBUG
	dest -= VPDSIZE;
	DEBUG_0("put_vpd(): Dump of destination\n");
	hexdump(dest,(long) VPDSIZE);
#endif
}
