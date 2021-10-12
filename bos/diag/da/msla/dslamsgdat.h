/* @(#)23	1.3  src/bos/diag/da/msla/dslamsgdat.h, damsla, bos411, 9428A410j 12/10/92 09:14:24 */
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#define   MENUNO_MSLA_1      0x858001
#define   MENUNO_MSLA_2      0x858002
#define   MENUNO_MSLA_3      0x858003
#define   MENUNO_MSLA_4      0x858004
#define   MENUNO_MSLA_5      0x858005
#define   MENUNO_MSLA_6      0x858006
#define   MENUNO_MSLA_7      0x858007
#define   MENUNO_MSLA_8      0x858008

   /* advanced mode error messages.( depends on the definitions in dsla.msg*/
struct  msglist err_reg[]= {
                        NULL,NULL
};

struct  msglist err_ram[]= {
                        NULL,NULL
};
struct  msglist err_pos[]= {
                        NULL,NULL
};
struct  msglist err_hw[]=
{
                        NULL,NULL
};
struct  msglist err_wr[]=
{
                        NULL,NULL
};
struct  msglist err_int[]=
{
                        NULL,NULL
};
struct  msglist err_dma[]=
{
                        NULL,NULL
};

/**************************************************************************/
/*                                                                        */
/*      da_title is an array of structure holds the message set           */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_msg to tell the user        */
/*      MSLA device testing is about to start.                            */ 
/*                                                                        */
/**************************************************************************/
struct msglist da_title[] = {
		{ MSL_TITLES, MSL_NOTIFY },
		{ MSL_TITLES, MSL_SBY },
		{ NULL, NULL }
};
