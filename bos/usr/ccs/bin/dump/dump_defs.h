/*
 * @(#)71       1.15  src/bos/usr/ccs/bin/dump/dump_defs.h, cmdaout, bos411, 9428A410j 3/22/94 11:27:41
 */
/*
 * COMPONENT_NAME: CMDAOUT (dump command)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Message strings for the dump command
 */
#include <nl_types.h>
#include "dump_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_DUMP, Num, Str)


#define	DUMP_ONE "dump: Specify at least one of -acdgHhlnoRrsTtz.\n"
#define	USAGE "Usage: dump -acdgHhlnopRrsTtuv -t[Index] +tIndex\n\
\t-zName[,Number] +zNumber File ...\n"
#define	AR_SYM_TITLE "    Offset\tName\n\n"
#define	AR_HD_TITLE "Member Name            Date                Uid        Gid     Mode      Size\n"
#define	PARM_HASH "\tParameter Hash: "
#define	NO_HASH "no hash"
#define	LD_UNAVAIL "\nLoader Section is not available\n"
#define	LD_RLHEAD "\t     Vaddr      Symndx      Type      Relsect"
#define	LD_RLNAME "    Name\n"
#define	LD_SYMHEAD "[Index]\t     Value      Scn     IMEX Sclass   Type           IMPid Name\n\n"
#define	NO_IMID "[noIMid]"
#define	NO_SYM " [no sym]\n"
#define	LD_HDR "%22sLoader Header Information\n"
#define	LD_HDRHEAD1 "VERSION#         #SYMtableENT     #RELOCent        LENidSTR\n"
#define	LD_HDRHEAD2 "#IMPfilID        OFFidSTR         LENstrTBL        OFFstrTBL\n"
#define	OBJ_HDRHEAD "# Sections	Symbol Ptr	# Symbols	Opt Hdr Len	Flags\n"
#define	OBJ_FLAGS "Flags=("
#define	OPT_HDRHEAD1 "Tsize	     Dsize	 Bsize	     Tstart 	 Dstart\n"
#define	OPT_HDRHEAD2 "\nSNloader     SNentry     SNtext	     SNtoc       SNdata\n"
#define	OPT_HDRHEAD3 "TXTalign     DATAalign   TOC         vstamp	 entry\n"
#define	LN_HEAD "\tSymndx/Paddr    Lnno\n"
#define	LN_FCN "\n\t Fcn %6ld   %5hu"
/* RL_HEAD is replaced by RL_HEAD2.  Do not use RL_HEAD!   */
#define	RL_HEAD2 "\t     Vaddr      Symndx  Sign  Fixup     Len      Type"
#define	RL_HDNAME "  Name\n"
#define	SH_TITLE "\t \t \t Section Header for %-8s\n"
#define	SH_HEAD1 "PHYaddr      VTRaddr	 SCTsiz      RAWptr 	 RELptr\n"
#define	SH_HEAD2 "LN#ptr       #RELent	 #LINent     Flags\n"
#define	ST_HEAD "\t Offset     Name\n\n"
#define	SYM_HEAD "[Index]\tm        Value       Scn   Aux  Sclass           Type     Name\n\
[Index]\ta0                                                        Fname\n\
[Index]\ta1      Tagndx      Lnno  Size  Lnoptr    Endndx\n\
[Index]\ta2      Tagndx            Fsiz  Lnoptr    Endndx\n\
[Index]\ta3      Tagndx      Lnno  Size  Dimensions\n\
[Index]\ta4   CSlen     PARMhsh SNhash SMtype SMclass Stab SNstab\n\
[Index]\ta5      SECTlen    #RELent    #LINnums\n\n"
#define	LD_SCN "Loader Section"
#define	LD_RLINFO "Relocation Information"
#define	LD_SYMTAB "Loader Symbol Table Information"
#define	AR_HEADER "Archive Header"
#define	OBJ_HDR "Object Module Header"
#define	OPT_HDR "Optional Header"
#define	LN_INFO "Line Number Information"
#define	RL_INFO "Relocation Information"
#define	SH_INFO "Section Header Information"
#define	SCN_DATA "Section Data in Hexadecimal"
#define	ST_INFO "String Table Information"
#define	SYM_INFO "Symbol Table Information"
#define	AR_SYMTAB "Archive Symbol Table"
#define	BAD_OPTHDR "Unknown Optional Header"
#define IMP_FILE_STRS "Import File Strings"
#define INDEX_STR "INDEX"
#define PATH_STR "PATH"
#define BASE_STR "BASE"
#define MEMBER_STR "MEMBER"
#define	BAD_SYMBOL_NAME "**Invalid Symbol Name**"
#define	BAD_AUX_NAME "**Invalid Auxiliary Name**"
#define	NO_SYMBOL_NAME "**No Symbol**"
#define	NO_STRINGS "There are no string entries for the specified file.\n"
#define	FCN_NOT_FOUND "The function is not found.\n"
#define	NO_LDREL "No loader relocation entries found.\n"
#define	NO_AUX_ENT "0654-100 There is no auxiliary entry for the function.\n"
#define	NO_SCN_HDR "0654-101 Cannot find the section headers.\n"
#define	SCN_HDR_ERR "0654-102 Cannot read a section header.\n"
#define	READ_ERR "0654-103 Cannot read from the specified file.\n"
#define	SEEK_ERR "0654-104 The fseek system call failed.\n"
#define	NOT_AN_OBJ "0654-105 The file is not in a recognized format.\n\
\tSpecify an executable file, object file, or archive file.\n"
#define	CANT_OPEN "0654-106 Cannot open the specified file.\n"
#define	NO_MEM "0654-107 There is not enough memory available now.\n"
#define OPT_HDRHEAD4 "maxSTACK     maxDATA\n"
