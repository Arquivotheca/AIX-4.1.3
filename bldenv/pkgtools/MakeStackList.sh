# @(#)06        1.3  src/bldenv/pkgtools/MakeStackList.sh, pkgtools, bos41J, 9507A_notx 2/5/95 19:06:30
#!/bin/ksh
#
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS:  OrderImages
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#------------------------------------------------------------------------------
#
#
export PATH=$ODE_TOOLS/usr/bin:$PATH
# 
# 01/17/95 - Aubertine:  Added code to point SoftWindows to aix412 
#
if [[ -d $images ]]
then 
	print "\n\tChanging directory to $images\n"
	cd $images
	if [[ $? != 0 ]]
	then
		print "\n\tCannot cd to $images"
		exit 1
	fi
fi
# This shell should be exec'd while in the images directory.
  if (basename ${PWD} | grep images) > /dev/null
  then continue
  else 
    echo "\n\tYou must be in inst.images to use `basename $0`\n"
    exit 1
  fi
# Check to make sure afs is installed on this machine
  if [ -d /afs/austin/local/bin ]
  then
    # Link the IDD and Other LPP images into inst.images directory
      ln -s -f /afs/austin/aix/project/aix411/build/IDD_IMAGES/* .
      ln -s -f /afs/austin/aix/project/aix411/build/GOLD.PII/* .
    # Link the Other images into inst.images directory
      ln -s -f /afs/austin/aix/project/aix411/build/other.images/* .
      ln -s -f /afs/austin/aix/project/aix412/build/other.images/* .
      ln -s -f \
       /afs/austin/aix/project/aix412/build/other.images/SoftWindows SoftWindows
      ln -s -f /afs/austin/aix/project/aix41B/build/other.images/* .
      rm -f README
      rm -f checksums.gold
      rm -f *.hypertext
      rm -f stack.list
    # Order each of the built lists
      for i in client hypertape server \
               blkmux des dps dsmit essl hcon \
               hypercd icraft infoxl netware \
               opengl perfagent perfmgr pex powerdt \
               sx25 sx25lite uimx ums development
      do
        if [ -s MASTER_STACK/stack.$i ]
        then
          mv MASTER_STACK/stack.$i MASTER_STACK/.stack.$i
        fi
        echo "Ordering stack.$i"
        if [ "$i" != "development" ]
        then
          $build/ode_tools/power/usr/bin/OrderImages MASTER_STACK/.stack.$i > /tmp/OrderIm.out 2>&1
          /bin/ls -rt *error* | tail -1 | xargs rm
          mv stack.list MASTER_STACK/stack.$i
        else
          if [ -s stack.$i ]
          then
            mv stack.$i .stack.$i
            $build/ode_tools/power/usr/bin/OrderImages -s .stack.$i
          fi
        fi
      done
    # Remove des from stack.list and stack.list.lpp
    grep des stack.list* && {
      grep -v des stack.list > stack.list0
      mv stack.list0 stack.list

    }
  else
      echo "\n\tafs must be mounted and accessible\n"
  fi
