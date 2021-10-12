# @(#)98	1.4  src/bos/usr/lib/nim/objrepos/cmdnim_mdefs.awk, cmdnim, bos411, 9428A410j  6/13/94  15:29:35
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: ./usr/lib/nim/objrepos/cmdnim_mdefs.awk
#		begin_array
#		end_array
#		
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


# script to generate an include file that contains the default messages

BEGIN {
	name = ""
	debug_file = "debug"
	if ( debug )
		print >debug_file
}

function begin_array() {
	line[0] = "\n/*---------------------------- " name "_msg      --------------------*/"
	line[2] = "static char *" name "_default[] = {\n      \"null message\""
	lc = 2
}

function end_array() {
	if ( name == "" )
		return 

	line[++lc] = "};\n"
	line[++lc] = "char *" name "_msg( num )\nint num;\n{"
	line[++lc] = "   if ( (num >= 1) && (num <= " name "_SET_max) )"
	line[++lc] = "      return( catgets(niminfo.msgcat_fd," name "_SET,num," name "_default[num]) );"
	line[++lc] = "   else\n      return( NULL );\n}\n"
	line[++lc] = "int " name "_num( char *msg )\n{   int i;\n"
	line[++lc] = "   for (i=1; i <= " name "_SET_max; i++)"
	line[++lc] = "      if (! strcmp( " name "_default[i], msg ) )"
	line[++lc] = "         return( i );\n"
	line[++lc] = "   return( 0 );\n}"
	line[1] = "#define " name "_SET_max\t\t" count "\n"
	for (i=0; i <= lc; i++)
		print line[i]
}

END {
	end_array()
}

# $set starts new set, hence starts new default need_comma array
/^\$set[ 	]+/ {
	# terminate previous set
	end_array()
	ignore = 0

	# ignore if not symbolic name
	ignore = match( $2, /^[0123456789]+$/ )
	if ( debug )
		print "new set = " $2 "; ignore = " ignore>>debug_file
	if ( ignore )
	{
		name = ""
		next
	}
	
	# construct the base name
	name = $2
	gsub(/_SET/,"",name)
	begin_array()
	count = 0
	in_msg = 0
}

# ignore comments & empty lines
/^\$[ 	]*.*$/ || /^$/ || ignore {
	next
}

debug > 2 {
	print "\tline = " $0 >>debug_file
}

# beginning of new message?
$0 ~ /^([^ 	])+([ 	])+["\].*/ && in_msg == 0 {
	# start of new msg
	if ( debug )
		print "start = " $1 >>debug_file
	in_msg = 1
	count++
	$1 = ""

	# add comma to previous
	line[lc] = line[lc] ","

	# skip other stuff if nothing here
	if ( $2 == "\\" )
		next
}

in_msg == 0 {
	next
}

# end of message?
$0 ~ /"([ 	])*$/ {
	# end of a current message
	in_msg = 0

	if ( debug )
		print "end = " $0 >>debug_file
}
		

# print message
{
	line[++lc] = $0

	if ( debug > 1 )
		print "\tline[ " lc " ] = " line[lc] >>debug_file
}

