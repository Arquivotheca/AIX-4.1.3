/* @(#)08        1.5 8/27/91 12:19:02  */
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: header file
 *
 * ORIGINS: IBM
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 */
                                                            
#define   U_KIERR  (     -2 )   /* get code error       */

/* triger key define    */
#define   U_TABKEY     ( 0x012d )  /* TAB key      */
#define   U_BTABKEY    ( 0x012e )  /* Back TAB key */
#define   U_CRKEY      ( 0x000a )  /* C/R          */
#define   U_ENTERKEY   ( 0x012f )  /* Enter key    */
#if defined(EXTCUR)
#define   U_ACTIONKEY  ( KEY_ACT )
#else
#define   U_ACTIONKEY  ( KEY_MAXC1 + 1 )  /* Action key   */
#endif
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

#define   U_2SPACE     ( 0x8140 )  /* DBCS byte code space      */
#define   U_2SPACEH    ( 0x0081 )  /* DBCS space code high byte */
#define   U_2SPACEL    ( 0x0040 )  /* DBCS space code low  byte */

#define  U_2SEMCOLN    ( 0x8146 )  /* DBCS :    */
#define  U_2COLN       ( 0x8147 )  /* DBCS ;    */
#define  U_2QUES       ( 0x8148 )  /* DBCS ?    */
#define  U_2BIKKU      ( 0x8149 )  /* DBCS !    */
#define  U_2SURA       ( 0x815e )  /* DBCS /    */
#define  U_2MLKA       ( 0x8169 )  /* DBCS (    */
#define  U_2MRKA       ( 0x816a )  /* DBCS )    */
#define  U_2LKUCHI     ( 0x8183 )  /* DBCS <    */
#define  U_2RKUCHI     ( 0x8184 )  /* DBCS >    */
#define  U_2PLUS       ( 0x817b )  /* DBCS +    */
#define  U_2MAI        ( 0x817c )  /* DBCS -    */
#define  U_2IQU        ( 0x8181 )  /* DBCS =    */
#define  U_2SPOCHI     ( 0x818c )  /* DBCS '    */
#define  U_2DPOCHI     ( 0x818d )  /* DBCS "    */
#define  U_2PER        ( 0x8193 )  /* DBCS %    */
#define  U_2SHAR       ( 0x8194 )  /* DBCS #    */
#define  U_2AND        ( 0x8195 )  /* DBCS &    */
#define  U_2HOSHI      ( 0x8196 )  /* DBCS *    */
#define  U_2TANKA      ( 0x8197 )  /* DBCS @    */

#define  U_2CHOUON  0x815b
#define  U_2DOLLAR  0x8190

#define   E_2SPACE     ( 0xa1a1 )  /* EUC byte code space      */
#define   E_2SPACEH    ( 0x00a1 )  /* DBCS space code high byte */
#define   E_2SPACEL    ( 0x00a1 )  /* DBCS space code low  byte */

#define  E_2SEMCOLN    ( 0xa1a7 )  /* DBCS :    */
#define  E_2COLN       ( 0xa1a8 )  /* DBCS ;    */
#define  E_2QUES       ( 0xa1a9 )  /* DBCS ?    */
#define  E_2BIKKU      ( 0xa1aa )  /* DBCS !    */
#define  E_2SURA       ( 0xa1bf )  /* DBCS /    */
#define  E_2MLKA       ( 0xa1ca )  /* DBCS (    */
#define  E_2MRKA       ( 0xa1cb )  /* DBCS )    */
#define  E_2LKUCHI     ( 0xa1e3 )  /* DBCS <    */
#define  E_2RKUCHI     ( 0xa1e4 )  /* DBCS >    */
#define  E_2PLUS       ( 0xa1dc )  /* DBCS +    */
#define  E_2MAI        ( 0xa1dd )  /* DBCS -    */
#define  E_2IQU        ( 0xa1e1 )  /* DBCS =    */
#define  E_2SPOCHI     ( 0xa1ec )  /* DBCS '    */
#define  E_2DPOCHI     ( 0xa1ed )  /* DBCS "    */
#define  E_2PER        ( 0xa1f3 )  /* DBCS %    */
#define  E_2SHAR       ( 0xa1f4 )  /* DBCS #    */
#define  E_2AND        ( 0xa1f5 )  /* DBCS &    */
#define  E_2HOSHI      ( 0xa1f6 )  /* DBCS *    */
#define  E_2TANKA      ( 0xa1f7 )  /* DBCS @    */

#define  E_2CHOUON     ( 0xa1bc )  /* DBCS -    */
#define  E_2DOLLAR     ( 0xa1f0 )  /* DBCS $    */

