#!/bin/ksh
# @(#)57        1.7.1.1  src/bos/usr/ccs/bin/lorder/lorder.sh, cmdaout, bos41J, 9508A 2/10/95 10:29:51
#
#  COMPONENT_NAME: CMDAOUT (lorder command)
#
#  FUNCTIONS: lorder
#
#  ORIGINS: 27, 9, 3
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1993
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	COMMON LORDER

tmp=${TMPDIR-/tmp}
trap "rm -f $tmp/$$sym?ef $tmp/$$lorder.err; exit 1" 1 2 13 15

case $# in
0)	dspmsg lorder.cat 1 "Usage: lorder file ...\n" >&2
	exit 1;;
1)	case $1 in
	*.o)	set $1 $1
	esac
esac

# Should only use the supplied nm for this command to ensure that the
#	sed script works correctly.
NM=nm
BINNAME=`dirname $0`
[ "$BINNAME" != "." -a -x $BINNAME/$NM ] && NM=$BINNAME/$NM

#	The following sed script is commented here.
#	The "nm -g" ensures that we only have lines
#	that contain file names and the external
#	declarations associated with each file.
#	The first two parts of the sed script put the pattern
#	(in this case the file name) into the hold space
#	and creates the "filename filename" lines and
#	writes them out. The first part is for .o files,
#	the second is for .o's in archives.
#	The next 3 sections of code are exactly alike but
#	they handle different external symbols, namely the
#	symbols that are defined in the text section, data section
#	or symbols that are referenced but not defined in this file.
#	A line containing the symbol (from the pattern space) and 
#	the file it is referenced in (from the hold space) is
#	put into the pattern space.
#	If it's text or data it is written out to the symbol definition
#	(symdef) file, otherwise it was referenced but not declared
#	in this file so it is written out to the symbol referenced
#	(symref) file.
#	Stderr is redirected to capture any error messages for printing
#	at the end.
$NM -g $* 2>>$tmp/$$lorder.err  | sed '
	/\.o:$/{
		s/://
		s/^.* //
		h
		s/.*/& &/
		p
		d
	}
	/\.o ‚Å.*:$/{
		s/\.o ‚Å.*://
		s/.*/&.o/
		h
		s/.*/& &/
		p
		d
	}
	/\.o]:$/{
		s/]://
		s/^.*\[//
		h
		s/.*/& &/
		p
		d
	}
	/\.o] ‚Å.*:$/{
		s/\.o] ‚Å.*://
		s/^.*\[//
		s/.*/&.o/
		h
		s/.*/& &/
		p
		d
	}
	/ T /{
		s/ .*$//
		G
		s/\n/ /
		w '$tmp/$$symdef'
		d
	}
	/ D /{
		s/ .*$//
		G
		s/\n/ /
		w '$tmp/$$symdef'
		d
	}
	/ A /{
		s/ .*$//
		G
		s/\n/ /
		w '$tmp/$$symdef'
		d
	}
	s/ .*$//
	G
	s/\n/ /
	w '$tmp/$$symref'
	d
'
shift

sort $tmp/$$symdef -o $tmp/$$symdef
sort $tmp/$$symref -o $tmp/$$symref
join $tmp/$$symref $tmp/$$symdef | sed 's/[^ ]* *//'

# Display errors if any occurred
if [ -s $tmp/$$lorder.err ] ; then
	cat $tmp/$$lorder.err | sed 's/^nm:/lorder:/' >&2
	dspmsg lorder.cat 1 "Usage: lorder file ...\n" >&2
	RET_CODE=1
else
	RET_CODE=0		#successful completion
fi

# Clean up and exit
rm -f $tmp/$$sym?ef $tmp/$$lorder.err
exit $RET_CODE
