static char sccsid[] = "@(#)13	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_Mifmst.c, libKJI, bos411, 9428A410j 7/23/92 03:22:45";
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
 * MODULE NAME:         _Mifmst
 *
 * DESCRIPTIVE NAME:    Input Field Message Set.
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
 * FUNCTION:            1. Save Input FIeld Info.
 *                      2. Set  Input Field Information.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        884 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         _Mifmst
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _Mifmst( pt,
 *                               msg_type,
 *                               csr_col,csr_row,csr_lft,csr_rgt,
 *                               str_len,str_ptr)
 *
 *  INPUT:              pt      :Pointer to Kanji Control Block.
 *                      msg_type:Message Type.
 *                      csr_col :Input Firld Cusror Column Position.
 *                      csr_row :Input Firld Cusror Row    Position.
 *                      csr_lft :Input Field Left Margin.
 *                      csr_rgt :Input Field Right Margin.
 *                      str_len :Output String Lengt.
 *                      str_ptr :Output DBCS String.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Success of Execution.
 *                      IMTRUSTW:Truncate Output Message Warning.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              _Mdisv  :Display Information Save.
 *                              _Msetch :Input Field Redraw Range Set.
 *                      Standard Library.
 *                              memcpy  :Copy # of Character.
 *                              memset  :Set  # of Specified Character.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              indlen   kjsvpt
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              realcol
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 *   OUTPUT:            Kanji Monitor Control Block(KCB).
 *                              curcol   currow   hlatst  kjsvpt
 *                              string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              curleft  curright msetflg
 *                      Trace Block(TRB).
 *                              NA.
 *                      Kana Kanji Control Block(KKCB).
 *                              NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              DBCSTOCH:DBCS Set Character Pointer.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Tuesday Aug. 23 1988 Satoshi Higuchi
 *                      Changed from pt->lastch = C_COL to
 *                      pt->lastch = str_len.
 *                      This change makes pt->lastch set right value.
 *                      See problem collection sheet P-2 and
 *                      Monitor Improvement Spec. 3.2.5.
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
 *      Input Field Message Set.
 */
int     _Mifmst( pt,
                 msg_type,
                 csr_col,csr_row,csr_lft,csr_rgt,
                 str_len,str_ptr)

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */
uchar   msg_type;       /* Display Message Type.                        */
short   csr_col,csr_row;/* Cousor Column Offset & Cursor Row Position.  */
short   csr_lft,csr_rgt;/* Cusror Column Move Area Left Margin and      */
                        /* Right Margin Offset,which contains each      */
                        /* Side.                                        */
short   str_len;        /* Length of Display String(Per Byte).          */
register uchar *str_ptr;/* Pointer to Display String.                   */

{

        int     _Mdisv();       /* Active Display Information Save.     */
        int     _Msetch();      /* Cusror Position Set.                 */

        char    *memcpy();      /* Memory Copy Operation.               */
        char    *memset();      /* Memory Set Operation.                */

        register KMISA *kjsvpt; /* Pointer to Kanji Monitor Internal    */
                                /* Save Area.                           */
        int     ret_code;       /* Return Value.                        */


        register uchar *str_adr;/* String Operation Work Variable.      */
        register int counter;   /* Loop Work Variable.                  */
        int     max_len;        /* Length of Maxminum Display String.   */

        /*
         *      Debuggin Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA|SNAP_FSB,SNAP_Mifmst,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initialize Work Pointer & Intitial Value.
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to KMISA.                */

        /*
         ****************************************************************
         *      1. Input Parameter Check.
         ****************************************************************
         */
        /*
         *      Parameter Check & Truncate too long String.
         */
        /*
         *      Get Maximum Number of Input Field Avaiable Length.
         */
        max_len  = kjsvpt->realcol - pt->indlen;

        /*
         *      Input FIeld Overflow Check.
         */
        if( str_len > max_len ) {
                str_len  = max_len;
                ret_code = IMTRUSTW;
        } else
                ret_code = IMSUCC;

        /*
         ****************************************************************
         *      2. Display Information Save.
         ****************************************************************
         */
        /*
         *      Save Display Information.
         *      ,if allready Save Display Information
         *      then msetflg(KMISA) sets specified message type
         *      data bits.
         */
        if( !(kjsvpt->msetflg & msg_type ) )
                (void)_Mdisv( pt,msg_type,M_SVIF);

        /*
         ****************************************************************
         *      3. Init Kanji Control Block Input Field Data.
         ****************************************************************
         */
        /*
         *      Display String To KCB(String) Set.
         */
        (void)memcpy( (char *)pt->string,(char *)str_ptr,str_len );

        /*
         *      Set Kanji Space Code to Remain Display Area
         */
        counter = ( max_len - str_len ) / C_DBCS;
        str_adr = &pt->string[ str_len ];

        while( counter--> 0 ) {
                DBCSTOCH(str_adr,C_SPACE);
                str_adr += C_DBCS;
        };

        /*
         *      Display Attribute(Normal) Set.
         */
        (void)memset( (char *)pt->hlatst,K_HLAT0,max_len);


        /*
         *      Display Area Cursor Position & Last Character Position Set.
         */
        pt->curcol = csr_col;   /* Input Field New Cursor Col Position. */
        pt->currow = csr_row;   /* Input Field New Cursor Row Position. */

        /*
         *      Cursor is Visible ?
         */
        if( (csr_col == C_FAUL) || (csr_row)==C_FAUL ) {
                /*
                 *      Field is Nothing.
                 */
/*----------------------------------------------------------------------*/
/*      #(B) Deleted by S,Higuchi on Aug. 23 1988                       */
/*              pt->lastch = C_COL;                                     */
/*----------------------------------------------------------------------*/
		/* #(B) added by S,Higuchi on Aug. 23 1988              */
		pt->lastch = str_len;

        } else {
                /*
                 *      Last Character Position is Cursor Position.
                 */
                pt->lastch =
                          ( csr_row + C_OFFSET ) * kjsvpt->realcol
                        +   csr_col;
        };

        /*
         *      Display String Refresh Range Set.
         */
        (void)_Msetch( pt,C_COL,max_len );

        /*
         ****************************************************************
         *      4. Allocate Auxiliary Area Resource &
         *      Cursor Left,Right Margin Set.
         ****************************************************************
         */
        /*
         *      Now Dsplay message type is Saved.
         */
        kjsvpt->msetflg |= msg_type;

        /*
         *      Cursor Left,Right Margin Set.
         */
        kjsvpt->curleft = csr_lft;      /* Input Field Left Margin Set. */
        kjsvpt->curright= csr_rgt;      /* Input Field Right Margin Set.*/

        /*
         *      Debuggin Output.
         */
        snap3(SNAP_KCB|SNAP_KMISA|SNAP_FSB,SNAP_Mifmst,"End");

        /*
         ****************************************************************
         *      5. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

