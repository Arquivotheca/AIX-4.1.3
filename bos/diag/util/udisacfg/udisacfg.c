static char sccsid[] = "@(#)11	1.4  src/bos/diag/util/udisacfg/udisacfg.c, dsauisacfg, bos41J, 9521A_all 5/21/95 09:18:47";
/*
 * COMPONENT_NAME: DSAUISACFG 
 *
 * FUNCTIONS:   main
 *              add
 *              delete
 * 		get_ISA_list
 * 		process_ISA_adap
 * 		get_ISA_bus_io_addr
 * 		disp_err_ISA
 * 		read_supp_dskt
 * 		read_isa_supp_diskette
 * 		check_ISA_busses
 * 		list_supported_isa
 * 		list_defined_isa
 * 		remove_ISA_adap
 *              int_handler
 *              genexit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <locale.h>
#include <sys/cfgdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ustat.h>
#include <dirent.h>
#include <asl.h>
#include <odmi.h>
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "udisacfg_msg.h"

#define ADD_FLAG	1
#define REMOVE_FLAG 	2
#define ODM_DEPTH	1
#define MAX_ODM_ENT 	4	

/* GLOBAL VARIABLES     */
nl_catd fdes;                   /* catalog file descriptor               */

/* LOCAL FUNCTION PROTOTYPES */

int    add(void);
void	read_supp_dskt(void);
void    delete(void);
void    list_defined_isa(void);
void    list_supported_isa(void);
void    int_handler(int);
void    genexit(int);
void    process_ISA_adap( struct PdDv *, char * );
char 	*get_ISA_bus_io_addr( struct PdDv * );
void 	disp_err_ISA( int, char *, char * );
char	*check_ISA_busses(void);
int 	read_diskette(char *);
void	remove_ISA_adap(char *);

typedef	struct ISA_dev_ptr_s {
		struct	PdDv 		*pddv_ptr;
		char			*text;
		}  *ISA_dev_ptr_t, ISA_dev_ptr;

/* CALLED FUNCTIONS     */
extern char *diag_cat_gets();
extern char *diag_device_gets();
extern nl_catd diag_catopen(char *, int);
extern nl_catd diag_device_catopen(char *, int);
extern int file_present(char *);

/*  */
/*
 * NAME: main
 *
 * FUNCTION: This unit displays the ISA Configuration Menu List
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

main(int argc, char *argv[])
{

        int     status = -1;
        struct sigaction act;

        static struct msglist menulist[] = {
                {ISA_SET, ISA_MSG_T},
                {ISA_SET, ISA_MSG_ADD},
                {ISA_SET, ISA_MSG_DEL},
                {ISA_SET, ISA_MSG_DEF},
                {ISA_SET, ISA_MSG_SP},
                {ISA_SET, ISA_LLINE},
                {(int )NULL, (int )NULL}
        };

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* initialize ASL       */
        diag_asl_init("DEFAULT");

        /* open the catalog file containing the menus */
        fdes = diag_catopen(MF_UDISACFG,0);

        /* initialize the ODM data base */
        init_dgodm();

	while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT) {
          status = diag_display(0x802300, fdes, menulist, DIAG_IO,
                   ASL_DIAG_LIST_CANCEL_EXIT_SC, NULL, NULL);
	  if ( status == DIAG_ASL_COMMIT ) {
            switch ( DIAG_ITEM_SELECTED(dm_menutype) )  {
                    case 1 :
                           add ();
                           break;
                    case 2 :
                           delete ();
                           break;
                    case 3 :
                           list_defined_isa ();
                           break;
                    case 4 :
                           list_supported_isa ();
                           break;
               }
	   }
	}
        genexit(0);
}

/*  */
/*
 * NAME: get_ISA_list
 *
 * FUNCTION: Searches PDiagDev and PdDv for any supported ISA adapters. 
 *
 * NOTES:
 *
*/

ISA_dev_ptr *get_ISA_list ( int *pddv_found )
{
	int	pdiagcnt, pddvcnt;
	struct	PDiagDev	*pdiagdev;
	struct	PdDv		*pddv;
        struct	listinfo	obj_pdiagdev,	obj_pddv;
        char	criteria[256];

	nl_catd	pddv_catd;
	char	*isa_dev_desc;

	ISA_dev_ptr	*ISA_dev_list=NULL;

	/* search all PDiagDev for any isa adapter */
        sprintf(criteria, "DClass = adapter AND DSClass = isa");
	pdiagdev = (struct PDiagDev *) diag_get_list(PDiagDev_CLASS, criteria,
                   &obj_pdiagdev, MAX_ODM_ENT, ODM_DEPTH);

	if (pdiagdev == (struct PDiagDev *) -1)
		return ( (ISA_dev_ptr *) -1);

	/* Found all entries in the PDiagDev. Now grab all the PdDv entries for the
	   device */
	sprintf(criteria, "class = adapter AND subclass = isa");
	pddv = (struct PdDv *) diag_get_list(PdDv_CLASS, criteria,
                   &obj_pddv, MAX_ODM_ENT, ODM_DEPTH);
	if (pddv == (struct PdDv *) -1)
		return ( (ISA_dev_ptr *) -1);

	/* Found all PdDv entries, now match the PDiagDev and PdDv entries. */
	/* Create an array to hold supported ISA devices that have a matching */
	/* PdiagDev and PdDv entry. 					      */
	
        ISA_dev_list = (struct ISA_dev_ptr_s *)calloc(obj_pdiagdev.num, 
						sizeof(struct ISA_dev_ptr_s));

	for (pdiagcnt=0, (*pddv_found)=0; pdiagcnt < obj_pdiagdev.num; pdiagcnt++)
	{
		for (pddvcnt=0; pddvcnt < obj_pddv.num; pddvcnt++) {
	       		if ((strcmp(pdiagdev[pdiagcnt].DType, pddv[pddvcnt].type))==0)
			{
				ISA_dev_list[*pddv_found].pddv_ptr = &pddv[pddvcnt];
				pddv_catd = diag_device_catopen(
					ISA_dev_list[*pddv_found].pddv_ptr->catalog,0);
				isa_dev_desc = diag_device_gets(pddv_catd, 
					ISA_dev_list[*pddv_found].pddv_ptr->setno,
					ISA_dev_list[*pddv_found].pddv_ptr->msgno,
					"ISA description not available");
				ISA_dev_list[*pddv_found].text = (char *)malloc 
								(strlen(isa_dev_desc)+1);
				strcpy( ISA_dev_list[*pddv_found].text, isa_dev_desc);
				catclose ( pddv_catd );
				(*pddv_found)++;
      			}
		}
  	}
	if ( pdiagdev )
		diag_free_list ( pdiagdev, &obj_pdiagdev );
	return ( ISA_dev_list );
}

/*  */
/*
 * NAME: add
 *
 * FUNCTION: Searches PDiagDev and PdDv for any supported ISA adapters. When
 * found, a 'mkdev' is run to define and configure the selected device. 
 *
 * NOTES:
 *
*/

int add (void)
{
	int	i=0,	status=-1;
	int	index = 0;	/* index into asl structure display buffer */

        int     pddv_found=0;
	static	ASL_SCR_TYPE	dm_menutype = DM_TYPE_DEFAULTS;
	static	ASL_SCR_INFO	*dm_menuinfo;
	ISA_dev_ptr	*ISA_dev_list;

	ISA_dev_list = get_ISA_list ( &pddv_found );

	/* Create a structure to hold the menu data 			     */
	/* The number of fields to be allocated is equal to the title(1) +   */
        /* supplemental (1) + number of devices (pddv_found) +               */
   	/* last  line(1).                                                    */
        dm_menuinfo =  (ASL_SCR_INFO *)  calloc (3 + pddv_found,
                                                 sizeof(ASL_SCR_INFO));

	/* get the title */
        dm_menuinfo[index++].text = diag_cat_gets(fdes, ISA_ADD, ADD_ADP);

	/* Get the supported ISA adapters device text now */
	for (i=0; i < pddv_found; i++ )
        	dm_menuinfo[index++].text = ISA_dev_list[i].text; 

	/* add in the supplemental line */
	dm_menuinfo[index++].text = diag_cat_gets (fdes, ISA_ADD, ADD_SUP);
	/* lastly put in the last line */
	dm_menuinfo[index].text = diag_cat_gets (fdes, ISA_SET, ISA_LLINE);

	dm_menutype.max_index = index;

	status = diag_display(0x802301, fdes, NULL, DIAG_IO,
                         ASL_DIAG_LIST_CANCEL_EXIT_SC, &dm_menutype, dm_menuinfo);
  	if ( status == DIAG_ASL_COMMIT ) {
		if ( (!pddv_found && DIAG_ITEM_SELECTED(dm_menutype) == 1 ) || 
		     (DIAG_ITEM_SELECTED(dm_menutype) == pddv_found +1 ) ) 
			read_supp_dskt();
		else
			process_ISA_adap( 
			   ISA_dev_list[DIAG_ITEM_SELECTED(dm_menutype)-1].pddv_ptr,
			   ISA_dev_list[DIAG_ITEM_SELECTED(dm_menutype)-1].text);
	}
	free ( dm_menuinfo );
	/* Free up all memory allocated */
	for (i=0; i < pddv_found; i++ )
        	free ( ISA_dev_list[i].text); 
	free ( ISA_dev_list );
}

/*  */
/* 
 *
 * NAME: process_ISA_adap
 *
 * FUNCTION: This function displays menu 802304. The adapter
 * being added is invoked by the command:
 *      mkdev -c class -s subclass -t type -p "parent"
 * The "parent" is the ISA bus selected from function 
 * check_ISA_busses.
 *
 * NOTES:
 *
*/
void process_ISA_adap( struct PdDv *pddv_ptr, char *desc )
{

	int rc;
	char *parent;
	char *bus_addr;
	char *outbuf, *errbuf;
	char *opstring, *add_text, *buffer;
	char method_params[128];
	char new_device[NAMESIZE];
	static struct  msglist add[] = {
                        { ISA_ADD, ADD_IN_PROCESS, },
                        { ISA_ADD, ADD_MSG3, },
                	{(int )NULL, (int )NULL}
	};
        static ASL_SCR_INFO     addinfo[ DIAG_NUM_ENTRIES(add) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	
	parent = check_ISA_busses();
	if ( parent == (char)NULL)
		return;
	bus_addr = get_ISA_bus_io_addr( pddv_ptr );
	if ( bus_addr == (char *) -1)
		return;
	
        rc = diag_display(NULL, fdes, add, DIAG_MSGONLY, NULL,
                                        &menutype, addinfo);
	opstring = (char *)malloc(strlen(addinfo[1].text) + strlen(desc) + 16);
        sprintf(opstring, addinfo[1].text, desc);
        free(addinfo[1].text);
        addinfo[1].text = opstring;
        rc = diag_display(0x802304, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_OUTPUT_LEAVE_SC,
                                        &menutype, addinfo);

        /* odm_invoke( mkdev )  */
	if ( bus_addr == (char *) NULL )
        	sprintf(method_params, "-c %s -s %s -t %s -p %s",
                                pddv_ptr->class,
				pddv_ptr->subclass,
				pddv_ptr->type,
				parent);
	else
        	sprintf(method_params, "-c %s -s %s -t %s -p %s -a bus_io_addr=%s",
                                pddv_ptr->class,
				pddv_ptr->subclass,
				pddv_ptr->type,
				parent, bus_addr);
        rc = invoke_method( MKDEV, method_params, &outbuf, &errbuf);
	if ( rc == 0 ) {
        	sscanf ( outbuf, "%s", new_device );
		add_text = diag_cat_gets(fdes,ISA_ADD,ADD_MSG4_POP);
		buffer = (char *)malloc(strlen(add_text) + strlen(new_device) + 16);
		sprintf( buffer, add_text, new_device);
		diag_asl_msg( buffer );
        }
	else 
		disp_err_ISA( ADD_FLAG, desc, errbuf );

        free ( outbuf );
        free ( errbuf );
	free ( buffer );
	free ( opstring );

	return;

} 
/*  */
/* 
 *
 * NAME: get_ISA_bus_io_addr
 *
 * FUNCTION: This function searches the PdAt for the selected
 * adapter for attributes relating to the bus_io_addr.
 * If found, a dialog is presented letting the user   
 * select the bus_io_addr from a list.
 *
 * "bus_io_addr" - Hardware attribute
 * Value needs to match the dip switch setup on the device.
 * Value modifiable by user through SMIT or command line.
 * Displayable with "lsattr" command.
 * PdAt:
 *         uniquetype      = "adapter/isa/tokenring"
 *         attribute       = "bus_io_addr"
 *         deflt           = 0xa20
 *         values          = "0xa20,0xa24"
 *         width           = 4
 *         type            = "O"
 *         generic         = "DU"
 *         rep             = "nl"
 *         nls_index       = 3
 * 
 * NOTES:
 *
*/
char *get_ISA_bus_io_addr( struct PdDv *pddv_ptr )
{
        int     rc;
        int     line = 0;
	char 	criteria[256];
        char    *entered_buid;
        struct  PdAt    *pdat;
        struct  listinfo pinfo;

	static struct msglist menulist[] = {
        	{ BUS_SET, BUSIOTITLE },
        	{ BUS_SET, BUSIOPROMPT },
        	{ BUS_SET, BUSIODEFAULT },
        	{ (int )NULL, (int )NULL}
	};
	static ASL_SCR_INFO dialog_info[DIAG_NUM_ENTRIES(menulist)];
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        rc = diag_display(NULL,fdes,menulist,DIAG_MSGONLY,
                                NULL,&menutype,dialog_info);

	/* Search PdAt for bus_io_addr attributes for this adapter */
	sprintf(criteria,"uniquetype=%s AND attribute = bus_io_addr",pddv_ptr->uniquetype);
        pdat = (struct PdAt *) diag_get_list(PdAt_CLASS, criteria, &pinfo, 1, 1);
        if (pdat == (struct PdAt *) -1 || pinfo.num == 0 )
                return ( (char *) NULL);

        /* slot prompt */
	entered_buid = (char *)malloc (strlen (pdat->deflt) + 1);
        strcpy( entered_buid, pdat->deflt);
        dialog_info[1].op_type = ASL_RING_ENTRY;
        dialog_info[1].entry_type = ASL_NO_ENTRY;
        dialog_info[1].required = ASL_NO;
        dialog_info[1].disp_values = pdat->values;
        dialog_info[1].data_value = entered_buid;
        dialog_info[1].entry_size = strlen(pdat->deflt) + 2;
        dialog_info[1].changed = ASL_NO;
        dialog_info[1].cur_value_index = 0;
        dialog_info[1].default_value_index = 0;
	

        rc = diag_display( 0x802302, fdes, NULL, DIAG_IO,
                        ASL_DIAG_DIALOGUE_SC, &menutype, dialog_info );
        if ( rc == DIAG_ASL_EXIT || rc == DIAG_ASL_CANCEL )
                        return((char *)-1);
	if ( dialog_info[1].changed == ASL_YES) {
		while ( *entered_buid == ' ' )
			entered_buid++;
        	return (entered_buid);
	}
	else
		return ( (char *)NULL );
}
/*  */
/* 
 *
 * NAME: disp_err_ISA
 *
 * FUNCTION: This function displays menu 802305. This menu
 * is displayed whenever there is an error from the 'mkdev'
 * or 'rmdev' command.
 *
 * NOTES:
 *
*/
void disp_err_ISA( int flag, char *desc, char *errbuf )
{
	int rc;
	char *operation, *opstring, *errstring;
        static struct  msglist err[] = {
                        { ADD_ERR, ADD_ERR_MSG, },
                        { ADD_ERR, ADD_CMD_MSG, },
                        { ADD_ERR, ADD_CONTINUE, },
                        {(int )NULL, (int )NULL}
        };
        static ASL_SCR_INFO     errinfo[ DIAG_NUM_ENTRIES(err) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        rc = diag_display(NULL, fdes, err, DIAG_MSGONLY, NULL,
                                        &menutype, errinfo);
	if ( flag == ADD_FLAG )
		operation = diag_cat_gets(fdes, ADD_ERR, ADD_STRING);
	else
		operation = diag_cat_gets(fdes, ADD_ERR, REM_STRING);
	
	opstring = (char *)malloc(strlen(errinfo[0].text) + 32 + 
					strlen(desc) + strlen(operation));
        sprintf(opstring, errinfo[0].text, operation, desc);
        free(errinfo[0].text);
        errinfo[0].text = opstring;

	errstring = (char *)malloc(strlen(errinfo[1].text) + 32 + strlen(errbuf));
        sprintf(errstring, errinfo[1].text, errbuf);
        free(errinfo[1].text);
        errinfo[1].text = errstring;

        rc = diag_display(0x802305, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_KEYS_ENTER_SC, &menutype, errinfo);
	free ( opstring );
	return;

}


/*  */
/* 
 * NAME: read_supp_dskt
 *
 * FUNCTION: This function displays menu 802303. A diskette
 * is read and then status checked from diskette script file.
 * The script file must be called 'diagstartS'.
 *
 * NOTES:
 *
*/
void read_supp_dskt(void)
{
	int	status = -1;
	char	*parent;
        static struct msglist menulist9[] = {
                { READ_DSKT, DSKT_T},
                { READ_DSKT, DSKT_MSG},
                { READ_DSKT, DSKT_INST},
                {(int )NULL, (int )NULL}
        };

	parent = check_ISA_busses();
	if ( parent == (char)NULL)
		return;
	
        status = diag_display(0x802303, fdes, menulist9, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

	if ( read_isa_supp_diskette( parent ) )
		diag_asl_msg( diag_cat_gets(fdes, READ_DSKT, DSKT_ERR_POP) );
	else
		diag_asl_msg( diag_cat_gets(fdes, READ_DSKT, DSKT_GOOD_POP) );
	return;
}

/*
 * NAME: read_isa_supp_diskette
 *
 * FUNCTION: Reads a diskette
 * 
 * NOTES: Invokes cpio to read diskette
 *	  Run diagstartS script 
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *	-1 - failure
 *	 0 - success
 */

int read_isa_supp_diskette( char *parent )
{
	int	rc;
	char buffer[128];
	char *outb=NULL;
	char *ebuf=NULL;

	strcpy(buffer,"-iduC36 </dev/rfd0");

	rc=odm_run_method(CPIO, buffer, &outb, &ebuf);

	if(rc < 0) return(-1);

	sprintf(buffer, "/etc/diagstartS %s 1>/dev/null 2>&1", parent );
	rc = system(buffer);

	return(rc);
}
/*  */
/* 
 *
 * NAME: check_ISA_busses
 *
 * FUNCTION: Searches CuDv database for any ISA bus type. If 
 * there is only one bus, then return the name of the bus. 
 * If no bus found, return a (char)NULL.
 * Else present menu requesting user to select a bus. Return name of
 * bus selected. 
 *
 * NOTES:
 *
*/
char *check_ISA_busses()
{

        int     i=0,    status=-1,      depth = 1,	count=0;
        int     index = 0;      /* index into asl structure display buffer */

        nl_catd cudv_catd;

        static  ASL_SCR_TYPE    dm_menutype = DM_TYPE_DEFAULTS;
        static  ASL_SCR_INFO    *dm_menuinfo;
	struct CuDv *cudv;
        struct	listinfo	obj_cudv;
        char	criteria[256];
        char    *isa_bus_desc;
        char    *bus_buffer;
	char	*save_bus_buffer;
	char 	*bus_name = (char *)NULL;

   	sprintf(criteria, "PdDvLn = bus/pci/isa");
        cudv = (struct CuDv *) diag_get_list(CuDv_CLASS, criteria,
                   &obj_cudv, 2, 2);
        if ( (cudv == (struct CuDv *) -1) || !obj_cudv.num  )
		return ( bus_name );
	else if ( obj_cudv.num == 1 ) {
		bus_name = (char *)malloc(strlen(cudv->name) + 1);
		strcpy(bus_name,cudv->name);
		diag_free_list ( cudv, &obj_cudv );
		return ( bus_name );
	}

	/* Create a structure to hold the menu data.  The number of fields */
	/* to be allocated is equal to the title (1) + the                   */
        /* number of busses found (obj_cudv.num) + the last line (1) */

	dm_menuinfo = (ASL_SCR_INFO *) calloc (2+obj_cudv.num,
						sizeof(ASL_SCR_INFO));
	dm_menuinfo[index++].text = diag_cat_gets (fdes, ISA_ADD, ADD_BUS);
	save_bus_buffer = bus_buffer = (char *)malloc (obj_cudv.num * 132);

	/* Get the ISA bus text now */
	for ( i=0; i < obj_cudv.num; i++) {
		cudv_catd = diag_device_catopen (cudv[i].PdDvLn->catalog, 0);
		isa_bus_desc = diag_device_gets (cudv_catd,
				cudv[i].PdDvLn->setno,
				cudv[i].PdDvLn->msgno,
				"BUS description not available");

		sprintf (bus_buffer, "%-12s %-12s %s", cudv[i].name, cudv[i].location,
							isa_bus_desc);
		dm_menuinfo[index++].text = bus_buffer;
		bus_buffer = bus_buffer + strlen (bus_buffer)+1;
	}

	/* Get the last line */
	dm_menuinfo[index].text = diag_cat_gets (fdes, ISA_SET, ISA_LLINE);

	dm_menutype.max_index = index;
	status = diag_display (0x802302, fdes, NULL, DIAG_IO, 
		ASL_DIAG_LIST_CANCEL_EXIT_SC, &dm_menutype, dm_menuinfo);
	if ( status == DIAG_ASL_COMMIT ) {
		bus_name = (char *)malloc(strlen(
			cudv[DIAG_ITEM_SELECTED(dm_menutype)-1].name) + 1);
		strcpy( bus_name, cudv[DIAG_ITEM_SELECTED(dm_menutype)-1].name);
	}

	free (save_bus_buffer);
	free (dm_menuinfo);
	diag_free_list ( cudv, &obj_cudv );
	return ( bus_name );
}

/*  */
/* 
 * NAME: list_supported_isa
 *
 * FUNCTION: This function displays all the supported ISA adapters	
 * found in the system by matching the PDiagDev and PdDv entries. 
 *
 * NOTES:
 *
*/
void list_supported_isa(void)
{

	int	i=0,	status=-1;
	int	index = 0;	/* index into asl structure display buffer */

        int     pddv_found=0;
	char	*buffer=NULL;
	char	*save_buffer;
	char	*tmp;
	static	ASL_SCR_TYPE	dm_menutype = DM_TYPE_DEFAULTS;
	static	ASL_SCR_INFO	*dm_menuinfo;
	ISA_dev_ptr	*ISA_dev_list;

	ISA_dev_list = get_ISA_list ( &pddv_found );

	/* Create a structure to hold the menu data 			     */
	/* The number of fields to be allocated is equal to the title(1) +   */
        /* number of devices (pddv_found)                                    */
        dm_menuinfo =  (ASL_SCR_INFO *)  calloc (2 + pddv_found,
                                                 sizeof(ASL_SCR_INFO));

	/* get the title */
        dm_menuinfo[index++].text = diag_cat_gets(fdes, LIST_SUPP, LIST_SUPP_T);

	save_buffer = buffer = (char *)malloc(pddv_found * 132);
	/* Get the supported ISA adapters device text now */
	for (i=0; i < pddv_found; i++ ) {
		sprintf(buffer, "%s/%s/%s\t", ISA_dev_list[i].pddv_ptr->class,
				ISA_dev_list[i].pddv_ptr->subclass,
				ISA_dev_list[i].pddv_ptr->type);
		tmp = buffer + strlen(buffer);
        	copy_text( strlen(buffer), tmp, ISA_dev_list[i].text);
        	dm_menuinfo[index++].text = buffer; 
		buffer = buffer + strlen(buffer) + 1;
	}

	dm_menutype.max_index = index;

	status = diag_display(0x802309, fdes, NULL, DIAG_IO,
                         ASL_DIAG_KEYS_ENTER_SC, &dm_menutype, dm_menuinfo);

	if (buffer)
		free(save_buffer);
	free(dm_menuinfo);
	/* Free up all memory allocated */
	for (i=0; i < pddv_found; i++ )
        	free ( ISA_dev_list[i].text); 
	free ( ISA_dev_list );
}

/*  */
/* 
 * NAME: list_defined_isa
 *
 * FUNCTION: This function displays all the defined and configured	
 * ISA adapters found in the system.
 *
 * NOTES:
 *
*/
void list_defined_isa(void)
{
	int	i=0,	status=-1;
	int	index = 0;	/* index into asl structure display buffer */
	nl_catd	pddv_catd;

 	struct  CuDv            *cudv;
        struct  listinfo        obj_cudv;
        char    criteria[256];
	char	*device_status;
	char	*isa_dev_desc;
	char 	*tmp;
	char	*buffer=(char *)NULL, *save_buffer;
	static	ASL_SCR_TYPE	dm_menutype = DM_TYPE_DEFAULTS;
	static	ASL_SCR_INFO	*dm_menuinfo;

        static struct msglist menulist[] = {
                { LIST_DEF, LIST_DEF_T},
                {(int )NULL, (int )NULL}
        };
	/* First search the CuDv database for any ISA adapters */
	sprintf(criteria, "chgstatus != 3 AND PdDvLn LIKE adapter/isa/*" );
        cudv = (struct CuDv *) diag_get_list(CuDv_CLASS, criteria,
                   &obj_cudv, 2, 2);

	/* Create a structure to hold the menu data 			     */
	/* The number of fields to be allocated is equal to the title(1) +   */
        /* number of devices (pddv_found)                                    */
        dm_menuinfo =  (ASL_SCR_INFO *)  calloc (2 + obj_cudv.num,
                                                 sizeof(ASL_SCR_INFO));

	/* get the title */
        dm_menuinfo[index++].text = diag_cat_gets(fdes, LIST_DEF, LIST_DEF_T);

	if ( obj_cudv.num )
		save_buffer = buffer = (char *)malloc(obj_cudv.num * 132);
	/* Get the defined ISA adapters information now */
	for (i=0; i < obj_cudv.num; i++ ) {
		if (cudv[i].status == DEFINED)
			device_status = diag_cat_gets(fdes, LIST_DEF, DEFINE_MSG);
		else if (cudv[i].status == AVAILABLE)
			device_status = diag_cat_gets(fdes, LIST_DEF, AVAIL_MSG);
		else if (cudv[i].status == DIAGNOSE)
			device_status = diag_cat_gets(fdes, LIST_DEF, DIAGN_MSG);
		else 
			device_status = diag_cat_gets(fdes, LIST_DEF, UNDEF_MSG);
		
		pddv_catd = diag_device_catopen(cudv[i].PdDvLn->catalog,0);
		isa_dev_desc = diag_device_gets(pddv_catd, 
				cudv[i].PdDvLn->setno,
				cudv[i].PdDvLn->msgno,
				"ISA description not available");

		sprintf(buffer, "%-12s %-12s %-16s ", cudv[i].name,
				device_status, cudv[i].location);
		tmp = buffer + strlen(buffer);
        	copy_text( strlen(buffer), tmp, isa_dev_desc);
        	dm_menuinfo[index++].text = buffer; 
		buffer = buffer + strlen(buffer) + 1;
	}

	dm_menutype.max_index = index;

	if ( obj_cudv.num )
		status = diag_display(0x802308, fdes, NULL, DIAG_IO,
                         ASL_DIAG_KEYS_ENTER_SC, &dm_menutype, dm_menuinfo);
	else 
		diag_asl_msg( diag_cat_gets(fdes, LIST_DEF, NO_DEF_ISA) );

	if (buffer)
		free(save_buffer);
	if (cudv)
		diag_free_list(cudv, &obj_cudv);
	free(dm_menuinfo);
}

/*  */
/*
 * NAME: delete
 *
 * FUNCTION: Displays Delete Resource Menu
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */

void delete(void)
{
	int	i=0,	status=-1;
	int	index = 0;	/* index into asl structure display buffer */
	nl_catd	pddv_catd;

 	struct  CuDv            *cudv;
        struct  listinfo        obj_cudv;
        char    criteria[256];
	char	*isa_dev_desc;
	char	*device_status;
	char	*tmp;
	char	*buffer=NULL, *save_buffer;
	static	ASL_SCR_TYPE	dm_menutype = DM_TYPE_DEFAULTS;
	static	ASL_SCR_INFO	*dm_menuinfo;

	/* First search the CuDv database for any ISA adapters */
	sprintf(criteria, "chgstatus != 3 AND PdDvLn LIKE adapter/isa/*" );
        cudv = (struct CuDv *) diag_get_list(CuDv_CLASS, criteria,
                   &obj_cudv, 2, 2);

	/* Create a structure to hold the menu data 			     */
	/* The number of fields to be allocated is equal to the title(1) +   */
        /* number of devices (pddv_found) + last line                        */
	if ( obj_cudv.num ) {
        	dm_menuinfo =  (ASL_SCR_INFO *)  calloc (2 + obj_cudv.num,
                                                 sizeof(ASL_SCR_INFO));
		/* get the title */
        	dm_menuinfo[index++].text = diag_cat_gets(fdes, ISA_DEL, ISA_DEL_T);

		save_buffer = buffer = (char *)malloc(obj_cudv.num * 132);

		/* Get the defined ISA adapters information now */
		for (i=0; i < obj_cudv.num; i++ ) {
			if (cudv[i].status == DEFINED)
				device_status = diag_cat_gets(fdes, LIST_DEF, DEFINE_MSG);
			else if (cudv[i].status == AVAILABLE)
				device_status = diag_cat_gets(fdes, LIST_DEF, AVAIL_MSG);
			else if (cudv[i].status == DIAGNOSE)
				device_status = diag_cat_gets(fdes, LIST_DEF, DIAGN_MSG);
			else 
				device_status = diag_cat_gets(fdes, LIST_DEF, UNDEF_MSG);
			pddv_catd = diag_device_catopen(cudv[i].PdDvLn->catalog,0);
			isa_dev_desc = diag_device_gets(pddv_catd, 
					cudv[i].PdDvLn->setno,
					cudv[i].PdDvLn->msgno,
					"ISA description not available");

			sprintf(buffer, "%-12s %-12s %-16s ", cudv[i].name,
					device_status, cudv[i].location);
			tmp = buffer + strlen(buffer);
       		 	copy_text( strlen(buffer), tmp, isa_dev_desc);
       		 	dm_menuinfo[index++].text = buffer; 
			buffer = buffer + strlen(buffer) + 1;
		}

		/* lastly put in the last line */
		dm_menuinfo[index].text = diag_cat_gets (fdes, ISA_SET, ISA_LLINE);

		dm_menutype.max_index = index;
		status = diag_display(0x802306, fdes, NULL, DIAG_IO,
                         ASL_DIAG_LIST_CANCEL_EXIT_SC, &dm_menutype, dm_menuinfo);
  		if ( status == DIAG_ASL_COMMIT )
			remove_ISA_adap( cudv[DIAG_ITEM_SELECTED(dm_menutype)-1].name);

		free(dm_menuinfo);

	} else 
		diag_asl_msg( diag_cat_gets(fdes, LIST_DEF, NO_DEF_ISA) );

	if (buffer)
		free(save_buffer);
}

/*  */
/* 
 *
 * NAME: remove_ISA_adap
 *
 * FUNCTION: This function displays menu 802307. The adapter
 * being removed is invoked by the command:
 *      rmdev -l "device_name" -d
 *
 * NOTES:
 *
*/
void remove_ISA_adap( char *name )
{

	int rc;
	char *outbuf, *errbuf;
	char *opstring, *add_text, *buffer;
	char method_params[128];
	char new_device[NAMESIZE];
	static struct  msglist rem[] = {
                        { ISA_DEL, REM_IN_PROCESS, },
                        { ISA_DEL, REM_ADP, },
                	{(int )NULL, (int )NULL}
	};
        static ASL_SCR_INFO     reminfo[ DIAG_NUM_ENTRIES(rem) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	
        rc = diag_display(NULL, fdes, rem, DIAG_MSGONLY, NULL,
                                        &menutype, reminfo);
	opstring = (char *)malloc(strlen(reminfo[1].text) + strlen(name) + 16);
        sprintf(opstring, reminfo[1].text, name);
        free(reminfo[1].text);
        reminfo[1].text = opstring;
        rc = diag_display(0x802307, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_OUTPUT_LEAVE_SC,
                                        &menutype, reminfo);

        /* odm_invoke( rmdev )  */
        sprintf(method_params, "-l %s -d", name );
        rc = invoke_method( RMDEV, method_params, &outbuf, &errbuf);
	if ( rc == 0 ) {
		add_text = diag_cat_gets(fdes,ISA_DEL,REM_ADP_POP);
		buffer = (char *)malloc(strlen(add_text) + strlen(name) + 16);
		sprintf( buffer, add_text, name);
		diag_asl_msg( buffer );
        }
	else 
		disp_err_ISA( REMOVE_FLAG, name, errbuf );

        free ( outbuf );
        free ( errbuf );
	free ( buffer );
	free ( opstring );

	return;

} 

/* ^L  */
/* NAME: copy_text
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: 0
 */

int copy_text(int string_length,        /* current length of text already in buffer     */
              char *buffer,             /* buffer to copy text into     */
              char *text)               /* text to be copied            */
{

        int     i;
        int     space_count;
        int     char_positions;

        /* determine if length of text string will fit on one line */
        char_positions = LINE_LENGTH - string_length;
        if ( char_positions < strlen(text))  {

                /* dont break the line in the middle of a word */
                if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
                        while ( --char_positions )
                                if( text[char_positions] == ' ')
                                        break;
                if ( char_positions == 0 )
                        char_positions = LINE_LENGTH - string_length;

                for ( i = 0; i < char_positions; i++, buffer++, text++ )
                        *buffer = ( *text == '\n' ) ? ' ' : *text;
                *buffer++ = '\n';
                while ( *text == ' ' )   /* remove any leading blanks */
                        text++;
                space_count = string_length;
                while ( space_count-- )
                        *buffer++ = ' ';
                copy_text( string_length, buffer, text);
        }
        else
                sprintf(buffer, "%s", text);

        return(0);
}

/*  */
/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
        diag_asl_clear_screen();
        genexit(1);
}



/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void genexit(int exitcode)
{
        diag_asl_quit();
        catclose(fdes);
        exit(exitcode);
}

