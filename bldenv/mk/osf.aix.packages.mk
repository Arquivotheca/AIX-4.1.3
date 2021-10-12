# @(#)30	1.108  src/bldenv/mk/osf.aix.packages.mk, pkgtools, bos41J  5/18/95  15:29:04 
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# References to <option> below will refer to options from SHARE_OPTIONS,
# ROOT_OPTIONS or OPTIONS.
#
# A packaging build may be run in three different ways:
# 1. build install images BUT do not copy files into the selfix update tree.
#    (this type of build will also create some initial versions of files 
#    out in the selfix history tree, e.g. the odm history add files).
#    triggered by PTF_UPDATE != yes (forces BUILD_INSTALL_IMAGES = yes)
# 2. build update stuff, but DON'T build the install images.  This type
#    of build will not spend time building the install images.  It will
#    only populate the selfix tree.
#    triggered by PTF_UPDATE = yes && BUILD_INSTALL_IMAGES != yes
# 3. build install images AND the update stuff
#    triggered by PTF_UPDATE = yes && BUILD_INSTALL_IMAGES = yes

.if !defined(_OSF_PACKAGES_MK_)
_OSF_PACKAGES_MK_=

# To build an initial selfix tree, set BUILD_SELFIX_TREE = "yes".  This
# will force BUILD_INSTALL_IMAGES to "no" and PTF_UPDATE to "yes".  The
# selfix/UPDATE and selfix/HISTORY trees should be empty when you run
# this way.
.if !defined(BUILD_SELFIX_TREE)
BUILD_SELFIX_TREE = no
.endif

.if (${BUILD_SELFIX_TREE} == "yes")
PTF_UPDATE = yes
BUILD_INSTALL_IMAGES = no
.endif # BUILD_SELFIX_TREE == "yes"

# Force BUILD_INSTALL_IMAGES to yes if we're not doing an update build.
# Force it to no if we're doing an update build and its not set.
.if (${PTF_UPDATE} != "yes")
BUILD_INSTALL_IMAGES	= yes
.else
BUILD_INSTALL_IMAGES	?= no
.endif

#
#  MACROS used for making lpp packages
#
BLD_LOG_DIR		=${BASE}/selfix/LOG/${TD_BLDCYCLE}
DATAFLAG		= -D
OPTIONAL_MEMBER_EXT  	= inventory al size tcb odmadd odmdel unodmadd
LINKFLAG		?=
LPP_NAME		= lpp_name
LPPFILEFORMAT		?= 4
LPPFIXLEVEL		?= 0000
LIBLPP			= liblpp.a
LPPMAINT		?= 0000
LPPMEDIATYPE		?= I
.if defined(LPPIFORLSFLAG)
LPPNAMEFLAGS		= -f ${LPPFILEFORMAT} -v ${LPPVERSION} \
			  -r ${LPPRELEASE} -m ${LPPMAINT} -F ${LPPFIXLEVEL} \
			  -p ${PLATCODE} -t ${LPPMEDIATYPE} -c ${COMPIDSTABLE} -L
.else
LPPNAMEFLAGS		= -f ${LPPFILEFORMAT} -v ${LPPVERSION} \
			  -r ${LPPRELEASE} -m ${LPPMAINT} -F ${LPPFIXLEVEL} \
			  -p ${PLATCODE} -t ${LPPMEDIATYPE}
.endif
LPPRELEASE		?= 01
LPPVERSION		?= 04
OBJCLASSDBGLOBAL	?= ${ODE_TOOLS}/usr/lib/objclassdb
OBJCLASSDBUNIQ		?= ${ODE_TOOLS}/usr/lib/uniqueobjclassdb
OBJCLASSDB              ?= ${OBJCLASSDBGLOBAL}
.if defined(IN_SANDBOX)
# Then strip off the last path component.
PKGSRCTOP		= ${BACKED_SOURCEDIR:!${ECHO} ${BACKED_SOURCEDIR} | \
				${SED} -e "s?.*:??" -e "s?//?/?g"!}
.else
PKGSRCTOP		= ${MAKETOP}../../src
.endif
PKGOBJTOP		= ${MAKETOP}packages
PLATCODE		?= R

# Provide developers with a way to specify odm add files that are preloaded
# into the base, but still will need to be updated.  The format of the
# list is the same as the format of the *_ODMADD_LIST, i.e. one entry for
# each .add file, with the entry having the format of <addfile>%<option>
# for usr, root and share parts.
# The typical use will be:
# PRELOADED_USR_ODMADD_LIST += <addfile>%<option> for USR part
# PRELOADED_SHARE_ODMADD_LIST += <addfile>%<option> for DATA part
# PRELOADED_ROOT_ODMADD_LIST += <addfile>%<option> for ROOT part
# This variable is then converted to a list of the actual history .add files
# that need to be copied over.  In order to trigger the copy of the history
# .add files, this variable will be added to the OTHERS list for non update
# builds.  Note that the history file depends on the original .add file
# which must be found on the VPATH somewhere.
.if (${PTF_UPDATE} != "yes")
PRELOADED_ODMADD_HISTORY_LIST = ${PRELOADED_USR_ODMADD_LIST:! \
			${ECHO} ${PRELOADED_USR_ODMADD_LIST} \
			${PRELOADED_SHARE_ODMADD_LIST} \
			${PRELOADED_ROOT_ODMADD_LIST} | \
			${AWK} 'BEGIN {RS=" "} \
			/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
			print "${UPDATEODMHIST}/" opt "/" addf}'!}
OTHERS			+= ${PRELOADED_ODMADD_HISTORY_LIST}
.else
PRELOADED_ODMADD_HISTORY_LIST = 
.endif # (${PTF_UPDATE} != "yes")

PRODUCTID		= productid
ROOTFLAG		= -r
SELFIXTOP		= ${BASE}/selfix
UPDATETOP		= ${SELFIXTOP}/UPDATE
UPDATEODMHIST		= ${SELFIXTOP}/HISTORY/odmhist
XREF			= ${SELFIXTOP}/HISTORY/XREF
INSLISTS		= ${TDDIR:H}/inslists
ADECRFLAGS		?= -c
STRICTFLAG		?= -Y

# The shipfile_dependency files should already exist, but if this is the
# first time this code is executed, then create the shipfile_dependency
# with a very old date.  This works because the .il and .cr dependencies
# will take care of building a new install image if it is out of date.
# Normally the shipfile_dependency file will be touched if a file ships
# in the fileset.  These files are located in the obj/power/packages/fileset
# directory.
MK_SHIPFILE_DEPENDENCY  != \
	for fsdir in ${SHARE_OPTIONS:@FS@${FS:S?.?/?g}@} \
		${OPTIONS:@FS@${FS:S?.?/?g}@}; \
	do \
	    depfile=${PKGOBJTOP}/$$fsdir/shipfile_dependency; \
	    if [ -f $$depfile ]; \
	    then : ; \
	    else \
		${MAKEPATH} $$depfile; \
		${TOUCH} -t 197001010000 $$depfile; \
	    fi; \
	done

.if defined(SHARE_OPTIONS)
DATA_BFF		= ${BFF}.data
SHARE_LPP_NAME		= data/${LPP_NAME}
SHARE_LIBLPP		= data/${LIBLPP}
SHARE_PRODUCTID		= data/${PRODUCTID}
.if (${BUILD_INSTALL_IMAGES} == "yes")
_COMP_STANDARD_TARGETS_ += ${DATA_BFF}
BINARIES		+= ${DATA_BFF}
.endif # BUILD_INSTALL_IMAGES == yes
.if (${PTF_UPDATE} == "yes")
# Define the share files that must be created when PTF_UPDATE is "yes".
OTHERS			+= \
	${SHARE_INFO_FILES:M*.upsize:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@} \
	${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.lp@} \
	${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.il@} \
	${SHARE_OPTIONS:@OPT@${INSLISTS}/${OPT}.il@} \
	${SHARE_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/data.${OPT}@} \
	${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/lpp_info@} \
	${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/data.liblpp.a@} \
	${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@}

#	${SHARE_OPTIONS:@OPT@${XREF}/${OPT}.share.xref@} \

.endif # PTF_UPDATE == yes
.endif # defined SHARE_OPTIONS

.if defined(ROOT_OPTIONS)
ROOT_LIBLPP		= root/${LIBLPP}
.if (${PTF_UPDATE} == "yes")
# Define the root files that must be created when PTF_UPDATE is "yes".
OTHERS			+= \
	${ROOT_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/root.liblpp.a@} \
	${ROOT_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@} \
	${ROOT_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/root.${OPT}@}
.endif # PTF_UPDATE == yes
.endif # defined ROOT_OPTIONS

.if defined(OPTIONS)
USR_PRODUCTID		= usr/${PRODUCTID}
.if (${BUILD_INSTALL_IMAGES} == "yes")
BINARIES		+=${BFF}
_COMP_STANDARD_TARGETS_ +=${BFF}
.endif # BUILD_INSTALL_IMAGES
.if (${PTF_UPDATE} == "yes")
# Define the usr files that must be created when PTF_UPDATE is "yes".
OTHERS			+= \
	${INFO_FILES:M*.upsize:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@} \
	${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.lp@} \
	${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.il@} \
	${OPTIONS:@OPT@${INSLISTS}/${OPT}.il@} \
	${USR_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/usr.${OPT}@} \
	${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/lpp_info@} \
	${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/usr.liblpp.a@} \
	${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@}

#	${OPTIONS:@OPT@${XREF}/${OPT}.usr.xref@} \

.endif # PTF_UPDATE == yes
.endif # defined OPTIONS

#
#  Include the scripts.mk file so we know how to build scripts
#

.sh:
	@if [ -n ""${.TARGET:M*/*} ]; then ${MAKEPATH} ${.TARGET}; fi
	${CP} ${.IMPSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}
	${CHMOD} +x ${.TARGET}

.S:
	@if [ -n ""${.TARGET:M*/*} ]; then ${MAKEPATH} ${.TARGET}; fi
	@${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} "/^#/d" <${.IMPSRC} >${.TARGET} || { ${RM} -f ${.TARGET}; ${FALSE}; }
	${CHMOD} +x ${.TARGET}

.if (${PTF_UPDATE} != "yes")
# Copy over preloaded add files to the appropriate history directory
${PRELOADED_ODMADD_HISTORY_LIST}: $${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}
.endif

.if defined(SHARE_OPTIONS)

# The changevrmf tool is run to just before adepackage to change the 
# vrmf number of filesets that have not changed to 4.1.0.0. 
.if (${BUILD_INSTALL_IMAGES} == "yes")
${DATA_BFF} :	data/SHARE_INSLISTS ${SHARE_LPP_NAME}
	${CHANGEVRMF} -f data/_SHARELPPNAME_ -l ${SHARE_LPP_NAME};
	${RM} -f lpp_name
	${CP} ${SHARE_LPP_NAME} lpp_name
	${ADEPACKAGE} ${ADEPACKAGEFLAGS} ${DATAFLAG} ${STRICTFLAG} ${LINKFLAG} \
		      -f ${.TARGET}.X -i data/SHARE_INSLISTS \
		      -l ${BFF} -s "${SHIP_PATH}"
	${MV} -f ${.TARGET}.X ${.TARGET}
	${RM} -f lpp_name

data/SHARE_INSLISTS : \
		${SHARE_OPTIONS:S?$?.il?g}
	@if [ ! -d data ]; then ${MKDIR} data; fi
	@# creating INSLISTS file from .il files.  This used to be
	@# done with cat .ALLSRC > .TARGET but the arg list got too
	@# long, so we're trying to split it up based on
	@# ${PKGSRCTOP}.  First we strip the directory off the
	@# front of each file in .ALLSRC that we can, then we add it
	@# back when we do the cat.  Then we cat everything that
	@# didn't match PKGSRCTOP.	
	${RM} -f ${.TARGET}
	@for i in ""${.ALLSRC:M${PKGSRCTOP}*:S?${PKGSRCTOP}??g}; \
	do\
	    if [ -n "$$i" ]; \
	    then \
	        ${ECHO} "${CAT} ${PKGSRCTOP}$$i >>${.TARGET}"; \
	        ${CAT} ${PKGSRCTOP}$$i >>${.TARGET} \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	    fi; \
	done; \
	for i in ""${.ALLSRC:N${PKGSRCTOP}*}; \
	do\
	    if [ -n "$$i" ]; \
	    then \
	        ${ECHO} "${CAT} $$i >>${.TARGET}"; \
	        ${CAT} $$i >>${.TARGET} || { ${RM} -f ${.TARGET}; exit 1; }; \
	    fi; \
	done

# Rule for data/lpp_name.
${SHARE_LPP_NAME} : \
		data/_SHARELPPNAME_ \
		${SHARE_LIBLPP} \
		${SHARE_INFO_FILES:M*.prereq} \
		${SHARE_INFO_FILES:M*.supersede}
	${ADELPPNAME} ${LPPNAMEFLAGS} -l ${BFF} -u data/_SHARELPPNAME_ \
			 -o ${.TARGET}.X -a ${SHARE_LIBLPP}
	${MV} -f ${.TARGET}.X ${.TARGET}

data/_SHARELPPNAME_ : \
		${SHARE_OPTIONS:S?$?.lp?g}
	@if [ ! -d data ]; then ${MKDIR} data; fi
	${CAT} ${.ALLSRC} | ${SED} -e '/^[#	 ]/d' > ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}


# Rule for data/liblpp.a.
${SHARE_LIBLPP} : \
		SHARE_OPTIONAL_MEMBERS \
		${SHARE_OPTIONS:@BASE@${BASE}.copyright@} \
		${SHARE_LIBLPP_LIST} \
		${SHARE_LPPACF_LIST:Ddata/lpp.acf} \
		${SHARE_PRODUCTID}
	@# Ensure that the data directory exists
	@if [ ! -d data ]; then ${MKDIR} data; fi

	@# Archive in the productid and the out of date members explicitly
	@# listed in the dependency line.
	${RM} -f ${.TARGET}
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${.ALLSRC:NSHARE_OPTIONAL_MEMBERS} \
		|| { ${RM} -f ${.TARGET}; ${FALSE}; }

	# Now add in any files generated by mkodmupdt and adeinv.
	@# This code currently will archive all the optional members.
	@if [ -n ""${.ALLSRC:MSHARE_OPTIONAL_MEMBERS} ] ;\
	then \
	   for option in ${SHARE_OPTIONS}; \
	   do \
	      list=""; \
	      for ext in ${OPTIONAL_MEMBER_EXT}; \
	      do \
	         if [ -f $$option*.$$ext ]; \
	         then \
		    list="$$list $$option*.$$ext"; \
	         fi; \
	      done; \
	      ${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	      ${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	   done; \
	fi

data/lpp.acf: ${SHARE_LPPACF_LIST}
	@if [ ! -d data ]; then ${MKDIR} data; fi
	${CAT} ${.ALLSRC} >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

SHARE_OPTIONAL_MEMBERS: SHARE_ODM_LIST SHARE_INVENTORY_LIST
	${RM} -f ${.TARGET} && ${TOUCH} ${.TARGET}

SHARE_INVENTORY_LIST: ${SHARE_OPTIONS:@BASE@${BASE}.il ${BASE}.lp@} \
		${SHARE_LPPACF_LIST} \
		${SHARE_INFO_FILES:M*.insize} \
		${SHARE_OPTIONS:@FS@ \
		    ${PKGOBJTOP}/${FS:S?.?/?g}/shipfile_dependency@}
	@if [ ! -d data ]; then ${MKDIR} data; fi
	@# .ALLSRC got too long so we need to create a tmpfile in two
	@# steps, similar to what was done with the INSLISTS.  First
	@# step is to strip off PKGSRCTOP from every file in .ALLSRC
	@# that starts with PKGSRCTOP, echo that to awk where we put
	@# the directory portion back in the front of each file.
	@# Then echo out the files that didn't match PKGSRCTOP.
	@${RM} -f tmpfile
	@${ECHO} ${.ALLSRC:M${PKGSRCTOP}*:S?${PKGSRCTOP}??g} \
	    | ${AWK} 'BEGIN { RS=" " } {print "${PKGSRCTOP}"$$0}' >tmpfile
	@${ECHO} ${.ALLSRC:N${PKGSRCTOP}*} \
	    | ${AWK} 'BEGIN { RS=" " } {print $$0}' >>tmpfile
	@# Run adeinv on all the out of date files.
	@# Get a complete list of the options that are out of date.
	@# Determine the out of date options by taking the basename
	@# of the out of date file and stripping off the extension.
	@# Then pull off the fully qualified .il and .lp
	@# files from .ALLSRC to send as input to adeinv for both
	@# the usr and root parts.
	@# If we are also doing update builds, then we've got to add
	@# the data/liblpp.a to the .xref file that was created by
	@#  adeinv.  Then copy that file to where it
	@# belongs.
	@# We also use tmpfile to find out if we need to pass an
	@# input .acf file to adeinv to allow it to find libraries
	@# to properly determine the sizes.
	@# liblppPath is the path name for the liblpp.a file.  The
	@# power/packages/ prefix is required for bldnormalize to generate
	@# the normalized path name from the xref file.  The rest of
	@# the path is calculated by changing the . to / in the option
	@# name.
	@oods=`${ECHO} ${.OODATE:T:R:N*shipfile_dependency} \
		${.OODATE:M*shipfile_dependency:H:S?${PKGOBJTOP}/??g:S?/?.?g} | \
		${AWK} 'BEGIN { RS=" " } {print $$0}' | ${SORT} -u`; \
	for i in ""$$oods; \
	do \
	    ilfile=`${GREP} $$i.il tmpfile`; \
	    lpfile=`${GREP} $$i.lp tmpfile`; \
	    if ${GREP} $$i.acf tmpfile >/dev/null 2>&1; \
	    then acfopt="-a `${GREP} $$i.acf tmpfile`"; \
	    else acfopt=""; \
	    fi; \
	    ${ECHO} ${ADEINVENTORY} ${ADEINVENTORYFLAGS} ${STRICTFLAG} ${LINKFLAG} \
			    ${DATAFLAG} -l ${DATA_BFF} \
			    -i $$ilfile \
			    -u $$lpfile \
			    -s "${SHIP_PATH}" $$acfopt; \
	    ${RM} -f $$i.al $$i.tcb $$i.inventory $$i.size $$i.xref; \
	    ${ADEINVENTORY} ${ADEINVENTORYFLAGS} ${STRICTFLAG} ${LINKFLAG} \
			    ${DATAFLAG} -l ${DATA_BFF} \
			    -i $$ilfile \
			    -u $$lpfile \
			    -s "${SHIP_PATH}" $$acfopt || exit 1; \
	    liblppPath=power/packages/`${ECHO} $$i | ${SED} -e "s?\.?/?g"`; \
	    ${ECHO} $${liblppPath}/data.liblpp.a $$i >> $$i.xref; \
	done
	@${RM} -f tmpfile
	${RM} -f ${.TARGET} && ${TOUCH} ${.TARGET}

# Refer to the comments at the begining of USR_ODM_LIST
# for an explanation of this rule.
SHARE_ODM_LIST : \
		${SHARE_ODMADD_LIST:!${ECHO} ${SHARE_ODMADD_LIST} | \
		${SED} "s/%[^ ]* */ /g"!}
	@${RM} -f tmpfile
	@${ECHO} "${SHARE_ODMADD_LIST} ^ " >tmpfile
	@${ECHO} " ${.OODATE} " >>tmpfile
	@# Run mkodmupdt on all the out of date .add files.
	@${AWK} < tmpfile "BEGIN { RS=\" \" ; \
				mkodmupdt=\"${MKODMUPDT} -i -t ${OBJCLASSDB}\"; \
				addfile_flag = 0; n_uols = 0; \
				n_addfiles = 0 ;} "' \
	      $$0 == "^" { addfile_flag = 1 ; next } \
	      $$0 == ""	 { next } \
	      $$0 == "\n"  { next } \
	      { if ( addfile_flag ) { \
		    n_addfiles++ ; addfiles[n_addfiles] = $$0 ; \
		} \
		else { n_uols++ ; uol[n_uols] = $$0 ; }	 \
	      } \
	      END { \
		for ( i=1; i<=n_uols; i++ ) { \
		    split( uol[i], tmp, "%" ); \
		    addfn = tmp[1]; option = tmp[2]; \
		    n = split(addfn,ary,"."); \
		    for ( j=1; j<n; j++ ) base=base "." ary[j]; \
		    for ( j=1; j<=n_addfiles; j++ ) { \
			n = split( addfiles[j], ary, "/" ); \
			if ( ary[n] == addfn ) { \
			    printf( "%s -o %s -c %s\n", \
				    mkodmupdt, option, addfiles[j]); \
			    system("${RM} -f "option base".odmadd " option \
				   base".unodmadd " option base".odmdel"); \
			    if ( 0 != system(mkodmupdt " -o " option \
					" -c " addfiles[j])) \
				exit 1; \
			    break; \
			}; \
		    }; \
		}; \
	      }'
	@${RM} -f tmpfile
	${RM} -f ${.TARGET} && ${TOUCH} ${.TARGET}

.endif # BUILD_INSTALL_IMAGES == yes


# Create the .copyright files from the .cr files.
# From looking at the packaging tree, it appears that the .cr file may
# be found in two places.  It will be in the source tree either in the
# data subdirectory or the current directory.  So if one puts the file
# in the data subdirectory, one must be sure to include it in the VPATH.
# Examples: bos/data and bos/txt/spell
${SHARE_OPTIONS:@BASE@${BASE}.copyright@}: $${.TARGET:R}.cr \
		${PKGOBJTOP}/$${.TARGET:T:R:S?.?/?g}/shipfile_dependency
	${ADECOPYRIGHT} ${ADECRFLAGS} -l ${DATA_BFF} ${${.TARGET:R}.cr:P} \
	    > ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Create the share product id file in the data subdirectory.  If for
# some reason adeprodid fails then dummy up a share/productid file,
# for THE BUILD MUST GO ON.
# Refer to the comments at the begining of USR_PRODUCTID 
# for the explanation of this rule.
${SHARE_PRODUCTID}: ${COMPIDSTABLE}
	@if [ -s ${SHARE_PRODUCTID} ] ; \
	then \
	    ${CAT} ${SHARE_PRODUCTID} | read product compid; \
	    newcompid=`${ECHO} $${compid} | ${AWK} -F"-" '{print $$1 $$2}'`; \
	    compidsid=`${AWK} < ${COMPIDSTABLE} -F":" \
			"/$$product/"' {print $$2; exit 0}'`; \
	    if [ "$${compidsid}" = "$${newcompid}" ] ; \
	    then \
		exit 0; \
	    else \
		if [ ! -d data ]; then ${MKDIR} data; fi; \
		${RM} -f ${.TARGET}; \
		${ECHO} "${ADEPRODID} -l ${DATA_BFF} -d data ||" \
			"${TOUCH} ${SHARE_PRODUCTID}" ; \
		${ADEPRODID} -l ${DATA_BFF} -d data || ${TOUCH} ${.TARGET}; \
	    fi; \
	else \
	    if [ ! -d data ]; then ${MKDIR} data; fi; \
	    ${RM} -f ${.TARGET}; \
	    ${ECHO} "${ADEPRODID} -l ${DATA_BFF} -d data ||" \
		"${TOUCH} ${SHARE_PRODUCTID}"; \
	    ${ADEPRODID} -l ${DATA_BFF} -d data || ${TOUCH} ${.TARGET}; \
	fi

.if (${PTF_UPDATE} == "yes")

# Rule for copying the .lp, and .il files out to the
# update tree. The target magic takes each option and figures out where
# in the update tree the file should go.  The location relative to
# ${UPDATETOP} is just the option name split with slashes instead
# of dots.  For example if the option were lpp.bar.opt the location to
# copy to would be ${UPDATETOP}/lpp/bar/opt/lpp.bar.opt.il
${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.lp@} \
${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.il@}: $${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for copying the .inv_u, and .upsize files out to the update tree.
# This is similar to the .lp and .il rule except that prior to converting
# the option to a directory we must strip off the suffix.  We do not need
# to prefix these with data. since they are not supplied for root parts and
# there will not be any share/usr name conflicts.
# For example lpp.bar.opt.inv_u goes in:
# ${UPDATETOP}/lpp/bar/opt/lpp.bar.opt.inv_u
${SHARE_INFO_FILES:M*.inv_u:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@} \
${SHARE_INFO_FILES:M*.upsize:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@}: \
		$${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for copying the [option].acf files out to the update tree.
# This is similar to the .inv_u rule.
# For example lpp.bar.opt.acf goes in:
# ${UPDATETOP}/lpp/bar/opt/lpp.bar.opt.acf
${SHARE_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@}: \
		$${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# If the inslist file has changed then we need to regenerate the xref file.
${SHARE_OPTIONS:@OPT@${XREF}/${OPT}.share.xref@}: $${.TARGET:T:R:R}.il
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${RM} -f ${.TARGET}
	${AWK} <${.ALLSRC} >${.TARGET} '/^[A-Z] *[0-9]/ \
	    {if (NF == 5 || NF == 6) print "."$$5,"${.TARGET:T:R}"}'
	@# liblppPath is the path name for the liblpp.a file.  The
	@# power/packages/ prefix is required for bldnormalize (subptf)
	@# to normalize the path name from the xref file.  The rest of
	@# the path is calculated by changing the . to / in the option
	@# name.
	liblppPath=power/packages/${.TARGET:T:R:R:S?\.?/?g} ; \
	${ECHO} $${liblppPath}/usr.liblpp.a ${.TARGET:T:R} >> ${.TARGET}

# If the inslist file has changed then we need to copy it for update
${SHARE_OPTIONS:@OPT@${INSLISTS}/${OPT}.il@}: $${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

# Rule for creating the lpp_info file in the update tree.
${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/lpp_info@}:
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${ECHO} "${LPPFILEFORMAT} ${PLATCODE} ${BFF}" >${.TARGET}.X
	${ECHO} "${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}\
		${LPPVERSION}.${LPPRELEASE}.${LPPMAINT}.${LPPFIXLEVEL}" \
		>> ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating the vrmf file.
${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@}:
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${AWK} '{if ( $$1 == pat) { if (NF >= 7) print $$2; } }' \
	    pat=${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g} ${SHARE_LPP_NAME} \
	    >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating data.liblpp.a in the update tree.    The same
# magic is applied here as for the .il and .lp files to get the
# update directory, but in this case the above lpp.bar.opt.data example
# would result in a target of ${UPDATETOP}/lpp/bar/opt/data/data.liblpp.a.
# This result makes the dependency on the lpp.bar.opt.data.copyright file
# much more difficult to compute.  From the target we have to strip off
# the first part (${UPDATETOP}/), then take off the /data.liblpp.a from
# the end, then change the slashes in what's left back to dots, and
# finally add the .copyright to the end.  The target dependency
# logic is similar to the above lpp_info logic.  There is a dependency
# problem in that we cannot depend just on the options in the
# SHARE_LIBLPP_UPDT_LIST that are for this option.  We've got to depend on
# every one of them, so if any changes then we will have to rebuild the
# data.liblpp.a even if none of the entries for this option actually was
# updated.  The same is true for the SHARE_ODMADD_LIST history files.

# SHARE_INV_U is needed because the matching :M did not work on a dependency
# line.  SHARE_INFO_FILES contains entries for all filesets.  If a
# <fileset>.inv_u file exists for this fileset and is out of date then
# rebuild data.liblpp.a.

SHARE_INV_U = ${SHARE_INFO_FILES:M${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}.inv_u:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@}

${SHARE_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/data.liblpp.a@}: \
		$${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}.copyright \
		${SHARE_PRODUCTID} \
		${SHARE_LIBLPP_UPDT_LIST} \
		$${SHARE_INV_U} \
		${SHARE_ODMADD_LIST:!${ECHO} ${SHARE_ODMADD_LIST} \
			${PRELOADED_SHARE_ODMADD_LIST} | \
			${AWK} 'BEGIN {RS=" "} \
			/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
			print "${UPDATEODMHIST}/" opt "/" addf}'!}

	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	[ ! -f ${.TARGET} ] || ${MV} -f ${.TARGET} ${.TARGET}.X
	@# The following .ALLSRC line searches the dependency list for all
	@# members that contain the same option name as the target.
	@# This is necessary because SHARE_LIBLPP_UPDT_LIST can contain
	@# entries for multiple options.  So strip off the leading $UPDATETOP
	@# from the target, use :H to remove the /data.liblpp.a from the
	@# end, change the remaining / back to dots and you have the
	@# option name to match against.  Note that it does not pick up
	@# any odm history files since they will be in the UPDATEODMHIST
	@# directory instead of UPDATETOP.

	${AR} ${DEF_ARFLAGS} ${.TARGET} ${SHARE_PRODUCTID} \
	    ${.ALLSRC:M*${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}*:N${UPDATEODMHIST}*:N*.inv_u}

	@# We must also include any .odmadd, .unodmadd, and .odmdel
	@# files that have been generated.  These most likely will have
	@# been built by a change in one of the .add files that the
	@# history .add file depends on.  The files (if they have been
	@# generated at all) should be located in the data subdirectory
	@# for the option out in the update tree.
	@if [ -d ${.TARGET:H}/data ]; then \
	cd ${.TARGET:H}/data; \
	option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	for ext in odmadd unodmadd odmdel; \
	do \
	    if [ -f $$option*.$$ext ]; \
	    then \
		list="$$list $$option*.$$ext"; \
	    fi; \
	done; \
	${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; ${FALSE}; }; \
	fi

	# Since liblpp.a files changed, indicate that it needs to be shipped
	# if it is really different than it was before.  Ignore differences
	# in the copyright file.
	@option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	if ${FGREP} -x "${.TARGET} $$option" ${UPDATELOG} >/dev/null 2>&1 ; \
	then dont_do_anything=1; \
	elif [ ! -f ${.TARGET}.X ]; \
	then \
	    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
	    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
	else \
	    ${RM} -rf tmpold tmpnew; \
	    ${MKDIR} tmpold tmpnew; \
	    ${AR} -t ${.TARGET} | ${SORT} >tmpnew/ar_t; \
	    ${AR} -t ${.TARGET}.X | ${SORT} >tmpold/ar_t; \
	    cd tmpnew; ${AR} -x ${.TARGET}; ${RM} -f $$option.copyright; \
	    cd ../tmpold; ${AR} -x ${.TARGET}.X; ${RM} -f $$option.copyright; \
	    for i in *; \
	    do \
		if ${CMP} $$i ../tmpnew/$$i; \
		then continue; \
		else \
		    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
		    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
		    break; \
		fi; \
	    done; \
	    cd ..; ${RM} -rf tmpold tmpnew; \
	fi

	${RM} -f ${.TARGET}.X


# target is ${UPDATEODMHIST}/[option]/[name.add]
# If the file doesn't exist at all then this is a brand new .add file that
# did not exist when we were building the install images, so we need to
# touch it so we're able to show that everything is different.  We are
# not only building the history .add file.  The odmadd, odmdel, and unodmadd
# files are built and placed in the update tree under ${UPDATETOP}/[option].
# For example if the odmadd_list entry is foo.add%fee.fie.fum, then we
# build ${UPDATEODMHIST}/fee.fie.fum/foo.add (the history file), and possibly
# ${UPDATETOP}/fee/fie/fum/data/foo.odmadd,
# ${UPDATETOP}/fee/fie/fum/data/foo.unodmadd, and
# ${UPDATETOP}/fee/fie/fum/data/foo.odmdel.
# In case of an error in mkodmupdate, the history .add file should not be
# updated.  This is accomplished by having mkodmupdt wait until the very
# end to update the history .add file.  There is one case though that we
# have to take care of here in the makefile.  If we touch the history .add
# file to create it and the mkodmupdt fails, then we've got to clean it up.
${SHARE_ODMADD_LIST:!${ECHO} ${SHARE_ODMADD_LIST} \
	${PRELOADED_SHARE_ODMADD_LIST} | \
	${AWK} 'BEGIN {RS=" "} \
		/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
		 print "${UPDATEODMHIST}/" opt "/" addf}'!}: \
		$${.TARGET:T}
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
.if (${BUILD_SELFIX_TREE} == "yes")
	${CP} ${.ALLSRC} ${.TARGET}.X && ${MV} -f ${.TARGET}.X ${.TARGET}
.else
	@option=${.TARGET:H:T};\
	addfile=${.ALLSRC};\
	base=`${BASENAME} $$addfile .add`;\
	compound_odm=${.TARGET:T}$$option;\
	odmadd_dir=${UPDATETOP}/${.TARGET:H:T:S?.?/?g}/data;\
	if [ ! -d $$odmadd_dir ]; then ${MKDIR} $$odmadd_dir; fi;\
	if [ -L ${.TARGET} ]; \
	then ${CP} -p ${.TARGET} ${.TARGET}.X; \
	     ${MV} -f ${.TARGET}.X ${.TARGET}; \
	fi; \
	if ${ECHO} ${GLOBAL_ODMUPDT_LIST}"" | \
	    ${GREP} $$compound_odm >/dev/null 2>&1;\
	then\
	    ${ECHO} ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option \
			-c $$addfile -d $$odmadd_dir;\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option -c $$addfile \
			-d $$odmadd_dir;\
	    ${TOUCH} ${.TARGET};\
	else\
	    if [ ! -f ${.TARGET} ];\
	    then\
	        ${ECHO} "${TOUCH} ${.TARGET}";\
	        ${TOUCH} ${.TARGET};\
		addfile_was_created_after_gold=1; \
	    else \
		addfile_was_created_after_gold=0; \
	    fi; \
	    ${ECHO} ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option \
			-c $$addfile -d $$odmadd_dir -p ${.TARGET};\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option -c $$addfile \
			-d $$odmadd_dir -p ${.TARGET} \
		|| { if [ $$addfile_was_created_after_gold = 1 ]; \
		     then ${RM} -f ${.TARGET}; fi; ${FALSE}; }; \
	fi
.endif # BUILD_SELFIX_TREE == "yes"

.endif # PTF_UPDATE == yes
.endif # defined SHARE_OPTIONS

.if defined(ROOT_OPTIONS)

.if (${BUILD_INSTALL_IMAGES} == "yes")
root/_ROOTLPPNAME_ : ${ROOT_OPTIONS:S?$?.lp?g}
	@if [ ! -d root ]; then ${MKDIR} root; fi
	${CAT} ${.ALLSRC} | ${SED} -e '/^[# 	]/d' > ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}


# Rule for root/liblpp.a.
# The root/liblpp.a file should not contain productid or copyright files.
# ASSERT ${ROOT_OPTIONS} != NULL
${ROOT_LIBLPP} : ROOT_ODM_LIST INVENTORY_LIST \
		${ROOT_LPPACF_LIST:Droot/lpp.acf} \
		${ROOT_LIBLPP_LIST:S?^?root/?g}
	@# Ensure that the root directory exists
	@if [ ! -d root ]; then ${MKDIR} root; fi

	@# Archive in the out of date root_liblpp_list files.
	${RM} -f ${.TARGET}
	members="${.ALLSRC:NROOT_ODM_LIST:NINVENTORY_LIST}" ; \
	if [ -n "$$members" ]; \
	then \
	    ${AR} ${DEF_ARFLAGS} ${.TARGET} $$members \
		|| { ${RM} -f ${.TARGET}; ${FALSE}; } \
	fi

	@# Now add in any files generated by mkodmupdt and adeinv.
	@# This code currently will archive all the optional members.
	@if [ -n ""${.ALLSRC:MROOT_ODM_LIST} -o \
	     -n ""${.ALLSRC:MINVENTORY_LIST} ] ;\
	then \
	   for option in ${ROOT_OPTIONS}; \
	   do \
	      list=""; \
	      for ext in ${OPTIONAL_MEMBER_EXT}; \
	      do \
	         if [ -f root/$$option*.$$ext ]; \
	         then \
		    list="$$list root/$$option*.$$ext"; \
	         fi; \
	      done; \
	      ${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	      ${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	   done; \
	fi

root/lpp.acf: ${ROOT_LPPACF_LIST:S?^?root/?g}
	${CAT} ${.ALLSRC} >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Refer to the comments at the begining of USR_ODM_LIST
# for an explanation of this rule.
ROOT_ODM_LIST : ${ROOT_ODMADD_LIST:!${ECHO} ${ROOT_ODMADD_LIST} | \
		${SED} "s/%[^ ]* */ /g"!}
	@${RM} -f tmpfile
	@# Run mkodmupdt on any out of date .add files.
	@${ECHO} "${ROOT_ODMADD_LIST} ^ " >tmpfile
	@${ECHO} " ${.OODATE} " >>tmpfile
	@${AWK} < tmpfile "BEGIN { RS=\" \" ; \
				mkodmupdt=\"${MKODMUPDT} -i -t ${OBJCLASSDB}\"; \
				addfile_flag = 0; n_uols = 0; \
				n_addfiles = 0 ;} "' \
	      $$0 == "^" { addfile_flag = 1 ; next } \
	      $$0 == ""	 { next } \
	      $$0 == "\n"  { next } \
	      { if ( addfile_flag ) { \
		    n_addfiles++ ; addfiles[n_addfiles] = $$0 ; \
		} \
		else { n_uols++ ; uol[n_uols] = $$0 ; }	 \
	      } \
	      END { \
		for ( i=1; i<=n_uols; i++ ) { \
		    split( uol[i], tmp, "%" ); \
		    addfn = tmp[1]; option = tmp[2]; \
		    n = split(addfn,ary,"."); \
		    for ( j=1; j<n; j++ ) base=base "." ary[j]; \
		    for ( j=1; j<=n_addfiles; j++ ) { \
			n = split( addfiles[j], ary, "/" ); \
			if ( ary[n] == addfn ) { \
			    printf( "%s -o %s -c %s -d root\n", \
				    mkodmupdt, option, addfiles[j]); \
			    system("${RM} -f root/"option base".odmadd root/" \
				   option base".unodmadd root/" option \
				   base".odmdel"); \
			    if ( 0 != system(mkodmupdt " -o " option \
					" -c " addfiles[j] " -d root ")) \
				exit 1; \
			    break; \
			}; \
		    }; \
		}; \
	      }'
	@${RM} -f tmpfile
	${RM} -f ${.TARGET} && ${TOUCH} ${.TARGET}
.endif # BUILD_INSTALL_IMAGES

.if (${PTF_UPDATE} == "yes")

# Rule for copying the lpp.acf file into the update tree if it exists.
# Note that the .inv_u, .upsize, .lp and .il files are copied over
# by the usr rules and do not need to be recopied.
# The target magic takes each option and figures out where
# in the update tree the file should go.  The location relative to
# ${UPDATETOP} is just the option name split with slashes instead
# of dots.  For example, if the option were lpp.bar.opt the location to
# copy to would be the ${UPDATETOP}/lpp/bar/opt directory.
${ROOT_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/root.${OPT}@}:  \
		root/$${.TARGET:T:S?^root.??}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating the vrmf file.
${ROOT_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@}:
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${AWK} '{if ( $$1 == pat) { if (NF >= 7) print $$2; } }' \
	    pat=${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g} LPP_NAME \
	    >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating root.liblpp.a in the update tree.  There is a dependency
# problem in that we cannot depend just on the options in the
# ROOT_LIBLPP_UPDT_LIST that are for this option.  We've got to depend on
# every one of them, so if any changes then we will have to rebuild the
# root.liblpp.a even if none of the entries for this option actually was
# updated.  The same is true for the USR_ODMADD_LIST history files.
${ROOT_OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/root.liblpp.a@}: \
		${ROOT_LIBLPP_UPDT_LIST:S?^?root/?g} \
		${ROOT_ODMADD_LIST:!${ECHO} ${ROOT_ODMADD_LIST} \
			${PRELOADED_ROOT_ODMADD_LIST} | \
			${AWK} 'BEGIN {RS=" "} \
			/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
			print "${UPDATEODMHIST}/" opt "/" addf}'!}
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	[ ! -f ${.TARGET} ] || ${MV} -f ${.TARGET} ${.TARGET}.X
	@# The following .ALLSRC line searches the dependency list for all
	@# members that contain the same option name as the target.
	@# This is necessary because ROOT_LIBLPP_UPDT_LIST can contain
	@# entries for multiple options.  So strip off the leading $UPDATETOP
	@# from the target, use :H to remove the /data.liblpp.a from the
	@# end, change the remaining / back to dots and you have the
	@# option name to match against.  Note that it does not pick up
	@# any odm history files since they will be in the UPDATEODMHIST
	@# directory instead of UPDATETOP.
	${AR} ${DEF_ARFLAGS} ${.TARGET} \
	    ${.ALLSRC:M*${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}*:N${UPDATEODMHIST}*}

	@# We must also include any .odmadd, .unodmadd, and .odmdel
	@# files that have been generated.  These most likely will have
	@# been built by a change in one of the .add files that the
	@# history .add file depends on.  The files (if they have been
	@# generated at all) should be located in the root subdirectory
	@# for the option out in the update tree.
	@if [ -d ${.TARGET:H}/root ]; then \
	cd ${.TARGET:H}/root; \
	option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	for ext in odmadd unodmadd odmdel; \
	do \
	    if [ -f $$option*.$$ext ]; \
	    then \
		list="$$list $$option*.$$ext"; \
	    fi; \
	done; \
	${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; ${FALSE}; }; \
	fi

	# Since liblpp.a files changed, indicate that it needs to be shipped
	# if it is really different than it was before.  Ignore differences
	# in the copyright file.
	@option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	if ${FGREP} -x "${.TARGET} $$option" ${UPDATELOG} >/dev/null 2>&1 ; \
	then dont_do_anything=1; \
	elif [ ! -f ${.TARGET}.X ]; \
	then \
	    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
	    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
	else \
	    ${RM} -rf tmpold tmpnew; \
	    ${MKDIR} tmpold tmpnew; \
	    ${AR} -t ${.TARGET} | ${SORT} >tmpnew/ar_t; \
	    ${AR} -t ${.TARGET}.X | ${SORT} >tmpold/ar_t; \
	    cd tmpnew; ${AR} -x ${.TARGET}; ${RM} -f $$option.copyright; \
	    cd ../tmpold; ${AR} -x ${.TARGET}.X; ${RM} -f $$option.copyright; \
	    for i in *; \
	    do \
		if ${CMP} $$i ../tmpnew/$$i; \
		then continue; \
		else \
		    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
		    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
		    break; \
		fi; \
	    done; \
	    cd ..; ${RM} -rf tmpold tmpnew; \
	fi

	${RM} -f ${.TARGET}.X


# target is ${UPDATEODMHIST}/[option]/[name.add]
# If the file doesn't exist at all then this is a brand new .add file that
# did not exist when we were building the install images, so we need to
# touch it so we're able to show that everything is different.  We are
# not only building the history .add file.  The odmadd, odmdel, and unodmadd
# files are built and placed in the update tree under ${UPDATETOP}/[option].
# For example if the odmadd_list entry is foo.add%fee.fie.fum, then we
# build ${UPDATEODMHIST}/fee.fie.fum/foo.add (the history file), and possibly
# ${UPDATETOP}/fee/fie/fum/root/foo.odmadd,
# ${UPDATETOP}/fee/fie/fum/root/foo.unodmadd, and
# ${UPDATETOP}/fee/fie/fum/root/foo.odmdel.
# In case of an error in mkodmupdate, the history .add file should not be
# updated.  This is accomplished by having mkodmupdt wait until the very
# end to update the history .add file.  There is one case though that we
# have to take care of here in the makefile.  If we touch the history .add
# file to create it and the mkodmupdt fails, then we've got to clean it up.
${ROOT_ODMADD_LIST:!${ECHO} ${ROOT_ODMADD_LIST} \
	${PRELOADED_ROOT_ODMADD_LIST} | \
	${AWK} 'BEGIN {RS=" "} \
	/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
	print "${UPDATEODMHIST}/" opt "/" addf}'!}: \
		$${.TARGET:T} 
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
.if (${BUILD_SELFIX_TREE} == "yes")
	${CP} ${.ALLSRC} ${.TARGET}.X && ${MV} -f ${.TARGET}.X ${.TARGET}
.else
	@option=${.TARGET:H:T};\
	addfile=${.ALLSRC};\
	base=`${BASENAME} $$addfile .add`;\
	compound_odm=${.TARGET:T}%$$option;\
	odmadd_dir=${UPDATETOP}/${.TARGET:H:T:S?.?/?g}/root;\
	if [ ! -d $$odmadd_dir ]; then ${MKDIR} $$odmadd_dir; fi;\
	if [ -L ${.TARGET} ]; \
	then ${CP} -p ${.TARGET} ${.TARGET}.X; \
	     ${MV} -f ${.TARGET}.X ${.TARGET}; \
	fi; \
	if ${ECHO} ${GLOBAL_ODMUPDT_LIST}"" | \
	    ${GREP} $$compound_odm >/dev/null 2>&1;\
	then\
	    ${ECHO} ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option \
			-c $$addfile -d $$odmadd_dir;\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option -c $$addfile \
			-d $$odmadd_dir;\
	    ${TOUCH} ${.TARGET};\
	else\
	    if [ ! -f ${.TARGET} ];\
	    then\
	        ${ECHO} "${TOUCH} ${.TARGET}";\
	        ${TOUCH} ${.TARGET};\
		addfile_was_created_after_gold=1; \
	    else \
		addfile_was_created_after_gold=0; \
	    fi; \
	    ${ECHO} ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option \
			-c $$addfile -d $$odmadd_dir -p ${.TARGET};\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option -c $$addfile \
			-d $$odmadd_dir -p ${.TARGET} \
		|| { if [ $$addfile_was_created_after_gold = 1 ]; \
		     then ${RM} -f ${.TARGET}; fi; ${FALSE}; }; \
	fi
.endif # BUILD_SELFIX_TREE == "yes"

.endif # PTF_UPDATE == yes
.endif # defined ROOT_OPTIONS

.if defined(OPTIONS)

.if (${BUILD_INSTALL_IMAGES} == "yes")
# The .bff file is the entry point to this build for the usr and
# root parts.  All the input files that ade_package requires are
# generated by the make of the LPP_NAME's with the exception of
# the INSLISTS.
# The changevrmf tool is run to just before adepackage to change the 
# vrmf number of filesets that have not changed to 4.1.0.0. 
.if defined(ROOT_OPTIONS)
${BFF} : INSLISTS LPP_NAME root/_ROOTLPPNAME_ ${ROOT_LIBLPP}
.else
${BFF} : INSLISTS LPP_NAME
.endif # defined(ROOT_OPTIONS)
	${CHANGEVRMF} -f LPPNAME -l LPP_NAME;
	${RM} -f lpp_name
	${CP} LPP_NAME lpp_name
	${ADEPACKAGE} ${ADEPACKAGEFLAGS} ${STRICTFLAG} ${LINKFLAG} \
		-s "${SHIP_PATH}" -f ${.TARGET}.X -i INSLISTS -l ${BFF}
	${MV} -f ${.TARGET}.X ${.TARGET}
	${RM} -f lpp_name

INSLISTS: ${OPTIONS:S?$?.il?g}
	@# creating INSLISTS file from .il files.  This used to be
	@# done with cat .ALLSRC > .TARGET but the arg list got too
	@# long, so we're trying to split it up based on
	@# ${PKGSRCTOP}.  First we strip the directory off the
	@# front of each file in .ALLSRC that we can, then we add it
	@# back when we do the cat.  Then we cat everything that
	@# didn't match PKGSRCTOP.	
	${RM} -f ${.TARGET}
	@for i in ""${.ALLSRC:M${PKGSRCTOP}*:S?${PKGSRCTOP}??g}; \
	do\
	    if [ -n "$$i" ]; \
	    then \
	        ${ECHO} "${CAT} ${PKGSRCTOP}$$i >>${.TARGET}"; \
	        ${CAT} ${PKGSRCTOP}$$i >>${.TARGET} \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	    fi; \
	done; \
	for i in ""${.ALLSRC:N${PKGSRCTOP}*}; \
	do\
	    if [ -n "$$i" ]; \
	    then \
	        ${ECHO} "${CAT} $$i >>${.TARGET}"; \
	        ${CAT} $$i >>${.TARGET} || { ${RM} -f ${.TARGET}; exit 1; }; \
	    fi; \
	done

# LPP_NAME has hidden dependencies on the following files:
# - [option].size	which is generated by the liblpp.a dependency
# - root/[option].size	which is also generated by the liblpp.a dependency.
# There is still an open question about whether the prereq file should be
# copied up to the current directory, or whether we need to change the tool
# to use it from the usr directory.
LPP_NAME :	${LIBLPP} \
		${INFO_FILES:M*.prereq:S?^?usr/?g} \
		${INFO_FILES:M*.supersede:S?^?usr/?g} \
		${OPTIONS:S?$?.lp?g}
	@# Make the LPPNAME file from the all the .lp files.
	${CAT} ${.ALLSRC:M*.lp} | ${SED} -e '/^[# 	]/d' > LPPNAME.X
	${MV} -f LPPNAME.X LPPNAME

	@# Create the LPP_NAME file
	${ADELPPNAME} ${LPPNAMEFLAGS} -l ${BFF} -u LPPNAME -d usr \
		      -o ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}


# Rule for liblpp.a.
# ASSERT ${OPTIONS} != NULL
${LIBLPP} :	OPTIONAL_MEMBERS \
		${OPTIONS:@BASE@usr/${BASE}.copyright@} \
		${USR_LIBLPP_LIST:S?^?usr/?g} \
		${USR_LPPACF_LIST:Dusr/lpp.acf} \
		${USR_PRODUCTID}

	@# Ensure that the usr directory exists
	@if [ ! -d usr ]; then ${MKDIR} usr; fi

	@# Archive in the productid and the out of date members explicitly
	@# listed in the dependency line.
	${RM} -f ${.TARGET}
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${.ALLSRC:NOPTIONAL_MEMBERS} \
		|| { ${RM} -f ${.TARGET}; ${FALSE}; }

	# Now add in any files generated by mkodmupdt and adeinv.
	@# This code currently will archive all the optional members.
	@if [ -n ""${.ALLSRC:MOPTIONAL_MEMBERS} ] ;\
	then \
	   for option in ${OPTIONS}; \
	   do \
	      list=""; \
	      for ext in ${OPTIONAL_MEMBER_EXT}; \
	      do \
	         if [ -f usr/$$option*.$$ext ]; \
	         then \
		    list="$$list usr/$$option*.$$ext"; \
	         fi; \
	      done; \
	      ${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	      ${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	   done; \
	fi

usr/lpp.acf: ${USR_LPPACF_LIST:S?^?usr/?g}
	${CAT} ${.ALLSRC} >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

OPTIONAL_MEMBERS: ODM_LIST INVENTORY_LIST
	${TOUCH} ${.TARGET}.X && ${MV} -f ${.TARGET}.X ${.TARGET}


# The USR_ODMADD_LIST in the packdep.mk file has the following format:
# USR_ODMADD_LIST = file1%option1 file2%option2 file3%option3
# This rule performs the following functions:
# - Generate .ALLSRC by stripping option name from the list
# - Copy USR_ODMADD_LIST into temporary file
# - put "^" seperator into temporary file
# - Copy out of date filenames into temporary file
# - Read the temporary file, copying USR_ODMADD_LIST entries into
#   odmlist array and .OODATE entries into addfile array
# - For each file in addfile find its option name in odmlist and
#   invoke mkodmupdt with file and option name.
# Note: The temporary file implementation was required because the
# USR_ODMADD_LIST for bos.dev.printer package exceeded awk
# buffer size.
# For updates to work we must copy the original .add file out to the
# update history directory.  The odm update history directory is:
# ${UPDATEODMHIST}/[option]/
ODM_LIST :	${USR_ODMADD_LIST:!${ECHO} ${USR_ODMADD_LIST} | \
		${SED} "s/%[^ ]* */ /g"!}
	@${RM} -f tmpfile
	@# Run mkodmupdt on any out of date .add files.
	@${ECHO} "${USR_ODMADD_LIST} ^ " >tmpfile
	@${ECHO} " ${.OODATE} " >>tmpfile
	@${AWK} < tmpfile "BEGIN { RS=\" \" ; \
				mkodmupdt=\"${MKODMUPDT} -i -t ${OBJCLASSDB}\"; \
				addfile_flag = 0; n_uols = 0; \
				n_addfiles = 0 ;} "' \
	      $$0 == "^" { addfile_flag = 1 ; next } \
	      $$0 == ""	 { next } \
	      $$0 == "\n"  { next } \
	      { if ( addfile_flag ) { \
		    n_addfiles++ ; addfiles[n_addfiles] = $$0 ; \
		} \
		else { n_uols++ ; uol[n_uols] = $$0 ; }	 \
	      } \
	      END { \
		for ( i=1; i<=n_uols; i++ ) { \
		    split( uol[i], tmp, "%" ); \
		    addfn = tmp[1]; option = tmp[2]; \
		    n = split(addfn,ary,"."); \
		    for ( j=1; j<n; j++ ) base=base "." ary[j]; \
		    for ( j=1; j<=n_addfiles; j++ ) { \
			n = split( addfiles[j], ary, "/" ); \
			if ( ary[n] == addfn ) { \
			    printf( "%s -o %s -c %s -d usr\n", \
				    mkodmupdt, option, addfiles[j]); \
			    system("${RM} -f usr/" option base".odmadd usr/" \
				   option base".unodmadd usr/" option \
				   base".odmdel"); \
			    if ( 0 != system(mkodmupdt " -o " option \
					" -c " addfiles[j] " -d usr ")) \
				exit 1; \
			    break; \
			}; \
		    }; \
		}; \
	      }'
	@${RM} -f tmpfile
	${RM} -f ${.TARGET} && ${TOUCH} ${.TARGET}

# We must copy the usr insize file over to the root directory
# so it is available to adeinv.
${INFO_FILES:M*.insize:S?^?root/?g}: $${.TARGET:S?^root/?usr/?g}
	@if [ ! -d root ]; then ${MKDIR} root; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Runs adeinv on all the OPTIONS and ROOT_OPTIONS to produce the
# corresponding [option].inventory, [option].size, [option].al, and
# [option].tcb files for both the usr and root parts.
INVENTORY_LIST: ${OPTIONS:@BASE@${BASE}.il ${BASE}.lp@} \
		${USR_LPPACF_LIST:S?^?usr/?g} \
		${ROOT_LPPACF_LIST:S?^?root/?g} \
		${INFO_FILES:M*.insize:S?^?usr/?g} \
		${INFO_FILES:M*.insize:S?^?root/?g} \
		${OPTIONS:@FS@${PKGOBJTOP}/${FS:S?.?/?g}/shipfile_dependency@}
.if defined(ROOT_OPTIONS)
	@# We wouldn't have to do this if adeinv would create the directory.
	@if [ ! -d root ]; then ${MKDIR} root; fi
.endif
	@if [ ! -d usr ]; then ${MKDIR} usr; fi
	@# .ALLSRC got too long so we need to create a tmpfile in two
	@# steps, similar to what was done with the INSLISTS.  First
	@# step is to strip off PKGSRCTOP from every file in .ALLSRC
	@# that starts with PKGSRCTOP, echo that to awk where we put
	@# the directory portion back in the front of each file.
	@# Then echo out the files that didn't match PKGSRCTOP.
	@${RM} -f tmpfile
	@${ECHO} ${.ALLSRC:M${PKGSRCTOP}*:S?${PKGSRCTOP}??g} \
	    | ${AWK} 'BEGIN { RS=" " } {print "${PKGSRCTOP}"$$0}' >tmpfile
	@${ECHO} ${.ALLSRC:N${PKGSRCTOP}*} \
	    | ${AWK} 'BEGIN { RS=" " } {print $$0}' >>tmpfile
	@# Run adeinv on any out of date .il, .lp, or .insize files.
	@# Determine the out of date options by taking the basename
	@# of the out of date file and stripping off the extension.
	@# Then pull off the fully qualified .il and .lp
	@# files from .ALLSRC to send as input to adeinv for both
	@# the usr and root parts.
	@# The liblpp.a and root/liblpp.a need to be added to the
	@# xref file for future update processing.
	@# Note that we only run adeinv on the root part for options
	@# that have a root part (i.e. the option shows up in the
	@# ROOT_OPTIONS list).
	@# We also use tmpfile to find out if we need to pass an
	@# input .acf file to adeinv to allow it to find libraries
	@# to properly determine the sizes.
	@# liblppPath is the path name for the liblpp.a file.  The
	@# power/packages/ prefix is required for bldnormalize to generate
	@# the normalized path name from the xref file.  The rest of
	@# the path is calculated by changing the . to / in the option
	@# name.
	@oods=`${ECHO} ${.OODATE:T:R:N*shipfile_dependency} \
		${.OODATE:M*shipfile_dependency:H:S?${PKGOBJTOP}/??g:S?/?.?g} | \
		${AWK} 'BEGIN { RS=" " } {print $$0}' | ${SORT} -u`; \
	for i in ""$$oods; \
	do \
	    ilfile=`${GREP} $$i.il tmpfile`; \
	    lpfile=`${GREP} $$i.lp tmpfile`; \
	    if ${GREP} usr/$$i.acf tmpfile >/dev/null 2>&1; \
	    then acfopt="-a `${GREP} usr/$$i.acf tmpfile`"; \
	    else acfopt=""; \
	    fi; \
	    ${ECHO} ${ADEINVENTORY} ${ADEINVENTORYFLAGS} ${STRICTFLAG} \
			    ${LINKFLAG} \
			    -d usr -l ${BFF} -i $$ilfile -u $$lpfile \
			    -s "${SHIP_PATH}" $$acfopt; \
	    ${RM} -f $$i.al $$i.tcb $$i.inventory $$i.size $$i.xref; \
	    ${ADEINVENTORY} ${ADEINVENTORYFLAGS} ${STRICTFLAG} ${LINKFLAG} \
			    -d usr -l ${BFF} -i $$ilfile -u $$lpfile \
			    -s "${SHIP_PATH}" $$acfopt || exit 1; \
	    liblppPath=power/packages/`${ECHO} $$i | ${SED} -e "s?\.?/?g"`; \
	    ${ECHO} $${liblppPath}/usr.liblpp.a $$i >> usr/$$i.xref; \
	    if ${ECHO} ${ROOT_OPTIONS}"" | ${GREP} $$i >/dev/null 2>&1;\
	    then \
	        if ${GREP} root/$$i.acf tmpfile >/dev/null 2>&1; \
	        then acfopt="-a `${GREP} root/$$i.acf tmpfile`"; \
	        else acfopt=""; \
	        fi; \
		${ECHO} ${ADEINVENTORY}  ${ADEINVENTORYFLAGS} ${STRICTFLAG} \
				${ROOTFLAG} \
				${LINKFLAG} -l ${BFF} -d root \
				-i $$ilfile \
				-u $$lpfile \
				-s "${SHIP_PATH}" $$acfopt; \
		${RM} -f $$i.al $$i.tcb $$i.inventory $$i.size $$i.xref; \
		${ADEINVENTORY}  ${ADEINVENTORYFLAGS} ${STRICTFLAG} \
				${ROOTFLAG} \
				${LINKFLAG} -l ${BFF} -d root \
				-i $$ilfile \
				-u $$lpfile \
				-s "${SHIP_PATH}" $$acfopt || exit 1; \
		liblppPath=power/packages/`${ECHO} $$i | ${SED} -e "s?\.?/?g"`; \
		${ECHO} $${liblppPath}/root.liblpp.a $$i >> usr/$$i.xref; \
	    fi; \
	done
	@${RM} -f tmpfile
	${TOUCH} ${.TARGET}.X && ${MV} -f ${.TARGET}.X ${.TARGET}
.endif # BUILD_INSTALL_IMAGES

# Create the .copyright files from the .cr files.
# Note that the target depends on the .cr file in the current directory,
# not in the usr subdirectory.
${OPTIONS:@BASE@usr/${BASE}.copyright@}: $${.TARGET:T:R}.cr \
		${PKGOBJTOP}/$${.TARGET:T:R:S?.?/?g}/shipfile_dependency
	@if [ ! -d usr ]; then ${MKDIR} usr; fi
	${ADECOPYRIGHT} ${ADECRFLAGS} -l ${BFF} ${${.TARGET:T:R}.cr:P} \
	    > ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Create the product id file in the usr subdirectory.  If for
# some reason adeprodid fails then dummy up a usr/productid file,
# for as we said before THE BUILD MUST GO ON.
# The productid file depends on the compids.table file. If
# the component id in productid file and compids.table file
# do not match then we need to create productid file with 
# new information in it else we should leave it as such.
${USR_PRODUCTID}: ${COMPIDSTABLE}
	@if [ -s ${USR_PRODUCTID} ] ; \
	then \
	    ${CAT} ${USR_PRODUCTID} | read product compid; \
	    newcompid=`${ECHO} $${compid} | ${AWK} -F"-" '{print $$1 $$2}'`; \
	    compidsid=`${AWK} < ${COMPIDSTABLE} -F":" \
			"/$$product/"' {print $$2; exit 0}'`; \
	    if [ "$${compidsid}" = "$${newcompid}" ] ; \
	    then \
		exit 0; \
	    else \
		if [ ! -d usr ]; then ${MKDIR} usr; fi; \
		${RM} -f ${.TARGET}; \
		${ECHO} ${ADEPRODID} -l ${BFF} -d usr || \
		    ${TOUCH} ${USR_PRODUCTID}; \
		${ADEPRODID} -l ${BFF} -d usr || ${TOUCH} ${USR_PRODUCTID}; \
	    fi; \
	else \
	    if [ ! -d usr ]; then ${MKDIR} usr; fi; \
	    ${RM} -f ${.TARGET}; \
	    ${ECHO} ${ADEPRODID} -l ${BFF} -d usr || ${TOUCH} ${USR_PRODUCTID}; \
	    ${ADEPRODID} -l ${BFF} -d usr || ${TOUCH} ${USR_PRODUCTID}; \
	fi

.if (${PTF_UPDATE} == "yes")

# Rule for copying the .lp, and .il files out to the
# update tree. The target magic takes each option and figures out where
# in the update tree the file should go.  The location relative to
# ${UPDATETOP} is just the option name split with slashes instead
# of dots.  For example if the option were lpp.bar.opt the location to
# copy to would be ${UPDATETOP}/lpp/bar/opt/lpp.bar.opt.il
${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.lp@} \
${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/${OPT}.il@}: $${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for copying the .inv_u, and .upsize files out to the update tree.
# This is similar to the .lp and .il rule except that prior to converting
# the option to a directory we must strip off the suffix.  We do not need
# to prefix these with usr. since they are not supplied for root parts and
# there will not be any share/usr name conflicts.
# For example lpp.bar.opt.inv_u goes in:
# ${UPDATETOP}/lpp/bar/opt/lpp.bar.opt.inv_u
${INFO_FILES:M*.inv_u:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@} \
${INFO_FILES:M*.upsize:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@}: \
		usr/$${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for copying the [option].acf files out to the update tree.
# This is similar to the .inv_u rule except that we need to prefix the
# option with "usr." in case of a name conflict between root and usr
# .acf files.  For example lpp.bar.opt.acf goes in:
# ${UPDATETOP}/lpp/bar/opt/usr.lpp.bar.opt.acf
${USR_LPPACF_LIST:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/usr.${OPT}@}:  \
		usr/$${.TARGET:T:S?^usr.??}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${CP} ${.ALLSRC} ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# If the inslist file has changed then we need to regenerate the xref file.
${OPTIONS:@OPT@${XREF}/${OPT}.usr.xref@}: $${.TARGET:T:R:R}.il
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${RM} -f ${.TARGET}
	${AWK} <${.ALLSRC} >${.TARGET} '/^[A-Z] *[0-9]/ \
	    {if (NF == 5 || NF == 6) print "."$$5,"${.TARGET:T:R}"}'
	@# liblppPath is the path name for the liblpp.a file.  The
	@# power/packages/ prefix is required for bldnormalize (subptf)
	@# to normalize the path name from the xref file.  The rest of
	@# the path is calculated by changing the . to / in the option
	@# name.
	liblppPath=power/packages/${.TARGET:T:R:R:S?\.?/?g} ; \
	${ECHO} $${liblppPath}/usr.liblpp.a ${.TARGET:T:R} >> ${.TARGET}

# If the inslist file has changed then we need to copy it for update
${OPTIONS:@OPT@${INSLISTS}/${OPT}.il@}: $${.TARGET:T}
	@# Create the target directory if it doesn't exist.
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${RM} -f ${.TARGET}
	${CP} ${.ALLSRC} ${.TARGET}

# Rule for creating the vrmf file.
${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/vrmfFile@}:
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${AWK} '{if ( $$1 == pat) { if (NF >= 7) print $$2; } }' \
	    pat=${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g} LPP_NAME \
	    >${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating the lpp_info file in the update tree.
# The format of the file is:
# LPPFILEFORMAT PLATCODE BFF
# OPTION
# the OPTION must be followed by a blank for processPtf to pick it out
# correctly.
${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/lpp_info@}:
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	${ECHO} "${LPPFILEFORMAT} ${PLATCODE} ${BFF}" \
		>${.TARGET}.X
	${ECHO} "${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}\
		${LPPVERSION}.${LPPRELEASE}.${LPPMAINT}.${LPPFIXLEVEL}" \
		>> ${.TARGET}.X
	${MV} -f ${.TARGET}.X ${.TARGET}

# Rule for creating usr.liblpp.a in the update tree.    The same
# magic is applied here as for the .il and .lp files to get the
# update directory, but in this case the above lpp.bar.opt.data example
# would result in a target of ${UPDATETOP}/lpp/bar/opt/usr.liblpp.a.
# This result makes the dependency on the lpp.bar.opt.copyright file
# much more difficult to compute.  From the target we have to strip off
# the first part (${UPDATETOP}/), then take off the /usr.liblpp.a from
# the end, then change the slashes in what's left back to dots, and
# finally add the .copyright to the end.  There is a dependency
# problem in that we cannot depend just on the options in the
# USR_LIBLPP_UPDT_LIST that are for this option.  We've got to depend on
# every one of them, so if any changes then we will have to rebuild the
# usr.liblpp.a even if none of the entries for this option actually was
# updated.  The same is true for the USR_ODMADD_LIST history files.

# USR_INV_U is needed because the matching :M did not work on a dependency
# line.  SHARE_INFO_FILES contains entries for all filesets.  If a
# <fileset>.inv_u file exists for this fileset and is out of date then
# rebuild data.liblpp.a.

USR_INV_U = ${INFO_FILES:M${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}.inv_u:@OPT@${UPDATETOP}/${OPT:R:S?.?/?g}/${OPT}@}

${OPTIONS:@OPT@${UPDATETOP}/${OPT:S?.?/?g}/usr.liblpp.a@}: \
		usr/$${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}.copyright \
		${USR_PRODUCTID} \
		${USR_LIBLPP_UPDT_LIST:S?^?usr/?g} \
		$${USR_INV_U} \
		${USR_ODMADD_LIST:!${ECHO} ${USR_ODMADD_LIST} \
			${PRELOADED_USR_ODMADD_LIST} | \
			${AWK} 'BEGIN {RS=" "} \
			/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
			print "${UPDATEODMHIST}/" opt "/" addf}'!}

	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
	[ ! -f ${.TARGET} ] || ${MV} -f ${.TARGET} ${.TARGET}.X
	@# The following .ALLSRC line searches the dependency list for all
	@# members that contain the same option name as the target.
	@# This is necessary because USR_LIBLPP_UPDT_LIST can contain
	@# entries for multiple options.  So strip off the leading $UPDATETOP
	@# from the target, use :H to remove the /data.liblpp.a from the
	@# end, change the remaining / back to dots and you have the
	@# option name to match against.  Note that it does not pick up
	@# any odm history files since they will be in the UPDATEODMHIST
	@# directory instead of UPDATETOP.  Also exclude the inv_u file.
	${AR} ${DEF_ARFLAGS} ${.TARGET} ${USR_PRODUCTID} \
	    ${.ALLSRC:M*${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}*:N${UPDATEODMHIST}*:N*.inv_u}

	@# We must also include any .odmadd, .unodmadd, and .odmdel
	@# files that have been generated.  These most likely will have
	@# been built by a change in one of the .add files that the
	@# history .add file depends on.  The files (if they have been
	@# generated at all) should be located in the usr subdirectory
	@# for the option out in the update tree.
	@if [ -d ${.TARGET:H}/usr ]; then \
	cd ${.TARGET:H}/usr; \
	option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	for ext in odmadd unodmadd odmdel; \
	do \
	    if [ -f $$option*.$$ext ]; \
	    then \
		list="$$list $$option*.$$ext"; \
	    fi; \
	done; \
	${ECHO} "${AR} ${DEF_ARFLAGS} ${.TARGET} $$list"; \
	${AR} ${DEF_ARFLAGS} ${.TARGET} $$list \
		    || { ${RM} -f ${.TARGET}; exit 1; }; \
	fi

	# Since liblpp.a files changed, indicate that it needs to be shipped
	# if it is really different than it was before.  Ignore differences
	# in the copyright file.
	@option="${.TARGET:S?${UPDATETOP}/??:H:S?/?.?g}"; \
	if ${FGREP} -x "${.TARGET} $$option" ${UPDATELOG} >/dev/null 2>&1 ; \
	then dont_do_anything=1; \
	elif [ ! -f ${.TARGET}.X ]; \
	then \
	    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
	    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
	else \
	    ${RM} -rf tmpold tmpnew; \
	    ${MKDIR} tmpold tmpnew; \
	    ${AR} -t ${.TARGET} | ${SORT} >tmpnew/ar_t; \
	    ${AR} -t ${.TARGET}.X | ${SORT} >tmpold/ar_t; \
	    cd tmpnew; ${AR} -x ${.TARGET}; ${RM} -f $$option.copyright; \
	    cd ../tmpold; ${AR} -x ${.TARGET}.X; ${RM} -f $$option.copyright; \
	    for i in *; \
	    do \
		if ${CMP} $$i ../tmpnew/$$i; \
		then continue; \
		else \
		    ${ECHO} "echo ${.TARGET} $$option >> ${UPDATELOG}"; \
		    ${ECHO} "${.TARGET} $$option" >> ${UPDATELOG}; \
		    break; \
		fi; \
	    done; \
	    cd ..; ${RM} -rf tmpold tmpnew; \
	fi

	${RM} -f ${.TARGET}.X


# target is ${UPDATEODMHIST}/[option]/[name.add]
# If the file doesn't exist at all then this is a brand new .add file that
# did not exist when we were building the install images, so we need to
# touch it so we're able to show that everything is different.  We are
# not only building the history .add file.  The odmadd, odmdel, and unodmadd
# files are built and placed in the update tree under ${UPDATETOP}/[option].
# For example if the odmadd_list entry is foo.add%fee.fie.fum, then we
# build ${UPDATEODMHIST}/fee.fie.fum/foo.add (the history file), and possibly
# ${UPDATETOP}/fee/fie/fum/usr/foo.odmadd,
# ${UPDATETOP}/fee/fie/fum/usr/foo.unodmadd, and
# ${UPDATETOP}/fee/fie/fum/usr/foo.odmdel.
# In case of an error in mkodmupdate, the history .add file should not be
# updated.  This is accomplished by having mkodmupdt wait until the very
# end to update the history .add file.  There is one case though that we
# have to take care of here in the makefile.  If we touch the history .add
# file to create it and the mkodmupdt fails, then we've got to clean it up.
${USR_ODMADD_LIST:!${ECHO} ${USR_ODMADD_LIST} \
	${PRELOADED_USR_ODMADD_LIST} | \
	${AWK} 'BEGIN {RS=" "} \
		/%/ {n=split($$0,tmp,"%"); addf=tmp[1]; opt=tmp[2];\
		 print "${UPDATEODMHIST}/" opt "/" addf}'!}:  $${.TARGET:T}
	@if [ ! -d ${.TARGET:H} ]; then ${MKDIR} -p ${.TARGET:H}; fi
.if (${BUILD_SELFIX_TREE} == "yes")
	${CP} ${.ALLSRC} ${.TARGET}.X && ${MV} -f ${.TARGET}.X ${.TARGET}
.else
	@option=${.TARGET:H:T};\
	addfile=${.ALLSRC};\
	base=`${BASENAME} $$addfile .add`;\
	compound_odm=${.TARGET:T}%$$option;\
	odmadd_dir=${UPDATETOP}/${.TARGET:H:T:S?.?/?g}/usr;\
	if [ ! -d $$odmadd_dir ]; then ${MKDIR} $$odmadd_dir; fi;\
	if [ -L ${.TARGET} ]; \
	then ${CP} -p ${.TARGET} ${.TARGET}.X; \
	     ${MV} -f ${.TARGET}.X ${.TARGET}; \
	fi; \
	if ${ECHO} ${GLOBAL_ODMUPDT_LIST}"" | \
	    ${GREP} $$compound_odm >/dev/null 2>&1;\
	then\
	    ${ECHO} ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option \
			-c $$addfile -d $$odmadd_dir;\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -u -t ${OBJCLASSDBGLOBAL} -o $$option -c $$addfile \
			-d $$odmadd_dir;\
	    ${TOUCH} ${.TARGET};\
	else\
	    if [ ! -f ${.TARGET} ];\
	    then\
	        ${ECHO} "${TOUCH} ${.TARGET}";\
	        ${TOUCH} ${.TARGET};\
		addfile_was_created_after_gold=1; \
	    else \
		addfile_was_created_after_gold=0; \
	    fi; \
	    ${ECHO} ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option \
			-c $$addfile -d $$odmadd_dir -p ${.TARGET};\
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmadd; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.odmdel; \
	    ${RM} -f $$odmadd_dir/$$option.$$base.unodmadd; \
	    ${MKODMUPDT} -t ${OBJCLASSDBUNIQ} -o $$option -c $$addfile \
			-d $$odmadd_dir -p ${.TARGET} \
		|| { if [ $$addfile_was_created_after_gold = 1 ]; \
		     then ${RM} -f ${.TARGET}; fi; ${FALSE}; }; \
	fi
.endif # BUILD_SELFIX_TREE == "yes"
.endif # PTF_UPDATE == yes
.endif # defined OPTIONS

.endif # defined _OSF_PACKAGES_MK_
