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
#      7135_get_lun_info -freelun
#                        -group2dac darname grpnum
#                        -otherdac grpnum
#                        -group2free darname grpnum
#
#-----------------------------------------------------------------------

case "$1" in
   "-freelun")   raidmgr -O -l $3 | tail -1 | cut -d":" -f4
                 break;;

  "-ownerdac")   raidmgr -O -l $3 | tail -1 | cut -d":" -f2
                 break;;

  "-raidlevel")  raidmgr -O -l $3 | tail -1 | cut -d":" -f5
                 break;;

   "-blocksize") raidmgr -O -l $3 | tail -1 | cut -d":" -f7
                 break;;

  "-segsize")    raidmgr -O -l $3 | tail -1 | cut -d":" -f9
                 break;;

  "-seg0size")   raidmgr -O -l $3 | tail -1 | cut -d":" -f10
                 break;;

   "-size")      if [ $3 = "-" ]; then
                    echo ERROR: An invalid LUN was selected.
                    exit 1
                 fi
                 raidmgr -O -l $3 | tail -1 | cut -d":" -f8
                 break;;

  "-recondelay") if [ $3 = "-" ]; then
		    echo N/A
                    exit 0
                 fi
                 raidmgr -O -l $3 | tail -1 | cut -d":" -f11
                 break;;

  "-reconblocks") if [ $3 = "-" ]; then
		     echo N/A
                     exit 0
                  fi
                  raidmgr -O -l $3 | tail -1 | cut -d":" -f12
                  break;;

  "-rsvlock")   if [ $3 = "-" ]; then
		   echo N/A
                   exit 0
                else
                   lock=`raidmgr -O -l $3 | tail -1 | cut -d":" -f15`
                   if [ $lock = "yes" ]; then
                      echo yes
                      echo no
                   else
                      echo no
                      echo yes
                   fi
                fi;;

  "-qdepth")      if [ $3 = "-" ]; then
		     echo N/A
                     exit 0
                  fi
                  raidmgr -O -l $3 | tail -1 | cut -d":" -f14
                  break;;

  "-group2dac")  for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       echo $device | cut -d: -f7
                       exit 0
                    fi
                 done

                 echo Invalid device
                 exit 1
                 break;;

  "-group2raid") for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       echo $device | cut -d: -f4
                       exit 0
                    fi
                 done

                 echo Invalid device
                 exit 1
                 break;;

  "-group2blk")  for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       devname=`echo $device | cut -d: -f5`
                       break
                    fi
                 done

                 raidmgr -Y -l $devname | cut -d: -f7 | tail -1
                 break;;

  "-group2free") for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` -a "-" = `echo $device | cut -d: -f5` ]
                    then
                       echo $device | cut -d: -f6
                       exit 0
                    fi
                 done

                 echo 0
                 exit 1
                 break;;

  "-group2seg")  for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       devname=`echo $device | cut -d: -f5`
                       break
                    fi
                 done

                 raidmgr -Y -l $devname | cut -d: -f9 | tail -1
                 break;;

  "-group2seg0") for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       devname=`echo $device | cut -d: -f5`
                       break
                    fi
                 done

                 raidmgr -Y -l $devname | cut -d: -f10 | tail -1
                 break;;

  "-group2recont") for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       devname=`echo $device | cut -d: -f5`
                       break
                    fi
                 done

                 raidmgr -Y -l $devname | cut -d: -f11 | tail -1
                 break;;

  "-group2reconb") for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       devname=`echo $device | cut -d: -f5`
                       break
                    fi
                 done

                 raidmgr -Y -l $devname | cut -d: -f12 | tail -1
                 break;;

  "-otherdac")   for device in `7135_device_config -l $2 | tail +2`
                 do
                    if [ $3 = `echo $device | cut -d: -f1` ]
                    then
                       owner=`echo $device | cut -d: -f7`
                       break
                    fi
                 done

                 for dac in `7135_subsys_config -l $2 | grep $2 | cut -d: -f2`
                 do
                    if [ $dac != $owner ]; then
                       echo $dac
                       break
                    fi
                 done
                 break;;

  *)             exit 1
                 break;;

esac
