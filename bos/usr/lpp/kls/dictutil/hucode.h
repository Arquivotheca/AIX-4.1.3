/* @(#)87	1.1  src/bos/usr/lpp/kls/dictutil/hucode.h, cmdkr, bos411, 9428A410j 5/25/92 14:40:29 */
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hucode.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hucode.h
 *
 *  Description:  header file.
 *
 *  History:      5/20/90  Initial Creation.
 *
 ********************************************************************************/
#ifndef _HUCODE_H_
#define _HUCODE_H_

/*------------------------------------------------------------------------------*/
/*              		Define KEYS.					*/
/*------------------------------------------------------------------------------*/
                                                            
#define   U_KIERR  (     -2 )   /* get code error       */

/* triger key define    */
#define   U_TABKEY     ( 0x012d )  /* TAB key      */
#define   U_BTABKEY    ( 0x012e )  /* Back TAB key */
#define   U_CRKEY      ( 0x000a )  /* C/R          */
#define   U_ENTERKEY   ( 0x012f )  /* Enter key    */
#define   U_ACTIONKEY  ( KEY_MAXC1 + 1 )  /* Action key   */
#define   U_RESETKEY   ( KEY_MAXC1 + 2 )  /* Rset key     */

/* function key define  */
#define   U_PF1KEY     ( 0x0181 )  /* PF1 key      */
#define   U_PF2KEY     ( 0x0182 )  /* PF2 key      */
#define   U_PF3KEY     ( 0x0183 )  /* PF3 key      */
#define   U_PF4KEY     ( 0x0184 )  /* PF4 key      */
#define   U_PF5KEY     ( 0x0185 )  /* PF5 key      */
#define   U_PF6KEY     ( 0x0186 )  /* PF6 key      */
#define   U_PF7KEY     ( 0x0187 )  /* PF7 key      */
#define   U_PF8KEY     ( 0x0188 )  /* PF8 key      */
#define   U_PF9KEY     ( 0x0189 )  /* PF9 key      */
#define   U_PF10KEY    ( 0x018a )  /* PF10 key     */
#define   U_PF11KEY    ( 0x018b )  /* PF11 key     */
#define   U_PF12KEY    ( 0x018c )  /* PF12 key     */

/* cursor key define    */
#define   U_C_DOWN     ( 0x0102 )  /* DOWN cursor key      */
#define   U_C_UP       ( 0x0103 )  /* UP cursor key        */
#define   U_C_LEFT     ( 0x0104 )  /* LEFT cursor key      */
#define   U_C_RIGHT    ( 0x0105 )  /* RIGHT cursor key     */

/* other key    */
#define   U_DELKEY     ( 0x010a )  /* DELETE key   */
#define   U_INSERTKEY  ( 0x010b )  /* INSERT key   */
#define   U_HOMEKEY    ( 0x0106 )  /* HOME key     */
#define   U_BSPACEKEY  ( 0x0107 )  /* BackSpace key     */
#define   U_ENDKEY     ( 0x0128 )  /* END key      */

#define   U_1START     ( 0x0020 )  /* 1 byte code start    */
#define   U_1END       ( 0x007e )  /* 1 byte code end      */
#define   U_1STARTK    ( 0x00a1 )  /* 1 byte code start    */
#define   U_1ENDK      ( 0x00df )  /* 1 byte code end      */
#define   U_1SPACE     ( 0x0020 )  /* SBCS byte code space      */
#define   U_2SPACE     ( 0xa1a1 )  /* DBCS byte code space      */
#define   U_2SPACEH    ( 0x00a1 )  /* DBCS space code high byte */
#define   U_2SPACEL    ( 0x00a1 )  /* DBCS space code low  byte */

#define  U_2SEMCOLN    ( 0x8146 )  /* DBCS :    */
#define  U_2COLN       ( 0x8147 )  /* DBCS ;    */
#define  U_2QUES       ( 0x8148 )  /* DBCS ?    */
#define  U_2BIKKU      ( 0x8149 )  /* DBCS !    */
#define  U_2SLASH       ( 0x815e )  /* DBCS /    */
#define  U_2LBRACKET       ( 0x8169 )  /* DBCS (    */
#define  U_2RBRACKET       ( 0x816a )  /* DBCS )    */
#define  U_2LESS     ( 0x8183 )  /* DBCS <    */
#define  U_2GREATER     ( 0x8184 )  /* DBCS >    */
#define  U_2PLUS       ( 0x817b )  /* DBCS +    */
#define  U_2MINUS        ( 0x817c )  /* DBCS -    */
#define  U_2EQUAL        ( 0x8181 )  /* DBCS =    */
#define  U_2SGUOTA     ( 0xfa56 )  /* DBCS '    */
#define  U_2DGUOTA     ( 0xfa57 )  /* DBCS "    */
#define  U_2PER        ( 0x8193 )  /* DBCS %    */
#define  U_2SHAR       ( 0x8194 )  /* DBCS #    */
#define  U_2AND        ( 0x8195 )  /* DBCS &    */
#define  U_2ASTERISK      ( 0x8196 )  /* DBCS *    */
#define  U_2TANKA      ( 0x8197 )  /* DBCS @    */

/*------------------------------------------------------------------------------*/
/*              		Define KEYS.					*/
/*------------------------------------------------------------------------------*/
#endif
