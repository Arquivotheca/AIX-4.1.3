# @(#)85	1.13  src/bos/usr/lib/microcode/serdasd/serdasd.mc.il, ucodserdasd, bos411, 9428A410j 12/6/93 05:03:18
#
#   COMPONENT_NAME: UCODSERDASD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1989,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
*
* The serdasd.mc.il file is an ASCII file which contains
* the definitions associated with the serdasd
* microcode diskette.  This file is in stanza format.
* The name of the stanza is the pathname of
* the file.  Each attribute is given as
* attribute = value on a separate line.

* Each attribute is terminated by a newline and each stanza
* is terminated by an additional newline. The stanza name
* must be terminated by a colon ':' followed by a newline.
* Each stanza may have one or more of the following
* attributes. All stanzas must have the type attribute:
*
*	filename:                        -- full path of microcode location
*		type = FILE|DIRECTORY|FIFO|BLK_DEV|CHAR_DEV|MPX_DEV
*		class = classname
*		owner = file owner
*		group = file group
*		mode = file mode
*		acl = access control list [in aclget format]
*		links = path,path...
*		symlinks = path,path...
*		checksum = number
*		size = filesize
*		program = path [args]
*		source = path to the actual file
*		microcode = common | feature | base:disk | base:network
*		mcodename = lpp where microcode was packaged
*		level = vers.rel.mod.fix
*		partnum = (mcode diskette fru partnumber)
*		preload = (preload image partnumber)
* (Note the last 4 entries only go in the first stanza )
*
*
*****************************  IMPORTANT  **************************************
*
* Ensure that the "partnum = <number>" and "preload = <number>" fields are
* correct.  This is how the EC process and the packaging process are tied
* together.  If the part number and preload number are not correct, the correct
* microcode will not go onto the diskette and it will not get preinstalled
* during manufacturing.
*
* Ensure that the "class = ... <lppname>.mc" line correct.
*
* Ensure that the "mcodename = ... <lppname>.mc" line correct.
*
*****************************  IMPORTANT  **************************************
*
*


default:
        type = FILE
        class = inventory,apply,serdasd.mc
        size =
        checksum =
	mcodename = serdasd.mc
	level =  03.02.00.16
	partnum = 67G3324
	preload = 00G0000

*****************************  IMPORTANT  **************************************
* There must be a stanza for each microcode file which will go onto the diskette
* Each microcode file must be in /etc/microcode.
*****************************  IMPORTANT  **************************************

/etc/microcode/8f78.00.16:
        owner = root
        group = system
        mode = 644
	microcode = common
