# @(#)70        1.1.6.11  src/bos/diag/dutil/CEREADME.S, dutil, bos41J, 9517A_all 4/24/95 08:59:00
#
#   COMPONENT_NAME: DUTIL
#
#   FUNCTIONS: CEREADME file
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#




                      SERVICE MEMORANDUM

                           FOR THE

        POWERstation and POWERserver

        FOR USE WITH:  Diagnostics Version 4.1.3

        DATE:  April 21, 1995

        EC HISTORY:
        None




        (C) Copyright International Business Machines Corp.,
        1992, 1995.  All rights reserved.

        ORIGINS:  27.

        References in this document to products or services do not
        imply that they will be made available outside of the
        United States.

        IBM is a trademark of International Business Machines Corp.


                                                                        1




        INTRODUCTION


        The information in this memo updates and supplements the
        instructions and descriptions printed in the service manuals.  
        It contains service hints and errata information about the 
        maintenance package.

        This memo covers Diagnostics version 4.1.3 and its supporting
        documentation.

        The information in this memo is divided into several sections.
        The sections are:

        - MAP:  Problem Resolution

          This section contains a MAP that helps resolve problems that
          are not resolved by the service manuals.

        - Service Manual's Errata Information

          This section contains errata information for the service guides.

        - Service Hints for Problems

          This section contains hints and actions that may be useful in
          diagnosing problems.

        - Diagnostic Information

          This section contains information about the Diagnostic programs
          that are not included in the service manuals.

        - Installation Information

          This section contains installation information that
          is not included in the service manuals.

        - Engineering Change (EC) Information

          This section contains information about Diagnostic ECs.

                                                                        2




        TABLE OF CONTENTS


        MAP:  Problem Resolution.......................................3
        Service Manual's Errata Information............................4
          None
        Service Hints For Problems.....................................4
          None
        Printing the Service Hints.....................................4
          Printing the Service Hints from disk.........................4
          Printing the Service Hints from tape.........................4
          Printing the Service Hints from CDROM........................5
        Diagnostic Information.........................................6
          Error Log Analysis...........................................6
          Running Diagnostics from a mounted CDROM.....................6
          LED displayed during tape IPL................................6
          High function terminal (HFT).................................6
          Booting in stand alone from removable media..................6
          Supplemental diskettes.......................................7
          Changing the default refresh rate of Low
              Function Terminal (LFT)..................................7
          Console configuration diskette When Running
              From CDROM or tape.......................................7
          Supplemental display diskettes when running
              From CDROM or tape.......................................8
          Running Diagnostics from X-Windows...........................8
          New features and changes in Diagnostics version 4.1.3........8
            Testing of display adapters................................8
            Display test patterns service aid..........................8
          Devices not supported by Diagnostics version 4.1.3...........9
          Running disk based Diagnostics on version 4.1.3..............9
	  Fiber Distributed Data Interface (FDDI) adapter wrap plugs...9
	Devices Supported on Removable Media...........................10
        Installation Information.......................................12
          None
        Engineering Change (EC) Information............................12
          None





                                                                        3




        MAP:  PROBLEM RESOLUTION


        Use this MAP to handle problems that were not resolved by the
        MAPs in the service documentation.

        Step 1

        Do the following:

        1.  Check the Service Manual's Errata Information in the Table
            of Contents for corrections that may be related to your
            problem.  If you find a correction that is related to your
            problem that is not in the manual, follow the procedures 
            again using the corrected material.

        2.  In some cases the Service Request Numbers in the service
            manual may not list the same parts as reported by the
            Diagnostic Programs.  Check if there are any parts identified
            by the Diagnostics that are not listed in the service manual.
            If there are any additional parts listed, suspect them.

        3.  Look in the Table of Contents under Service Hints for Problems
            for the symptom.  If you find the symptom, follow the
            procedure for it.

        4.  If the failure is related to an external resource, then use
            the problem determination procedures, if available, for the
            external resource.

        Did one of the above resolve your problem?
        NO      Go to Step 2.
        YES     Go to MAP 0410 in the service manual.


        Step 2

        Have you tried running Diagnostics from CDROM or tape?
        NO      Run the Diagnostics from CDROM or tape.  If you get a
                new symptom, follow the procedures for it.
        YES     Call your support person.



        SERVICE MANUAL'S ERRATA INFORMATION
          
        None.






                                                                        4




        SERVICE HINTS FOR PROBLEMS

            None.

        PRINTING THE SERVICE HINTS


          Printing the Service Hints from disk


            Service Hints are stored in the CEREADME file.
            The CEREADME file that is on disk may be printed using the cat
            command. The path to this file is as follows:

            /usr/lpp/diagnostics/CEREADME

            lp0 is normally the printer attached to the parallel port. If a
            printer is attached to the parallel port and is configured as lp0,
            the command for printing the file is as follows:

            cat /usr/lpp/diagnostics/CEREADME > /dev/lp0


          Printing the Service Hints from tape

            Service hints are stored in the CEREADME file.
            The CEREADME file on tape cannot be printed when running from
            the diagnostics tape.  The file can be printed on a system 
            when the operating system is running in a normal user 
            environment.  The procedure involves copying the file from 
            tape to a temporary file on disk, printing the file and then 
            deleting the file from disk.  Check for directory /tmp/diag.
            To determine if this directory already exists, enter:

                  cd /tmp/diag

            If the message "/tmp/diag: not found" is displayed, then insert
            Diagnostics version 4.1.3 tape into the tape drive and type the 
            following commands:

                  mkdir /tmp/diag
                  cd /tmp/diag
                  tctl -f /dev/rmt0.1 fsf 8
                  restbyname -qvSf/dev/rmt0 ./usr/lpp/diagnostics/CEREADME
                  cd /tmp/diag/usr/lpp/diagnostics
                  cat CEREADME > /dev/lp0
                  cd /tmp
                  rm -rf diag


            The preceding commands will print the file on lp0, which is the
            printer normally attached to the parallel port.




                                                                        5





          Printing the Service Hints from CDROM

            Service hints are stored in the CEREADME file.
            The CEREADME file on CDROM cannot be printed when running 
            from CDROM.  The file can be printed on a system when the
            operating system is running in a normal user environment.  The 
            procedure involves mounting a CDROM file system by using the 
            diagnostic CDROM disc, printing the file and then unmounting
            the file system.  (NOTE:  The operating system level must be
            at least AIX 4.1).

            Check for directory /tmp/diag.  To determine if this directory
            already exists, enter:

                  cd /tmp/diag

            If the message "/tmp/diag: not found" is displayed, then insert
            Diagnostics version 4.1.3 CDROM disk into the CDROM drive
	    and type the following commands:

                  mkdir /tmp/diag
                  mount -o ro -v cdrfs /dev/cd0 /tmp/diag
                  cd /tmp/diag/usr/lpp/diagnostics
                  cat CEREADME > /dev/lp0
                  cd /tmp
                  unmount /dev/cd0

            NOTE:  The CEREADME file cannot be copied to another directory.





                                                                        6



        DIAGNOSTIC INFORMATION



        Error Log Analysis

            Error log analysis will not be performed unless the bos.sysmgt
            package is installed. 


        Running Diagnostics from a mounted CDROM

            The version 4.1.3 diagnostic CDROM can be mounted, and
	    diagnostics run in concurrent mode, without having to install
	    the bos.diag and its corresponding devices diagnostic packages.

            To do this, the /tmp file system requires at least 
            2048 1024 KB  blocks of free space.
            To find out the free space available under /tmp, do the
            following:

                df -k /tmp

            If a CDROM drive is available on the system, the diag command will
            prompt for the diagnostic CDROM disc, and allow diagnostics
            to be executed from the mounted CDROM file system.


        LED displayed during tape IPL

            0xC40 LED indicates that the tape restore is in progress.
            0xC43 LED indicates that a tape I/O error was encountered during
                  the restore operation.


        High function terminal (hft)

            The hft or high function terminal no longer exists.  The new 
            terminal type that replaces hft is lft.


        Booting Diagnostic CDROM or tape in stand alone.

            When booting in stand alone from removable media, the system
            will attempt to read the diskette 3 times.  The first time to
            see if there is a configuration diskette present.  The second
            time to see if there is a supplemental graphic diskette
            present.  And the third time to see if there is a patch diskette
            present.

            If any of the 3 types of diskettes mentioned above are to be
            used, they need to be inserted into the diskette drive prior
            to powering up the system.




                                                                        7



	    When booting most systems that are not supported by AIX
	    diagnostics, using the diagnostic CDROM, the CDROM disc
	    will be ejected from the drive, and the system halted.

            When booting from CDROM or when running diagnostics from a 
            mounted CDROM, DO NOT remove the CDROM disc from the drive
            while diagnostics are running. The CDROM disc cannot be
	    manually ejected.

            When booting from tape, if the tape device used to IPL the system 
            needs to be tested, be sure to re-insert the diagnostic tape which
            you used to boot the system into the drive once the tape drive's
            diagnostics have completed.


        Supplemental diskettes

            All graphics supplemental diskettes which worked on a version
            of Diagnostics prior to version 4.1, will NOT work on 
            version 4.1 or later.  Also, the ESCON and HIPPI supplemental
	    diagnostic diskettes will not work with version 4.1 or later.
	    Obtain a version 2.4.3 CD-ROM or diskette package to use with
	    any supplemental diskette that does not work with version 4.1
	    or later.
            Other supplemental diskettes should work with version 4.1.X
            of Diagnostics.


        Changing the default refresh rate of Low Function Terminal (LFT)

            Version 4.1.3 of the Diagnostic stand alone package uses the 
            default refresh rate of 60 Hz for an LFT.

            In order to change the default refresh rate to 77 Hz, a 
            configuration diskette can be created using the Diagnostic 
            Package Utility Service Aid.  Upon choosing to run the 
            Diagnostic Package Utility Service Aid, you will be given the 
            opportunity to format a diskette or create a stand alone 
            Diagnostic package configuration diskette.  If you choose to 
            create a configuration diskette for use by Diagnostics, you 
            will be prompted for the new configuration information
            for the console configuration file.  If there are no changes
            or you are done making changes, press the commit key and you will 
            be presented with the option to modify the refresh rate.  When
            done selecting the refresh rate, choose commit.


        Console configuration diskette When Running From CDROM or tape

            If you have a console configuration diskette which you created
            using the Diagnostics Package Utility Service Aid, the diskette
            should be inserted into the diskette drive prior to booting 
            from the CDROM or tape.





                                                                        8




        Supplemental display diskettes when running From CDROM or tape

            To use a supplemental display diskette, the diskette should
            be in the diskette drive prior to booting the system.  When
            the CDROM or tape boot process gets to configuring the
            display devices, the diskette will be read so that the
            supplemental displays can be configured.


        Running Diagnostics from X-Windows 

            When running diagnostics from X-Windows, the LFT related devices
            will not show up in the test list.  These devices include all 
            display adapters, keyboard, mouse, and tablet.  To test these 
            devices, either run from a non X-window, or from removable media.


        New features and changes in Diagnostics version 4.1.3

          Testing of display adapters

            In version 4.1.3 of Diagnostics, some of the the display adapter
            tests have been changed from previous versions of Diagnostics. 
            You will see a difference in the way the red, green, and blue test 
            patterns are displayed in version 4.1.3 when compared with 
            earlier diagnostic versions.  The change to these display
            tests is that when asked to check for a certain color,
            the pattern will stay on the screen for 7 seconds whereas
            before the screen would stay up until you answered yes or
            no.  Note that if you need to have a particular color screen 
            displayed for an extended period of time so that
            you can adjust the display, you can use the Display Test 
            Patterns Service Aid on systems that this Service Aid is
	    supported.


          Display test patterns service aid

            While running the Display Test Patterns service aid, use the
            `Cancel` or F3 key to return to the previous menu and choose
            `Exit` or F10 to return to the Service Aid selection menu.





                                                                        9



        Devices not supported by diagnostics version 4.1.1 and above.

            The following list of devices are not supported by Diagnostics
            version 4.1.1 and 4.1.2.  If you need to test one of these 
	    devices, you will need to use an earlier version of diagnostics.

                8-Bit 3D Color Graphics Processor
                24-Bit 3D Color Graphics Processor
		High Speed 3D Graphics Accelerator
		7235 POWER GtO
                M-Audio Capture and Playback Adapter
                M-Video Capture Adapter
                Async Expansion Units
	    	ESCON Channel Adapter
		5085/86/88 Attachment Adapaters
		T1/E1/J1 Portmaster Adapter
		64-Port Async Adapter and Concentrators

	   The 7235 POWER Gt0 and 5085/86/88 Attachment Adapters are now
	   supported in diagnostics version 4.1.3.

        Running disk based Diagnostics on version 4.1.3
            
            To run Diagnostics from the hardfile, the bos.diag package
            must have been previously loaded on the hardfile.  The 
            packages for the devices that you wish to test should
            also have been loaded.
            
            If a device does not show up in the test list or you think 
            that a device's Diagnostic package may not have been loaded, 
            you can check by using the lscfg command.  At an AIX prompt,
            type lscfg.  If the device which you were wanting to test
            has a '+' or '-' preceding its name, the package was loaded. 
            If the device has a '*' in front of its name, a diagnostic 
            package for the device has not been loaded or is not
	    available
            

	Fiber Distributed Data Interface (FDDI) adapter wrap plugs

	    The FDDI diagnostic application prompts for wrap plug Part
	    Number 81F9016.  Wrap plug Part Number 81F9016 has been replaced
	    by Part Number 92F9003.  The wrap plugs are functionally
	    equivalent and either one can be used when running the FDDI
	    diagnostics.



									10

	DEVICES SUPPORTED ON REMOVABLE MEDIA


	   Below is the list of devices that are supported on the
	   diagnostics removable media:

                Color Graphics Display Adapter
                Grayscale Graphics Display Adapter
                POWER Gt1 Graphics Adapter
                POWER Gt1x Graphics Adapter
                POWER GXT100 Graphics Adapter
                POWER GXT150 Graphics Adapter
                POWER GXT150M Graphics Adapter
                POWER GXT150L Graphics Adapter
                POWER Gt3 Graphics Subsystem
                POWER Gt3i Graphics Subsystem
                POWER Gt4 8-Bit Graphics Subsystem
                POWER Gt4 24-Bit Graphics Subsystem
                POWER Gt4e Graphics Subsystem
                POWER Gt4x 8-Bit Graphics Subsystem
                POWER Gt4x 24-Bit Graphics Subsystem
                POWER Gt4i 24-Bit Graphics Subsystem
                POWER Gt4xi 8-Bit Graphics Subsystem
                POWER Gt4xi 24-Bit Graphics Subsystem
                POWER GXT1000 Graphics Subsystem
		POWER GXT500D Graphics Adapter
		POWER GXT500 Graphics Adapter
		7235 POWER Gt0
		E15 Graphics Adapter
		S15/H10 Graphics Adapter
		PCI 16-bit SCSI I/O Controller
                SCSI Internal/External I/O Controller
                SCSI-2 Fast/Wide Adapter/A
                Wide SCSI I/O Controller
                CDROM Drive
                3.5 Inch Diskette Drive
                5.25 Inch Diskette Drive




									11

                1/4-Inch Cartridge Tape Drive
                2.3 GB 8mm Tape Drive
                5 GB 8mm Tape Drive
                5 GB 8mm DE Tape Drive
                4mm Tape Drive
                9-Track Tape Drive
                1/2-Inch 3490E C10,C11,C22 Tape Drive
                160 MB SCSI Disk Drive
                200 MB SCSI Disk Drive
                320 MB and 400 MB SCSI Disk Drives
                120 MB Direct Bus Attach Disk
                160 MB Direct Bus Attach Disk
                355 MB and 670 MB SCSI Disk Drives
                540 MB SCSI Disk Drive
                857 MB SCSI Disk Drive
                1.0 GB SCSI Disk Drive
                1.0 GB SCSI DE Disk Drive
                1.0 GB 16 Bit SCSI DE Disk Drive
                1.37 GB SCSI Disk Drive
                2.0 GB SCSI Disk Drive
                2.0 GB SCSI DE Disk Drive
                2.0 GB 16 Bit SCSI DE Disk Drive
                2.0 GB 16 Bit SCSI Disk Drive
                2.4 GB SCSI Disk Drive (2 1.2 GB each)
		2.2 GB SCSI Disk Drive
		2.2 GB 16 Bit SCSI Disk Drive
		2.2 GB 16 Bit Differential SCSI Disk Drive
		4.5 GB 16 Bit SCSI Disk Drive
		4.5 GB 16 Bit Differential SCSI Disk Drive
		7135 RAIDiant Array
                High Performance Disk Drive Subsystem
                Ethernet High-Performance LAN Adapter
		PCI Ethernet Adapter
		IBM PCI Token Ring Adapter
                Portmaster Adapter/A
                4-Port Multiprotocol Communications Controller
                X.25 Interface Co-Processor/2
                Multiport/2 Adapter
                3270 Connection
	        5085/86/88 Attachment Adapters



                                                                        12

		Integrated Ethernet
		Token Ring High-Performance Adapter (8fa2)
		Token Ring High-Performance Adapter (8fc8)
		FDDI Adapter (8ef4)
		ISA X.25 CoProcessor/1 Adapter
		370 Parallel Channel Attach Adapter
		Block Multiplexor Channel Adapter
                Serial Optical Channel Converter
                Standard I/O Serial Ports
		Standard I/O Parallel Port
                Graphics Input Device Adapter
                Dials
                Lighted Program Function Keys
                8-Port Async Adapters
                16-Port Async Adapter
                128-port Async Controller and Remote Async Nodes
		ISA 8-Port Async Adapter
		ISA 128-Port Async Controller and Remote Async Nodes

		
        INSTALLATION INFORMATION


            None.




        ENGINEERING CHANGE (EC) INFORMATION


            None.
