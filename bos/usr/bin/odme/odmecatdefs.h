/* @(#)04	1.8  src/bos/usr/bin/odme/odmecatdefs.h, cmdodm, bos411, 9428A410j 4/5/93 13:01:13 */
/*
 * COMPONENT_NAME: (ODME) ODMECATDEFS.H - defines for message catalogue
 *
 * FUNCTIONS: 
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
/**************************************************************************/
/*                                                                        */
/*  FILENAME    : odmecatdefs.h                                           */
/*                                                                        */
/*  DESCRIPTION : This file contains all constants used for message       */
/*                retrieval in the ODME.                                  */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
nl_catd		catd;
#define		MSGS(set,num,str) catgets(catd, set, num, str)
/*----------------------------------------------------------*/
/* Headings for Object Data Manager Editor screens          */
/*----------------------------------------------------------*/
#define EDITOR_NAME   		5
#define CREATE_NAME   		5
#define CRITERIA_NAME 		5
#define DISPLAY_NAME  		5
#define GRAPH_NAME    		10
#define DISP_MY_DESC_NAME 	25
#define CONT_NAME      		10
#define NEW_NAME	 	5

/*----------------------------------------------------------*/
/* Defines for message set numbers                          */
/*----------------------------------------------------------*/
#define MN			1	/* odme.c         */
#define CT			2	/* odmecontrol.c  */
#define CR			3	/* odmecreate.c   */
#define SC			4	/* odmecriteria.c */
#define NW			5	/* odmenewobj.c   */
#define OD			6	/* odmeobjdisp.c  */
#define OS			7	/* odmeobjstore.c */
#define RG			8	/* odmerelgraph.c */
#define WD			9	/* odmewindow.c   */
#define KS			10	/* Keys           */
#define ER			12	/* Errors         */

/*----------------------------------------------------------*/
/* Error codes for function prerr () to print               */
/*----------------------------------------------------------*/
#define OBJECT_EXISTS		2
#define DELETE_FAILED		4
#define CT_OPEN_FAILED		6
#define CT_OPENED_RO		8
#define CR_DMALLOC_FAILED	10
#define CR_CREATE_FAILED	12		
#define PRINT_SCREEN_ERROR	14
#define CR_INPUT_ERROR		16
#define SC_SEARCH_STRING	18
#define NW_DMALLOC_FAILED	20
#define NW_CREATE_FAILED	22
#define NW_ADD_FAILED		24
#define OD_NO_OBJECTS		26
#define OD_MALLOC_FAILED	28
#define OD_BAD_RETURN		30
#define OD_NOT_A_LINK		32
#define OD_EXPAND_OPEN		34
#define OD_CLOSE_FAILED		36
#define OD_DELETE_FAILED	38
#define OD_ADD_FAILED		40
#define OD_CHANGE_FAILED	42
#define OD_READ_ONLY		44
#define OD_DELETE_MALLOC	46
#define OS_NO_DESC		47
#define OS_UNKNOWN_DESC		48
#define OS_CRITERIA		50
#define OS_SYNTAX		52
#define OS_ODM_ERROR		54
#define OS_STORAGE_MALLOC	56
#define OS_DESC_MALLOC		58
#define OS_INVALID_TYPE		60
#define OS_OPEN_FAILED		62
#define OS_DELETE_FAILED	64
#define RG_MALLOC_FAILED	66
#define RG_PRINT_SCREEN_ERROR	68
#define WD_MALLOC_FAILED	70



/*----------------------------------------------------------------------------*/
/*  Bottom function key display strings                                       */
/*----------------------------------------------------------------------------*/
#define CRITERIA_KEYS	26
#define DISPLAY_KEY_1 	22
#define DISPLAY_KEY_2 	24
#define CREATE_KEYS   	18
#define PANEL_KEYS    	15
#define MAIN_KEYS     	10
#define DISP_KEY 	20
#define STD_KEYS  	16 

/*-----ODME.C (MAIN)-----*/

#define menu1		15
#define menu2		20
#define menu3		25
#define menu4		30
#define menu5		35
#define menu6		40
#define menu7		45
#define menu8		50

#define main_help	55
#define delete_warning	60
#define delete_ok	65

/* object_name_edit */

#define objclpar	70
#define objclname	75
#define node_msg	80
#define path_msg	85
#define openro		90
#define openrw		95

/* set_master_path */

#define sm_path		100
#define master_path	105
#define disallowed_class 110

/*-----ODMECONTROL.C-----*/

/* control */

#define cfunc1		1
#define cfunc2		2
#define cfunc3		3
#define cfunc4		4
#define cfunc5		5

#define ct_question  11
#define screen_msg	20
#define ct_warning   22
#define control_help	25
#define ct_delete_ok	30

/* get_func */

#define functions	35

/*-----ODMECREATE.C-----*/

/* create_class */ 

#define cr_objclass	10
#define create_help	15
#define create_msg	20

/* permissions */

#define permissions_msg	25

#define owner_w		30
#define owner_rw	32
#define owner_n		34
#define owner_r		36
#define group_w		38
#define group_rw	40
#define group_n		42
#define group_r		44
#define other_w		46
#define other_rw	48
#define other_n		50
#define other_r		52

/* edit_descriptor */

#define cr_perm_obj	54
#define cr_desc_edit	55
#define cr_obj_desc_edit 56
#define cr_ln_desc_edit 57
#define cr_ch_desc_edit 58
#define cr_bin_desc_edit 59
#define cr_name		60
#define cr_type		65
#define cr_iterator	70

#define type1		72
#define type2		74
#define typev		75
#define type3		76
#define type4		78
#define type5		80
#define type6		82
#define type7		84
#define type8		86
#define type9		88
#define type10		90

/* edit_by_type */

#define cr_class_msg	95
#define link_msg1	97
#define link_msg2	100
#define keymsg1		105
#define keymsg2		110
#define fieldsize	115

/* error messages */
#define cr_errmsg1	15	
#define cr_errmsg2	17	

/*-----ODMECRITERIA.C-----*/

/* search_criteria */

#define sc_objclass	10
#define criteria_help	20

/*-----ODMENEWOBJ.C-----*/

/* make_new_obj */

#define new_help 	10
#define nw_question	15
#define nw_create_ok	20

/*-----ODMEOBJDISP.C-----*/

/* object_display */

#define od_objclass	10
#define loading		15
#define display_help	20
#define commit		25

/*-----ODMEOBJSTORE.C-----*/
#define os_delete_ok	2

/*-----ODMERELGRAPH.C-----*/

/* rel_graph */

#define gfunc1		2
#define gfunc2		4
#define gfunc3		6
#define gfunc4		8

#define rg_screen	15
#define relgraph_help	20

/* disp_my_desc */

#define dd_class	30
#define dd_screen	35
#define dispdesc_help	40
#define dd_heading	45
   
/*-----ODMEWINDOW.C-----*/

/* question */

#define yes_or_no	5

/* signal_error */

#define signal_msg	10

/* prerr */

#define pr_quit		15
#define pr_keys		20

/* odme_prscr */

#define pr_screen_hdng  25

/* setup_curses */

#define no_term		30

