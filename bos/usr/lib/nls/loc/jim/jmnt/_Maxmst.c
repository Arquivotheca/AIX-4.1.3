static char sccsid[] = "@(#)85	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Maxmst.c, libKJI, bos411, 9428A410j 7/23/92 03:20:45";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _Maxmst
 *
 * DESCRIPTIVE NAME:    Auxiliary Area No.1 Message Data And
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            1. Save Cursor Information.
 *                      2. Auxiliary Area Information Set.
 *                      3. Auxiliary Usage Information Set.
 *
 * NOTES:               This Routine is Auxiliary Area Message Set
 *                      Rotutine,But Japanese DBCS Monitor is
 *                      need No.1 Only,anyway Auxiliary Area No
 *                      Parameter Any Value is Ignore,and Set Allways
 *                      Auxiliary Area No.1
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        804 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Maxmst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Maxmst( pt,msg_type,area_num,src_col,src_row,
 *                               csr_col,csr_row,csr_left,csr_rgt,
 *                               str_len,str_ptr)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      msg_type:Output Message Type.
 *                      area_num:Auxiliary Area Number(Available No.1 Only).
 *                      scr_col :Auxiliary Area Number of Column Position.
 *                      scr_row :Auxiliary Area Number of Row   Lines.
 *                      csr_col :Auxiliary Area Cursor Column Position.
 *                      csr_row :Auxiliary Area Cursor Row Position.
 *                      csr_lft :Cursor Column Move Left Margin.
 *                      csr_rgt :Cursor Column Move Right Margin.
 *                      str_len :Auxiliary Area Set Initialize String Length
 *                      str_ptr :Auxiliary Area Initial DBCS String.
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *                      IMTRUSTW:Message Truncat Warning.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              _Mdisv  :Display Information Save.
 *                      Standard Library.
 *                              memcpy  :Copy # of Character.
 *                              memset  :Set # of Specfied Character.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              NA.
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              NA.
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              aux1     ax1col   ax1row  axuse1
 *                              chlna1   chpsa1   cura1c  cura1r
 *                              pt       hlata1   kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              auxflg1  curleft  curright msetflg
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              DBCSTOCH:DBCS String Set Char Pointer.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standar I/O Header.                          */
#include <memory.h>     /* Memory Operation.                            */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Define File.                           */
#include "kcb.h"        /* Kanji Monitor Control Block.                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Auxiliary Area Message Data Set.
 */
int     _Maxmst( pt,
                 msg_type,area_num,
                 scr_col,scr_row,csr_col,csr_row,csr_lft,csr_rgt,
                 str_len,str_ptr)

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
uchar   msg_type;       /* Display Message Type.                        */
short   area_num;       /* Auxiliary Area Number.                       */
short   scr_col,scr_row;/* Display Column Width & Row Height.           */
short   csr_col,csr_row;/* Cousor Column Offset & Cursor Row Position.  */
short   csr_lft,csr_rgt;/* Cusror Column Move Area Left Margin and      */
                        /* Right Margin Offset,which contains each      */
                        /* Side.                                        */
short   str_len;        /* Length of Display String(Per Byte).          */
register uchar *str_ptr;/* Pointer to Display String.                   */

{

        int     _Mdisv();       /* Active Display Information Save.     */

        char    *memcpy();      /* Memory Copy Operation.               */
        char    *memset();      /* Memory Set Operation.                */

        register KMISA *kjsvpt; /* Pointer to Kanji Monitor Internal    */
                                /* Save Area.                           */
        int     ret_code;       /* Return Value.                        */


        register uchar *str_adr;/* String Operation Work Variable.      */
        register int counter;   /* Loop Work Variable.                  */
        int     max_len;        /* Length of Maxminum Display String.   */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Maxmst,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initialize Work Pointer & Intitial Value.
         */
        kjsvpt  = pt->kjsvpt;

        /*
         ****************************************************************
         *      1. Parameter Analize,Currect.
         ****************************************************************
         */
        /*
         *      Parameter Check & Truncate too long String.
         */
        /*
         *      Maximum Screen Position Get.
         */
        max_len  = scr_col * scr_row;

        /*
         *      Screen Position Overflow Check.
         */
        if( str_len > max_len ) {
                str_len  = max_len;
                ret_code = IMTRUSTW;
        } else
                ret_code = IMSUCC;

        /*
         ****************************************************************
         *      2. Save Input Field Information.
         ****************************************************************
         */
        /*
         *      Save Display Information.
         *      ,if allready Save Display Information
         *      then msetflg(KMISA) sets specified message type
         *      data bits.
         */
        if( !(kjsvpt->msetflg & msg_type ) )
                (void)_Mdisv( pt,msg_type,M_NSVIF);

        /*
         ****************************************************************
         *      3. Kanji Control Block Information Set Up.
         ****************************************************************
         */
        /*
         *      Display String To KCB(aux1) Set.
         */
        (void)memcpy( (char *)pt->aux1,(char *)str_ptr,str_len );

        /*
         *      Set Kanji Space Code to Remain Display Area
         */
        /*
         *      If Initial String Shorter Than Auxiliary Area Size,
         *      Remain Space Padding DBCS Space Code.
         */
        counter = ( max_len - str_len ) / C_DBCS;
        str_adr = &pt->aux1[ str_len ];

        while( counter--> 0 ) {
                DBCSTOCH(str_adr,C_SPACE);
                str_adr += C_DBCS;
        };

        /*
         *      Display Attribute(Normal) Set.
         */
        (void)memset((char *)pt->hlata1,K_HLAT0,max_len);

        /*
         *      Display Area Height/Width ,Cursor Position Set.
         */
        pt->ax1col = scr_col;   /* Auxiliary Area No.1 Column Number Set*/
        pt->ax1row = scr_row;   /* Auxiliary Area No.1 Row    Number Set*/

        pt->cura1c = csr_col;   /* Auxiliary Area No.1 Cursor Column Set*/
        pt->cura1r = csr_row;   /* Auxiliary Area No.1 Cursor Row    Set*/

        /*
         *      Display String Refresh Range Set.
         */
        pt->chpsa1 = C_COL;     /* Auxiliary Area No.1 Display Start Pos*/
        pt->chlna1 = max_len;   /* Auxiliary Area No.1 Display Length.  */

        /*
         *      Auxiliary Area Use Flag & Display Message Type
         *      Set.
         */
        pt->axuse1      = K_AINUSE;     /* Auxiliary Area No.1 Use      */
                                        /* Indicator Set.               */
        kjsvpt->auxflg1 = msg_type;     /* Auxiliary Area No.1 Use      */
                                        /* Message ID Save.             */
        /*
         ****************************************************************
         *      4. Kanji Monitor Internal Save Area,Message Save Info-
         *         mation Set.
         ****************************************************************
         */
        /*
         *      Now Dsplay message type is Saved.
         */
        kjsvpt->msetflg |= msg_type;    /* Active Message ID Check.     */

        /*
         *      Cusror Movement Range Set.
         */
        kjsvpt->curleft = csr_lft;      /* Field Cursor Move Left Limit */
                                        /* Set.                         */
        kjsvpt->curright= csr_rgt;      /* Field Cursor Move Right Limit*/
                                        /* Set.                         */

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA,SNAP_Maxmst,"End");

        /*
         ****************************************************************
         *      5. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

