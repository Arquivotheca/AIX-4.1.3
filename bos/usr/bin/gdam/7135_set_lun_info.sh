#! /usr/bin/ksh
#-----------------------------------------------------------------------
#
#  IBM 7135 RAIDiant Array Subsystem script for the Graphical
#       Disk Array Manager
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   Invocation:
#      7135_set_lun_info -changeowner <darname> <dacname> -y <groupnum>
#                        -mklun <dac> <raid> <blksize> <size> <segsize> <seg0size> <recondelay>
#                               <reconblks> <rsvlock> <qdepth> <drvlist>
#                        -mksublun <subsysname> <group> <devname> <dac> <raid> <blksize> <size>
#                                  <segsize> <seg0size> <recondelay> <reconblks> <rsvlock>
#                                  <qdepth> <drvlist>
#                        -chglun <hdisk> <recondelay> <reconblks> <rsvlock> <qdepth>
#                        -chkpar <hdisk>
#
#-----------------------------------------------------------------------

case "$1" in
   "-changeowner")
        for g in `7135_device_config -l $2 | tail +2 | tr -d ' '`; do
           if [ $5 = `echo $g | cut -d: -f1` ]; then
              hdisk=`echo $g | cut -d: -f5`
              status=`echo $g | cut -d: -f3`
              case $status in
                 [a-z]) echo ERROR: Cannot change controller ownership for an array owned by another host.
                        exit 1;;
                 [A-Z]) group=`raidmgr -T -l $3 | grep $hdisk | cut -d: -f1`
                        echo raidmgr -J -l $3 -g $group
                        raidmgr -J -l $3 -g $group
                        exit $?;;
                 *) break;;
              esac
           fi
        done
        break;;

   "-mklun")
        if [ $5 -gt `7135_get_drive_info -freespace $2 $3 "${12}"` ]; then
           echo ERROR: Requested array size exceeds the available space.
           exit 1
        fi
        if [ $8 -lt 1 -o $8 -gt 9999 ]; then
           echo ERROR: Reconstruction delay interval must be in the range of 1 to 9999.
           exit 1
        fi
        if [ $9 -lt 1 -o $9 -gt 9999 ]; then
           echo ERROR: Reconstruction block count must be in the range of 1 to 9999.
           exit 1
        fi
        if [ ${11} -gt 64 -o ${11} -lt 1 ]; then
           echo ERROR: Queue depth must be in the range of 1 to 64.
           exit 1
        fi

        echo raidmgr -A -l $2 -r $3 -b $4 -n $5 -s $6 -0 $7 -t $8 -R $9 -v ${10} -q ${11} -e "${12}"
        raidmgr -A -l $2 -r $3 -b $4 -n $5 -s $6 -0 $7 -t $8 -R $9 -v ${10} -q ${11} -e "${12}"

        break;;

   "-mksublun")
        # abort if not a free-space area
        if [ "$4" != "-" ]; then
           echo ERROR: Free space on the drive must be selected to create a sub array.
           exit 1
        fi
        if [ $8 -gt `7135_get_lun_info -group2free $2 $3` ]; then
           echo ERROR: Requested sub array size exceeds the available space.
           exit 1
        fi
        if [ ${11} -lt 1 -o ${11} -gt 9999 ]; then
           echo ERROR: Reconstruction delay interval must be in the range of 1 to 9999.
           exit 1
        fi
        if [ ${12} -lt 1 -o ${12} -gt 9999 ]; then
           echo ERROR: Reconstruction block count must be in the range of 1 to 9999.
           exit 1
        fi
        if [ ${14} -gt 64 -o ${14} -lt 1 ]; then
           echo ERROR: Queue depth must be in the range of 1 to 64.
           exit 1
        fi

        # Get a hdisk in same drive group
        for device in `7135_device_config -l $2 | tail +2 | tr -d ' '`
        do
           if [ $3 = `echo $device | cut -d: -f1` ]; then
              devname=`echo $device | cut -d: -f5`
              break
           fi
        done

        # Create the sub-array
        echo raidmgr -A -l $devname -r $6 -b $7 -n $8 -s $9 -0 ${10} -t ${11} -R ${12} -v ${13} -q "${14}"
        raidmgr -A -l $devname -r $6 -b $7 -n $8 -s $9 -0 ${10} -t ${11} -R ${12} -v ${13} -q "${14}"

        break;;

   "-chglun")
        if [ "$2" = "-" ]; then
           echo ERROR: A valid array or subarray must be selected.
           exit 1
        fi
        if [ $3 -lt 1 -o $3 -gt 9999 ]; then
           echo ERROR: Reconstruction delay must be in range of 1 to 9999.
           exit 1
        fi
        if [ $4 -lt 1 -o $4 -gt 9999 ]; then
           echo ERROR: Reconstruction block count must be in range of 1 to 9999.
           exit 1
        fi
        if [ $6 -lt 1 -o $6 -gt 64 ]; then
           echo ERROR: Queue depth must be in range of 1 to 64.
           exit 1
        fi

        echo raidmgr -M -l $2 -t $3 -R $4 -v $5 -q $6
        raidmgr -M -l $2 -t $3 -R $4 -v $5 -q $6
        break;;

   "-chkpar")
        if [ "$2" = "-" ]; then
           echo ERROR: A valid array or subarray must be selected.
           exit 1
        fi

        echo raidmgr -Z -l $2
        raidmgr -Z -l $2
        break;;

   "-delete")
        if [ "$2" = "-" ]; then
           echo ERROR: A valid array or subarray must be selected.
           exit 1
        fi

        echo raidmgr -D -l $2
        raidmgr -D -l $2
        break;;

   *) echo Unknown option: $1
      break;;

esac

