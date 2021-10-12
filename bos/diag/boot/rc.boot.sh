# COMPONENT_NAME: DIAGBOOT
# @(#)98	1.29  src/bos/diag/boot/rc.boot.sh, diagboot, bos41J, 9523A_all 5/31/95 15:18:55
#
# FUNCTIONS: Diagnostic rc.boot Script
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# DIAG_TAPE_BOOT
# Change block size attribute of tape device and general
# config stuff for tape boot.

diag_tape_boot(){

	# Ensure tape is configured for 512 byte blocks

	/usr/lib/methods/showled 0xA40
	lsdev -Cc tape | while read tapename dummy
	do
		/usr/lib/methods/ucfgdevice -l $tapename >>$F1 2>&1
		/usr/lib/methods/chggen -l $tapename -a block_size=512 \
				>>$F1 2>&1
		`lsdev -Cl$tapename -rConfigure` -l $tapename >>$F1 2>&1
	done

	# cleanup drivers and config methods no longer needed
	# during boot.
	cd /
	for i in /usr/lib/drivers/fd /usr/lib/drivers/hscsidd \
		/usr/lib/drivers/pscsiddpin /usr/lib/drivers/tapepin \
		/usr/lib/drivers/pscsi720ddpin /usr/lib/drivers/ascsiddpin \
		/usr/lib/drivers/vscsiddpin \
		/usr/lib/methods/cfgfda /usr/lib/methods/cfgfdd \
		/usr/lib/methods/cfghscsi /usr/lib/methods/cfgsctape \
		/usr/lib/methods/cfgpscsi /usr/lib/methods/cfgascsi \
		/usr/lib/methods/cfgvscsi /usr/lib/microcode/*
	do
		init -c "unlink $i" 	>>$F1 2>&1
	done


	# Rewind tape then advance 1 tape marks to position the
	# tape for the first restore.

	tctl -f /dev/$BOOTDEV rewind
	read_tape /etc/config1.dep

	/usr/lib/methods/showled 0xFFF


	# Need to check for config diskette now.

	process_diskette "/etc/diagconf"
	# Check for configuration diskette here so the refresh rate can
	# be updated. The consdef file will also be read into the RAM
	# file system if it is on the diskette.
#
# The following piece of code is a KLUDGE to take care of
# 8f9a adapter.
#
	odmdelete -q"uniquetype like */nep" -o PdDv	>>$F1 2>&1
	odmdelete -q"uniquetype like */nep" -o PdAt	>>$F1 2>&1
	odmdelete -q"uniquetype like */nep" -o PdCn	>>$F1 2>&1

	for i in inputdev.add commo.add media.add display.add
	do
		ODMDIR=/etc/objrepos /usr/bin/odmadd $DIAGDATADIR/$i
		init -c "unlink $DIAGDATADIR/$i"	>>$F1 2>&1
	done
	if [ -f /refresh.add ]
	then
		list=`cat ./refresh.add | grep uniquetype | cut -d '"' -f 2`
		for i in $list
		do
			odmdelete \
			 -q"uniquetype = $i and attribute=refresh_rate" -o PdAt
		done

		/usr/bin/odmadd ./refresh.add	>>$F1 2>&1
		init -c "unlink ./refresh.add"	>>$F1 2>&1
		init -c "unlink /usr/bin/grep"	>>$F1 2>&1
	fi	
	cfgmgr -s -d		>>$F1 2>&1

	# Remove config methods and drivers not needed

	for i in /usr/lib/methods/cfgktsm /usr/lib/methods/cfgkma \
		/usr/lib/methods/cfgkbd_std /usr/lib/methods/cfgmouse_std \
		/usr/lib/methods/cfgtablet_std /usr/lib/methods/ucfgktm_std \
		/usr/lib/drivers/kbddd /usr/lib/drivers/tabletdd \
		/usr/lib/drivers/ktsdd /usr/lib/drivers/mousedd
	do
		init -c "unlink $i" >>$F1 2>&1
	done

	# Now populate the diagnostic data base

	/usr/bin/odmadd $DIAGDATADIR/diagodm.add	>>$F1 2>&1
	init -c "unlink $DIAGDATADIR/diagodm.add"	>>$F1 2>&1

	# Now process supplemental diskette

	process_diskette "supplemental"

	# Need to determine if there are any existing Graphic
	# adapters present on the system. If not, go to the
	# 4th backup file, to speed up the IPL process.

	graphics1=0
	graphics2=0
	for i in gda ppr sga wga bbl nep mag
	do
		x=`odmget -q"name like $i*" CuDv`
		[ "x$x" != x ] && {
			graphics1=1
			>/tmp/graphics1.dep
			break
		}
	done
	for i in hispd3d rby
	do
		x=`odmget -q"name like $i*" CuDv`
		[ "x$x" != x ] && {
			graphics2=1
			>/tmp/graphics2.dep
			break
		}
	done
	if [ $graphics1 -eq 1 -o $graphics2 -eq 1 ]
	then
		if [ $RAMFS_INCREASED -eq 0 ]
		then
			for i in /tmp/graphics*
			do
	
				filelist=`echo $i | cut -f 3 -d '/'`

				read_tape /etc/$filelist

				cfgmgr -s -d			>>$F1 2>&1
				for j in `cat /etc/removed_graphics.dep`
				do
					init -c "unlink $j"     >>$F1 2>&1
				done
				init -c "unlink $i"		>>$F1 2>&1
			done
		else 
		# RAM File system size has been increased, can read in
		# everything at once.

			init -c "unlink /tmp/graphics1.dep"	>>$F1 2>&1
			init -c "unlink /tmp/graphics2.dep"	>>$F1 2>&1

			read_tape /etc/graphics1.dep /etc/graphics2.dep

			cfgmgr -s -d			>>$F1 2>&1
			for j in `cat /etc/removed_graphics.dep`
			do
				init -c "unlink $j"     >>$F1 2>&1
			done
			init -c "unlink $i"		>>$F1 2>&1
		fi
	fi

	cleanup_mcode

	# Now patch diskette need to be read in here

	read_tape /etc/base.dep /etc/config2.dep

	init -c "unlink /etc/base.dep" 	>>$F1 2>&1

	ODMDIR=/etc/objrepos /usr/bin/odmadd $DIAGDATADIR/async.add
	init -c "unlink $DIAGDATADIR/async.add" >>$F1 2>&1

	configure_console


	# Clean up files read in from the tape, used to configure the
	# system console. Also clean up files used during pre-test.

	for i in `cat /etc/graphics1.dep /etc/graphics2.dep`
	do
		init -c "unlink $i" 				>>$F1 2>&1
	done
	init -c "unlink /etc/graphics1.dep"			>>$F1 2>&1
	init -c "unlink /etc/graphics2.dep"			>>$F1 2>&1
	init -c "unlink /usr/lpp/diagnostics/bin/diagpt"	>>$F1 2>&1

	for i in `cat /etc/config2.dep`
	do
		init -c "unlink $i" >>$F1 2>&1
	done
	
	# Remove lft related files
	for i in `cat /etc/config1.dep`
	do
		init -c "unlink $i" >>$F1 2>&1
	done

	# Now proceed to configure the rest of the hardware in
	# the system

	ODMDIR=/etc/objrepos /usr/bin/odmadd $DIAGDATADIR/disks.add
	init -c "unlink $DIAGDATADIR/disks.add"	>>$F1 2>&1
	ODMDIR=/etc/objrepos /usr/bin/odmadd $DIAGDATADIR/cdrom.add
	init -c "unlink $DIAGDATADIR/cdrom.add"	>>$F1 2>&1

        config5=0
        for i in msla cca
        do
        	x=`odmget -q"name like $i*" CuDv`
        	if [ "x$x" != x ]
        	then
        		config5=1
        		break
		fi
	done
	if [ $RAMFS_INCREASED = 0 ]
	then
		for i in 3 4 5
		do
	
			if [ $i = 5 -a $config5 -ne 1 ]
			then
				break
			fi
			read_tape /etc/config$i.dep

			cfgmgr -t -d			>>$F1 2>&1
			for j in `cat /etc/config$i.dep`
			do
				init -c "unlink $j" >>$F1 2>&1
			done
		done		
       		/usr/lib/methods/showled 0xFFF
	else
	# RAM File system has been increased, read everything in one
	# restbyname.

		read_tape /etc/config3.dep /etc/config4.dep /etc/config5.dep

		cfgmgr -t -d			>>$F1 2>&1
		for j in `cat /etc/config3.dep /etc/config4.dep /etc/config5.dep`
		do
			init -c "unlink $j" >>$F1 2>&1
		done
       		/usr/lib/methods/showled 0xFFF
	fi
	flattendb 					>>$F1 2>&1
	for i in 1 2 3 4 5
	do
		init -c "unlink /etc/config$i.dep" 	>>$F1 2>&1
	done

}

# DIAG_CDROM_BOOT
# Mount CDROM file systems and various config stuff for
# CDROM boot.

diag_cdrom_boot(){

	>/etc/filesystems
	mount -v cdrfs -o ro /dev/$BOOTDEV /SPOT
	[ "$?" -ne 0 ] && loopled 0x518

	cd /
	/SPOT/usr/bin/rm -r /usr/lib/drivers/* \
		/usr/lib/microcode/* /usr/sbin/*

	/SPOT/usr/bin/rm -f /usr/bin/ln /usr/bin/odmadd \
	 	/usr/bin/rm /usr/bin/tty /usr/bin/sleep

	/SPOT/usr/bin/ln -s /SPOT/usr/* /usr
	/SPOT/usr/bin/ln -s /SPOT/usr/lpp/fonts /usr/lpp/fonts
	/SPOT/usr/bin/ln -s /SPOT/usr/bin/* /usr/bin
	/SPOT/usr/bin/ln -s /SPOT/usr/sbin/* /usr/sbin
	/SPOT/usr/bin/ln -s /SPOT/usr/share/lib/terminfo \
		/usr/share/lib/terminfo

	# DO NOT link the config methods directory at this
	# point, because we want to add the stanzas files,
	# and run cfgmgr to define all resources first.
	# Since /usr/lib already existed before the above
	# ln command, we now need to populate that dir

	for i in drivers microcode
	do
		ln -s /SPOT/usr/lib/$i/* /usr/lib/$i
	done
	for i in asw nls
	do
		ln -s /SPOT/usr/lib/$i /usr/lib/$i
	done
	ln -s /SPOT/usr/lpp/diagnostics/CEREADME /usr/lpp/diagnostics/CEREADME
	ln -s /SPOT/usr/lpp/diagnostics/da/* /usr/lpp/diagnostics/da
	ln -s /SPOT/usr/lpp/diagnostics/bin/* /usr/lpp/diagnostics/bin
	ln -s /SPOT/usr/lpp/diagnostics/slih/* /usr/lpp/diagnostics/slih
	ln -s /SPOT/usr/lpp/diagnostics/catalog/default/* \
		/usr/lpp/diagnostics/catalog/default

	# Link libraries from CDROM file systems over to RAM file
	# system. These libraries are used by diagnostics.

	for i in libdiag.a libcur.a libasl.a
	do
		ln -s /SPOT/$i /usr/lib/$i
	done

	# cleanup drivers and config methods no longer needed
	# during boot.

	for i in /usr/lib/drivers/fd /usr/lib/drivers/hscsidd \
		/usr/lib/drivers/pscsiddpin /usr/lib/drivers/tapepin \
		/usr/lib/drivers/pscsi720ddpin /usr/lib/drivers/ascsiddpin \
		/usr/lib/drivers/vscsiddpin /usr/lib/drivers/fdisa \
		/usr/lib/methods/cfgfda /usr/lib/methods/cfgfdd \
		/usr/lib/methods/cfghscsi /usr/lib/methods/cfgsccd \
		/usr/lib/methods/cfgpscsi /usr/lib/methods/cfgascsi \
		/usr/lib/methods/cfgvscsi /usr/lib/methods/cfgncr_scsi \
		/usr/lib/methods/cfgfda_isa
	do
		init -c "unlink $i" 	>>$F1 2>&1
	done


	# Link the methods needed to configure input devices

	for i in cfgkma cfgkbd_isa cfgkm_isa cfgmouse_isa cfgktsm \
		cfgkbd_std cfgmouse_std cfgtablet_std
	do
		ln -s /SPOT/usr/lib/methods/$i /usr/lib/methods    >>$F1 2>&1
	done

	# Add stanzas and run cfgmgr to have the resources define.

	for i in inputdev.add commo.add media.add display.add
	do
		if [ "$i" = "display.add" ]
		then
		#
		# The following piece of code is a KLUDGE to take care of
		# 8f9a adapter.
		#
			odmdelete -q"uniquetype like */nep" -o PdDv >>$F1 2>&1
			odmdelete -q"uniquetype like */nep" -o PdAt >>$F1 2>&1
			odmdelete -q"uniquetype like */nep" -o PdCn >>$F1 2>&1
		fi

		ODMDIR=/etc/objrepos /usr/bin/odmadd /SPOT/$i   >>$F1 2>&1
		if [ "$i" = "display.add" ]
		then

		# Check for configuration diskette here so the refresh rate can
		# be updated. The consdef file will also be read into the RAM
		# file system if it is on the diskette.

			process_diskette "/etc/diagconf"

		# Once the configuration diskette is read in and 
		# the refresh.add file existed, use it.
		# Otherwise, use the refresh.add from the CDROM file system.

			if [ -f /refresh.add ]
			then
				list=`cat ./refresh.add | grep uniquetype | cut -d '"' -f 2`
				for i in $list
				do
					odmdelete \
			 			-q"uniquetype = $i and attribute=refresh_rate" -o PdAt
				done
				/usr/bin/odmadd ./refresh.add	>>$F1 2>&1
				init -c "unlink ./refresh.add"	>>$F1 2>&1
			fi
		fi
		cfgmgr -s -d				>>$F1 2>&1
	done


	# Now populate the diagnostics data base

	ODMDIR=/etc/objrepos /usr/bin/odmadd /SPOT/diagodm.add	>>$F1 2>&1

	# Now process supplemental diskette

	process_diskette "supplemental"

	# Link the methods for graphics adapters, then
	# configure all the display adapters

	for i in `cat /etc/graphics1.dep /etc/graphics2.dep /etc/graphics3.dep`
	do
		n=`echo $i | cut -c 2-`
		ln -s /SPOT$n $n			>>$F1 2>&1
	done

	cfgmgr -s -d					>>$F1 2>&1
	

	# Now link the directories /usr/lib/methods/*

	/usr/bin/rm -r /usr/lib/methods/*
	ln -s /SPOT/usr/lib/methods/* /usr/lib/methods

	# Make sure the methods for graphics adapters and i/o devices
	# are removed, since cfgmgr will be called with -t flag.

	cd /usr/lib/methods

	# Need to temporarely remove these methods since cfgmgr -t
	# will configure and unconfigure devices in the CuDv.

	for i in cfgkma cfgkbd_isa cfgkm_isa cfgmouse_isa cfgktsm \
		cfgkbd_std cfgmouse_std cfgtablet_std
	do
		/usr/bin/rm -f $i			>>$F1 2>&1
	done
	for i in `cat /etc/graphics1.dep /etc/graphics2.dep`
	do
		n=`echo $i | cut -f 4 -d '/'`
		if [ "$n" = "methods" ]
		then
			/usr/bin/rm -f `echo $i | cut -f 5 -d '/'` \
				>>$F1 2>&1
		fi
	done
	cd /
	ODMDIR=/etc/objrepos odmadd /SPOT/async.add    	>>$F1 2>&1
	ODMDIR=/etc/objrepos odmadd /SPOT/disks.add	>>$F1 2>&1
	ODMDIR=/etc/objrepos odmadd /SPOT/tapes.add	>>$F1 2>&1

	configure_console

	# Link the methods needed to configure input devices

	for i in cfgkma cfgkbd_isa cfgkm_isa cfgmouse_isa cfgktsm \
		cfgkbd_std cfgmouse_std cfgtablet_std
	do
		ln -s /SPOT/usr/lib/methods/$i /usr/lib/methods    >>$F1 2>&1
	done
	flattendb 					>>$F1 2>&1
	if [ "`bootinfo -T`" = rspc ] ; then
		ODMDIR=/etc/objrepos odmadd /SPOT/isadev.add  >>$F1 2>&1
	fi

}

# Clean up microcode from graphics adapter to save RAM file system

cleanup_mcode()
{
	X=`odmget -q"name like ppr*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lib/microcode/8ee3xb.01.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3p4d1.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3p4d2.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3p4d3.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3p4d4.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3p4d5.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3lb.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3l2b.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3ld1.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3ld2.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3ld3.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3ld4.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ee3ld5.00.* >>$F1 2>&1
		rm -f /usr/lpp/diagnostics/da/dped >>$F1 2>&1
		rm -f /usr/lpp/diagnostics/da/dlega >>$F1 2>&1
	fi
	X=`odmget -q"name like hispd3d*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lib/microcode/8ffdgu.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdgt.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdgv.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdvp.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdc2.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdsh.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffdsh.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffddc25.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffddbif.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8ffddgcp.00.* >>$F1 2>&1
		rm -f /usr/lpp/diagnostics/da/graph >>$F1 2>&1
	fi
	X=`odmget -q"name like gda*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dsky >>$F1 2>&1
		rm -f /usr/lpp/diagnostics/da/dskym >>$F1 2>&1
	fi
	X=`odmget -q"name like sga*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dsga >>$F1 2>&1
	fi
	X=`odmget -q"name like wga*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dwga >>$F1 2>&1
	fi
	X=`odmget -q"name like bbl*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dbblue >>$F1 2>&1
	fi
	X=`odmget -q"name like nep*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dnep >>$F1 2>&1
		rm -f /usr/lib/microcode/8f9a.00.* >>$F1 2>&1
	fi

	X=`odmget -q"name like rby*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/druby  >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbccp.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbcap.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbccp1.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbcap1.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbclogp.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbclogp1.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbchigp.00.* >>$F1 2>&1
		rm -f /usr/lib/microcode/8fbchigp1.00.* >>$F1 2>&1
	fi
	X=`odmget -q"name like mag*" CuDv`
	if [ "X$X" = X ]
	then
		rm -f /usr/lpp/diagnostics/da/dmag >>$F1 2>&1
	fi
}

# CONFIGURE_CONSOLE
# Configure console and run pre-test

configure_console()
{

	# If the consdef does not exist then copy it from the CDROM
	# file system. In the case of tape, it should already exists
	# in the RAM file system, even if the configuration diskette
	# was inserted.

	if [ ! -f /etc/consdef ]
	then
		ln -s /SPOT/etc/consdef /etc/consdef	  	>>$F1 2>&1
	fi

	# Load the streams modules

	/usr/sbin/strload -f /dev/null

	# Configure lft

	/usr/lib/methods/startlft			  	>>$F1 2>&1
	if [ $? -eq 0 ] 
	then
		/usr/lib/methods/showled 0x593
		/usr/lib/methods/cfglft -l lft0
		/usr/lib/methods/showled 0xFFF
	fi

	# Configure rcm

	/usr/lib/methods/startrcm				>>$F1 2>&1
	if [ $? -eq 0 ]
	then
		/usr/lib/methods/showled 0x594
		/usr/lib/methods/cfgrcm -l rcm0
		/usr/lib/methods/showled 0xFFF
	fi

	/usr/bin/rm -f /usr/lib/methods/cfgrcm			>>$F1 2>&1
	/usr/bin/rm -f /usr/lib/methods/startrcm		>>$F1 2>&1

	/usr/bin/rm -f /usr/lib/methods/cfglft			>>$F1 2>&1
	/usr/bin/rm -f /usr/lib/methods/startlft		>>$F1 2>&1

	# Run config/unconfig for async

	cfgmgr -t -d					   	>>$F1 2>&1

	#
	# Get customized name for console parent name
	#
	NAME=`/usr/lib/methods/starttty -3`

	#
	#	Configure first native I/O serial port, and then tty
	#	If /etc/consdef file is modified, configure all them too.
	#
	for i in $NAME
	do
 		A=`odmget -q "name=$i" CuDv`
 		STATUS=`echo $A | cut -f7 -d" " `
 		[ $STATUS = 0 ] && {
 			# this is for the grandparent
 			B=`echo $A | cut -f8 -d\"`
 			PARENT_ODM=`odmget -q "name=$B" CuDv`
 			PARENT_STAT=`echo $PARENT_ODM | cut -f7 -d" " `
 
 			[ $PARENT_STAT = 0 ]  && {
 				U_TYPE=`echo $PARENT_ODM | cut -f12 -d\" `
 				TEMP=`odmget -q"uniquetype=$U_TYPE" PdDv`
 				CFG=`echo $TEMP | cut -f18 -d\"`
 				`echo $CFG -l $B`  >>$F1 2>&1
 
 			}
 
 			# configure the parent of the tty
 			U_TYPE=`echo $A | cut -f12 -d\" `
 			TEMP=`odmget -q"uniquetype=$U_TYPE" PdDv`
 			CFG=`echo $TEMP | cut -f18 -d\"`
			`echo $CFG -l $i`  >>$F1 2>&1
 		}
	done
	unset STATUS B A PARENT_ODM PARENT_STAT U_TYPE TEMP CFG i

	if [ $? = 0 ]
	then
	# Create a temporary tty for use as a console on the first serial port 
	# of the native I/O board, and any specified in the /etc/consdef file. 
		/usr/lib/methods/starttty -1			>>$F1 2>&1
	else
		init -c "unlink /etc/consdef"			>>$F1 2>&1
		NAME=`/usr/lib/methods/starttty -3`		>>$F1 2>&1
		/usr/lib/methods/cfgasync -l $NAME 		>>$F1 2>&1
	fi
	unset NAME
	
	# Process any patch diskettes

	process_diskette "/etc/diagpatch"

	#
	# Run pre-test
	#

	/usr/lpp/diagnostics/bin/diagpt  			>>$F1 2>&1
  
	# Configure the console driver

	/usr/lib/methods/cfgcon
	/usr/lib/methods/showled 0xFFF

	echo "Processing the configuration."
	echo "Please stand by."

	# clean up any files from supplemental graphics diskettes
	# or from files used to do pre-test of graphics adpaters.

	[ -f /etc/diag3S.dep ] && {
		for i in `cat /etc/diag3S.dep`
		do
			init -c "unlink $i"			>>$F1 2>&1
		done
		init -c "unlink /etc/diag3S.dep"		>>$F1 2>&1
	}
}

# LOOPLED
# display LED and loop forever

loopled() {
	/usr/lib/methods/showled $1
	while :
	do
		:
	done
}

# SET_CURRENT_DISKETTE
# Determine the current diskette read in

set_current_diskette()
{
        [ ! -x /etc/diagstart* ] && {
        # No script file, must be diskette 4 is last read in
                CURRENT_DISKETTE="4"
		return
	}
        SCRIPT1=`echo /etc/diagstart*`
        TEMP_IFS=$IFS
        IFS='t'
        SCRIPT2=`echo $SCRIPT1`
        IFS=$TEMP_IFS
        for i in $SCRIPT2
        do
                disk=$i
        done
        CURRENT_DISKETTE=$disk
}

# PROCESS_DISKETTE
# This function check to see if a diskette is in the diskette drive.
# If it is, it will check for the signature file and compares it
# against what is passed in to the function to see what kind
# of diskette it is.
# If $1 = "etc/diagconf", then check for configuration/consdef diskette
# If $1 = "supplemental", then check for supplemental diskette
# If $1 = "/etc/diagpatch", then check for patch diskette

process_diskette()
{

	# If there is no diskette drive available then
	# return immediately.

	x=`odmget -q"PdDvLn like diskette/* and status != 3" CuDv`
	[ "x$x" = x ] && return
	cd /
	# if the flag DISKETTE_READ is set, then a diskette has been
	# processed. Need to see if the current diskette in the drive
	# is the same one already processed. If so, then showled to
	# prompt for next diskette, and wait 10 seconds. If diskette
	# doesn't change, then go on and do not process diskette anymore.

	# Check to see if there was a diskette previously read in
	# by reading in the value of the signatue file

	[ -f /.signature ] && read pdiskette < /.signature || pdiskette=""

	case "$1" in
	"/etc/diagconf" | "/etc/diagpatch" )

	# Configuration and patch diskette, use restbyname
	# to obtain signature file

	init -c "unlink /.signature" >>$F1  2>&1
	restbyname -xqSvf - ./.signature >>$F1 2>&1 </dev/rfd0
	# No diskette or diskette error, exit
	[ $? -ne 0 ] && return
	# If there is no signature file then exit

	[ ! -f /.signature ] && return
	while :
	do
		read a < /.signature
		[ "$a" = "$pdiskette" ] && {
			[ $DISKETTE_READ -eq 1 ] && {
			# Show 0xA07 and wait 10 seconds ?
			    /usr/lib/methods/showled 0XA07
			    /usr/bin/sleep 10
			    restbyname -xqSvf - ./.signature >>$F1 2>&1 </dev/rfd0
			    # Error, need to break in order to clear led
			    # instead of return
		    	    [ $? -ne 0 ] && break
			    DISKETTE_READ=0
			}
			break
		}
		break
	done
	# Only continue if the diskette currently in the drive
	# was meant to be processed now.

	[ "$a" != "$1" ] && {
		init -c "unlink /.signature"	>>$F1 2>&1
		return
	}
	[ "$a" = "/etc/diagpatch" ] && {
		/usr/lib/methods/showled 0xA09
		restbyname -xqSvf/dev/rfd0 >>$F1 2>&1
		[ $? -ne 0 ] && break
		# The diagpatch script exists
		[ -f /etc/diagpatch ] && {
			/etc/diagpatch >>$F1 2>&1
			init -c "unlink /etc/diagpatch" >>$F1 2>&1
		}
		DISKETTE_READ=1
		break
	}
	[ "$a" = "/etc/diagconf" ] && {
		/usr/lib/methods/showled 0xA09
		restbyname -xqSvf/dev/rfd0 >>$F1 2>&1
		# Cannot read, exit  Could be bad diskette
		[ $? -ne 0 ] && break
		# If diagconf script exists, execute it
		[ -f /etc/diagconf ] && {
			/etc/diagconf	>>$F1 2>&1
			init -c "unlink /etc/diagconf"	>>$F1 2>&1
		}
		[ -f /REFRESH ] &&  mv /REFRESH /refresh.add  >>$F1 2>&1
		[ -f /CONSDEF ] && mv /CONSDEF /etc/consdef  >>$F1 2>&1
		DISKETTE_READ=1
		break
	}
	;;

	"supplemental")

	# Supplemental diskette, use cpio to check for valid diskette

	rc=`/etc/chkdskt -s DIAG`
	[ "$rc" = "3S" ] && {
		/usr/lib/methods/showled 0xA09
		cpio -iudC36 </dev/rfd0 	>>$F1 2>&1
		[ $? -eq 0 ] && {
			/etc/diagstart$rc	>>$F1 2>&1
			init -c "unlink /etc/diagstart$rc" >>$F1 2>&1
			DISKETTE_READ=1
		}
		break
	}
	# Invalid label check to see if this is a previously
	# read diskette

	init -c "unlink /.signature" >>$F1  2>&1
	restbyname -xqSvf - ./.signature >>$F1 2>&1 </dev/rfd0
	# No diskette or diskette error, exit
	[ $? -ne 0 ] && break
	# If the .signature file does not exist
	# after the restbyname, exit because we don't
	# really know what kind of diskette it is.

	[ ! -f /.signature ] && return

	while :
	do
	   read a < /.signature
	   # Same diskette in drive. pause 10 secs to allow
	   # user to insert another one in.
	   [ "$a" = "$pdiskette" ] && {
		[ $DISKETTE_READ -eq 1 ] && {

	           # Show 0xA07 & wait 10 secs.

		   /usr/lib/methods/showled 0XA07
		   /usr/bin/sleep 10

		   # Now try to use cpio to read
		   # the diskette label again.

		   rc=`/etc/chkdskt -s DIAG`
		   [ "$rc" = "3S" ] && {
		   	/usr/lib/methods/showled 0XA09
			cpio -iudC36 </dev/rfd0 >>$F1 2>&1
			[ $? -eq 0 ] && {
				/etc/diagstart$rc >>$F1 2>&1
				init -c "unlink /etc/diagstart$rc" >>$F1 2>&1
				break
			}
		   }
	   	   DISKETTE_READ=0
		   break
		}
		break
	   }
	   break
	done
	;;
	*)
	;;
	esac
	/usr/lib/methods/showled 0xFFF		>>$F1 2>&1
} 

# Read files from tape

read_tape()
{

	/usr/lib/methods/showled 0xA40
	tctl -f $TAPEDEV fsf 1
	restbyname -qvSf/dev/$BOOTDEV -x `cat $*`\
			>>$F1 2>&1
	[ $? -ne 0 ] && {
		dspmsg dctrl.cat -s 1 7 \
		    "Error restoring files from tape device $BOOTDEV"
		loopled 0xA43
	}
	/usr/lib/methods/showled 0xFFF

}

# RESPAWN_FUNCTION
# This function is entered when an interrupt is detected 

respawn_function()
{
	RESPAWNFLAG=0
}

# Processes the list of files to be symbolically linked

create_symlinks()
{
	while read from to
	do
		/usr/bin/ln -s $from $to	>>$F1 2>&1
	done</etc/linkedfiles.dep
	rm -f /etc/linkedfiles.dep		>>$F1 2>&1

}

##########################################################################
#				Start of Code                            #
##########################################################################

F1="/dev/null"
HOME=/
PATH=/usr/sbin:/bin:/usr/bin:/etc
ODMDIR=/etc/objrepos
LIBPATH=/lib:usr/lib:/usr/ccs/lib:
DISKETTE_READ=0
LANG=en_US
NLSPATH=/usr/lpp/diagnostics/catalog/default/%N:/usr/lib/nls/msg/%L/%N:/usr/lib/methods/%N
LOCPATH=/usr/lib/nls/loc
BOOTDEV=""
TAPEDEV=""
DIAG_IPL_SOURCE=0
RAMFS_INCREASED=0
DIAG_KEY_POS=1
export LIBPATH HOME ODMDIR PATH F1 NLSPATH LANG LOCPATH BOOTDEV
export DIAG_IPL_SOURCE TAPEDEV DIAG_KEY_POS

# Set up to trap interrupts ( INT, QUIT, TERM )
trap respawn_function 2 3 15

# Set execution environment
DIAG_ENVIRONMENT=2

# Now set up basic environment variables
DIAGNOSTICS=/usr/lpp/diagnostics

# Diagnostic Applications path
DADIR=$DIAGNOSTICS/da

# Diagnostic data directory - diagnostic conclusion files, etc
DIAGDATADIR=$DIAGNOSTICS/data
DIAGX_SLIH_DIR=$DIAGNOSTICS/slih

CLRSCR="\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

export DIAGNOSTICS DIAGDATADIR DADIR
export DIAG_ENVIRONMENT DIAGX_SLIH_DIR

# At this point, the libc.a library is pulled into memory as a shared 
# library. Thus, the file size can be truncated to conserve file space
>/usr/lib/libc.a

# create shell version of cat
echo 'for i in $*
do
	OIFS="$IFS"
		IFS="
"
	while read j
	do
		echo "$j"
	done < $i
	IFS="$OIFS"
done' > /bin/cat
init -c "chmod /bin/cat 0777" >/dev/null

BOOTYPE=`bootinfo -t`
if [ "$BOOTYPE" = "" ]
then
	BOOTYPE=3
fi

cd /etc/stanzas
for i in *.add
do
	ODMDIR=/etc/objrepos /usr/bin/odmadd $i		>>$F1 2>&1
	init -c "unlink $i"				>>$F1 2>&1
done
cd /

# Run config manager phase 1 to configure the base system
# stuff, along with the boot device.

cfgmgr -f						>>$F1 2>&1

case "$BOOTYPE" in

	3)	# CDROM
		BOOTDEV=`bootinfo -b`
		if [ "$BOOTDEV" = "" ]
		then
			BOOTDEV=cd0
		fi
		DIAG_IPL_SOURCE=1
		# Check to see if systems has diagnostics support 
		# if not eject cd

		if [ "`bootinfo -T`" = rspc ] ; then
			bootinfo -M >/dev/null
			if [ "$?" -ne 101 -a "$?" -ne 103 ] ; then
				/etc/eject /dev/$BOOTDEV
				# give time for cd to be ejected
				/usr/bin/sleep 3
				# now reboot the machine
				/usr/sbin/reboot -n -q
				while :
				do
					:
				done
			fi
		fi
		diag_cdrom_boot
		;;

	4)	# TAPE
		BOOTDEV=`bootinfo -b`
		if [ "$BOOTDEV" = "" ]
		then
			BOOTDEV=rmt0
		fi
		DIAG_IPL_SOURCE=2
		TAPEDEV=/dev/$BOOTDEV.1
		mem_size=`bootinfo -r`
		# Check memory size. If there is only 8MB then loop
		# on led 0x558. If there is more than 16MB of memory
		# then increase the size of the RAM file system
		# to eliminate multiple passes through
		# the tape to do the restore.

		if [ "$mem_size" -le "8192" ]
		then
			loopled 0x558
		fi
		if [ "$mem_size" -gt "26624" ]
		then
			/usr/lib/boot/chramfs -t >>$F1 2>&1
			if [ $? -eq 0 ]
			then
				RAMFS_INCREASED=1
			fi
		fi
		init -c "unlink /usr/lib/boot/chramfs" >>$F1 2>&1
		diag_tape_boot
		;;
	*)	# Error ?
		loopled 0xA41
		;;
esac

create_symlinks

TERM=dumb
cd /
TRM=`/usr/bin/tty`
case $TRM in
	/dev/lft* ) 
		TERM=lft
		export TERM
		if [ "$BOOTYPE" = "4" ]
		then
		   	cpio -idu <$DIAGDATADIR/term.cpio \
				usr/share/lib/terminfo/l/lft >>$F1 2>&1

		fi
		;;
	* );;
esac
export TERM

/usr/lib/methods/showled 0xFFF					>>$F1 2>&1
if [ -f /etc/NOKEYPOS ]
then
	DIAG_KEY_POS=0
fi

# Link En_US with default for compatibility support of
# supplemental diskettes who has catalog files in En_US
# directory

ln -s /usr/lpp/diagnostics/catalog/default \
	/etc/lpp/diagnostics/catalog/En_US		>>$F1 2>&1

while :
do
	tty >/dev/null 2>&1
	if [ $? -eq 1 ]
	then
		/usr/lpp/diagnostics/bin/dctrl -s -c >/dev/null 2>&1
		loopled 0xA99
	fi
	while :
	do
		/usr/lpp/diagnostics/bin/diags
		# debug mode
		if [ $DIAG_KEY_POS -eq 0 ]
		then
			/bin/bsh
		else
			# if booting from cdrom then eject cd, then
			# hang forever

			if [ "$BOOTYPE" = "3" ]
			then
				/etc/eject /dev/$BOOTDEV
				# Now loop forever
				while :
				do
					:
				done
			fi
		fi
	done
done

exit 0
