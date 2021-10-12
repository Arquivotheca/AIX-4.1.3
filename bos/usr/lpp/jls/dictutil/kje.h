/* @(#)75	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kje.h, cmdKJI, bos411, 9428A410j 7/23/92 00:57:10 */
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kje.h
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 */

#define U_LANG   ("LANG")       /* Langage Type         (Getenv Parm.)  */
#define U_MSGLNG ("Langage Type Not Found\n")
#if defined(CNVEVT)
#define U_MSGCLN ("Conversion Code Not Found\n")
#define U_MSGBCN ("Base(SJIS) Code Not Found\n")
#define U_MSGCNV ("Conversion File Open Error\n")

#define U_SJISCD ("IBM-932")    /* Shift Jis Code ( 1978)               */
#define U_SJISLG ("Ja_JP")      /* Shift Jis Code value for LANG        */
#define U_EUCCD  ("IBM-eucJP")  /* EUC Code (1983)                      */
#define U_EUCLG  ("ja_JP")      /* EUC Code value for LANG              */
#define U_SJIS   ( 1 )          /* Shift JIS Code Flag                  */
#define U_EUC    ( 2 )          /* EUC Code Flag                        */
#define U_DEF    ( 3 )          /* Default Code Flag                    */
#endif

#define U_PARM   ( 3 )          /* argc,argv Parameter Array Area       */
#define U_MEMSIZ ( 256 )        /* Data Array Area                      */
#define U_TERM   ("TERM")       /* Terminal Name        (Getenv Parm.)  */
#define U_HOME   ("HOME")       /* Home Directory       (Getenv Parm.)  */
#define U_PWD    ("PWD")        /* Current Directory    (Getenv Parm.)  */
#define U_JIMUSR ("JIMUSRDICT") /* User Dictionary      (Getenv Parm.)  */
#define U_PRTDEV ("PRINTER")    /* Printer Device       (Getenv Parm.)  */
#define U_TRJAIX ("jaixterm")   /* Terminal Type                        */
#define U_TRAIX  ("aixterm")    /* Terminal Type                        */
#define U_TRAIXM ("aixterm-m")  /* Terminal Type                        */
#define U_CURDIC (".usrdict")   /* Current Dictionary                   */
#define U_STDUDC ("/usr/lpp/jls/dict/usrdict")  /* User Dictionary      */
#define U_MSGDIR ("/usr/lpp/msg")       /* Message Facility Directory   */
#define U_CATNM  ("kjdict.cat") /* Message Facility File Name           */
#define U_CURDIR ("./")         /* Current Directory                    */
#define U_CURENT (".")          /* Current Position                     */
#define U_MSKAWT ( 02 )         /* Write Parmission                     */
#define U_MSKARD ( 04 )         /* Read Parmission                      */
#define U_FLENMX ( 10 )         /* File Name Maxmamn Byte               */
#define U_MINBYT ( 59392 )      /* User Dictionary Lower Limit          */
#define U_MAXBYT ( 281120 )     /* User Dictionary Upper Limit          */
#define U_URDC1K ( 1024 )       /* 1K Bundary                           */
#define U_DEFDEF ( "0" )        /* kudichnd Default Paramater Defintion */

#define U_PRMJSY ("-d")         /* User Dictionary Parameter Data       */
#define U_PRMYMI ("-y")         /* Yomi Parameter Data                  */
#define U_PRMGKU ("-g")         /* Goku Parameter Data                  */
#define U_PRMPDV ("-P")         /* Printer Device Parameter             */

#define U_MSBON  ( 0x80 )       /* Most Signe Bit ON                    */
#define U_SDBHLL ( 0x81 )       /* S-jis Duble Byte Code Range (L-L)    */
#define U_SDBHLU ( 0x9f )       /* S-jis Duble Byte Code Range (L-U)    */
#define U_SDBHUL ( 0xe0 )       /* S-jis Duble Byte Code Range (U-L)    */
#define U_SDBHUU ( 0xfc )       /* S-jis Duble Byte Code Range (U-U)    */
#define U_NOMSKD ( 0x00000000 ) /* Clear Return Code                    */
#define U_INUDCT ( 0x00000001 ) /* Input User Dictonary Data Flag       */
#define U_INPRDV ( 0x00000002 ) /* Input Printer Device Flag Definition */
#define U_DUPPRD ( 0x00000010 ) /* Duplicate '-d' Data Error Code       */
#define U_DUPPRP ( 0x00000020 ) /* Duplicate '-P' Data Error Code       */
#define U_NOTYDT ( 0x00000100 ) /* Not Yomi Data Error Code             */
#define U_NOTGDT ( 0x00000200 ) /* Not Goku Data Error Code             */
#define U_YNOLDB ( 0x00001000 ) /* Illigal Duble Byte (Yomi)            */
#define U_GNOLDB ( 0x00002000 ) /* Illigal Duble Byte (Goku)            */
#define U_IGPTRD ( 0x00010000 ) /* Illigal Pattrn (User Dictionary)     */
#define U_IGPTRY ( 0x00020000 ) /* Illigal Pattrn (Next Yomi Not Found) */
#define U_IGPTRG ( 0x00040000 ) /* Illigal Pattrn (Next Goku Not Found) */
#define U_IGDATA ( 0x00080000 ) /* Illigal Data                         */
#define U_NOTYMI ( 0x00100000 ) /* Yomi Data Not Found                  */
#define U_NOTGKU ( 0x00200000 ) /* Goku Data Not Found                  */
#define U_PRMADD ( 0x80000000 ) /* Parameter Add Control Code           */
#define U_DBBYT  ( 2 )          /* Duble Byte Code Offset               */
#define U_CLEARF ( 0 )          /* Control Flag Clear Code              */
#define U_USDCFL ( 1 )          /* Control Flag User Dictionary Code    */
#define U_YOMIFL ( 2 )          /* Control Flag Yomi Code               */
#define U_GOKUFL ( 3 )          /* Control Flag Goku Code               */
#define U_PRDVFL ( 4 )          /* Control Flag Printer Device Code     */

#define U_TMPDIR ("/tmp/")      /* Temprary Directory Name              */
#define U_DICLST ("/tmp/dict.list      ")       /* Tmp. File Name       */
#define U_CPFILE ("cp ")        /* Copy Definiton                       */
#define U_RMFILE ("rm -f ")     /* Remove Definition                    */
#define U_BLANK  (" ")          /* Blank Definition                     */
#define U_SLASH  ("/")          /* Slash Definition                     */
#define U_BAK    (".bak")       /* Back File Definition                 */
#define U_TMP    (".tmp")       /* Temporary File Definition            */

#define U_MAX(a,b)      (( a > b ) ? a : b)
#define U_PATTRN(a,b,c) (((a == U_CLEARF) || (a == b)) ? U_NOMSKD : c)
#define U_DBCHK(dbcs)   (((dbcs>=U_SDBHLL && dbcs<=U_SDBHLU)||\
			  (dbcs>=U_SDBHUL && dbcs<=U_SDBHUU)) ? 0 : 1)


extern char *CU_MSGTIL;
extern char *CU_MSGCPL;
extern char *CU_MSGTRE;
extern char *CU_MSGIVT;
extern char *CU_MSGCUR;
extern char *CU_MSGHME;
extern char *CU_MSGNUD;
extern char *CU_MSGWER;
extern char *CU_MSGRER;
extern char *CU_MSGINU;
extern char *CU_MSGINP;
extern char *CU_MSGIV0;
extern char *CU_MSGIV1;
extern char *CU_MSGIV2;
extern char *CU_MSGLOV;
extern char *CU_MSGNDW;
extern char *CU_MSGFWE;
extern char *CU_MSGROT;
extern char *CU_MSGDPD;
extern char *CU_MSGDPP;
extern char *CU_MSGNYM;
extern char *CU_MSGNGK;
extern char *CU_MSGIDY;
extern char *CU_MSGIDG;
extern char *CU_MSGIGP;
extern char *CU_MSGNXY;
extern char *CU_MSGNXG;
extern char *CU_MSGNFY;
extern char *CU_MSGNFG;
extern char *CU_MSGSUC;
extern char *CU_MSGYOR;
extern char *CU_MSGHAM;
extern char *CU_MSGIYM;
extern char *CU_MSGYGE;
extern char *CU_MSGERG;
extern char *CU_MSGSER;
extern char *CU_MSGSAE;
extern char *CU_MSGUAE;
extern char *CU_MSGUUP;
extern char *CU_MSGURC;
extern char *CU_MSGEFL;
extern char *CU_MSGESE;
extern char *CU_MSGNYR;
extern char *CU_MSGETE;
extern char *CU_MSGONC;
extern char *CU_MSGMAE;
extern char *CU_MSGUPD;
extern char *CU_MSGYDT;
extern char *CU_MSGGDT;
extern char *CU_MNTITL;
extern char *CU_MNUFNM;
extern char *CU_MNSMSG;
extern char *CU_MNFMSG;
extern char *CU_MNDAT1;
extern char *CU_MNDAT2;
extern char *CU_MNDAT3;
extern char *CU_MNDAT4;
extern char *CU_MNDAT5;
extern char *CU_MNDAT6;
extern char *CU_MNMAE1;
extern char *CU_MNDERR;
extern char *CU_MNBLNK;
extern char *CU_MNATIT;
extern char *CU_MNYGEM;
extern char *CU_MNYOMI;
extern char *CU_MNGOKU;
extern char *CU_MNCMGT;
extern char *CU_MNCMGK;
extern char *CU_MNCMGF;
extern char *CU_MNCMGO;
extern char *CU_MNADIC;
extern char *CU_MNKAK1;
extern char *CU_MNNDIC;
extern char *CU_MNKMSG;
extern char *CU_MNCTIT;
extern char *CU_MNKEYE;
extern char *CU_MNKEY2;
extern char *CU_MNKEY3;
extern char *CU_MNKEY5;
extern char *CU_MNKEY7;
extern char *CU_MNKEY8;
extern char *CU_MNKEY9;
extern char *CU_MNKY12;
extern char *CU_MNKAK2;
extern char *CU_MNKAK3;
extern char *CU_MNYAG1;
extern char *CU_MNYAG2;
extern char *CU_MNUTIT;
extern char *CU_MNDTIT;
extern char *CU_MNLTIT;
extern char *CU_MNFMSS;
extern char *CU_MNPDT1;
extern char *CU_MNPDT2;
extern char *CU_MNPDT3;
extern char *CU_MNPMSG;
extern char *CU_MNQMSG;
extern char *CU_AMSGN;
extern char *CU_BMSGN;
extern char *CU_CMSGN;
extern char *CU_DMSGN;
extern char *CU_EMSGN;
extern char *CU_FMSGN;
extern char *CU_GMSGN;
extern char *CU_HMSGN;
extern char *CU_IMSGN;
extern char *CU_JMSGN;
extern char *CU_KMSGN;
extern char *CU_LMSGN;
extern char *CU_MMSGN;
extern char *CU_NMSGN;
extern char *CU_OMSGN;
extern char *CU_PMSGN;
extern char *CU_QMSGN;
extern char *CU_RMSGN;
extern char *CU_SMSGN;
extern char *CU_TMSGN;
extern char *CU_UMSGN;
extern char *CU_VMSGN;
extern char *CU_WMSGN;
extern char *CU_XMSGN;
extern char *CU_YMSGN;
extern char *CU_ZMSGN;
extern char *CU_AAMSGN;
extern char *CU_ABMSGN;
extern char *CU_ACMSGN;
extern char *CU_ADMSGN;
extern char *CU_AEMSGN;
extern char *CU_AFMSGN;
extern char *CU_AGMSGN;
extern char *CU_AHMSGN;
extern char *CU_AIMSGN;
extern char *CU_AJMSGN;
extern char *CU_AKMSGN;
extern char *CU_ALMSGN;
extern char *CU_AMMSGN;
extern char *CU_ANMSGN;
extern char *CU_AOMSGN;
extern char *CU_APMSGN;
extern char *CU_AQMSGN;
extern char *CU_ARMSGN;
extern char *CU_ASMSGN;
extern char *CU_ATMSGN;
extern char *CU_AUMSGN;
extern char *CU_AVMSGN;
extern char *CU_AWMSGN;
extern char *CU_AXMSGN;
extern char *CU_AYMSGN;
extern char *CU_AZMSGN;
extern char *CU_BAMSGN;
extern char *CU_BBMSGN;
extern char *CU_BCMSGN;
extern char *CU_BDMSGN;
extern char *CU_BEMSGN;
extern char *CU_BFMSGN;
extern char *CU_BGMSGN;
extern char *CU_ADDING;
extern char *CU_UPDATING;
extern char *CU_WRERROR;
extern char *CU_WRFORCE;

