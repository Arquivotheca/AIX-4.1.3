$                    "@(#)72\t1.14  src/bos/usr/ccs/bin/dump/dump.msg, cmdaout, bos411, 9428A410j 3/22/94 13:33:26" 
$ COMPONENT_NAME: CMDAOUT (dump command)  
$  FUNCTIONS: none  
$  ORIGINS: 27  
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$  combined with the aggregated modules for this product)  
$                   SOURCE MATERIALS  
$  (C) COPYRIGHT International Business Machines Corp. 1989,1994
$  All Rights Reserved  
$  US Government Users Restricted Rights - Use, duplication or  
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ Message strings for the dump command  
$quote "         define quote character.

$len 
$set MS_DUMP        Define initial set#


DUMP_ONE_MSG         "dump: Specify at least one of -acdgHhlnoRrsTtz.\n"
$ \t--> Do not translate command name (dump) or options in the following  

USAGE_MSG            "Usage: dump -acdgHhlnopRrsTtuv -t[Index] +tIndex\n\
\t-zName[,Number] +zNumber File ...\n"

AR_SYM_TITLE_MSG     "    Offset\tName\n\n"
$ NOTE  
$ NOTE:  This, and the following 26 messages, are used as headers to  
$ NOTE:  output sections.  The length of these strings is critical  
$ NOTE:  to the format of the output.  When translating be sure that 
$ NOTE:  the starting character of each header item is in the same 
$ NOTE:  column as in the English version.  
$ NOTE  

AR_HD_TITLE_MSG      "Member Name            Date                Uid        Gid     Mode      Size\n"
$ Do not translate Uid, Gid. 

PARM_HASH_MSG        "\tParameter Hash: "
$ Do not translate. The message is not used. 

NO_HASH_MSG          "no hash"
$ The translation cannot be longer than eight characters. 
$ This appears in the output table when no hash string is found. 

LD_UNAVAIL_MSG       "\n\
Loader section is not available\n"

LD_RLHEAD_MSG        "\t     Vaddr      Symndx      Type      Relsect"
$ Vaddr  = Virtual address, Symndx = Symbol index, Relsect = Relocation Section. 

LD_RLNAME_MSG        "    Name\n"

LD_SYMHEAD_MSG       "[Index]\t     Value      Scn     IMEX Sclass   Type           IMPid Name\n\n"
$ Translation of "[Index]" cannot exceed eight characters. 
$ Scn=Section, IMEX=Import/Export, Sclass=Storage class, IMPid=Import Identifier 

NO_IMID_MSG          "[noIMid]"
$ noIMid = no import identifier. 
$ Translation of "[noIMid]" cannot exceed eight characters. 

NO_SYM_MSG           " [no sym]\n"
$ no sym = no symbol. 
$ Translation of "[no sym]" cannot exceed eight characters. 

LD_HDR_MSG           "%22sLoader Header Information\n"
$ Do not translate %22s. Text cannot exceed thirty six characters. 

LD_HDRHEAD1_MSG      "VERSION#         #SYMtableENT     #RELOCent        LENidSTR\n"
$ Version# = version number, #SYMtableENT = the number of symbol table entries, 
$ #RELOCent = Number of relocation entries, LENidST = Length of identifier strings. 

LD_HDRHEAD2_MSG      "#IMPfilID        OFFidSTR         LENstrTBL        OFFstrTBL\n"
$ #IMPfilID = Import file identifier, OFFidSTR = offset of identifier strings, 
$ LENstrTBL = Length of string table, OFFstrTBL = offset of string table. 

OBJ_HDRHEAD_MSG      "# Sections      Symbol Ptr      # Symbols       Opt Hdr Len     Flags\n"
$ # = number, Ptr = pointer, Opt Hdr Len = Optional header length.   

OBJ_FLAGS_MSG        "Flags=("
$ Do not translate =( 

OPT_HDRHEAD1_MSG     "Tsize        Dsize       Bsize       Tstart      Dstart\n"
$ T=text, D=data, B=BSS (Block storage space). 

OPT_HDRHEAD2_MSG     "\n\
SNloader     SNentry     SNtext\t     SNtoc       SNdata\n"
$ SN = section number. toc = table of content. 

OPT_HDRHEAD3_MSG     "TXTalign     DATAalign   TOC         vstamp      entry\n"
$ TXT = text, TOC = Table of Contents, vstamp = version stamp. 

LN_HEAD_MSG          "\tSymndx/Paddr    Lnno\n"
$ Symndx/Paddr = Symbol index/Physical address, Lnno = line number. 

LN_FCN_MSG           "\n\
         Fcn %1$6ld   %2$5hu"
$ Fcn = function. Translation of fcn must end in column 12.  
$ Do not translate the remainder of the message.   

RL_HEAD_MSG          "\t     Vaddr      Symndx  Sign     Len      Type"
$ DO NOT TRANSLATE
$ This message has been replaced by RL_HEAD2_MSG, later in this file.
$ This message is not used.

RL_HDNAME_MSG        "  Name\n"

SH_TITLE_MSG         "\t \t \t Section Header for %-8s\n"

SH_HEAD1_MSG         "PHYaddr      VTRaddr     SCTsiz      RAWptr      RELptr\n"
$ addr = address, PHY = physical, VTR = virtual, SCTsiz = section size,  
$ ptr = pointer, RAW = actual data, REL = relocation. 

SH_HEAD2_MSG         "LN#ptr       #RELent\t #LINent     Flags\n"
$ # = number, LN, LIN = line, REL = relocation, ent = entry. 

ST_HEAD_MSG          "\t Offset     Name\n\n"

SYM_HEAD_MSG "[Index]\tm        Value       Scn   Aux  Sclass    Type            Name\n\
[Index]\ta0                                                        Fname\n\
[Index]\ta1      Tagndx      Lnno  Size  Lnoptr    Endndx\n\
[Index]\ta2      Tagndx            Fsiz  Lnoptr    Endndx\n\
[Index]\ta3      Tagndx      Lnno  Size  Dimensions\n\
[Index]\ta4   CSlen     PARMhsh SNhash SMtype SMclass Stab SNstab\n\
[Index]\ta5      SECTlen    #RELent    #LINnums\n\n"
$ Translation of "[Index]" cannot exceed eight characters. 
$ Do not translate m, a0, a1, a2, a3, a4, a5 
$ aux = auxiliary, Fname = function name, Tagndx = tag index,  
$ Lnoptr = line number pointer, Endndx = end index. 
$ CSlen = Control Section length, PARMhsh = Parameter hash 
$ SNhash = Section number hash, SM = Storage Mapping 
$ stab is technical term do not translate 
$ #LINnums = NUmber of line number entries 

LD_SCN_MSG           "Loader Section"
$ NOTE  
$ NOTE: This, and the  following 14 message strings are used as 
$ NOTE: titles for various output sections.  The effects of translation  
$ NOTE: on these messages should be minimal.  
$ NOTE  

LD_RLINFO_MSG        "Relocation Information"

LD_SYMTAB_MSG        "Loader Symbol Table Information"

AR_HEADER_MSG        "Archive Header"

OBJ_HDR_MSG          "Object Module Header"

OPT_HDR_MSG          "Optional Header"

LN_INFO_MSG          "Line Number Information"

RL_INFO_MSG          "Relocation Information"

SH_INFO_MSG          "Section Header Information"

SCN_DATA_MSG         "Section Data in Hexadecimal"

ST_INFO_MSG          "String Table Information"

SYM_INFO_MSG         "Symbol Table Information"

AR_SYMTAB_MSG        "Archive Symbol Table"

BAD_OPTHDR_MSG       "Unknown Optional Header"

IMP_FILE_STRS_MSG    "Import File Strings"

INDEX_STR_MSG        "INDEX"
$ Translation cannot be longer than 7 characters. 

PATH_STR_MSG         "PATH"
$ Translation cannot be longer than 29 characters. 

BASE_STR_MSG         "BASE"
$ Translation cannot be longer than 19 characters. 

MEMBER_STR_MSG       "MEMBER"
$ Translation cannot be longer than 19 characters. 

BAD_SYMBOL_NAME_MSG  "**Invalid Symbol Name**"
$ NOTE  
$ NOTE: This, and the next two  message strings are used to indicate  
$ NOTE: special occurences in the output. Message   
$ NOTE: expansion should be kept to a minimum.   
$ NOTE  

BAD_AUX_NAME_MSG     "**Invalid Auxiliary Name**"

NO_SYMBOL_NAME_MSG   "**No Symbol**"

NO_STRINGS_MSG       "There are no string entries for the specified file.\n"

FCN_NOT_FOUND_MSG    "The function is not found.\n"

NO_LDREL_MSG         "No loader relocation entries found.\n"

NO_AUX_ENT_MSG       "dump: 0654-100 There is no auxiliary entry for the function.\n"

NO_SCN_HDR_MSG       "dump: 0654-101 Cannot find the section headers.\n"

SCN_HDR_ERR_MSG      "dump: 0654-102 Cannot read a section header.\n"

READ_ERR_MSG         "dump: 0654-103 Cannot read from the specified file.\n"

SEEK_ERR_MSG         "dump: 0654-104 The fseek system call failed.\n"
$ Do not translate fseek.  

NOT_AN_OBJ_MSG       "dump: 0654-105 The file is not in a recognized format.\n\
\tSpecify an executable file, object file, or archive file.\n"

CANT_OPEN_MSG        "dump: 0654-106 Cannot open the specified file.\n"

NO_MEM_MSG           "dump: 0654-107 There is not enough memory available now.\n"

OPT_HDRHEAD4_MSG     "maxSTACK     maxDATA\n"
$ Part of header text. 
$ When translating be sure that the starting character of each header item 
$ is in the same column as in the English version.  

RL_HEAD2_MSG         "\t     Vaddr      Symndx  Sign  Fixup     Len      Type"
$ Vaddr = virtual address, Symndx = symbol index, Len = length. 
$ When translating be sure that the ENDING character of each header item 
$ is in the same column as in the English version.  
