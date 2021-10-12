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
#      7135_subsys_config -l darname
#
#-----------------------------------------------------------------------

echo subsysname:compname:status:location:adapterid:busid:type:icon:selectable:description

# Look for the dar name that was passed in
darlist=`lsdev -Ct dar -F name`
for dar in $darlist
do
   if [ "$dar" = "$2" ]
   then

      # get controller information
      b=`get_config -l $dar | head -1`
      dac1=`echo $b | cut -d" " -f1`
      dac1_string=`echo $b | cut -d" " -f2`
      dac2=`echo $b | cut -d" " -f3`
      dac2_string=`echo $b | cut -d" " -f4`

      case "$dac1_string" in
         "ACTIVE") if [ $dac2_string = "ACTIVE" ]; then
                      dac1_status=A
                      dac1_string="Dual Active"
                   else
                      dac1_status=B
                      dac1_string="Active"
                   fi;;
         "PASSIVE") dac1_status=C
                    dac1_string="Passive";;
         "RESET")   dac1_status=D
                    dac1_string="Failed";;
         *)         dac1_status=E
                    dac1_string="None";;
      esac

      # convert controller state to output format
      case "$dac2_string" in
         "ACTIVE") if [ $dac1_status = "A" ]; then
                      dac2_status=A
                      dac2_string="Dual Active"
                   else
                      dac2_status=B
                      dac2_string="Active"
                   fi;;
         "PASSIVE") dac2_status=C
                    dac2_string="Passive";;
         "RESET")   dac2_status=D
                    dac2_string="Failed";;
         *)         dac2_status=E
                    dac2_string="None";;
      esac

      if [ "$dac1_status" != "E" ]; then
         location_dac1=`lsdev -Cl $dac1 | awk '{ print $3 }'`
         adapter_dac1=`echo $location_dac1 | cut -c4,5`
         bus_dac1=`echo $location_dac1 | cut -c7,8`

         echo $dar:$dac1:$dac1_status:$location_dac1:$adapter_dac1:$bus_dac1:device:dac.icon:yes:IBM 7135 $dac1_string Array Controller
      fi

      if [ "$dac2_status" != "E" ]; then
         location_dac2=`lsdev -Cl $dac2 | awk '{ print $3 }'`
         adapter_dac2=`echo $location_dac2 | cut -c4,5`
         bus_dac2=`echo $location_dac2 | cut -c7,8`

         echo $dar:$dac2:$dac2_status:$location_dac2:$adapter_dac2:$bus_dac2:device:dac.icon:yes:IBM 7135 $dac2_string Array Controller
      fi

      # Assimilate the SCSI adapter information
      adapters=`lsparent -C -k 'scsi' -Fname`
      for adapter in $adapters
      do
        adapter_info=`lsdev -Cl $adapter | sed 's/ /:/g'`
        adapter_location=`echo $adapter_info | cut -d: -f3`
        adapter_id=`echo $adapter_location | cut -c4,5`
        bus_id=`echo $adapter_location | cut -c7,8`
        if [ "$bus_id" = "" ]
        then
           bus_id="00"
        fi

        if [ "$dac1_status" != "E" ]; then
           if [ $adapter_id = $adapter_dac1 ]; then
              if [ $bus_id = $bus_dac1 ]; then
                 echo $adapter::0:$adapter_location:$adapter_id:$bus_id:controller:scsi.icon:no:SCSI I/O Controller
                 continue
              fi
           fi
        fi
        if [ "$dac2_status" != "E" ]; then
           if [ $adapter_id = $adapter_dac2 ]; then
              if [ $bus_id = $bus_dac2 ]; then
                 echo $adapter::0:$adapter_location:$adapter_id:$bus_id:controller:scsi.icon:no:SCSI I/O Controller
                 continue
              fi
           fi
        fi

      done
   fi
done


