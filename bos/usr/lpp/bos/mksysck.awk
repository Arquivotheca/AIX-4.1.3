# @(#)66	1.1  src/bos/usr/lpp/bos/mksysck.awk, cmdsadm, bos411, 9428A410j 4/27/90 17:30:55
# COMPONENT_NAME: (TCBADM) convert ls output to SYSCK stanzas
#
# FUNCTIONS: mksysck.awk
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
BEGIN {
	lastinum = -1
	lastlinks = ""
	mpxcount = 0
}

{
#
# Input is sorted by inumber.  Output a stanza after each inumber changes.
#
	if (lastinum != $1) {
		if (lastinum != -1) {
			printf "%s:\n", lastname
			printf "\ttype = %s\n", lasttype
			printf "\towner = %s\n", lastowner
			printf "\tgroup = %s\n", lastgroup
			printf "\tmode = %s\n", lastmode
			if ( lastlinks != "" )
				printf "%s\n", lastlinks

			print ""
			lastinum = -1
		}
#
# Each new inumber means a new file to be started.  Save off the mode and
# compute the type for each file when it is first seen.
#
		lastmode = substr ($2, 2, 9)
		type = substr ($2, 1, 1)

		if (type == "s" || type == "d") {
			next
		} else if (type == "-") {
			lasttype = "FILE"
		} else if (type == "p") {
			lasttype = "FIFO"
		} else if (type == "c") {
			if (substr (lastmode, 9, 1) == "t") {
				lastmode = substr (lastmode, 1, 8) "x"
				lasttype = "MPX_DEV"
			} else if (substr (lastmode, 9, 1) == "T") {
				lastmode = substr (lastmode, 1, 8) "-"
				lasttype = "MPX_DEV"
			} else
				lasttype = "CHAR_DEV"
		} else if (type == "b") {
			lasttype = "BLK_DEV"
		} else if (type == "l") {
			if (mpxsymlink[$12] == "")
				mpxsymlink[$12] = "\tsymlinks = " $10
			else
				mpxsymlink[$12] = mpxsymlink[$12] "," $10

			next
		}
		lastattr = ""
#
# The extended attributes are difficult to compute.  The SUID bit exists
# if there is an "S" or "s" in the "user execute" location.  The same is
# true for SGID, except it is the "group execute" location to check.
# Finally, the SVTX attribute is "t" or "T" in the "other execute" position,
# unless this is a multiplexed device, in which case it is ignored.
#
		if (substr (lastmode, 3, 1) == "s") {
			lastmode = substr(lastmode,1,2) "x" substr(lastmode,4)
			lastattr = "SUID,"
		}
		if (substr (lastmode, 3, 1) == "S") {
			lastmode = substr(lastmode,1,2) "-" substr(lastmode,4)
			lastattr = "SUID,"
		}
		if (substr (lastmode, 6, 1) == "s") {
			lastmode = substr(lastmode,1,5) "x" substr(lastmode,7)
			lastattr = lastattr "SGID,"
		}
		if (substr (lastmode, 6, 1) == "S") {
			lastmode = substr(lastmode,1,5) "-" substr(lastmode,7)
			lastattr = lastattr "SGID,"
		}
		if (substr (lastmode, 9, 1) == "t") {
			lastmode = substr(lastmode,1,8) "x" substr(lastmode,10)
			lastattr = lastattr "SVTX,"
		}
		if (substr (lastmode, 9, 1) == "T") {
			lastmode = substr(lastmode,1,8) "-" substr(lastmode,10)
			lastattr = lastattr "SVTX,"
		}
		lastmode = lastattr lastmode

		lastinum = $1

		linkcount = $3
#
# Now I save off the other pieces of useful information, like owner, group
# and filename.  The filename of the first entry will be the name of the
# stanza.
#
		lastowner = $4
		lastgroup = $5
		lastname = $10

		lastlinks = ""
	} else {
		if (lasttype == "MPX_DEV") {
#
# For multiplexed devices I have to save off the name of the head of the
# device.  This is so symbolic links won't be created for the multiplexed
# channels which may show up.
#
			mpx[mpxcount] = $10
			mpxcount = mpxcount + 1

			mpxowner[$10] = lastowner
			mpxgroup[$10] = lastgroup
			mpxmode[$10] = lastmode

			next
		}
		if ( lastlinks == "" )
			lastlinks = "\tlinks = " $10
		else
			lastlinks = lastlinks "," $10

		next
	}
}

END {
#
# Deal with the last record.  This is the information that would have been
# output had there been one more entry.  There wasn't, so ...
#
	if (lastinum != -1) {
		printf "%s:\n", lastname
		printf "\ttype = %s\n", lasttype
		printf "\towner = %s\n", lastowner
		printf "\tgroup = %s\n", lastgroup
		printf "\tmode = %s\n", lastmode
		if ( lastlinks != "" )
			printf "%s\n", lastlinks

		print ""
	}
#
# All of the multiplexed devices which were referenced in symbolic links
# are now output.  These entries are very special.  The device node won't
# exist, except as a reference to this symbolic link.
#
	for (i = 0;i < mpxcount;i = i + 1) {
		printf "%s:\n", mpx[i]
		printf "\ttype = MPX_DEV\n"
		printf "\towner = %s\n", mpxowner[mpx[i]]
		printf "\tgroup = %s\n", mpxgroup[mpx[i]]
		printf "\tmode = %s\n", mpxmode[mpx[i]]
		if (mpxsymlink[mpx[i]] != "")
			printf "%s\n", mpxsymlink[mpx[i]]

		printf "\n"
	}
}
