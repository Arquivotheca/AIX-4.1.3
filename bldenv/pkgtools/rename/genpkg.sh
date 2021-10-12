#!/bin/sh
# @(#)40        1.1  src/bldenv/pkgtools/rename/genpkg.sh, pkgtools, bos412, GOLDA411a 7/22/91 12:31:44
# COMPONENT_NAME: genpkg.sh 
#
# FUNCTIONS:
#	1. Copy BLDIR/lpp_name to the current dir (update tree).
#	2. Copy BLDIR/liblpp.a to the current dir.
#	3. If 'root type', copy BLDIR/root/liblpp.a to current dir.
#	4. Copy BLDIR/<lppname>.lp to current dir.
#	5. Copy BLDIR/<lpp-opt>.upsize to <lpp-opt>.insize, if exists.
#	6. If 'root type', copy BLDIR/root/<lpp-opt>.upsize to <lpp-opt>.insize.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#  This process is called from 'genptf.sh' as part of the update process.
#
################################################################################

# Put the name of this program into variable myName for error messages.
# Also create a variable myFill of spaces same length for 2nd line.

myName=`basename $0`
myFill=`echo $myName | sed -e 's/./ /g'`
tpath=""				# Init tpath flag to negative.
ef="0"				# Init error flag 

# See if we can access build tree from whereever we are.

if [ ! -d $BLDIR ]; then
    echo "$myName: Cannot access build tree $BLDIR from here."
    echo "$myFill: Update process cannot continue."
    exit 1
fi

# If tpath exists, use it to find <lpp>.lp & store existence flag.

if [ -f $BLDENV/usr/bin/tpath ]; then	# Does tpath exist?  If so,
    tpath="y"				# set flag that 'tpath' cmd exists.
    lpFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null $BLDIR/$LPPNAME.lp`
else
    lpFile="$BLDIR/$LPPNAME.lp"
fi

if [ ! -r "$lpFile" ]; then		# Can we read <lpp-name>.lp file?
    echo "$myName: Cannot read $BLDIR/$LPPNAME.lp file."
    ef="1" 
fi

# Copy lpp_name from build tree to update tree.

if [ "$tpath" = "y" ]; then
    inFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null $BLDIR/lpp_name`
else
    inFile="$BLDIR/lpp_name"
fi

if [ ! -r "$inFile" ]; then
    echo "$myName: Cannot read lpp_name file."
    ef="1"
else
    cp $inFile lpp_name
fi

# Copy liblpp.a from build tree to update tree.

if [ "$tpath" = "y" ]; then
    inFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null $BLDIR/liblpp.a`
else
    inFile="$BLDIR/liblpp.a"
fi

if [ ! -r "$inFile" ]; then
    echo "$myName: Cannot read liblpp.a file."
    ef="1"
else
    cp $inFile liblpp.a
fi

# If 'root type', copy root/liblpp.a from build tree to update tree.

if [ "$LF" = "r" ]; then
    if [ "$tpath" = "y" ]; then
	inFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null $BLDIR/root/liblpp.a`
    else
	inFile="$BLDIR/root/liblpp.a"
    fi
    if [ ! -r "$inFile" ]; then
	echo "$myName: Cannot read root/liblpp.a file."
	ef="1"
    else
	cp "$inFile" root/liblpp.a
    fi
fi

# If any step failed, then exit now

if [ "$ef" != "0" ]; then
    echo "$myName: Update process cannot continue."
    exit 1
fi

# Copy <lpp>.lp from build tree (or tpath path) to update tree.
# Strip all comments and blank lines in process.


sed -e 's/^[#*].*$//' -e '/^[   ]*$/d' < $lpFile > $LPPNAME.lp


# If any <lpp-opt>.upsize exist, copy them to .insize in update tree.

cat $LPPNAME.lp | while read sizeFile rst; do
    if [ $tpath = "y" ]; then
	inFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null \
	$BLDIR/$sizeFile.upsize`
    else
	inFile=$BLDIR/$sizeFile.upsize
    fi
    if [ -f "$inFile" ]; then
	cp $inFile $sizeFile.insize
    fi
done
      
   

# If 'root type' and any root/<lpp-opt>.upsize files exist,
# copy them to UPDIR/root/<lpp-opt>.insize.

if [ $LF = "r" ]; then			 # Does 'root type' dir exist?
    cat $LPPNAME.lp | while read sizeFile rst; do
    if [ $tpath = "y" ]; then
	inFile=`eval $BLDENV/usr/bin/tpath 2> /dev/null \
		$BLDIR/root/$sizeFile.upsize`
    else
	inFile=$BLDIR/root/$sizeFile.upsize
    fi
    if [ -f "$inFile" ]; then
	cp $inFile root/$sizeFile.insize
    fi
    done
fi

exit 0

#### End of genpkg #### End of genpkg #### End of genpkg #### end of genpkg ###
