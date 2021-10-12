#!/bin/ksh
# @(#)47	1.63  src/bldenv/pkgtools/ptfpkg.sh, pkgtools, bos41J, 9524C_all  6/12/95  16:20:19
#
# COMPONENT_NAME: (PKGTOOLS) BAI Build Tools
#
# FUNCTIONS: ptfpkg
#            Clean_Up
#            Usage
#            create_ptf_image
#            gen_boot_info
#	     get_boot_value
#            get_comment
#            get_lppname_info
#            process_ptfs
#            process_root_part
#            repackage
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: Clean_Up
#
# FUNCTION: Clean up after running.
#
# INPUT: DEFECTAPARS (global) -
#        TMPFILE (global) -
#
# OUTPUT: None.
#
# SIDE EFFECTS: None.
#
# RETURNS: 0 always.
#
function Clean_Up
{
   [[ "${LPP_locked}" = "${TRUE}" ]] && bldlock -u "${LPP} ${BLDCYCLE}"

   rm -f ${DEFECTAPARS} ${TMPFILE} ${TMPMAINTLEVELFILE} > /dev/null 2>&1
   rm -f ${TMPVRMFFILE} ${PTFREQSLIST} ${TMPPTFOPTFILE} > /dev/null 2>&1
   rm -f ${BOOTFILES} ${BOOTLIBS} > /dev/null 2>&1
   return 0
}

#
# NAME: Usage
#
# FUNCTION: Usage statement.
#
# INPUT: None.
#
# OUTPUT: None.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
function Usage
{
   print -u2 "Usage: ${command}  -c -b [ -m mediatype] [ -d outputDirectory ]"
   print -u2 "\twhere the -c and/or -b option is required."
   return 0
}

#
# NAME: create_ccss_image
#
# FUNCTION: Generate a ccss format ptf image from a bff (backup format file).
#
# INPUT: bff image in current directory.
#	 PTF (global)
#        LABEL_TABLE (set in bldinit)
#
# OUTPUT: ccss image named ${PTF}.ptf
#
# SIDE EFFECTS:  Upon successful completion the .bff is removed
#
# RETURNS:  Fatally exits if errors are detected.  Otherwise, return 0.
#
function create_ccss_image
{
    #----------------------------------------------------------
    # 1st define the various variables/commands that
    #     will be used
    #----------------------------------------------------------
    typeset TOC="${PTF}.toc"            # TOC fragment for image
    typeset infoFile="${PTF}.infofile"  # Info file
    typeset BFF="${PTF}.bff"            # File containing PTF.
    typeset ccssFile="${PTF}.ptf"       # File containing CCSS image.

    typeset GENTOC="gen_toc_entry"      # Cmd to generate toc file
    typeset GENINFO="gen_infofile"      # Cmd to generate the info file
    typeset CCSS_PACK="ccss_pack"       # Cmd to create ccss image.
    typeset rc

    rm -f ${TOC} ${INFOFILE}            # Always start from scratch.

    if [[ -z "${OUTPUTDIR}" ]]
    then
        ${GENTOC} -b ${BFF} -t ${TOC}       # create TOC section in a file
    else
	${GENTOC} -b "${OUTPUTDIR}/${BFF}" -t ${TOC}
    fi

    rc=$?
    if [ $rc -ne 0 ]
    then
        $log -e "gen_toc_entry ended with code of $rc "
    else
        #--------------------------------------------------
        # IFF everything OK so far, generate the CCSS
        #     info in a file.
        #--------------------------------------------------
	if [[ -z "${OUTPUTDIR}" ]]
	then
            ${GENINFO} -i ${infoFile} -t ${COMPIDS} -p ${PTF} \
                   -o ${PTFOPTIONS} -v ${TMPVRMFFILE} -b ${BFF} \
                   -l ${LABEL_TABLE}
        else
            ${GENINFO} -i ${infoFile} -t ${COMPIDS} -p ${PTF} \
                   -o ${PTFOPTIONS} -v ${TMPVRMFFILE} -b "${OUTPUTDIR}/${BFF}" \
                   -l ${LABEL_TABLE}
	fi
        rc=$?
        if [ $rc -ne 0 ]
        then
            $log -e "gen_infofile ended with code of $rc "
        fi
    fi

    if [ $rc -eq 0 ]
    then
        #--------------------------------------------------
        # IFF everything OK so far, generate the CCSS
        #     image.
        #--------------------------------------------------
        headerFlag="N"               # Do not do headerFlag yet!!!
        if [ "$headerFlag" = "N" ]
        then
	    if [[ -z "${OUTPUTDIR}" ]]
	    then
                ${CCSS_PACK} -p ${BFF} -i ${infoFile} -t ${TOC} \
                          -n ${PTF} -c ${ccssFile}
            else
                ${CCSS_PACK} -p "${OUTPUTDIR}/${BFF}" -i ${infoFile} \
			  -t ${TOC} -n ${PTF} -c "${OUTPUTDIR}/${ccssFile}"
            fi
            rc=$?
        else
	    if [[ -z "${OUTPUTDIR}" ]]
	    then
                ${CCSS_PACK} -p ${BFF} -i ${INFOFILE} -t ${TOC} \
                          -n ${PTF} -c ${CCSSFILE} -H "${classValue}"
            else
                ${CCSS_PACK} -p "${OUTPUTDIR}/${BFF}" -i ${INFOFILE} -t ${TOC} \
                          -n ${PTF} -c "${OUTPUTDIR}/${CCSSFILE}" \
			  -H "${classValue}"
            fi
            rc=$?
        fi
        if [ "$rc" -ne 0 ]
        then
            $log -e "ccss_pack ended with code of $rc "
        fi
    fi

    rm -f ${TOC} ${infoFile}          # remove work files

    # If anything failed, then exit
    if [[ $rc -ne 0 ]]
    then
        Clean_Up
        exit 200
    else
        return 0
    fi
}

#
# NAME: create_ptf_image
#
# FUNCTION: This function creates the actual ptf image.  An update
#           inslist is generated for the files in this ptf.  The
#           adeinv command is run on the inslist to generate the
#           liblpp.a files.
#
# INPUT: BOOT (global) -
#        BLDCYCLE (global) -
#        COMMENT[] (global) -
#        COMMENT_INDEX (global) -
#        CONTENT (global) -
#        FIX (global) -
#        FILENAMESLIST (global) -
#        FORMAT (global) -
#        MOD (global) -
#        LANG (global) -
#        ODE_TOOLS (global) -
#        OPTIONPATH (global) -
#        OPTION (global) -
#        PLATFORM (global) -
#        PRODNAME (global) -
#        PKGPTFNAME (global) -
#        PTF (global) -
#        RELEASE (global) -
#        SHIP_PATH (global) -
#        TMPFILE (global) -
#        VERSION (global) -
#
# OUTPUT: COMMENT (global) - May be altered if apar is found in ${PKGPTFNAME}
#                            file.
#         CONTENT (global) - Will be altered if root part is found and not
#                            set to B.
#
# SIDE EFFECTS: Product the PTF image.
#
# RETURNS: 0 if PTF image is created, 1 if PTF image creation fails.
#
function create_ptf_image
{
   typeset    DATA_LIBLPPA="${OPTIONPATH}/data.liblpp.a.${BLDCYCLE}"
   typeset    USR_LIBLPPA="${OPTIONPATH}/usr.liblpp.a.${BLDCYCLE}"
   typeset    OPTIONLP="${OPTIONPATH}/${OPTION}.lp.${BLDCYCLE}"
   typeset    INSLIST="${OPTIONPATH}/${OPTION}.il.${BLDCYCLE}"
   typeset    DATA_OPTACF="${OPTIONPATH}/data.${OPTION}.acf"
   typeset    USR_OPTACF="${OPTIONPATH}/usr.${OPTION}.acf"
   typeset    UPSIZEFILE="${OPTIONPATH}/${OPTION}.upsize"
   typeset    ar_list=""		# Argument list to ar.
   typeset -i comment_index=1		# Used to index $COMMENT[].
   typeset    adeinv_aflag		# adeinv's -a flag if user supplied acf
   typeset    shareDataFlag=${FALSE}
   typeset -i rc=0			# Return code.

   #-------------------------------------------------------------
   # If the usr.liblpp.a, data.liblpp.a $OPTION.lp file and	|
   # inslist already exist for this build cycle then use them.	|
   # Else copy over the most recent copy (no BLDCYCLE suffix).	|
   #-------------------------------------------------------------
   [ ! -f ${DATA_LIBLPPA} ] && \
	cp ${OPTIONPATH}/data.liblpp.a ${DATA_LIBLPPA} > /dev/null 2>&1
   [ ! -f ${USR_LIBLPPA} ] && \
	cp ${OPTIONPATH}/usr.liblpp.a ${USR_LIBLPPA} > /dev/null 2>&1
   [ ! -f ${OPTIONLP} ] && \
	cp ${OPTIONPATH}/${OPTION}.lp ${OPTIONLP} > /dev/null 2>&1
   [ ! -f ${INSLIST} ] && \
	cp ${OPTIONPATH}/${OPTION}.il ${INSLIST} > /dev/null 2>&1

   ROOT_PART="${FALSE}"

   if [[ -f ${USR_LIBLPPA} ]]
   then
      cp ${USR_LIBLPPA} ./liblpp.a
   elif [[ -f ${DATA_LIBLPPA} ]]
   then
      cp ${DATA_LIBLPPA} ./liblpp.a
   else
      $log -e "${USR_LIBLPPA} or ${DATA_LIBLPPA} do not exist for ${PTF}."
      return 1
   fi

   #---------------------------------------------------------------
   # setup the -a flag for adeinv.  Variable is empty if user
   # did not supply an acf file.  Otherwise it contains the
   # flag itself and the full path to the user's acf.
   #---------------------------------------------------------------
   if [[ -f ${USR_OPTACF} ]]
   then
      adeinv_aflag="-a ${USR_OPTACF}"
   elif [[ -f ${DATA_OPTACF} ]]
   then
      adeinv_aflag="-a ${DATA_OPTACF}"
   fi


   # generate the inslist
   ptfins -f ${FILENAMESLIST} -o ${OPTION} -i ${INSLIST}
   if [[ $? -ne 0 ]]
   then
      $log -e "ptfins failed on ${PTF}"
      $log -e "f = '${FILENAMESLIST}', 0 = '${OPTION}', i = '${INSLIST}'"
      return 1
   fi

   #-------------------------------------------------------------
   # If the <option>.upsize file exists then copy it to		|
   # the current directory as <option>.insize before running	|
   # adeinv.							|
   #-------------------------------------------------------------
   [[ -f ${UPSIZEFILE} ]] && \
	cp ${UPSIZEFILE} ./${OPTION}.insize >/dev/null 2>&1

   #-------------------------------------------------------------
   # Set flag if share package.  
   # NOTE:
   #  This MUST be done after the call to get_lppname_info.
   #  The CONTENT variable is set in there.
   #-------------------------------------------------------------
   [[ "${CONTENT}" = "H" ]] && shareDataFlag=${TRUE}

   if [[ -s "${OPTION}.il" ]]
   then
      if [[ $shareDataFlag = ${FALSE} ]]
      then
	   adeinv -s ${SHIP_PATH} -i ${OPTION}.il -l ${PRODNAME} \
              -u ${OPTIONLP} -U "${VRMF}" ${adeinv_aflag} \
	      ${ADEINVENTORYFLAGS} 1> ${TMPFILE} 2>&1
	   rc=$?
      else
	   adeinv -D -s ${SHIP_PATH} -i ${OPTION}.il -l ${PRODNAME} \
              -u ${OPTIONLP} -U "${VRMF}" ${adeinv_aflag} \
              ${ADEINVENTORYFLAGS} 1> ${TMPFILE} 2>&1
	   rc=$?
      fi
     
      # Remove blank lines from adeinv output.  These blank lines
      # could cause confusion when mixed with the ptfpkg output.
      sed -e "/^$/d" ${TMPFILE}
      if [[ ${rc} -ne 0 ]]
      then
         $log -e "adeinv failed in create_ptf_image on ${PTF}"
         return 1
      fi
   fi

   #----------------------------------------------------
   # Add everything to liblpp.a.
   # Note that if the file is empty, there
   # is no need to add it to the archive.
   #----------------------------------------------------
   [ -s ${OPTION}.inventory ] && ar_list="${ar_list} ${OPTION}.inventory"
   [ -s ${OPTION}.size ] && ar_list="${ar_list} ${OPTION}.size"
   [ -s ${OPTION}.tcb ] && ar_list="${ar_list} ${OPTION}.tcb"
   [ -s ${OPTION}.al ] && ar_list="${ar_list} ${OPTION}.al"
   [ -s lpp.acf ] && ar_list="${ar_list} lpp.acf"
   [ -s lpp.doc ] && ar_list="${ar_list} lpp.doc"
   [ -s ${OPTION}.fixdata ] && ar_list="${ar_list} ${OPTION}.fixdata"
   [[ -n "${ar_list}" ]] && ar crl liblpp.a ${ar_list}

   if [[ $shareDataFlag = ${FALSE} ]] 
   then 
       process_root_part
       [[ $? -ne 0 ]] && return 1
   fi

   get_boot_value

   # Create <option>.lp for input to adelppname.
   print "${OPTION} ${BOOT} ${CONTENT} ${LANG} ${COMMENT[1]}" > ${OPTION}.lp
   while [[ ${comment_index} -lt ${COMMENT_INDEX} ]]
   do
      print "${COMMENT[comment_index]}" >> ${OPTION}.lp
      comment_index=comment_index+1
   done

   rm -fr ${PTFDIR}/aparsinfo
   while read apar
   do
        if [[ "${BUILD_TYPE}" = "production" ]]
        then
            abs=`grep "^$apar" $ABSTRACTS 2>/dev/null`
            [[ $? -ne 0 ]] && $log -x "APAR $apar was not found ${ABSTRACTS} file."
        else
            abs=`grep "^$apar" $ABSTRACTS 2>/dev/null`
            [[ $? -ne 0 ]] && abs="$apar This is a dummy apar."
        fi
	#--------------------------------------------------
	# If there is something in abs (ie. an abstract
	# was either found or created), then we need to
	# stick it in the aparinfo file along with the
	# number of filesets required to fix the APAR.
	#--------------------------------------------------
	if [[ -n $abs ]]
	then
	    typeset -i fsCnt=0
	    fsCnt=`fsCnt -a $apar 2>/dev/null`
	    RC=$?
	    if [[ $RC = 0 ]]
	    then
	        if [[ $fsCnt = 0 ]]
	        then
	            $log -x "Could not find any filesets fixing APAR $apar!"
	            fsCnt=1 ;
	        fi
	    else 
	        $log -x "Error encountered when counting filesets for APAR $apar"
	        fsCnt=1 ;
	    fi

	    print -n - `echo $abs | cut -d" " -f 1` >> aparsinfo
	    print - "  $fsCnt" `echo $abs |cut -d" " -f 2-` >> aparsinfo
	fi
   done < $APARSLIST

   adelppname -f ${FORMAT} -v ${VERSION} -r ${RELEASE} -m ${MOD} \
              -F ${FIX} -p ${PLATFORM} -u ${OPTION}.lp \
              -c ${COMPIDS} -o lpp_name -t ${MEDIATYPE} \
              -l ${PRODNAME} -k aparsinfo
   if [[ $? -ne 0 ]]
   then
      $log -e "adelppname failed on ${PTF}"
      return 1
   fi

   if [[ $shareDataFlag = ${TRUE} ]]
   then
      if [[ -n "${OUTPUTDIR}" ]]
      then
          adepackage -D -s ${SHIP_PATH} -l ${PRODNAME} -i ${OPTION}.il \
	      -f "${OUTPUTDIR}/${PTF}.bff" -U "${VRMF}" -o ${OPTION}
      else
          adepackage -D -s ${SHIP_PATH} -l ${PRODNAME} -i ${OPTION}.il -f ${PTF}.bff \
	      -U "${VRMF}" -o ${OPTION}
      fi
   else
      if [[ -n "${OUTPUTDIR}" ]]
      then
          adepackage -s ${SHIP_PATH} -l ${PRODNAME} -i ${OPTION}.il \
	       -f "${OUTPUTDIR}/${PTF}.bff" -U "${VRMF}" -o ${OPTION}
      else
          adepackage -s ${SHIP_PATH} -l ${PRODNAME} -i ${OPTION}.il -f ${PTF}.bff \
	       -U "${VRMF}" -o ${OPTION}
      fi
   fi
   if [[ $? -ne 0 ]]
   then
      $log -e "adepackage failed on ${PTF}"
      return 1
   else
	if [[ "${CCSSFLAG}" = "${TRUE}" ]]
	then
	    create_ccss_image
	    if [[ "${BFF_FLAG}" = "${FALSE}" ]]
	    then
                if [[ -z "${OUTPUTDIR}" ]]
	        then
		    rm ${PTF}.bff
                else
		    rm "${OUTPUTDIR}/${PTF}.bff"
                fi
	    fi
	fi
   fi

   return 0
}

#
# NAME: gen_boot_info
#
# FUNCTION: Built list of files that require system reboot if replaced on
#           system.
#
# INPUT: SPACETAB (global) -
#
# OUTPUT: BOOTFILES (global) -
#         BOOTLIBS (global) -
#
# SIDE EFFECTS:  None.
#
# RETURNS: 0 is success and 1 if failure.
#
function gen_boot_info
{
   typeset    bosbootlist="${ODE_TOOLS}/usr/lib/bosboot_list"
   typeset    diskproto=""
   typeset    entproto=""
   typeset    tokproto=""
   typeset    fddiproto=""
   typeset    diskprotoext=""
   typeset    entprotoext=""
   typeset    tokprotoext=""
   typeset    fddiprotoext=""
   typeset    filename
   typeset    basename

   # Search SHIP_PATH for the proto files.
   for shippath in $(echo ${SHIP_PATH} | sed -e "s/:/ /g")
   do
	ls -1 ${shippath}/usr/lib/boot/*disk.proto 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${diskproto} | grep ${basename} >/dev/null 2>&1 || \
			diskproto="${diskproto} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/protoext/disk.proto.ext.* 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${diskprotoext} | grep ${basename} >/dev/null 2>&1 || \
			diskprotoext="${diskprotoext} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/network/*ent.proto 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${entproto} | grep ${basename} >/dev/null 2>&1 || \
			entproto="${entproto} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/protoext/ent.proto.ext.* 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${entprotoext} | grep ${basename} >/dev/null 2>&1 || \
			entprotoext="${entprotoext} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/network/*tok.proto 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${tokproto} | grep ${basename} >/dev/null 2>&1 || \
			tokproto="${tokproto} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/protoext/tok.proto.ext.* 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
		echo ${tokprotoext} | grep ${basename} >/dev/null 2>&1 || \
			tokprotoext="${tokprotoext} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/network/*fddi.proto 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
	        echo ${fddiproto} | grep ${basename} >/dev/null 2>&1 || \
		    fddiproto="${fddiproto} ${filename}"
	    done
	ls -1 ${shippath}/usr/lib/boot/protoext/fddi.proto.ext.* 2>/dev/null | \
	    while read filename
	    do
		basename=`basename ${filename}`
	        echo ${fddiprotoext} | grep ${basename} >/dev/null 2>&1 || \
		    fddiprotoext="${fddiprotoext} ${filename}"
	    done
   done

   # Warnings if we don't find the proto files.'
   [[ -z "${diskproto}" ]] && \
      $log -w "Cannot find /usr/lib/boot/disk.proto file in \$SHIP_PATH"
   [[ -z "${entproto}" ]] && \
      $log -w "Cannot find /usr/lib/boot/network/ent.proto file in \$SHIP_PATH"
   [[ -z "${tokproto}" ]] && \
      $log -w "Cannot find /usr/lib/boot/network/tok.proto file in \$SHIP_PATH"
   [[ -z "${fddiproto}" ]] && \
      $log -w "Cannot find /usr/lib/boot/network/fddi.proto file in \$SHIP_PATH"
   [[ -z "${diskprotoext}" ]] && \
      $log -w "Cannot find /usr/lib/boot/protoext/disk.proto.ext file in \$SHIP_PATH"
   [[ -z "${entprotoext}" ]] && \
      $log -w "Cannot find /usr/lib/boot/protoext/ent.proto.ext file in \$SHIP_PATH"
   [[ -z "${tokprotoext}" ]] && \
      $log -w "Cannot find /usr/lib/boot/protoext/tok.proto.ext file in \$SHIP_PATH"
   [[ -z "${fddiprotoext}" ]] && \
      $log -w "Cannot find /usr/lib/boot/protoext/fddi.proto.ext file in \$SHIP_PATH"

   expand ${diskproto} ${entproto} ${tokproto} ${fddiproto} \
   ${diskprotoext} ${entprotoext} ${tokprotoext} ${fddiprotoext} \
   | awk 'BEGIN { RS=" *" }
          # A Entry is any space separated sequence of characters.
          # Entry beginning with a '/'.
          /^\/.*/ \
          {
             # Ignore entries beginning with '/tmp' or containing 'libc.a.min'.
             if (match($1,"/tmp") != 1 && match($1,"libc.a.min") == 0)
                printf(".%s\n",$1)
          }
          # Entry containing library names (i.e. lib*.a).
          /^lib.*\.a/ \
          {
             if ($1 == "libc.a")
                printf("%s.min\n",$1) >libfile
             else
                printf("%s\n",$1) >libfile
          }' \
          libfile=${BOOTLIBS} > ${BOOTFILES}

   for shippath in $(echo ${SHIP_PATH} | sed -e "s/:/ /g")
   do
	ls -1 ${shippath}/usr/lib/boot/unix_up 2>/dev/null >> ${BOOTFILES}
	ls -1 ${shippath}/usr/lib/boot/unix_mp 2>/dev/null >> ${BOOTFILES}
   done

   if [[ -f ${bosbootlist} ]]
   then
      sed -e '/^[#${SPACE_TAB}]/d' ${bosbootlist} \
      | sed -e '/^[${SPACE_TAB}]*$/d' >> ${BOOTFILES}
   else
      $log -w "Cannot find file ${bosbootlist}"
   fi

   if [[ -s "${BOOTFILES}" ]]
   then
	sort -u ${BOOTFILES} -o ${BOOTFILES} || \
	    { $log -e "Sort of ${BOOTFILES} failed"; return 1; }
   fi
   if [[ -s "${BOOTLIBS}" ]]
   then
	sort -u ${BOOTLIBS} -o ${BOOTLIBS} || \
	    { $log -e "Sort of ${BOOTLIBS} failed"; return 1; }
   fi

   return 0
}

#
# NAME: updateptfapardef
#
# FUNCTION: Update the ptfapardef file to include the current set of apars.
#
# INPUT: DEFECTAPARS
#
# OUTPUT: PTFAPARDEFMASTER
#
# SIDE EFFECTS:  PTFAPARDEFMASTER file updated to include current set of
#	apars.
#
# RETURNS: 0 is success and 1 if failure.
#

function updateptfapardef 
{
    # get the list of all apars from wk_ptf_pkg file for this ptf
    aparslist=""
    cat wk_ptf_pkg | while read line
    do
	apars=`echo $line | awk -F"|" '{print $2}'`
	for apar in $apars
	do
	    echo "$aparslist" | grep "$apar" > /dev/null 2>&1
	    [[ $? -ne 0 ]] && aparslist="$aparslist $apar"
	done
    done

    # get the abstract for the apar from $HISTORYTOP/abstracts file
    for apar in $aparslist
    do
	abstract=`grep -c "^$apar" ${ABSTRACTS}`
	if [[ $abstract -gt 1 ]]
	then
	    $log -w "There are multiple entries in ${ABSTRACTS} for apar ${apar}"
	    line=$(grep "^$apar" ${ABSTRACTS} | awk '
 		BEGIN { mismatch=0 }
                {
                if (NR == 1) { firstline = $0 }
                {currentline = $0}
                if ( currentline != firstline ) { mismatch=1 }
                }
                END { if ( mismatch == 1 ) {print "Error"}
		      else {print firstline}
                }') 

	    if [[ $line = "Error" ]]
	    then
	    	$log -x "The abstracts for apar ${apar} in ${ABSTRACTS} are not identical"
	    else
		abstract=`echo $line | sed -e "s/$apar[    ]//" 2> /dev/null`
	    fi
	else
	    abstract=`grep "^$apar" ${ABSTRACTS} | \
	    	sed -e "s/$apar[ 	]//" 2> /dev/null`
	fi

	# get the defect number, apar number, release name and family name
	# for the apar from $DEFECTAPARS file and put it in a temp file.
	# If it is not a special type ptf then we add the full option name 
	# for the apar to the ptfapardef.master file else we leave the 5th
	# field blank as special type ptfs are created in the LPP directory
	# under the UPDATE tree.
	defectapars="$BLDTMP/defectapars.$$"
	grep "|$apar|" $DEFECTAPARS > $defectapars
	if [[ $? -eq 0 ]]
	then
	    if [[ -s $defectapars ]]
	    then
		cat $defectapars |
		    awk -F "|" '{print $1 " " $2 " " $3 " " $4}' |
			while read defect apar release family
			do
				print -r "$PTF|$apar|$defect|$LPP|$OPTION|$release|$family|$abstract|${VRMF}" >> ${PTFAPARDEFMASTER}
			done
	    fi
	else
	    echo "\nPTFPKG: WARNING: APAR $apar is not in the \"defectapars\" file."
	    echo "PTFPKG       The file(ptfapardef.master) is not updated for this"
	    echo "              ptf $PTF with its associated APAR $apar"
	fi
	rm -f $defectapars
    done
}

#
# NAME: get_comment
#
# FUNCTION: Search file for key.  If key is found the COMMENT array is
#           changed to the comment found in the file.
#
# INPUT: key ($1) - key to search file on.
#        filename ($2) - name of file to read from.
#
# OUTPUT: COMMENT[] (global) - 
#         COMMENT_INDEX (global) - 
#         SPACE_TAB (global) - 
#
# SIDE EFFECTS:  None.
#
# RETURNS: 0 is success and 1 if failure.
#
function get_comment
{
   typeset    check_key			# Key from line.
   typeset    comment=""		# Comment from line.
   typeset    filename=$2
   typeset    key=$1
   typeset    keyFound=""
   typeset    line			# Line read from file.

   if [[ ! -f "${filename}" ]]
   then
      $log -w "${filename} does not exist"
   else
      while read line
      do
         # Ignore comment lines.  They begin with a '#'.
         [[ "${line}" = "#"* ]] && continue
         # Key has been found.  See if any addition comment lines are present.
         if [[ -n "${keyFound}" ]]
         then
            if [[ ${line} = "ADDITIONAL_COMMENT"[${SPACE_TAB}]* ]]
            then
               print ${line} | read check_key comment
               COMMENT_INDEX=COMMENT_INDEX+1
               COMMENT[COMMENT_INDEX]=${comment}
            else
               break
            fi
         fi
         # See if the line begins with key.  If so search is over.
         if [[ ${line} = "${key}"[${SPACE_TAB}]* ]]
         then
            print ${line} | read check_key comment
            COMMENT_INDEX=1
            COMMENT[COMMENT_INDEX]=${comment}
	    keyFound=y
	 else
	    keyFound=""
         fi
      done < ${filename}
      # Never found key in the file.
      [[ -z "${comment}" ]] && \
         $log -w "${key} not found in ${filename}"
   fi

   return 0
}

#
# NAME: get_lppname_info
#
# FUNCTION: Read required files for current option from the lpp_info file
#        and the .lp file.
#
# INPUT: 
#        BLDCYCLE (global) -
#        COMMANDLINE_MEDIATYPE (global) -
#        FILENAME (global) -
#        LPP (global) -
#        OPTION (global) -
#        OPTIONPATH (global) -
#        PTF (global) -
#        SPACE_TAB (global) -
#        TRUE (global) -
#        UPDATETOP (global) -
#
# OUTPUT: 
#         BOOT (global) -
#         COMMENT (global) -
#         CONTENT (global) -
#         FORMAT (global) -
#         LANG (global) -
#         MEDIATYPE (global) -
#         PLATFORM (global) -
#         PRODNAME (global) -
#
# SIDE EFFECTS:  None.
#
# RETURNS: 0 if success, 1 if error.
#
# The format of the .lp file is:
#    prolog
#    OPTION BOOT CONTENT LANG COMMENT
#    COMMENT ...
#
# The format of the lpp_info file is:
#    FORMAT PLATFORM PRODNAME
#
function get_lppname_info
{
   typeset    dummy			# Temporary variable.
   typeset -r lpp_info="${OPTIONPATH}/lpp_info"
   typeset -r OPTIONLP="${OPTIONPATH}/${OPTION}.lp"
   typeset    option			# Option read from file.
   typeset    saveline=""		# Line option information found on.
   typeset    vrmf			# Temporary variable to hold version,
					#release, mod and fix variables.

   if [[ ! -f ${lpp_info} ]] 
   then
      $log -e "Cannot open lpp_info file ${lpp_info}"
      return 1
   fi

   read FORMAT PLATFORM PRODNAME < ${lpp_info}
   if [[ $? -ne 0 ]]
   then
      $log -e "Read failure with ${lpp_info}"
      return 1
   fi

   case "${FILENAME}" in
      cum_ptf)
	 #------------------------------------------------
	 # Due to an install problem with ML types,
	 # cumPTFs must be regular S types.
	 #------------------------------------------------
	 MEDIATYPE="S"
         ;;
      *)
         ;;
   esac

   BOOT=""
   while read line
   do
      # Ignore comment lines.  They begin with a '#'.
      [[ "${line}" = "#"* ]] && continue
      # The first non '#' comment line is the option information line.
      if [[ -z "${BOOT}" ]]
      then
         echo ${line} | read option BOOT CONTENT LANG COMMENT[1]
      else
	 # Assume that any remaining lines are comments.
	 if [[ -n "${saveline}" ]]
	 then
	    COMMENT_INDEX=COMMENT_INDEX+1
	    COMMENT[COMMENT_INDEX]=${line}
	 fi
      fi
   done < ${OPTIONLP}

   return 0
}

#
# NAME: get_boot_value
#
# FUNCTION: For each entry in the ./${OPTION}.il file check ${BOOTLIBS}
#	and ${BOOTFILES}.  If a file name match is found set the global
#	BOOT variable.
#
# INPUT: ./${OPTION}.il
#       ${BOOTLIBS} (global)
#	${BOOTFILES} (global)
#
# OUTPUT: None.
#
# SIDE EFFECTS:  BOOT variable set if a match is found between the
#	files in the inslist and the files listed in ${BOOTLIBS}
#	and ${BOOTFILES}
#
# RETURNS: 0
#
function get_boot_value
{
        while read type uid gid mode filename junk
        do
            grep "${filename}" ${BOOTFILES} 1> /dev/null 2>&1
            if [[ $? -eq 0 ]]
            then
               [[ "${BOOT}" = N ]] && BOOT=b
               [[ "${BOOT}" = Y ]] && BOOT=B
            fi
            grep "${filename}" ${BOOTLIBS} 1> /dev/null 2>&1
            if [[ $? -eq 0 ]]
            then
               [[ "${BOOT}" = N ]] && BOOT=b
               [[ "${BOOT}" = Y ]] && BOOT=B
            fi
        done < ./${OPTION}.il

	return 0
}

#
# NAME: process_ptfs
#
# FUNCTION: Process the PTFS in the ptf_pkg file.
#
# INPUT: BOOTFILES (global) -
#        BUILD_TYPE (global) -
#        LPP (global) -
#        VRMF (global) -
#
# OUTPUT: BOOT (global) -
#         CUMSLIST (global) -
#         FILENAME (global) -
#         FILENAMESLIST (global) -
#         OPTION (global) -
#         OPTIONPATH (global) -
#         MEDIATYPE (global) -
#         PTF (global) -
#         PTFDIR (global) -
#         PTF_PKG (global) -
#         PTFSLIST (global) -
#
# SIDE EFFECTS:  None.
#
# RETURNS: 0 is success and 1 if failure.
#
function process_ptfs
{
   typeset    filename			# The filename from input.
   typeset    optionname		# The option name from input.
   typeset    optvrmf			# optionname:vrmf from ptf_pkg
   typeset    vrmf			# vrmf from ptf_pkg
   typeset    rc

   # Process each PTF in the ptf_pkg file.
   awk -F"|" '{print $1}' ${PTF_PKG} \
   | sort -u \
   | while read PTF
     do
        # Examine all entries for PTF, if a special type is found the PTF
        # will be created as a special PTF.
	# Default media type.
	[[ "${COMMANDLINE_MEDIAOPTION}" != "${TRUE}" ]] && MEDIATYPE="S"
        grep "^${PTF}" "${PTF_PKG}" \
        | awk -F"|" '{print $3 " " $4}' \
        | while read filename optvrmf
          do
		# option name have ":vrmf" if the default vrmf
		# is being overridden. So, we split the string into
		# optionname and vrmf
                # to be done
	     echo ${optvrmf} | sed -e "s/\:/ /g" | read optionname vrmf

   	     # The ${vrmf} variables contains four fields seperated by '.'.  Split
   	     # fields out into seperate variables.
             if [[ -n "$vrmf" ]] 
     	     then
   	     	echo ${vrmf} | sed -e "s/\./ /g" | read VERSION RELEASE MOD FIX
	     fi

             case "${FILENAME}" in
                # Not special case PTFs.
                *)
                    # All other PTFs created under option directory in
                    # UPDATE tree.
                    OPTION="${optionname}"
                    # Create pathname.  All '.' in $OPTION are changed to '/'.
                    OPTIONPATH="${UPDATETOP}/$(echo ${OPTION} \
                                               | sed -e 's?\.?/?g')"
                    PTFDIR=${OPTIONPATH}/${PTF}
                    FILENAME=${filename}
                    ;;
             esac
          done

        $log "Creating PTF ${PTF}"

        CUMSLIST=${OPTIONPATH}/cumsList
        PTFSLIST=${OPTIONPATH}/ptfsList
	cat ${OPTIONPATH}/vrmfFile | read oldvrmf
        PTFSLIST_BACK="${OPTIONPATH}/ptfsList.${vrmf}"
        FILENAMESLIST=${PTFDIR}/filenamesList
	APARSLIST=${PTFDIR}/aparsList

        # If PTF's build directory already exists insure PTF can really be
        # built again.
        if [[ -d ${PTFDIR} ]] 
        then
           repackage
           if [[ $? -ne 0 ]] 
           then
	      $log -e  "Failed to repackage the PTF"
              $log -e "Cannot continue processing ptf ${PTF}"
              continue
           fi
        fi

        mkdir -p ${PTFDIR}
        if [[ $? -ne 0 ]]
        then
           $log -e "Cannot create directory ${PTFDIR}"
           $log -e "Cannot continue processing ptf ${PTF}"
           continue
        fi

        awk 'BEGIN {FS="|"}; $1 == "'${PTF}'" {print $0}' ${PTF_PKG} \
        > ${PTFDIR}/wk_ptf_pkg

        cd ${PTFDIR}

        get_lppname_info
        if [[ $? -ne 0 ]] 
        then
	   $log -e "get_lppname_info failed"
           $log -e "Cannot continue processing ptf ${PTF}"
           continue
        fi

        if [[ -z "$vrmf" ]] 
	then
        	processPtf 
        else
		processPtf -v ${VERSION} -r ${RELEASE} -m ${MOD} -f ${FIX}
        fi
        if [[ $? -ne 0 ]] 
        then
           
           $log -e "processPtf failed on ${PTF}"
           $log -e "Cannot continue processing ptf ${PTF}"
	   $log -e "VERSION='${VERSION}'; RELEASE='${RELEASE}'; MOD='${MOD}'; FIX='${FIX}'"
           continue
	else
	   # if Ptf was packaged successfully then 
	   # set VERSION, RELEASE, MOD and FIX for adelppname
	   grep "^${PTF}" ${TMPVRMFFILE} \
	   | awk  '{print $3}' \
	   | read newvrmf
   	   echo ${newvrmf} | sed -e "s/\./ /g" | read VERSION RELEASE MOD FIX
        fi
 
	# set the vrmf (global) string for adepackage and adeinv
	VRMF="${VERSION}.${RELEASE}.${MOD}.${FIX}"

        grep "\/usr\/sbin\/installp" ${FILENAMESLIST} 1> /dev/null 2>&1
        [[ $? -eq 0 ]] && MEDIATYPE="SF"

	# Create the option.fixdata file 
	if [[ ${OPTION} != "bos.rte.install" ]]
	then
	    grep -v "_ML" ${APARSLIST} | \
	        lookupfixdata ${FIXDATADB} > ${OPTION}.fixdata
	    rc=$?
	else
	    lookupfixdata ${FIXDATADB} < ${APARSLIST} > ${OPTION}.fixdata
	    rc=$?
	fi
	[[ $rc -ne 0 && "$BUILD_TYPE" = "production" ]] && \
	$log -x "lookupfixdata failed to generate ${OPTION}.fixdata\n\
		file for production level build."
	[[ $rc -ne 0 && "$BUILD_TYPE" != "production" ]] && \
	$log -w "lookupfixdata failed to generate ${OPTION}.fixdata file."

        create_ptf_image
        if [[ $? -ne 0 ]]
        then
	   ML_NMBR=-1 ;
	   $log -e "create_ptf_image failed"
           $log -e "Cannot continue processing ptf ${PTF}"
           echo "May need to repackage the following PTF's"
           grep "${PTF}" ${PTFREQSLIST} | awk '{print $1}' 
           continue
	else
	   # remove the option.fixdata file
	   rm -f ${OPTION}.fixdata

	   # if Ptf was packaged successfully then update /HISTORY/ptfoptions
           # file and vrmfFile in option directory
	   grep "^${PTF}" ${TMPVRMFFILE} \
	   | awk  '{print $1 " " $2}' \
	   | read ptfname filesetname

	   newptfoptline="$ptfname $filesetname $newvrmf"
           
           rm -f ${TMPPTFOPTFILE}
           sed -e "/^${PTF}/d" ${PTFOPTFILE} > ${TMPPTFOPTFILE}
           mv -f ${TMPPTFOPTFILE} ${PTFOPTFILE}
           echo  ${newptfoptline} >> ${PTFOPTFILE}
           rm -f ${OPTIONPATH}/vrmfFile
	   echo ${newvrmf} > ${OPTIONPATH}/vrmfFile 

	   # Copy the cumptf to cumsList and empty ptfsList file
	   if [ "${FILENAME}" = "cum_ptf" ]; then
		grep "${PTF}" "${CUMSLIST}" > /dev/null 2>&1
		if [ $? -ne 0 ]; then
	            echo "${PTF}" >> ${CUMSLIST}
                fi
		if [ -f "${PTFSLIST}" ]; then
		    cp "${PTFSLIST}" "${PTFDIR}/ptfsList.${vrmf}"
	            mv "${PTFSLIST}" "${PTFSLIST_BACK}"
                fi

		if [[ $ML_NMBR != -1 ]]
		then
		    ML_NMBR=$MOD
		fi
	   else
               # Copy this ptf into ptfsList file
	       grep "${PTF}" "${PTFSLIST}" > /dev/null 2>&1
	       if [ $? -ne 0 ]; then
		   echo "${PTF}" >> ${PTFSLIST}
	       fi
            fi

           # Create ptfapardef.master file
             [[ ! -f ${ABSTRACTS} ]] && \
                $log -x "Cannot open abstracts file ${ABSTRACTS}"

           # Combine defectapars files from all build cycles.  This file is used
           # to create ptfapardef.master file.
           cat  ${PTFTOP}/*/defectapars > ${DEFECTAPARS} 2>/dev/null
           sort -u ${DEFECTAPARS} -o ${DEFECTAPARS}
           if [[ ! -s ${DEFECTAPARS} ]]
	   then
		$log -w "Cannot find any defectapars files in ${TOP}/PTF"
	   else
		updateptfapardef
	   fi
	fi

	# Lastly, remove it from the ptf_pkg file.
	grep -v "^${PTF}" ${PTF_PKG} > ${TMPFILE}
	if [[ $? -eq 2 ]]
	then
	    logmsg="Could not create ${TMPFILE} while removing valid PTFs"
	    logmsg="${logmsg} from ${PTF_PKG}"
	    $log -x "${logmsg}"
	fi
	cp ${TMPFILE} ${PTF_PKG} || \
	    $log -x "Failed to copy ${TMPFILE} into ${PTF_PKG}"
	rm -f ${TMPFILE}

        $log "Created PTF ${PTF}"
        [[ ${BUILD_TYPE} = "sandbox" ]] && \
           $log "PTF ${PTF} is located in ${PTFDIR}"

     done

   return 0
}

#
# NAME: process_root_part
#
# FUNCTION: If there is a root liblpp.a file run adeinv with -r option.
#           If the .inventory or .al files are non-empty this ptf has
#           a root part.
#
# INPUT: BLDCYCLE (global) -
#        FILENAMESLIST (global) -
#        FORCED_ROOT (global) -
#        OPTION (global) -
#        OPTIONPATH (global) -
#        PRODNAME (global) -
#        PTF (global) -
#        SHIP_PATH (global) -
#
# OUTPUT: ROOT_PART (global)
#
# SIDE EFFECTS: Creates root portion of PTF.
#
# RETURNS: 0 if PTF image is created, 1 if PTF image creation fails.
#
function process_root_part
{
   typeset    OPTIONLP="${OPTIONPATH}/${OPTION}.lp.${BLDCYCLE}"
   typeset    ROOT_LIBLPPA="${OPTIONPATH}/root.liblpp.a.${BLDCYCLE}"
   typeset    ROOT_OPTACF="${OPTIONPATH}/root.${OPTION}.acf"
   typeset    ar_list=""		# Argument list to ar.
   typeset    count
   typeset    adeinv_aflag		# adeinv's -a flag if user supplied acf
   typeset    rc

   #-------------------------------------------------------------
   # If the root.liblpp.a and $OPTION.lp file already exist for	|
   # this build cycle then use them.  Else copy over the most	|
   # recent copy (no BLDCYCLE suffix).				|
   #-------------------------------------------------------------
   [ ! -f ${OPTIONLP} ] && \
	cp ${OPTIONPATH}/${OPTION}.lp ${OPTIONLP} > /dev/null 2>&1
   [ ! -f ${ROOT_LIBLPPA} ] && \
	cp ${OPTIONPATH}/root.liblpp.a ${ROOT_LIBLPPA} > /dev/null 2>&1

   [[ ! -f ${ROOT_LIBLPPA} ]] && { CONTENT=U; return 0; }
     
   mkdir root
   cd root
   cp ${ROOT_LIBLPPA} ./liblpp.a

   #---------------------------------------------------------------
   # setup the -a flag for adeinv.  Variable is empty if user
   # did not supply an acf file.  Otherwise it contains the
   # flag itself and the full path to the user's acf.
   #---------------------------------------------------------------
   if [[ -f ${ROOT_OPTACF} ]]
   then
      adeinv_aflag="-a ${ROOT_OPTACF}"
   fi
 
  if [[ -s ../${OPTION}.il ]]
   then
      cp ../${OPTION}.il .
      adeinv -s ${SHIP_PATH} -i ${OPTION}.il -l ${PRODNAME} \
             -u ${OPTIONLP} -U "${VRMF}" ${adeinv_aflag} -r \
             ${ADEINVENTORYFLAGS} 1> ${TMPFILE} 2>&1
      rc=$?
      # Remove blank lines from adeinv output.  These blank lines
      # could cause confusion when mixed with the ptfpkg output.
      sed -e "/^$/d" ${TMPFILE}
      if [[ ${rc} -ne 0 ]]
      then
         cd ..
         $log -e "adeinv failed in process_root_part on ${PTF}"
         return 1
      fi
      [ -s ${OPTION}.inventory ] && \
         { ar_list="${ar_list} ${OPTION}.inventory"; ROOT_PART="${TRUE}"; }
      [ -s ${OPTION}.al ] && \
         { ar_list="${ar_list} ${OPTION}.al"; ROOT_PART="${TRUE}"; }
      [ -s ${OPTION}.size ] && ar_list="${ar_list} ${OPTION}.size"
      [ -s ${OPTION}.tcb ] && ar_list="${ar_list} ${OPTION}.tcb"
      [[ -n "${ar_list}" ]] && ar crl liblpp.a ${ar_list}
   fi

   if [[ ${ROOT_PART} != "${TRUE}" ]]
   then
      grep "inst_root\/liblpp.a" ${FILENAMESLIST} 1>/dev/null 2>&1 \
         && ROOT_PART="${TRUE}"
      grep "^${OPTION}" ${FORCED_ROOT} 1> /dev/null 2>&1 \
         && ROOT_PART="${TRUE}"
   fi
      
   count=`ar t ./liblpp.a | wc -l`

   # If we get here and ROOT_PART=FALSE but liblpp.a is not empty
   # then set ROOT_PART=TRUE because we are probably shipping some
   # config files or something that was archived in during the update
   # build when liblpp.a was initially created.
   [ ${ROOT_PART} = "${FALSE}" -a ${count} -ne 0 ] \
	&& ROOT_PART="${TRUE}"

   # If root_part not set then set CONTENT to U.
   [ ${ROOT_PART} != "${TRUE}" ] && CONTENT=U

   # If ./liblpp.a has no members then remove it so that adepackage
   # will not back up an empty root/liblpp.a file.
   [ ${count} -eq 0 ] && rm liblpp.a

   cd ..

   return 0
}

#
# NAME: repackage
#
# FUNCTION: Repackage a PTF.  A PTF can be repackaged only if the
#           following conditions are met:
#              1) Special PTFs can always be repackaged.
#              2) A cum_ptf PTF can be repackaged if the file ${PTFSLIST}
#                 is empty and PTF is the last entry or not in ${CUMSLIST}.
#              3) All other PTFs can be repackaged if the PTF is not in 
#                 $TOP/UPDATE/$OPTION/ptfsList or if the PTF is
#                 in $TOP/UPDATE/$OPTION/ptfsList all entries
#                 after the PTF are in $TOP/UPDATE/$LPP/ptf_pkg.$BLDCYCLE.
#
# INPUT: BLDCYCLE (global) - Build cycle.
#        FILENAME (global) - PTF type of ${PTF}.
#        PTF (global) - PTF number.
#        PTF_PKG (global) - ptf_pkg file.
#        PTFDIR (global) - Directory where specific PTF information stored.
#        TMPFILE (global) - Temporary file.
#        UPDATETOP (global) - Top of UPDATE tree.
#
# OUTPUT: none.
#
# SIDE EFFECTS: none.
#
# RETURNS: 0 is success and 1 if failure.
#
function repackage
{
   typeset -i rc=0			# Return code.
   typeset    check_PTF="${FALSE}"	# If set to $TRUE all PTFs read from
					# ptfsList must be in ptf_pkg file.
   typeset    logmsg="Cannot repackage ${PTF}"

   case "${FILENAME}" in
      # Cum PTFs.
      cum_ptf)
          # ptfsList file must NOT be empty.
          if [[ ! -s ${PTFSLIST} ]]
          then
	      cp ${PTFDIR}/ptfsList.${vrmf} ${PTFSLIST}
	      if [[ $? -eq 0 ]]
	      then
		  $log -w "repackaging; file ${PTFSLIST} has been restored from ${PTFDIR}/ptfsList.${vrmf}"
	      else
		  $log -e "${logmsg}, could not restore ${PTFSLIST} from ${PTFDIR}/ptfsList.${vrmf}"
		  return 1
	      fi
          fi
          # If PTF is in cumsList then it must be last entry.
          grep ${PTF} ${CUMSLIST} 1> /dev/null 2>&1
          if [[ $? -eq 0 ]]
          then
             if [[ "${PTF}" != $(tail -1 ${CUMSLIST}) ]]
             then
                $log -e "${logmsg}, ${PTF} must be last entry in ${CUMSLIST}"
                return 1
             fi
          fi
          ;;
      # All other PTFs.
      *)
          # if ${PTFSLIST} is empty PTF can be repackaged.
          if [[ -s ${PTFSLIST} ]]
          then
             # If PTF is not in ${PTFSLIST}, loop will read all entries in
             # ${PTFSLIST} and never set ${check_PTF}.  Will exit loop with
             # no error.
             while read work_PTF
             do
                # If ${check_PTF} is set, ${PTF} was found in ${PTFSLIST} and
                # rest of ${work_PTF} read must be in ${PKG_PTF}.
                if [[ ${check_PTF} = "${TRUE}" ]]
                then
                   grep ${work_PTF} ${PTF_PKG} 1> /dev/null 2>&1
                   if [[ $? -ne 0 ]]
                   then
                      $log -e "${logmsg}, ${work_PTF} needs to be repackaged"
                      $log -e "but has no entry in ${PTF_PKG}"
                      rc=1
                   fi
                fi
                # Set ${check_PTF} when ${PTF} is found in ${PTF_PKG}.
                if [[ "${work_PTF}" = "${PTF}" ]]
                then
                   check_PTF="${TRUE}"
                fi
             done < ${PTFSLIST}
          fi
          [[ ${rc} -ne 0 ]] && return 1
          ;;
   esac

   cd ${PTFDIR}/..
   rm -fr ${PTFDIR##*/}
   if [[ $? -ne 0 ]]
   then
      $log -e "${logmsg}, cannot remove directory ${PTFDIR}"
      rc=1
   fi

   # Remove ${PTF} from files it may have been written into.
   for file in ${PTFAPARDEFMASTER} ${CUMSLIST} ${PTFSLIST}
   do
      [[ ! -f ${file} ]] && continue
      sed -e "/^${PTF}|/d" -e "/^${PTF}$/d" ${file} > ${TMPFILE}
      if [[ $? -ne 0 ]]
      then
         $log -e "${logmsg}, cannot remove ${PTF} from ${file}"
         $log -e "on creation of temporary file ${TMPFILE}"
         rc=1
      else
         rm -f ${file}
         cp ${TMPFILE} ${file}
         if [[ $? -ne 0 ]]
         then
            $log -e "${logmsg}, copy of temporary file ${TMPFILE}"
            $log -e "to ${file} for ${PTF} failed"
            rc=1
         fi
      fi
   done

   rm -f ${TMPFILE}

   return ${rc}
}

################################################################################
#
# Start of MAIN
#
################################################################################

. display_msg
. bldkshconst
. bldinitfunc

OUTPUTDIR=.
CCSSFLAG="${FALSE}"
BFF_FLAG="${FALSE}"

[[ -z "${BLDTMP}" ]] && BLDTMP=/tmp

typeset    command=${0##*/}

#------------------------------------------------------
# display_msg function displays formatted messages
#------------------------------------------------------
log="display_msg -C ${command}"

while getopts :m:bcd: option
do
   case ${option} in
      c)
	 CCSSFLAG="${TRUE}"
         ;;
      b)
	 BFF_FLAG="${TRUE}"
         ;;
      m)
         MEDIATYPE="${OPTARG}"
         COMMANDLINE_MEDIAOPTION="${TRUE}"
         ;;
      d)
	 OUTPUTDIR=$(cd $OPTARG; pwd)
	 ;;
      :)
         print -u2 "$OPTARG requires a value"; Usage; Clean_Up; exit 1;;
     \?)
         print -u2 "unknown option $OPTARG"; Usage; Clean_Up; exit 1;;
   esac
done
shift OPTIND-1

trap 'Clean_Up; exit 4' INT QUIT HUP TERM

if [[ "${CCSSFLAG}" = "${FALSE}" && "${BFF_FLAG}" = "${FALSE}" ]]
then
	print -u2 "One or both of the -b and -c options are required.\n"
	Usage
	Clean_Up
	exit 1
fi

bldinit ptfpkg

#----------------------------------------------------------------
# Define TOP related variables first since they will be used	|
# by other variables.						|
#----------------------------------------------------------------
typeset    HISTORYTOP=${TOP}/HISTORY
typeset    PTFTOP=${TOP}/PTF
typeset    UPDATETOP=${TOP}/UPDATE              # Top of the update tree

typeset -r HOST="$(uname -n)"		# Machine ptfpkg is being
					# run on.
typeset    BOOT
typeset    BOOTFILES
typeset    BOOTLIBS
typeset    COMMANDLINE_MEDIAOPTION="${FALSE}"	# Will be ${TRUE} if -m
						# is entered from command line.
typeset    COMMENT[]				# Comment entry from .lp file
						# file.
typeset -i COMMENT_INDEX=1			# Number of elements in the
						# comment array.
typeset    CONTENT				# Content entry from .lp file
						# file.
typeset    CUMSLIST				# Location of cumsList file
						# for ptf.
typeset    DEFECTAPARS=${BLDTMP}/${command}.tmpdefectapars.${HOST}.$$
						# Combined defect apars file
						# from all build cycles.
typeset    FILENAME				# File name entry from 
						# ptf_pkg file.
typeset    FILENAMESLIST			# List of files in PTF,
						# created by processPtf.
typeset    FIX					# Fix entry from the vrmfFile
typeset    FIXDATADB=${HISTORYTOP}/fixdataDB	# database file
typeset    FORMAT				# Format entry from the
						# lpp_info file.
typeset    LANG					# Language entry from .lp file
						# file.
typeset    LPP					# Current LPP being processed.
typeset    LPP_locked="${FALSE}"		# If ${TRUE} then bldlock is
						# still in effect for this LPP.
typeset    LPPLIST				# File containing list of LPPs
						# that need to be processed.
typeset    MEDIATYPE
typeset	   ML_NMBR                              # ML value for mainLevelFile
typeset    MOD					# Mod entry from vrmfFile.
typeset    OPTION				# Option that current PTF is
						# from.
typeset    OPTIONPATH				# Directory under ${UPDATETOP}
						# information about ${OPTION}
						# is kept.
typeset    PLATFORM				# Platform entry from lpp_info
						# file.
typeset    PRODNAME				# Prodname entry from lpp_info
						# file.
typeset    PTF					# Current PTF being processed.
typeset    PTF_PKG				# Filename of ptf_pkg file for
						# this build cycle.
typeset    PTF_PKG_BACK				# Filename of backup ptf_pkg
typeset    PTFAPARDEFMASTER=${HISTORYTOP}/ptfapardef.master
						# file for this build cycle.
typeset    PTFDIR				# Location where the PTF will
						# be created and stored.
typeset    PTFOPTFILE=${HISTORYTOP}/ptfoptions
typeset    PTFREQSLIST=${HISTORYTOP}/ptfreqsFile.${BLDCYCLE}
typeset    PTFSLIST				# Location of ptfsList file
typeset    RC=0					# global return code
typeset    rc					# local return code
typeset    RELEASE				# Release entry from vrmfFile
						# file.
typeset    ROOT_PART				# ${TRUE} if current option
						# has a root part.  ${FALSE}
						# otherwise.
typeset    SPACE_TAB="${SPACECHARACTER}${TABCHARACTER}"
						# Space or Tab character.
typeset    TMPFILE=${BLDTMP}/${command}.tmpfile.${HOST}.$$
typeset    TMPPTFOPTFILE=${BLDTMP}/${command}.tmpptfoptfile.${HOST}.$$
typeset    TMPVRMFFILE=${HISTORYTOP}/internalvrmfTable.${BLDCYCLE}
typeset    TMPMAINTLEVELFILE="${BLDTMP}/${command}.tmpmaintLevelFile${HOST}.$$"
						# location of internal vrmf table 
typeset    VERSION				# Version entry from vrmfFile
typeset    VRMF				        # vrmf value of the current 
						# ptf's option 

typeset    ABSTRACTS=${HISTORYTOP}/abstracts    # Location of abstracts file

LPPLIST="${UPDATETOP}/lpplist.${BLDCYCLE}"

[[ ! -f ${LPPLIST} ]] && $log -x "Cannot open LPP list file ${LPPLIST}"


# check if ptf options file exists
if [ ! -f ${PTFOPTFILE} ] 
then
    # create the ptfoptions file
    $log -w "The ptfoptions file does not exist, touching ptfoptions file."
    touch ${PTFOPTFILE}
fi

# generate the internal vrmf table
getvrmf
[[ $? -ne 0 ]] && $log -x "Failed to generate internal vrmf table."

# create the fixdata database
# 
typeset -i cumCnt=0
typeset -i ptfCnt=0
typeset -i i=0

while read LPP
do
   PTF_PKG=${UPDATETOP}/${LPP}/ptf_pkg.${BLDCYCLE}
   ptfCnt=$ptfCnt+$(cat ${PTF_PKG} | wc -l)
   cumCnt=$cumCnt+$(grep "\|cum_ptf\|" ${PTF_PKG} | wc -l)
done <${LPPLIST}

if [[ $ptfCnt != $cumCnt ]]
then
    updatefixdata -f ${TMPVRMFFILE} -a ${ABSTRACTS}
    rc=$?
    [[ $rc -ne 0 && "$BUILD_TYPE" = "production" ]] && 
	$log -x "Updatefixdata failed to update fixdata database for \n\
		production level build."
    [[ $rc -ne 0 && "$BUILD_TYPE" != "production" ]] && 
	$log -w "Updatefixdata failed to update fixdata database."
elif [[ $cumCnt > 0 ]]
then
   echo "*** NEED to add auto-generation of PMP fixdata entry"
fi

while read LPP
do
   BOOTFILES="${UPDATETOP}/${LPP}/bootFiles"
   BOOTLIBS="${UPDATETOP}/${LPP}/bootLibs"
   PTF_PKG=${UPDATETOP}/${LPP}/ptf_pkg.${BLDCYCLE}
   PTF_PKG_BACK=${UPDATETOP}/${LPP}/ptf_pkg_back.${BLDCYCLE}
   MAINTLEVELFILE="${UPDATETOP}/${LPP}/maintLevelFile"
   ML_NMBR=0

   if [[ "${LPP}" = "bos" ]] 
   then
      gen_boot_info
      [[ $? -ne 0 ]] && $log -x "Failed to generate boot information"
   fi

   if [[ ! -f ${PTF_PKG} ]]
   then
      $log -e "Skipping ${LPP}"
      $log -e "Cannot find ${PTF_PKG}"
      continue
   fi

   bldlock -l "${LPP} ${BLDCYCLE}" -t 2
   LPP_locked="${TRUE}"

   # Make a backup of the ptf_pkg file.
   echo "#*** DATE: `date` ***" >> ${PTF_PKG_BACK}
   cat ${PTF_PKG} >> ${PTF_PKG_BACK} 2> /dev/null
   [[ $? -ne 0 ]] && $log -x "Cannot backup ${PTF_PKG} into ${PTF_PKG_BACK}"

   process_ptfs

   # Update the maintlevel file for the lpp after the pmp ptf 
   # is created successfully
   if [[ $cumCnt > 0 && $ML_NMBR > 0 && ! -s ${PTF_PKG} ]] 
   then
       cat ${MAINTLEVELFILE} | read junk keyword
       echo $ML_NMBR $keyword > ${TMPMAINTLEVELFILE}
       mv ${TMPMAINTLEVELFILE} ${MAINTLEVELFILE} >/dev/null 2>&1
       if [[ $? -ne 0 ]]
       then 
	   $log -w "Failed to move ${TMPMAINTLEVELFILE} into ${MAINTLEVELFILE}"
	   $log -w "May need to manually update the maintainance level file."
       fi
    fi


   bldlock -u "${LPP} ${BLDCYCLE}"
   LPP_locked="${FALSE}"

done < ${LPPLIST}


if [[ $RC -eq 0 ]]
then
    #---------------------------------------------------------
    # Be sure that all PTFs were successfully generated.
    # set RC to non-zero if they weren't
    #---------------------------------------------------------
    while read LPP
    do
	ppFile="${UPDATETOP}/${LPP}/ptf_pkg.${BLDCYCLE}"
	if [[ -f ${ppFile} && -s ${ppFile} ]]
	then
	    $log -e "Not all PTF's built successfully for '${LPP}'"
	    RC=-1
	fi
    done <${LPPLIST}
fi

Clean_Up
exit $RC
