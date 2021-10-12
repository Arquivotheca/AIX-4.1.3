#! /bin/ksh
# @(#)69	1.12  src/bldenv/pkgtools/mkodmupdt.sh, pkgtools, bos412, GOLDA411a  8/26/94  10:15:18 
#
# FUNCTIONS: mkodmupdt
#
# ORIGINS: 27
#
# mkodmupdt.sh --- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
rm=$ODE_TOOLS/usr/bin/rm
awk=$ODE_TOOLS/usr/bin/awk
chmod=$ODE_TOOLS/usr/bin/chmod
echo=$ODE_TOOLS/usr/bin/echo
getopt=$ODE_TOOLS/usr/bin/getopt
sed=$ODE_TOOLS/usr/bin/sed
cat=$ODE_TOOLS/usr/bin/cat
mv=$ODE_TOOLS/usr/bin/mv
egrep=$ODE_TOOLS/usr/bin/egrep
basename=$ODE_TOOLS/usr/bin/basename
cp=$ODE_TOOLS/usr/bin/cp
mkdir=$ODE_TOOLS/usr/bin/mkdir
uniq=$ODE_TOOLS/usr/bin/uniq

# NAME: cleanup
#
# FUNCTION: Clean up after running
#
# INPUT: None.
#
# OUTPUT:None
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
cleanup() 
{
	$rm -f previous_delete current_delete previous_get
	$rm -f $odmdel_script $unodmadd_script $odmadd_stanzas
	$rm -f ./curstanza.processed ./prevstanza.processed
	return 0
}

# NAME: usage
#
# FUNCTION: usage statement.
#
# INPUT: None.
#
# OUTPUT: Prints usage statement.
#
# SIDE EFFECTS: None
#
# RETURNS: 0 always.
#
usage() 
{
   $echo  "$1: Usage: [-p previous_stanza_list] -c <current_stanza_list> "
   $echo  "	      -t <objectclass table> -o <option> [-d output_files_dir]"
   $echo  "	      [-u] [-i]                                          "
   $echo  "           where: -u is for an update and -i is for an install"
   $echo  "                  Can generate either install or update type  "
   $echo  "                  scripts if the -p flag is not set.  If -p,  "
   $echo  "                  -i, or -u is not specified, will default to "
   $echo  "                  update type scripts.                        "
}

install="no"
update="no"

# ODE_TOOLS is set if running from 4.1 build environment not otherwise.
if [[ -z "$ODE_TOOLS" ]]
then
	mkodmextract=mkodmextract
	processStanza=processStanza
	odmupdate=odmupdate
else
	mkodmextract=$ODE_TOOLS/usr/bin/mkodmextract
	processStanza=$ODE_TOOLS/usr/bin/processStanza
	odmupdate=$ODE_TOOLS/usr/bin/odmupdate
fi

trap 'cleanup; exit 1' QUIT HUP INT TERM

DIR="./"   # Default to current directory for output files.

# Give usage if zero arguments are given
[ $# -eq 0 ] && usage $0 && exit

# set the positional parameters and specifiy valid flags
set -- `$getopt d:c:p:o:t:iu $*`
[ $? -ne 0 ] && usage $0 && exit

# parse the input parameters
while [ $1 != -- ]
do
  case $1 in
	-c) current_stanza_list=$2
	    shift; shift
	    ;;
	-p) previous_stanza_list=$2
	    shift; shift
	    ;;
	-o) option=$2
	    shift; shift
	    ;;
	-t) export OBJCLASSDB=$2
	    shift; shift
	    ;;
	-d) DIR=$2/
	    shift; shift
	    ;;
	-i) install="yes"
	    shift
	    ;;
	-u) update="yes"
	    shift
	    ;;
  esac
done
shift	# shift past the -- from getopt

# Check if install or update specified
if [[ $previous_stanza_list = "" ]]
then
    if [[ $install = "no" && $update = "no" ]]
    then
        update="yes"
    else
        if [[ $install = "yes" && $update = "yes" ]]
        then
            echo "$0: Cannot specify both install and update type scripts. "
            echo "$0:   Will default to update type scripts. "
            install="no"
        fi
    fi
else
    if [[ $install = "yes" ]]
    then
        echo "$0: Cannot generate install type scripts when a previous "
        echo "$0:   file is specified.  Update type scripts will be "
        echo "$0:   generated "
        install="no"
    fi
fi

# Make sure that current_stanza_list is set
if [[ -z "$current_stanza_list" ]]
then
	$echo "$0: You must specify the current stanza file with -c"
	usage $0
	exit 1
fi

# Make sure that OBJCLASSDB is set
if [[ -z "$OBJCLASSDB" ]]
then
	$echo "$0: You must specify the object class table with -t "
	$echo "		or the environment variable OBJCLASSDB"
	usage $0
	exit 1
fi

# Make sure that option is set
if [[ -z "$option" ]]
then
	$echo "$0: You must specify the option name with -o"
	usage $0
	exit 1
fi

#Make sure that the object class descriptor table exists
if [[ ! -s "$OBJCLASSDB" ]]
then
	   $echo "$0: $OBJCLASSDB file is empty or does not exixt"
           exit 1
fi
           
# Make sure that the current file exists
if [[ -s "$current_stanza_list" ]]
then
	bcurrent_stanza_list=$($basename $current_stanza_list)
else
	$echo "$0: $current_stanza_list does not exist or is empty" && exit 1
fi

# if the current file name does not end with .add issue a 
# warning and continue
cur=$current_stanza_list
$echo $cur | $egrep .add$ > /dev/null 2>&1
if [[ $? -ne 0 ]]
then
     $echo "$0: WARNING: The current_stanza_file $($basename $current_stanza_list)" >& 2
     $echo " 		should end with .add" >& 2
 fi

# initialize variables
odmadd_stanzas=""
odmdel_script=""
unodmadd_script=""
rodmadd_stanzas=""

if [[ -n "$DIR" && ! -d "$DIR" ]]
then 
    $mkdir -p `$basename $DIR` > /dev/null
    if [[ $? -ne 0 ]]
    then
        $echo "mkodmupdt: WARNING: Could not create directory $DIR.\n" >& 2
    fi
fi

# If previous stanza file was given then we are running mkodmupdt 
# to create update odm scripts. Make sure that previous stanza
# file exists. Also check to see if previous stanza file name end
# with .add. If not then give a warning and continue processing.
# Run processStanza for previous and current stanza files to
# remove any comments and embedded blank lines. This will create
# two temporary files in the current directory, prevstanza.processed
# and curstanza.processed that have stanzas seperated by blank
# lines. These files are used by odmupdate perl script to create
# odm scripts for updates.
if [[ -n "$previous_stanza_list" ]]
then
    if [[ ! -f "$previous_stanza_list" ]]
    then 
       $echo "$0: $previous_stanza_list does not exist"
       exit 1
    fi

    prev=$previous_stanza_list
    $echo $prev | $egrep .add$ > /dev/null 2>&1
    if [[ $? -ne 0 ]]
    then
      $echo "$0: WARNING: The previous_stanza_file $($basename $previous_stanza_list)" >& 2
      $echo "		 should end with .add" >& 2
    fi

    # call processStanza function to process the previous and 
    # current stanza files.
    $processStanza -p $previous_stanza_list -c $current_stanza_list
    if [[ $? -ne 0 ]]
    then
        $echo "mkodmupdt: ERROR: processStanza failed.\n"
        cleanup && exit 1
    fi

    # Call the perl script odmupdate to generate odmadd, odmdel and unodmadd
    # scripts for updates.
    bname=${bcurrent_stanza_list%.*}

    $odmupdate $OBJCLASSDB $option $previous_stanza_list $bname $DIR \
              prevstanza.processed curstanza.processed
    if [[ $? -ne 0 ]]
    then
        $echo "mkodmupdt: ERROR: odmupdate failed.\n"
        cleanup && exit 1
    else
	$rm -f ./curstanza.processed ./prevstanza.processed
    fi
 
else
    # The previous stanza list is optional.  If one was not given, 
    # then we assume that mkodmupdt command is being called for 
    # install only.

    #generate the output file names .
    odmadd_stanzas=$DIR${option}.${bcurrent_stanza_list%.*}.odmadd
    odmdel_script=$DIR${option}.${bcurrent_stanza_list%.*}.odmdel
    unodmadd_script=$DIR${option}.${bcurrent_stanza_list%.*}.unodmadd
    odmdel_name=${option}.${bcurrent_stanza_list%.*}.odmdel
    unodmadd_name=${option}.${bcurrent_stanza_list%.*}.unodmadd

    rodmadd_stanzas=`$echo $odmdel_name | $sed 's/odmdel$/rodmadd/'`

    # create the .odmadd file . For install .odmadd file is exactly the 
    # same as the current stanza file.

    $cp $current_stanza_list $odmadd_stanzas

    $rm -f previous_delete current_delete previous_get

    $echo $0: Creating odm scripts for $bcurrent_stanza_list
    $mkodmextract -d $OBJCLASSDB -f $current_stanza_list  > current_delete
    rc=$?
    if [ $rc != 0 ]
    then
       $echo "$0: mkodmextract failed, rc = $rc" >& 2
       cleanup && exit 1
    fi
 
    # There should be one uniq odmdelete command for all the stanzas that
    # have the same key value.
    $uniq current_delete > previous_delete
    $cp previous_delete current_delete

    if [[ $update = "yes" ]]
    then
        # converting the odmdelete command to odmget command.
        # the format of each command in the current_delete file is :
        # odmdelete -o <descriptor> [ -q creteria]
        # A command may span over multiple lines.
        # An odmdelete command will always have a pattern 
        # odmdelete -o <discriptor>  at the beginning and /dev/null at the end.
        # While processing each line we look for odmdelete in the 
        # first field. If found then we replace it with odmget, save the 
        # descriptor for later use which is in the third field and 
        # print the rest of the fields on the lines as such.
        # Then we need to print the spaces or tab string before each line,
        # and print the rest of the line that does not have "odmdelete"
        # string in it. This is done so that teh output looks exactly
        # like the input lines.
        # If the last field on the line matches "/dev/null" then replace it
        # with <descriptor> >> $SAVEDIR/<rodmadd_file> else print the last field
        # as such. 

        ${awk} '{
            if (match($1,"odmdelete")){
                printf("odmget  "); 
                descr=$3;
                for(i=4;i<NF;i++) printf("%s ", $i);
            }
            else
            { 
                n=match($0,/[^ 	]/);
                initspace=substr($0, 0, n-1);
                printf("%s", initspace);
                for(i=1;i<NF;i++) printf("%s ", $i);
            }

            if (match($NF,"\/dev\/null")){
                printf("%s >> $SAVEDIR/%s\n" , descr, rodmadd);
            }
            else
                printf("%s \n", $NF);
        }' rodmadd=$rodmadd_stanzas previous_delete > previous_get
    fi

    # We want to redirect stdout for the odmdelete commands because otherwise
    # it becomes too much for a customer when applying a ptf with many stanza
    # changes (i.e. > 100).  We have to do it for both previous and current.

    if [ -s previous_delete ] ; then
        # only add the odmgets for an update
        if [[ $update = "yes" ]]
        then
            echo "if [ \"\$INUSAVE\" = \"1\" ] ; then" > $odmdel_script
            $cat previous_get 			>> $odmdel_script
            $echo "chmod a+x \$SAVEDIR/$rodmadd_stanzas " >> $odmdel_script
            $echo "fi"                                 >> $odmdel_script
            $cat previous_delete 			>> $odmdel_script
        else
            $cat previous_delete 			> $odmdel_script
        fi
    fi

    if [ -s current_delete ] ; then
        $cat current_delete 		> $unodmadd_script
    fi
    $echo Done
    $rm -f previous_delete current_delete previous_get

    #The odm scripts have to be executables.
    $chmod a+x $odmdel_script
    $chmod a+x $unodmadd_script

fi
