#!/bin/sh
# @(#)26	1.7  src/bldenv/pkgtools/gen_toc_entry.sh, pkgtools, bos412, GOLDA411a 2/19/93 17:15:39
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: decide_format
#		do31frag
#		getoutpt
#		modified
#		
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#========================================================================
#
#   Purpose:
#       This shell creates the toc_entry data from the
#       ./lpp_name file in the tar or bff.
#
#   Syntax:
#       gen_toc_entry -b bff_filename | -r tar_filename  \
#                     -t toc_entry_name [-h] [-?]
#
#   Flags:
#       -b bff_filename     the name of the input 'backup' format file
#       -r tar_filename     the name of the input 'tar' format file
#       -t toc_entry_name   The name of the output file (toc_entry).
#       -h -?               options to list the usage message
#
#
#   Change history:
#      04/24/91 created from v3.1 utility
#
#========================================================================

decide_format() {
  typefmt=`echo $stuff | cut -f1 -d" "`
  case "$typefmt" in
    3 )
      thisfragment=3.2
      ;;
    4 )
      thisfragment=4.1
      ;;
    * )
      thisfragment=3.1
      ;;
  esac
}
 
do31frag() {
  if expr "$stuff" : "[{}]" > /dev/null
  then
    out="$stuff"
  else
    if expr "$stuff" : "[0-9]" >/dev/null
    then
      out="$stuff"
    else
      echo "$stuff" | fgrep "#" >/dev/null
      if [ $? -eq 0 ]
      then
        out=
        for wrd in $stuff
        do
          if [ "$wrd" = "#" ]
          then
            break
          else
            out="$out $wrd"
          fi
        done
        getoutpt
      else
        out="$stuff"
        getoutpt
      fi
    fi
  fi
}
getoutpt() {
  set $out
  out="$1 $2"
  shift
  shift
  shift
  out="$out $*"
}
########
## declare proper format of the command
########

USAGE="\
usage: gen_toc_entry -b bff_filename | -r tar_filename \\ \n\
                      -t toc_entry_name [-h] [-?]\n\
       where:\n\
            -b bff_filename     the name of the input 'backup' format file\n\
            -r tar_filename     the name of the input 'tar' format file\n\
            -t toc_entry_name   The name of the output file (toc_entry).\n\
            -h -?               options to list the usage message\n"
##########
## if no args were specified, exit
##########
 if [ $# -eq 0 ]
 then
        echo "$USAGE"
        exit 100
 fi

##########
## loop thru the flags if any
## test for syntax error
##########
 set -- `getopt "t:b:r:h?" $*`
 if [ $? != 0 ]
 then
        echo "$USAGE"
        exit 110
 fi

 TOC_FILENAME=""
 BFF_FILENAME=""
 TAR_FILENAME=""

##########
## loop thru the flags
##########
 while [ "$1" != "--" ]
 do
        case $1 in
                "-b")
                        BFF_FILENAME=$2
                        shift
                ;;
                "-r")
                        TAR_FILENAME=$2
                        shift
                ;;
                "-t")
                        TOC_FILENAME=$2
                        shift
                ;;
                -*)
                        echo $USAGE
                        exit
                ;;
        esac
        shift
 done

 EXIT="FALSE"
##########
## CHECK if neither or both tar and bff specified
##########
 if [ "x$BFF_FILENAME" = "x"  -a  "x$TAR_FILENAME" = "x" ]
 then
         echo "gen_toc_entry: either -r or -b option required"
         EXIT="TRUE"
 fi

 if [ "x$BFF_FILENAME" != "x"  -a  "x$TAR_FILENAME" != "x" ]
 then
       echo "gen_toc_entry: options -r and -b are mutually exclusive"
       EXIT="TRUE"
 fi

##########
## check if the files specified are readable
##########
 if [ "x$BFF_FILENAME" != "x" ]
 then
    if [ ! -r $BFF_FILENAME ]
    then
       echo "gen_toc_entry: file $BFF_FILENAME cannot be read"
       EXIT='TRUE'
    fi
 fi

 if [ "x$TAR_FILENAME" != "x" ]
 then
    if [ ! -r $TAR_FILENAME ]
    then
       echo "gen_toc_entry: file $TAR_FILENAME cannot be read"
       EXIT='TRUE'
    fi
 fi


##########
## check if toc filename provided
##########
 if [ "x$TOC_FILENAME" = "x" ]
 then
      echo "gen_toc_entry: option -t required"
      EXIT='TRUE'
 fi

 if [ $EXIT = TRUE ]
 then
     exit 120
 fi


##########
## set misc. variables
##########
if [ "$BLDTMP" = "" ]; then
   BLDTMP="/tmp"
fi
TEMP_DIR=$BLDTMP/mktoc.dir.$$
CURRENT_DIR=`pwd`
#
 cd `dirname $TOC_FILENAME`
 TOC_FILENAME=`pwd`/`basename $TOC_FILENAME`
 cd $CURRENT_DIR
#
 if [ "x$BFF_FILENAME" != "x" ]
 then
    cd `dirname $BFF_FILENAME`
    BFF_FILENAME=`pwd`/`basename $BFF_FILENAME`
 else
    cd `dirname $TAR_FILENAME`
    TAR_FILENAME=`pwd`/`basename $TAR_FILENAME`
 fi
 cd $CURRENT_DIR


##########
## extract lpp_name file from bff
##########
mkdir $TEMP_DIR > /dev/null 2>&1
cd    $TEMP_DIR
rm -fr *

 if [ "x$BFF_FILENAME" != "x" ]
 then
     restore -x -f $BFF_FILENAME -q ./lpp_name  2>/dev/null 1>/dev/null
 else
     ###########################
     ## modified (butchered? ) by T. Krueger so as not to take
     ## forever to get the lpp_name file out of bos.
     ###########################
     tar     -x -f $TAR_FILENAME  ./lpp_name  2>/dev/null  1>/dev/null &
     lastpid=$!
     sleep 2
     while [ ! -f ./lpp_name ]
     do
       sleep 2
     done
     echo "Ignore the following 'killed' message."
     kill -9 $lastpid > /dev/null 2>&1
 fi
 cd $CURRENT_DIR

 if [ ! -f $TEMP_DIR/lpp_name ]
 then
     echo "gen_toc_entry: backup or tar file does not contain ./lpp_name file"
     rm -r $TEMP_DIR
     exit 172
 fi


##########
## empty toc entry file
##########
newlppname=$TEMP_DIR/newlppname
rm -f $newlppname
first_line=yes
cat $TEMP_DIR/lpp_name | while read stuff
do
  out=""
  if [ "$first_line" = "yes" ]
  then
    decide_format
    first_line=no
    out="$stuff"
  else
    case "$thisfragment" in
      3.2 )
        out="$stuff"
        ;;
      4.1 )
	out="$stuff"
	;;
      * )
        do31frag
        ;;
    esac
  fi
  echo "$out" >> $newlppname
done
 rm -f $TOC_FILENAME

##########
## copy lpp_name to toc
##########
 cp $newlppname $TOC_FILENAME

##########
## delete temp directory
##########
rm -r $TEMP_DIR

# end of shell
