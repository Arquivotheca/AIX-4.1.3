# @(#)67	1.5  src/bos/usr/bin/sysdumpdev/cpydmpdskt.sh, cmdtrace, bos411, 9428A410j 6/30/94 15:58:05
#
# COMPONENT_NAME:   CMDDUMP   SMIT shell interface program
#
# FUNCTIONS:  interfaces to dd, sysdumpdev, backup
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

# $1 is the output device to where the dump will be copied.
DMP_TO_DEV=${1:-"/dev/rfd0"}

CLEANUP="mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename>/dev/null 2>&1"
trap "$CLEANUP; exit 1" 2

	mv /var/adm/ras/copyfilename /var/adm/ras/copyfilename.org>/dev/null 2>&1

        sysdumpdev -L 2>&1 | sed 's/^.*:  *//' | {
        read blank; read blank
        read DMP_SRC
        read major
        read minor
        read CNT bytes
        echo $DMP_SRC | fgrep : >/dev/null
        rc=$?
        if  [ $rc -eq 0 ]
        then
          echo `dspmsg cmddump.cat -s 1 536 "Copy a remote dump file is not allowed.\n"`
	  mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename>/dev/null 2>&1
          exit 1
        else
	  LVNAME=`echo $DMP_SRC | cut -c6-`	 
	  LVTYPE=`/usr/sbin/lslv $LVNAME | grep "TYPE:" | awk '{print $2}'` 
	  # If the dump device is paging, the dump isn't there anymore.
	  if [ "$LVTYPE" = "paging" ]
	  then
		dspmsg cmddump.cat -s 1 557 "The dump on %s is no longer valid.  To see if\n\
the dump was copied to a file on boot run the following command:\n\
       sysdumpdev -L\n" $DMP_SRC
	  	mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename>/dev/null 2>&1
	        exit 1
	  fi
          TEMP=/tmp/dump_image_to_be_copied_to_diskette
          BLK_SIZ=51200
          if [ $CNT -gt 0 ]
          then
            COUNT=`expr $CNT / $BLK_SIZ`
            OVFL=`expr $CNT % $BLK_SIZ`
            if [ "$OVFL" != "0" ]
            then COUNT=`expr $COUNT + 1`
            fi
            echo "dd <$DMP_SRC >$TEMP bs=$BLK_SIZ count=$COUNT"
                  dd <$DMP_SRC >$TEMP bs=$BLK_SIZ count=$COUNT
            if [ "$?" != "0" ]
            then rc=1
            else
              ls $TEMP | backup -qif$DMP_TO_DEV
              rc=$?
            fi
            rm $TEMP
          else rc=1
          fi
        fi
        }
mv /var/adm/ras/copyfilename.org /var/adm/ras/copyfilename>/dev/null 2>&1
exit $rc
