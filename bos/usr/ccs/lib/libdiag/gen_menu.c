static char sccsid[] = "@(#)39  1.4.1.5  src/bos/usr/ccs/lib/libdiag/gen_menu.c, libdiag, bos41B, bai4 12/23/94 10:30:19";
/*
 * COMPONENT_NAME: (LIBDIAG) DIAGNOSTIC LIBRARY
 *
 * FUNCTIONS:   gen_ds_menu
 *              generate_All
 *              generate_stack
 *              get_next_child
 *              min_child
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

#include <cf.h>
#include <sys/cfgdb.h>
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include "diag/class_def.h"

/* EXTERNAL VARIABLES */
extern struct CuDv **Parent_CuDv;

/* LOCAL VARIABLES */

#define CHGSTATUS( VAR1, VAR2 ) ( All[/**/VAR1]->T_CuDv->chgstatus == /**/VAR2 )
#define SHOULD_BE_IN_DSM( VAR1 ) ( All[/**/VAR1]->T_PDiagDev->Menu & DIAG_DS )
#define SHOULD_BE_IN_DTL( VAR1 ) ( All[/**/VAR1]->T_PDiagDev->Menu & DIAG_DTL )
#define CHILD_NOT_IN_DTL( VAR1 ) ( All[/**/VAR1]->T_PDiagDev->Menu & DIAG_SA_CON_CHILD )
#define DSM_CONDITIONAL( VAR1 ) ( All[/**/VAR1]->T_PDiagDev->Menu & DIAG_CON )
#define NOT_DELETED_DTL( VAR1 ) ( All[/**/VAR1]->T_CDiagDev->RtMenu == RTMENU_DEF )
#define SUPPORTS_CONCURRENT( VAR1 ) ( All[/**/VAR1]->T_PDiagDev->Conc != DIAG_NO )
#define TRUE_RESOURCE( VAR1 )  (                                               \
                        ( !All[/**/VAR1]->T_CDiagDev->Port ) ||           \
                        ( All[/**/VAR1]->T_CDiagDev->Port == NOT_IN_USE ) \
                       )

int                     All_count;      /* count of devices in All    */
diag_dev_info_ptr_t     *All;           /* all supported devices      */

/* CALLED FUNCTIONS */

diag_dev_info_ptr_t     *generate_All(); /* generate stacked devices   */
char			*substrg(int, char *);

/*  */
/* NAME: gen_ds_menu
 *
 * FUNCTION: Generate a list of devices matching a certain criteria.
 *
 * NOTES:
 *      This function will generate a list of devices to be used in generating a
 *      Diagnostic Selection Menu, New Device Menu, or Missing Device Menu.
 *      Devices will be listed in the following order:
 *              System Checkout
 *              Processor Complex
 *              Memory
 *              RIOS Serial Link Adapters
 *              NIO Card Adapters
 *              Adapter in slot 1 and all devices attached to it
 *              Adapter in slot 2 and all devices attached to it
 *              Etc....
 *      Space is allocated in this function for the list.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      diag_dev_info_ptr_t * : pointer to a structure array
 *
 */

diag_dev_info_ptr_t *
gen_ds_menu( num_cudv_devices, Top, num_All, devices_in_list, ex_environment)
int             num_cudv_devices;        /* number of supported devices  */
diag_dev_info_t         *Top;            /* all customized devices       */
int             *num_All;                    /* Number of devices in All */
int             *devices_in_list;         /* number of devices in list   */
int             ex_environment;           /* execution environment       */
{

        int                     i, j, index;
        int                     menucount;
        diag_dev_info_ptr_t     *Menu;         /* array with list of devices */
        diag_dev_info_ptr_t     *tmp;          /* array containing tmp list  */
        char                    *xptr;         /* get environment variable */
        char                    xchar[2];      /* Convert ptr to string */
        int                     xdiag;         /* running in an X-window? */

        /*
           if this is running in concurrent mode, check the environment
           variable X_DIAG to see if this is running in an X-window.
        */

        if (ex_environment == EXENV_CONC) {
                if((xptr = (char *)getenv("X_DIAG")) == (char *)NULL)
                        xdiag = DIAG_FALSE;
                else {
                        strncpy(xchar,xptr,1);
                        if (xchar[0] == '0')
                                xdiag = DIAG_FALSE;
                        else
                                xdiag = DIAG_TRUE;
                }
        }


        /*
          set up a stacked and sorted list of all devices starting
          with the SysPlanar0
        */
        if ( !All )  {
                if ( (All = generate_All(num_cudv_devices, Top, num_All)) ==
                                                (diag_dev_info_ptr_t *)-1 )
                        return( (diag_dev_info_ptr_t *) -1 );
        }

        /* Search the sorted and stacked array All for devices to be included
         * in the Diagnostic Selection Menu. Check the
         * descriptor field 'Menu' in the PreDefined Object Class and the
         * descriptor field 'RtMenu' in the CustDiag Object Class.
         */
        Menu = (diag_dev_info_ptr_t *) calloc(All_count,sizeof(Menu[0]));
        for ( i=0,index = -1, menucount=0 ; i<All_count; i++, index = -1 )  {
                if ( All[i]->T_PDiagDev  &&
                         ( DSM_CONDITIONAL (i) ) && ( NOT_DELETED_DTL(i) )
                     && (( CHGSTATUS (i,NEW) ) || ( CHGSTATUS (i,SAME) ) ||
                         ( CHGSTATUS (i,DONTCARE) ))) {
                        index = i;
                        for ( j=i; j<All_count; j++ ) {
                                if ( !strcmp(All[j]->T_CuDv->name,
                                             All[j+1]->T_CuDv->parent)) {
                                        if ( All[j+1]->T_PDiagDev  &&
                                                ( SHOULD_BE_IN_DSM (j+1) ) &&
                                                ( NOT_DELETED_DTL (j+1)  ) &&
                                                (( CHGSTATUS (j+1,NEW)) ||
                                                ( CHGSTATUS (j+1,SAME) ) ||
                                                ( CHGSTATUS (j+1,DONTCARE) ))) {
                                                index = i = j+1;
                                                break;
                                        }
                                }
                                else
                                        break;
                        }
                }
		else if ( All[i]->T_PDiagDev  &&
                          ( CHILD_NOT_IN_DTL (i) ) && ( NOT_DELETED_DTL(i) )
                      && (( CHGSTATUS (i,NEW) ) || ( CHGSTATUS (i,SAME) ) ||
                          ( CHGSTATUS (i,DONTCARE) ))) {
                         for ( j=i, i++; i<All_count; i++ ) {
                                 if ( strcmp(All[j]->T_CuDv->name,
                                              All[i]->T_CuDv->parent)) {
                                               --i;
                                                 break;
                                 }
			 }
		}
                else if ( All[i]->T_PDiagDev  &&
                         ( SHOULD_BE_IN_DSM (i) ) && ( NOT_DELETED_DTL(i) )
                     && (( CHGSTATUS (i,NEW) ) || ( CHGSTATUS (i,SAME) ) ||
                         ( CHGSTATUS (i,DONTCARE) )))
                        index = i;
                if ( index != -1 )
                        if (ex_environment != EXENV_CONC)
                                Menu[menucount++] = All[i];
                        else {
                                /* Only those devices which can be successfully */
                                /* tested in concurrent mode (excluding HFT     */
                                /* devices and adapters if running concurrent   */
                                /* mode under X-Windows) should be put into     */
                                /* selection list.                              */
                                if ((SUPPORTS_CONCURRENT (i) ) &&
                                   (((All[i]->T_PDiagDev->SupTests & SUPTESTS_HFT) == 0)||
                                     (xdiag == DIAG_FALSE)))
                                               Menu[menucount++] = All[i];
                        }
        }
        *devices_in_list = menucount;
        return(Menu);
}
/*  */
/*
 * NAME: generate_All
 *
 * FUNCTION: Searches recursively through a list of devices for parent
 *              children relationships.
 *
 * DATA STRUCTURES: If device is found in list, the flags.found_in_list
 *                      bit is set.
 *
 * RETURNS:
 *     >0 : address of tmp array holding stacked devices
 *     -1 : if base object not found
 */

diag_dev_info_ptr_t *
generate_All( num_devices, Top, num_All )
int                     num_devices;     /* number of devices in Top     */
diag_dev_info_t         *Top;            /* all customized devices       */
int                     *num_All;        /* number of devices in Top     */
{
        int                     i,j,k;
        diag_dev_info_ptr_t     *tmp;       /* temporary holding array  */
        char                    *base_name; /* Start with SystemObj0    */
	char			*ptype, *pclass, *psclass;
	struct	listinfo	p_info;
	struct	PDiagAtt	*Base_PDiagAtt_obj = (struct PDiagAtt *)NULL;
	int			num_PDiagAtt;


	Base_PDiagAtt_obj = get_PDiagAtt_list(PDiagAtt_CLASS,
				"attribute = diag_root_obj", &p_info,
				MAX_EXPECT, 1);
	num_PDiagAtt = p_info.num;

        All_count = 0;

	/* First build list of devices for BASE_OBJ	*/

        tmp = (diag_dev_info_ptr_t *) calloc(num_devices,sizeof(tmp[0]));
        for ( i=0; i < num_devices; i++ ) {
                if (!strcmp( Top[i].T_CuDv->name, BASE_OBJ )) {
                        generate_stack( tmp, num_devices, Top, i );
                        break;
                }
        }

        if(Base_PDiagAtt_obj != (struct PDiagAtt *)NULL)
        {
                for (k=0; k< num_devices; k++){
                        ptype = substrg(PTYPE, Top[k].T_CuDv->PdDvLn_Lvalue);
			pclass = substrg(PCLASS, Top[k].T_CuDv->PdDvLn_Lvalue);
                        psclass = substrg(PSCLASS, Top[k].T_CuDv->PdDvLn_Lvalue);
                        for (j = 0; j < num_PDiagAtt; j++)
                                if ( ( !strncmp(Base_PDiagAtt_obj[j].DType,
                                        ptype, TYPESIZE) ) &&
				     ( !strncmp(Base_PDiagAtt_obj[j].DClass,
					pclass, CLASSIZE) ) &&
                                     ( !strncmp(Base_PDiagAtt_obj[j].DSClass,
                                        psclass, CLASSIZE) ) )
                                  {
                                     generate_stack( tmp, num_devices, Top, k);
                                     break;
                                  }
                }
        }
        /* Now free the PDiagAtt list */

        if(Base_PDiagAtt_obj)
                odm_free_list(Base_PDiagAtt_obj, &p_info);

        *num_All = All_count;

        /* now reset all the flags for the next time around */
        if ( i != num_devices ) {
                for ( i=0; i < *num_All; i++ )
                        tmp[i]->flags.found_in_list = DIAG_FALSE;
                return ( tmp );
        }
        else
                return ( (diag_dev_info_ptr_t *) -1 );
}

/*  */
/*
 * NAME: generate_stack
 *
 * FUNCTION: Searches recursively through a list of devices for parent
 *              children relationships.
 *
 * DATA STRUCTURES: If device is found in list, the flags.found_in_list
 *                      bit is set.
 *
 * RETURNS: NONE
 */

generate_stack(tmp, num_devices, Top, parent)
diag_dev_info_ptr_t     *tmp;           /* array containing tmp list    */
int                     num_devices;     /* number of devices in Top     */
diag_dev_info_t         *Top;           /* all customized devices       */
int                     parent;         /* parent name to search for    */
{
        int                 i, child, more;

        do {
                more = get_next_child(num_devices, Top, parent, &child);
                if (child != -1){
                        Top[child].flags.found_in_list = DIAG_TRUE;
                        tmp[All_count++] = &Top[child];
                        generate_stack(tmp, num_devices, Top, child);
                }
                if (more)
                        generate_stack(tmp, num_devices, Top, parent);
        } while (more);

}

/*
 * NAME: get_next_child
 *
 * FUNCTION: Searches for child
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      an index into Top that represents the minimum child
 */

get_next_child(num_devices, Top, parent, min)
int                     num_devices;     /* number of devices in Top     */
diag_dev_info_t         *Top;
int                     parent;
int                     *min;
{
        int             i, more = 0;

        *min = -1;
        for ( i=0; i < num_devices; i++ ) {
                if ( (Top[i].flags.found_in_list == DIAG_FALSE)  &&
                     (Top[parent].T_CuDv == Parent_CuDv[i]) ) {
                        more = (*min != -1);
                        if (*min == -1)
                                *min = i;
                        else if (min_child(Top[i].T_CuDv, Top[i].T_Pdv->fru,
                                    Top[*min].T_CuDv,Top[*min].T_Pdv->fru))
                                *min = i;
                }
        }
        return( more );
}

/*
 * NAME: min_child
 *
 * FUNCTION:
 *
 *
 * DATA STRUCTURES:
 *
 *
 * RETURNS:  0: curr >= min
 *           1: curr <  min
 */

min_child( curr, cfru, min ,mfru)
struct CuDv *curr;
short cfru;
struct CuDv *min;
short mfru;
{

#define alpha "ABCDEFGH"

        int  lc, lm;

        if ( !strlen(curr->location) )
                return(1);

        if ( !strlen(min->location) )
                return(0);

        if ( ((char*)strrchr(curr->location,'-') == (char*)NULL)  &&
             ((char*)strrchr(min->location, '-') == (char*)NULL) )
                if ( strcmp(curr->location, min->location) > 0 )
                        return(0);

        if ( (lc = (int)((char*)strrchr(curr->location,'-')-curr->location)) <
             (lm = (int)((char*)strrchr(min->location, '-')-min->location)) )
                return(1);

        if ( (cfru != PARENT_FRU) &&
             ( mfru == PARENT_FRU) )
                return(0);

        if ( (cfru == PARENT_FRU) &&
             ( mfru != PARENT_FRU) )
                return(1);

        if ( (strpbrk(curr->location,alpha) != NULL)  &&
              (strpbrk( min->location,alpha) == NULL))
                        return(1);

        else if ( (strpbrk(curr->location,alpha) == NULL)  &&
              (strpbrk( min->location,alpha) != NULL) )
                        return(0);

        if ( ( lc == lm ) && (strcmp(&curr->location[0],
                                     &min->location[0]) < 0 ) )
                return(1);

        return(0);
}
