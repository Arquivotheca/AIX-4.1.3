# @(#)81        1.62  src/bldenv/pkgtools/Makefile, pkgtools, bos41J, 9513A_all 2/27/95 10:29:31
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH			=${MAKETOP}bos/usr/ccs/lib/libIN:${MAKETOP}packages/bos/bosboot

SUBDIRS			=common processPtf subptf getvrmf cdboot \
			updatefixdata lookupfixdata

SCRIPTS			=adelppname stack_32 Maketoc_32 gen_toc_entry \
			 makestack mkodmupdt dirCheck ptfpkg \
			 updatebuild makecdfs OrderImages adeprodid \
			 makediagboot odmupdate ptfcleanup gen_infofile \
			 promote_prod promote_ship gen_ptf_index \
			 genfsimage xmit_ptf update_RETAIN SendPtf \
			 gentape gendiskette gendis_toc 41_pkg_environment \
			 AddSizeNote LogonRetain unpack display_msg \
			 changevrmf what_fileset build_fileset_db MFSadmin \
			 fsCnt genPMPFile updatelppinfo MakeStackList \
			 AddFilesNote CreateApar CheckAparStatus


PROGRAMS		=adepackage adeinv adecopyright addvpd mkodmextract \
			 processStanza ilverify ptfins getstanza \
			 ccss_pack ccss_unpack media_list genabst \
			 prereq_list
OTHERS                  =masterfile
COMMON_OFILES		=common/util.o
adepackage_OFILES	=adepackage.o do_tape.o do_tar.o buffer.o warn.o \
			 readlist.o findfile.o do_backup.o backup_io.o \
			 list.o first.o XX.o
adeinv_OFILES		=adeinv.o findfile.o list.o readlist.o size.o \
			 adeinvmsg.o acfList.o ${COMMON_OFILES}
adecopyright_OFILES	=adecopyright.o crmsg.o ${COMMON_OFILES}
addvpd_OFILES		=addvpd.o addvpdmsg.o stanza.o
ccss_pack_OFILES	=ccss_pack.o
ccss_unpack_OFILES	=ccss_unpack.o
getstanza_OFILES	=getstanza.o
mkodmextract_OFILES	=mkodmextract.o mkodmextractmsg.o
processStanza_OFILES	=processStanza.o ${COMMON_OFILES}
ptfins_OFILES		=ptfins.o ptfinsmsg.o ${COMMON_OFILES} readlist.o
ilverify_OFILES		=${COMMON_OFILES} ilverify.o ilvmsg.o entryCheck.o \
			 readlist.o

do_backup.o_CFLAGS    += -qdebug=npage0

addvpd_LIBS		=-ls -lswvpd -lodm
mkodmextract_LIBS	=-lodm
processStanza_LIBS	=-lodm

ILIST			=${PROGRAMS} ${SCRIPTS} copyright.map compids.table \
			 objclassdb uniqueobjclassdb sstext.name masterfile \
			 label_text.table xmit_ptf.table uidtable \
			 VRMFexclusion no_update_list bosboot_list

copyright.map_IDIR	=/usr/lib/
compids.table_IDIR	=/usr/lib/
label_text.table_IDIR	=/usr/lib/
objclassdb_IDIR		=/usr/lib/
uniqueobjclassdb_IDIR	=/usr/lib/
masterfile_IDIR   	=/usr/lib/
xmit_ptf.table_IDIR   	=/usr/lib/
uidtable_IDIR   	=/usr/lib/
VRMFexclusion_IDIR   	=/usr/lib/
no_update_list_IDIR   	=/usr/lib/
bosboot_list_IDIR   	=/usr/lib/
IDIR			=/usr/bin/

.include <${RULES_MK}>

${COMMON_OFILES} :

BUNDLES_DIR=${MAKETOP}bos/usr/sys/inst.data/sys_bundles
masterfile: ${BUNDLES_DIR}/BOS.autoi ${BUNDLES_DIR}/GOS.autoi ${BUNDLES_DIR}/ASCII.autoi
	${RM} -f ${.TARGET}
	${ECHO} "bos.rte" >${.TARGET}
	${ECHO} "bos.rte.up" >>${.TARGET}
	${ECHO} "bos.rte.mp" >>${.TARGET}
	${ECHO} "devices.*" >>${.TARGET}
	${ECHO} "printers.*" >>${.TARGET}
	${ECHO} "bos.msg.*" >>${.TARGET}
	${ECHO} "bos.loc.*" >>${.TARGET}
	${ECHO} "bos.iconv.*" >>${.TARGET}
	${CAT} ${.ALLSRC} >>${.TARGET}

