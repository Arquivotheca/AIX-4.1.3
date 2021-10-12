#!/usr/local/bin/perl
# @(#)00	1.11  src/bldenv/pkgtools/odmupdate.pl, pkgtools, bos41J, 9524C_all  6/12/95  11:48:35 
#
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL 
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#************************************************************************
# NAME: odmupdate
# DESCRIPTION: odmupdate creates the .odmadd, .odmdel, and .unodmadd files
#              which are used to modify the odm database for an update.
#              odmupdate determines the differences between the current 
#              .add file and the history .add file and generates the 
#              appropriate commands.  odmupdate also updates the history 
#              file with added, modified, and deleted stanzas.
# PRE CONDITIONS:
#    The current and history stanza files which are given on the command line,
#    must be preprocessed where all the comments and embedded blank lines are 
#    stripped out and one blank line is inserted after every stanza(including
#    the last stanza).
# POST CONDITIONS:
#    Three files are generated
#       1. option.basename.odmadd
#       2. option.basename.odmdel
#       3. option.basename.unodmadd
#    The history file will be modified with any new, deleted, or 
#       modified stanzas
#
# PARAMETERS:
#    input: objclassdb - object class database - 
#                        defines the object class keys and fields
#           option     - option name
#           historyfile- fully qualified history file name
#           basename   - base name of the .add file
#           dir        - location where files will be placed          
# NOTES: 
#     When a stanza is added, deleted, or modified, only the object class name
#        and keys get added to the history file.  This is to ensure that the
#        appropriate commands get generated for all subsequent updates.
#        By adding only the keys to the history file, subsequent comparisons
#        to the history file will always detect the differences.
#
#        For example, if a stanza gets added after gold then deleted at a later
#        time, an odmdelete command will always get generated for subsequent 
#        updates since the object class and keys only appear in the 
#        history file.
# DATA STRUCTURES:
#  OBJCLASSDB related data structures
#   - objectClass - assoc array; indexed by ODM class Name.
#                   created during read of objclassDB file. Used to 
#                   be sure classes in stanza files are know.
#   - fieldname   - assoc array; indexed by class Name and 
#                   attribute(field) name.  Created during read of objclassDB.
#                   Contains complete list of attributes for the class.
#                   Used to validate attribute names in stanza files.
#   - objectclassKeys - assoc array; indexed by class Name.
#                   Created during read of objclassDB.  Contains list of
#                   attributes considered keys for the class.
#   - objectclassFields - assoc array; indexed by class Name.
#                   Created during read of objclassDB.  Contains complete
#                   list of attributes(fields) for the class.
#  HISTORY FILE related data structures.
#   - objects     - assoc array; indexed by history stanza Nmbr.
#                   Created during read of history file.  Contains name
#                   of class stanza belongs to.
#   - fieldvalue  - assoc array; indexed by class Name, attribute name, and
#                   stanza nmbr.  Created during read of history file.
#                   Used for comparison with new stanzas.
#   - stanza      - assoc array; indexed by history stanza nmbr.
#                   Created during comparison with curr stanza.  Contains
#                   "visited" flag used to mark visits to determine 
#                   deletions.
#   - modified    - assoc array; indexed by classname and key values.
#                   created during comparison with curr stanza.  Contains
#                   a flag indicating that all stanzas in the class and
#                   with the same key values should be considered modified,
#                   even if they really are not modified.  This treatment
#                   is required because the odmdelete command that is 
#                   generated will delete ALL stanzas matching the key
#                   values, so they all must be added back -- even if they
#                   were not modified.
#  CURRENT FILE related data structures.
#    - currFieldValue - assoc array; indexed by class Name, attribute name,
#                   and current stanza nmbr.  Created during read of
#                   current file.  Used to print stanza to odmadd file;
#                   Really required to handle multple stanzas matching a
#                   single key.
#  MISCELANEOUS data structures.
#    - keyToHist  - assoc array; indexed by class name and key names.
#                   Created during read of history file.  Contains
#                   a space seperated string of history stanza nmbrs
#                   of history stanzas matching the keys.
#    - keyToCurr  - assoc array; indexed by class name and key names.
#                   Created during read of current file.  Contains
#                   a space seperated string of current stanza nmbrs
#                   of current stanzas matching the keys.
#    - histStCnt  - assoc array; indexed by class name and key names.
#                   Created during read of history file.  Contains
#                   number of history stanzas matching the keys.
#    - currStCnt  - assoc array; indexed by class name and key names.
#                   Created during read of current file.  Contains
#                   number of current stanzas matching the keys.
# RETURNS: 0 success, -10 failure
#*****************************************************************************

# set up to call the CleanUp subroutine when a signal is detected
%SIG=('HUP','CleanUp','INT','CleanUp','QUIT','CleanUp',
      'TERM','CleanUp','ABRT','CleanUp');

$rm = "$ENV{'ODE_TOOLS'}/usr/bin/rm";
$cp = "$ENV{'ODE_TOOLS'}/usr/bin/cp";
$cat = "$ENV{'ODE_TOOLS'}/usr/bin/cat";
$chmod = "$ENV{'ODE_TOOLS'}/usr/bin/chmod";
$echo = "$ENV{'ODE_TOOLS'}/usr/bin/echo";
$cmd = $0;
$cmd =~ s#^.*/(\S*).*$#$1#;

$FATAL = -10;
$SUCCESS = 0;
$modifiedFlag = "B_L_D___M_O_D_I_F_I_E_D";

# make sure 7 input parameters are given
if ($#ARGV != 6) {
    &FatalExit("number of input parameters is incorrect\n");
}

# read the command line arguments into variables
($objclassdb, $option, $historyfile, $basename,$dir,$history,$current) = @ARGV;

$debug = (defined($ENV{DBGODMUPDATE})) ? 1 : 0;	# set debug based on env var.

# set names for the script files to be generated
$odmdel = "$dir/$option.$basename.odmdel";
$odmadd = "$dir/$option.$basename.odmadd";
$unodmadd = "$dir/$option.$basename.unodmadd";

# open the output files.
open(NEWHISTORY,">newhistory.$$") || die "Could not open new history; '$!'\n";
open(ODMADD,">$odmadd") || die "Could not open '$odmadd'; '$!'\n";
open(UNODMADD,">$unodmadd") || die "Could not open '$unodmadd'; '$!'\n";
open(TMP_ODMDELETE,">tmpodmdelete.$$") 
    || die "Could not open '$tmpodmdelete.$$'; '$!'\n";
open(TMP_ODMGET,">tmpodmget.$$") 
    || die "Could not open '$tmpodmget.$$'; '$!'\n";

# Do the real work
&loadObjClassDB($objclassdb);
&loadHistoryFile($history);
&processCurrentFile($current);
&handleDeletedStanzas($numstanzas);

# Finish up the script work.
close(TMP_ODMDELETE);
close(TMP_ODMGET);
close(UNODMADD);
close(NEWHISTORY);
close(ODMADD);

# Turn the working odmdelete file into the real odmdel script
if (-s "tmpodmdelete.$$") {
    open(ODMDEL,">$odmdel") || die "Could not create '$odmdel'; '$!'\n";
    print ODMDEL "if [ \"\$INUSAVE\" = \"1\" ] ; then\n\n";
    close(ODMDEL);
    system("$cat tmpodmget.$$ >> $odmdel");
    system("$echo \"chmod a+x \\\$SAVEDIR/$option.$basename.rodmadd\n\" >> $odmdel");
    system("$echo fi >> $odmdel");
    system("$cat tmpodmdelete.$$ >> $odmdel");
    
    system ("$chmod a+x $odmdel");
}
# remove the temporary files
system("$rm -f tmpodmdelete.$$ tmpodmget.$$");

# if the .unodmadd file has commands in it
if (-s $unodmadd) {
    # make the .unodmadd script executable
    system ("$chmod a+x $unodmadd");
}
else
{
    # remove the file since no odm commands were generated
    system("$rm $unodmadd");
}

# if the .odmadd file does not have stanzas in it
if (-z $odmadd) {
    system("$rm $odmadd");
}

# replace the history file with the new version and remove the temporary copy
system("$cp newhistory.$$ $historyfile; $rm newhistory.$$");
exit $SUCCESS;


#========================================================================
# NAME: load objclassDB
# DESCRIPTION: 
#      Read and store the object class data base information (objects,
#      keys, and field names) into associative arrays for later use when
#      processing the history and current stanza files.
# PARAMETERS: 
#    - $objclassdb - name of the file containing the objclassDB info.
# NOTES:
# DATA STRUCTURES:
#    - $objectClass - initialized
#    - $objectclassKeys - initialized
#    - $objectclassFields - initialized
# RETURNS: none
#========================================================================
sub loadObjClassDB
{
    local($objclassdb) = $_[0];
    local($objectname, $keys, $fields, $line, $keyname);

    if (!open(OBJCLASSDB,"<$objclassdb")) {
	&FatalExit("unable to open $objclassdb for reading:$!\n"); 
    }

    # for each line in the object class data base
    while ($line = <OBJCLASSDB>) {
	chop $line;

	#  ignore blank lines and comments
	next if (($line eq undef) || 
		 ($line =~ m/^\s*$/) || 
		 ($line =~ m/^\s*#/));

	#  split the line into objectname, keys, and fields
	($objectname, $keys, $fields) = split (':', $line);

        #  check for valid object class definition 
        #  if the object name is undefined or the keys are undefined or if the
        #  fields are undefined and the keys are not equal to 99, then the 
        #  definition is bad (if the keys = 99, then the object class in not 
        #  updateable)
	if (($objectname eq undef)  ||  
	    ($keys eq undef)  ||  
	    (($fields eq undef) && ($keys ne '99' ))) {

	    &PError(" unrecognized class definition line in $objclassdb.\n"
		    . "       $line\n");

            #  get the next object class definition
	    next;
	}

	#  if the object class was previously defined
	if (defined $objectClass{$objectname}) {
	    &PError("object $objectname was defined more than once in $objclassdb.)\n");

            #  get the next object class definition
	    next;
	}

	#  if the object class is updatable
        if ($keys ne '99'){
	    #  Create an associative array which contains the fields defined
	    #  for each object class
	    foreach $field (split(',', $fields)) {
		$fieldname{$objectname,$field} = $field;
	    }

	    #  loop for each object class key
	    foreach $keyname (split('%', $keys)) {
		#  If the key field does not appear in the list of fields
		if (! defined $fieldname{$objectname,$keyname}) {
		    &PError("keyname $keyname not field of object $objectname\n");
		    #  get the next object class definition
		    next;
		}
	    }
	}

	# save the information for this object class in associative arrays
        $objectClass{$objectname} = $objectname;
	$objectclassKeys{$objectname} = $keys;
	$objectclassFields{$objectname} = $fields;

    }  # end on loop on OBJCLASSDB
}

#========================================================================
# NAME: load history file
# DESCRIPTION: 
#     read and store the history stanza file and verify each stanza 
#     against the object class definition
# PARAMETERS: none
# NOTES:
# DATA STRUCTURES:
# RETURNS: none
#========================================================================
sub loadHistoryFile
{
    local($history) = $_[0];

    if (!open(HISTORY,"<$history")) {
	&FatalExit("unable to open $history file for reading:$!\n"); 
    }

    # initialize the stanza counter
    $numstanzas = 0;

    #  read each line of the history stanza file
    while ($line = <HISTORY>) {
	chop $line;
  
	#  ignore blank lines
	next if (($line eq undef) || ($line =~ m/^\s*$/));

	#  split the line into classname and the rest of the line
	#  to determine if this the beginning of a stanza
	($objectname, $junk) = split (' ', $line);
	if (($objectname =~ m/.*:$/) && 
	    (($junk eq undef) || ($junk =~ m/^\s*$/))) {
	    #  process the last stanza
	    &ProcessPrevHistory();

	    # increment the stanza counter
	    $numstanzas++;

	    #  Get rid of the : following the object class name
	    $classname = $line;
	    $classname =~ s/:.*//;
	    $objects{$numstanzas} = $classname;
	    
	    # if the object class name is not valid
	    if (!defined $objectClass{$classname}) {
		&FatalExit("unrecognized classname $classname.\n");
	    }

	    if ($objectclassKeys{$classname} == '99') {
		#  write the stanza name to the history file
		print NEWHISTORY "$line\n";
	    }
	}
	else
	{
	    # if the object class is not updatable
	    if ($objectclassKeys{$classname} == '99') {
		# write the line to the history file
		print NEWHISTORY "$line\n";
	    }
	    else
	    {
		# the line contains a field
            
		# split the line into field name and value
		($fldname,$fieldvalue) = split ("=",$line,2);

		# eliminate leading and trailing blanks
		$fldname =~ s/^\s*(\S*)\s*$/$1/;
		$fieldvalue =~ s/^\s*//;
		$fieldvalue =~ s/\s*$//;

		# if the field name is not valid for this object class name
		if ((!defined $fieldname{$classname,$fldname}) && 
		    ($fldname ne $modifiedFlag))
		{
		    &FatalExit("unrecognized field name $fldname for class $classname.\n");
		}

		# save the field value
		$fieldvalue{$classname,$fldname,$numstanzas} = $fieldvalue;


		# count the number of quotes in the string
		$junk = $fieldvalue;
		$num = &CountQuotes(*junk);

		# if the number of quotes are odd or there is a continuation
		# character at the end of the line, the value spans multiple lines
		if (($num == 1) || ($fieldvalue =~ m/\\$/)) {
		    # get another line
		    $restofline = <HISTORY>;
		    chop $restofline;

		    # count the number of quotes in the string
		    $junk = $fieldvalue . " " . $restofline;
		    $num = &CountQuotes(*junk);

		    # while there is a continuation character at the end of the
		    # line or the number of quotes are odd, keep reading lines
		    # until the end of the value is encountered
		    while (($restofline =~ m/\\$/) || ($num == 1)) {

			# concatinate the rest of the line to the value of the
			# field
			$fieldvalue{$classname,$fldname,$numstanzas} .= 
			    "\n" . $restofline;

			# get another line
			$restofline = <HISTORY>;
			chop $restofline;

			$junk = $fieldvalue{$classname,$fldname,$numstanzas} 
			        . " " . $restofline;
			# count the number of quotes in the string
			$num = &CountQuotes(*junk);
		    }

		    # process the last line of the field value
		    $fieldvalue{$classname,$fldname,$numstanzas} .= "\n" . $restofline;
		}
	    }
	}
    }  # end loop on HISTORY

    #  process the last stanza
    &ProcessPrevHistory();
} # END loadHistory 

#========================================================================
# NAME: process Current File
# DESCRIPTION: 
#     read the current stanza file.  For each stanza in the current stanza 
#     file, search the history stanza file for a key match.  If no match
#     is found, then the stanza is new.  If a key match is found, and a field
#     value has changed, then the stanza is modified.  Otherwise, the stanza 
#     is unchanged.
# PARAMETERS: none
# NOTES:
# DATA STRUCTURES:
# RETURNS: none
#========================================================================
sub processCurrentFile
{
    local($current) = $_[0];
    local($classname);

    if (!open(CURRENT, "<$current")) {
	&FatalExit("unable to open $current file for reading:$!\n"); 
    }

    # initialize the current stanza counter and class name
    $currentstanza = 0;
    undef $classname;


    #  read each line of the current stanza file
    while ($line = <CURRENT>) {
	chop $line;
    
	#  ignore blank lines
	next if (($line eq undef) || ($line =~ m/^\s*$/));

	#  split the line into classname and the rest of the line
	#  to determine if this the beginning of a stanza
	($objectname, $junk) = split (' ', $line);
	if (($objectname =~ m/.*:$/) && 
	    (($junk eq undef) || ($junk =~ m/^\s*$/))) {
	    # process the previous stanza
	    &ProcessPrevStanza();

	    # increment the stanza count
	    $currentstanza++;

	    #  Get rid of the : following the name
	    $classname = $line;
	    $classname =~ s/:.*//;

	    # if the object class name is not valid
	    if (!defined $objectClass{$classname}) {
		&FatalExit("unrecognized classname $classname.\n");
	    }
	}  # end of if processing object class name
	else
	    # processing a field value
	{
	    # if the object class is updateable
	    if ($objectclassKeys{$classname} != '99') {

		# split the line into field name and value
		($fldname,$fieldvalue) = split ("=",$line,2);
 
		# eliminate leading and trailing blanks  
		$fldname =~ s/^\s*(\S*)\s*$/$1/;
		$fieldvalue =~ s/^\s*//;
		$fieldvalue =~ s/\s*$//;

		# if the field name is not valid for this object class name
		if (!defined $fieldname{$classname,$fldname}) {
		    &FatalExit("unrecognized field name $fldname for class $classname.\n");
		}
          
		# save the field value
		$currFieldValue{$classname,$fldname,$currentstanza} = 
		    $fieldvalue;

		# count the number of quotes in the string
		$junk = $fieldvalue;
		$num = &CountQuotes(*junk);
 
		# if the number of quotes are odd or there is a continuation
		# character at the end of the line, the value spans multiple lines
		if (($num == 1) || ($fieldvalue =~ m/\\$/)) {
		    # get another line
		    $restofline = <CURRENT>;
		    chop $restofline;

		    # count the number of quotes in the string
		    $junk = $fieldvalue . " " . $restofline;
		    $num = &CountQuotes(*junk);

		    # while there is a continuation character at the end of the
		    # line or the number of quotes are odd, keep reading lines
		    # until the end of the value is encountered
		    while (($restofline =~ m/\\$/) || ($num == 1)) {
			# concatinate the rest of the line to field value
			$currFieldValue{$classname,$fldname,$currentstanza} .= 
			    "\n" . $restofline;

			# get another line
			$restofline = <CURRENT>;
			chop $restofline;

			# count the number of quotes in the string
			$junk = $currFieldValue{$classname,$fldname,
						$currentstanza} 
			        . " " . $restofline;
			$num = &CountQuotes(*junk);
		    }

		    # process the last line of the field value
		    $currFieldValue{$classname,$fldname,$currentstanza} .= 
			"\n" . $restofline;
		}
	    }
	}
    }  # end loop on CURRENT stanza file

    # process the last stanza
    &ProcessPrevStanza();
} # END processCurrentFile

#========================================================================
# NAME        : Handle Deleted Stanzas
# DESCRIPTION : 
#     for the stanzas not visited(deleted), add their stanza name and
#     keys to the history file
# PARAMETERS  :
#     $numstanzas - number of stanzas in the history array.
#               Used to control the loop through the history stanzas.
# NOTES       :
# DATA STRUCTURES:
#    - none are modified.  Several are looked at.
#    - ODM script files and history file may be modified.
# RETURNS     : none
#========================================================================
sub handleDeletedStanzas
{
    local($numstanzas) = $_[0];
    local($keyval, $keyValues, $keycount, $keystring);
    local($deleteOnly, $junk, $tmp);

    for ($i = 1; $i <= $numstanzas; $i++) {
	$deleteOnly = "no";
	if (!defined $stanza{$i}) {
	    # Generate the keyValue for this stanza
	    $keyValues = "";
	    foreach $key (split("%",$objectclassKeys{$objects{$i}}))
	    {
		$tmp = $fieldvalue{$objects{$i},$key,$i};
		$tmp =~ s/^"(.*)"$/$1/; # strip surronding quotes

		$keyValues .= "~~~" . $tmp;
	    }
	    if ($debug)
	    {
		print STDERR 
		    "@@ handleDeletes: Found unvisited stanza, # = $i\n";
	        print STDERR 
		    "@@\tclass = $objects{$i};keyvalues = $keyValues\n";
	    }
	    #-----------------------------------------------------
	    # Check for other stanzas with same key values.
	    #-----------------------------------------------------
	    if (($histStCnt{$objects{$i},$keyValues} > 1) || 
		($currStCnt{$objects{$i},$keyValues} > 1))
	    {
		if (!defined($modified{$objects{$i},$keyValues}))
		{
		    print STDERR "@@\t multiple stanza match key\n" if $debug;
		    #------------------------------------------------
		    # There are other stanzas with the same keys
		    # AND none of them have been modified.
		    # So, we must do the scripts AND also put 
		    # the still existing stanzas in the odmadd
		    # file.
		    #------------------------------------------------
		    if ($currStCnt{$objects{$i},$keyValues} > 0)
		    {
			$currStanzas = $keyToCurr{$objects{$i},$keyValues};
			for ($j = 0; $j < $currStCnt{$objects{$i},$keyValues}; 
			     $j++)
			{
			    ($currStanza, $restOfLine) = split(" ",
							       $currStanzas,2);
			    $currStanzas = $restOfLine;
			    #-----------------------------------------
			    # Do all of the ouput work the 1st time
			    # (odmaddOnly flag = 0).
			    # Do only the odmadd stuff thereafter.
			    #-----------------------------------------
			    if ($j == 0)
			    {
				&updOutputFiles("modified", 0, $keyValues, 
						$currStanza, $objects{$i});
			    }
			    else
			    {
				&updOutputFiles("modified", 1, $keyValues, 
						$currStanza, $objects{$i});
			    }
			}
		    }
		    else
		    {
			#--------------------------------------------
			#  All stanzas have been deleted from the
			#      current add file.  So, don't
			#      have any current stanzas to loop
			#      through.  Must delete differently.
			#--------------------------------------------
			print STDERR 
			"@@\t all stanza's deleted, set modified\n" if $debug; 
			$deleteOnly = "yes";
			$modified{$objects{$i},$keyValues} = 1;
		    }
		}
		else
		{
		    #------------------------------------------------
		    # There is nothing to do in this case.  
		    # Other stanza's matching these keys have
		    # already been processed.  The history file 
		    # has a "keys" only entry in it,
		    # and the script files have the deletes in
		    # them.  Both of these are true because of
		    # some other stanza in the group was modified.
		    #------------------------------------------------
		    $junk = "Do Nothing";
		    print STDERR 
		    "@@\t Other stanza matching key already modified; nothing todo\n" if $debug;
		}
	    }
	    else
	    {
		#-------------------------------------------------
		# Only 1 stanza matches the keys, so no other
		# stanzas are affected.  Need to do delete only.
		#-------------------------------------------------
		print STDERR 
		   "@@\t only 1 stanza matches keys, set modified\n" if $debug;
		$deleteOnly = "yes";
		$modified{$objects{$i},$keyValues} = 1;
	    }

	    #------------------------------------------------------
	    #   Need to generate delete entries only.
	    #   This must be handled as a special case
	    #   since no odmadd or unodmadd stuff is needed.
	    #------------------------------------------------------
	    if ($deleteOnly eq "yes")
	    {
		print STDERR "@@\t gen delete script entries only\n" if $debug;

		print NEWHISTORY "$objects{$i}:\n";
		print NEWHISTORY "\t$modifiedFlag = 1\n";

		# write the keys to the history file
		$numkeys = split("%",$objectclassKeys{$objects{$i}});
		$keycount = 0;
		undef $keystring;

		foreach $key (split("%",$objectclassKeys{$objects{$i}})) {
		    $keycount++;
		    $keyval = $fieldvalue{$objects{$i},$key,$i};

		    print NEWHISTORY "\t$key = $keyval\n";
		    
		    # strip off any surrounding double quotes.
		    $keyval =~ s/^"(.*)"$/$1/;
		    $keystring .= "$key = '$keyval'";
           
		    if ($keycount < $numkeys) {
			$keystring .= " AND ";
		    }
		}
		print NEWHISTORY "\n";

		# add an odmget and an odmdelete to the .odmdel file
		print TMP_ODMGET "odmget -q \"$keystring\" $objects{$i} >> \$SAVEDIR/$option.$basename.rodmadd\n";
		print TMP_ODMDELETE "odmdelete -o $objects{$i} -q \"$keystring\" > /dev/null\n";
	    }
	}
    }
} # END handleDeletes


#========================================================================
# NAME: ProcessPrevHistory
# DESCRIPTION: Process the previous stanza of the history file to
#              determine if the stanza is valid
# PARAMETERS: none
# NOTES:
# DATA STRUCTURES:
#    - $stanza - modified for non_updateable stanzas.
#    - NEWHISTORY file modified for non_updateable stanzas.
#    - $numstanzas - referenced only.
#    - $objectclassKeys - referenced only.
# RETURNS: none
#========================================================================
sub ProcessPrevHistory {
    local($keyVal, $tmp);

    if ($numstanzas > 0) {
        # if the previously processed stanza was not updateable
        if ($objectclassKeys{$classname} == '99') {
            #  put a blank line after the last line of the stanza
            print NEWHISTORY "\n";

            #  mark the stanza as visited
            $stanza{$numstanzas} = 'visited';
        }
        else
        #  previously processed stanza was updateable
        {
            # Get values for each key field.
            foreach $key (split("%",$objectclassKeys{$classname})) 
	    {
                if (!defined $fieldvalue{$classname,$key,$numstanzas}) 
		{
		    &FatalExit(
		    "not all keys were set for classname $classname ($key).\n");
                }
		else
		{
		    $tmp = $fieldvalue{$classname,$key,$numstanzas};
		    $tmp =~ s/^"(.*)"$/$1/; # strip surronding quotes
		    $keyVal .= "~~~" . $tmp;
		}
            }

	    #----------------------------------------------------
	    # update count and mapping of key to stanza.
	    # This info is required to handle multiple stanzas
	    # matching the same key.
	    #----------------------------------------------------
	    if (defined($histStCnt{$classname,$keyVal}))
	    {
		$histStCnt{$classname,$keyVal}++;
		$keyToHist{$classname,$keyVal} .=
		    " $numstanzas";
	    }
	    else
	    {
		$histStCnt{$classname,$keyVal} = 1;
		$keyToHist{$classname,$keyVal} =
		    "$numstanzas";
	    }
        }
    }
}

#
# NAME: ProcessPrevStanza
#
# DESCRIPTION: Process the previous stanza to determine if the stanza is
#              a valid stanza and if it is new, modified, or unchanged.
#
# PARAMETERS: none
#
# NOTES:
#
# DATA STRUCTURES: none
#
# RETURNS: none
#
sub ProcessPrevStanza {
    local($status, $tmp, $keyValues, $i);

    if ($currentstanza > 0) {
        # if the previously processed stanza was updateable
        if ($objectclassKeys{$classname} != '99') {

            # Get values for each key field.
            foreach $key (split("%",$objectclassKeys{$classname})) 
	    {
                if (!defined $currFieldValue{$classname,$key,$currentstanza}) 
		{
		    &FatalExit(
		    "not all keys were set for classname $classname ($key).\n");
                }
		else
		{
		    $tmp =  $currFieldValue{$classname,$key,$currentstanza};
		    $tmp =~ s/^"(.*)"$/$1/; # strip surronding quotes

		    $keyValues .= "~~~" . $tmp;
		}
            }

	    #----------------------------------------------------
	    # update count and mapping of keys to stanza.
	    # This info is required to handle multiple stanzas
	    # matching the same key.
	    #----------------------------------------------------
	    if (defined($currStCnt{$classname,$keyValues}))
	    {
		$currStCnt{$classname,$keyValues}++;
		$keyToCurr{$classname,$keyValues} .=
		    " $currentstanza";
	    }
	    else # stanza is new!
	    {
		$currStCnt{$classname,$keyValues} = 1;
		$keyToCurr{$classname,$keyValues} =
		    "$currentstanza";
	    }
	    #-------------------------------------------------------
	    # Must now determine if this current stanza is
	    # new, modified,unchanged.  Deleted stanzas
	    # are handled later.
	    # 
	    # New stanzas are detected by no history stanza
	    # matching the key values.  Modifieds are more
	    # difficult.
	    #
	    # If the $modified{$classname,$keyValues} is set,
	    # then the stanza's keys match another stanza 
	    # that has already been discovered as modified.
	    # In this case, the stanza is treated as modified
	    # without even looking -- the odmdelete command has
	    # already been generated to remove this stanza from
	    # the ODM on the system, so must make sure this stanza
	    # is included in the odmadd file that is shipped.
	    #--------------------------------------------------------
	    if (defined($modified{$classname,$keyValues}))
	    {
		$status = "modified";
	    }
	    elsif (!defined($histStCnt{$classname,$keyValues}))
	    {
		print STDERR "@@ ProcPrvSt: New stanza detected '$classname', '$keyValues', $currentstanza\n" if $debug;
	        $status = "new";
	    }
	    else
	    {
		#-------------------------------------------------------
		# For current stanzas that have key values matching
		# an existing history stanza, we have to loop through
		# all existing history stanzas looking for a match.
		# If we find an exact match, then the current stanza
		# is not modified.  If all existing history stanzas
		# do not match the current stanza, then the current
		# stanza is either new or modified.  
		#
		# We have to treat it as modified, because in the
		# script we cannot risk adding identical stanzas into
		# the ODM.  Therefore, we must generate the delete
		# command and put ALL current stanzas matching the 
		# key values into the odmadd file.
		#
		# There has also been a case discovered where the 
		# ODM class has ALL of its fields defined as keys.
		# This situation causes the original method of storing
		# changes in the history file to miss the fact that
		# the stanza has changed at some point.  Now a
		# dummy field is being written to the history file.
		# This field is identified by the $modifiedFlag
		# variable.  IF this field is defined in the history
		# field array, then the current stanza is treated
		# as modified regardless of whether any fields 
		# actually differ.
		#-------------------------------------------------------
		$histStanzas = $keyToHist{$classname,$keyValues};
		$status = "modified";
		for ($i = 0; 
		     $i < $histStCnt{$classname,$keyValues} &&
		     $status eq "modified";
		     $i++)
		{
		    ($histStanza, $restOfLine) = split(" ",$histStanzas, 2);
		    $histStanzas = $restOfLine;
		    next if (defined($stanza{$histStanza}));

		    if  (defined(
			    $fieldvalue{$classname,$modifiedFlag,$histStanza}))
		    {
			# stanza has been modified in the past.
			# continue to treat it as modified.
			if ($debug)
			{
			    printf(STDERR 
				 "@@procPrevSt: %s detected for class '%s'\n",
				   $modifiedFlag, $classname);
			    print  STDERR "@@\t  and keys '$keyValues'\n";
			    printf(STDERR 
				   "@@\t old st # = %d; new st # = %d\n",
				   $histStanza, $currentstanza);
			}
			$status = "modified";
		    }
		    else
		    {
			$status = "unchanged";
			foreach $fld 
			    (split(",",$objectclassFields{$classname})) 
			{
			    if ($fieldvalue{$classname,$fld,$histStanza} ne 
			       $currFieldValue{$classname,$fld,$currentstanza})
			    {
				# stanza has been modified
				if ($debug)
				{
				    print STDERR 
				    "@@procPrevSt: diff detected in field '$fld' of class '$classname', keys = '$keyValues'\n";
				    print STDERR 
				     "@@\t old val = '$fieldvalue{$classname, $fld, $histStanza}'\n";
				    print STDERR 
				     "@@\t new val = '$currFieldValue{$classname, $fld, $currentstanza}'\n";
				    print STDERR 
				     "@@\t old st # = $histStanza; new st # = $currentstanza\n";
				}
				$status = "modified";
				last;
			    }
			}
		    }
		}
		if ($status == "unchanged")
		{
		    $stanza{$histStanza} = 'visited';
		}
	    }
	    if (defined($modified{$classname,$keyValues}))
	    {
		$i = 1;		# flag to not update history file
	    }
	    else
	    {
		$i = 0;		# flag to update history file
	    }
	    &updOutputFiles($status, $i, $keyValues, 
			   $currentstanza, $classname);
	    
	    #------------------------------------------------------------
	    # Now must handle the case of a stanza chaning where 
	    # other stanzas have the same key values AND have already
	    # been processed.
	    # In this case, we need to go back through the set of
	    # already processed current stanzas matching the key
	    # and add them to the scripts.
	    #------------------------------------------------------------
	    if (($status ne "unchanged") && 
		(!defined($modified{$classname,$keyValues})))
	    {
		$modified{$classname,$keyValues} = 1;
		if (($histStCnt{$classname,$keyValues} > 1) || 
		    ($currStCnt{$classname,$keyValues} > 1))
		{
		    $currStanzas = $keyToCurr{$classname,$keyValues};
		    for ($i = 0; $i < $currStCnt{$classname,$keyValues}; $i++)
		    {
			($currStanza, $restOfLine) = split(" ",$currStanzas,2);
			$currStanzas = $restOfLine;
			if ($currStanza != $currentstanza)
			{
			    &updOutputFiles("modified", 1, $keyValues, 
				       $currStanza, $classname);
			}
		    }
		}
	    }
        }  # end of processing an updateable stanza
    } # end of if currentstanza > 0
}

#===================================================================
# NAME        : update output files
# DESCRIPTION : 
#     Generates appropriate entries in the script files,
#     the odmadd file, and the history file.
# PARAMETERS  :
#     - $status - flag indicating whether the stanza has
#               changed or not.  If it has not, no script
#               or odmadd entries are required.  Values
#               may be "unchanged", "modified", or "new".
#     - $odmaddOnly - boolean flag indicating that output
#               should be to the odmadd file only.  This
#               condition will be true IFF the stanza is
#               one that has keys matching other stanzas
#               and at least one of those other stanzas
#               has already been processed as "modified".
#               This flag keeps us from putting duplicate
#               ODM commands in the scripts.
#     - $fullKeyValues - all the key values contatenated
#               together with "~~~" seperating the fields
#               (and prefixing the 1st).  Used to index
#               into some of the mapping structures.
#     - $stanzaNmbr - the number of the current stanza
#               that is to be processed (used to index
#               into the $currFieldValue array).
#     - $classname - the name of the class the stanza
#               belongs to.  Used to get the set of
#               fields for the stanza and the set of keys.
# NOTES      :
#   - no global data structures are modified -- several
#     are referenced.
#   - files are modified.
#   - The history file work and the script work on done
#     in 1 place so that a single loop through the stanza
#     can be used.  Separate functions could have been
#     used, but the loop structure for each would have been
#     the same.
# RETURNS    : nothing.
#===================================================================
sub updOutputFiles
{
    local($status, $odmaddOnly, 
	  $fullKeyValues, $stanzaNmbr, $classname) = @_;
    local($keyCnt, $keystring, $fld, $fieldVal);

    if ($debug)
    {
	print STDERR 
	    "@@updOutputFiles: status = $status;\tkeys = $fullKeyValues\n";
	print STDERR 
	    "@@\todmaddOnly = $odmaddOnly, stanza = $stanzaNmbr\n";
    }

    $keyCnt = 0;
    if ($odmaddOnly == 0)
    {
	print NEWHISTORY "$classname:\n";
	#--------------------------------------------------------
	# IF the stanza is modified, be sure to tag it as 
	# such in the history file.  See commentary describing
	# detection of modified stanzas in ProcessPrevStanza
	# for a description of how and why this flag is needed.
	#--------------------------------------------------------
	if ($status ne "unchanged")
	{
	    print NEWHISTORY "\t$modifiedFlag = 1\n";
	}
    }

    if ($status ne "unchanged")
    {
	# only "changed" stanzas go to the odmadd file.
	print ODMADD "$classname:\n";
    }
    foreach $fld (split(",",$objectclassFields{$classname})) 
    {
	#-------------------------------------------------------------------
	# skip to next field in the list if this field is not
	# defined in the current stanza.  
	# NOTE:
	#   This skipping will only work as long as the requirement
	#   that ALL key fields have values.  Otherwise, this will
	#   not work correctly!
	#-------------------------------------------------------------------
	next if (! defined($currFieldValue{$classname,$fld,$stanzaNmbr}));

	$fieldVal = $currFieldValue{$classname,$fld,$stanzaNmbr};
	if ($status ne "unchanged")
	{
	    # add field to odmadd file only for changed stanzas.
	    print ODMADD "\t$fld = $fieldVal\n";
	    # Checking to see if field is a key.
	    if ($objectclassKeys{$classname} =~ /^$fld$/ ||
		$objectclassKeys{$classname} =~ /^$fld%/ ||
		$objectclassKeys{$classname} =~ /%$fld%/ ||
		$objectclassKeys{$classname} =~ /%$fld$/)
	    {
		if ($odmaddOnly == 0)
		{
		    print NEWHISTORY "\t$fld = $fieldVal\n";
		    # Add to the query string (note must strip
		    #   off surrounding double quotes to make
		    #   script work).
		    $fieldVal =~ s/^"(.*)"$/$1/;
		    if ($keyCnt > 0)
		    {
			$keystring .= " AND ";
		    }
		    $keystring .= "$fld = '$fieldVal'";
		    $keyCnt++;
		}
	    }
	}
	else # stanza is unchanged -- still goes to history file
	{
	    if ($odmaddOnly == 0) # this should always be true if we are here!
	    {
		print NEWHISTORY "\t$fld = $fieldVal\n";
	    }
	}
    } # END loop through fields
    if ($status ne "unchanged")
    {
	print ODMADD "\n";	             # blank line after stanza
	if ($odmaddOnly == 0)
	{
	    # Generate script entries for this stanza!
	    print TMP_ODMGET "odmget -q \"$keystring\" $classname >> \$SAVEDIR/$option.$basename.rodmadd\n";
	    print TMP_ODMDELETE "odmdelete -o $classname -q \"$keystring\" > /dev/null\n";
	
	    print UNODMADD "odmdelete -o $classname -q \"$keystring\" > /dev/null\n";		
	}
	#-------------------------------------------------------------
	# Must make sure that the unodmadd script removes
	#      the new stanza(s) during rejection processing.
	# But, only want the delete for the 1st new stanza
	#      matching the keys.
	#-------------------------------------------------------------
	elsif (($status eq "new") && 
	       ($currStCnt{$classname, $fullKeyValues} == 1))
	{
	    print STDERR "@@ updOutputFiles: adding unodmadd entry anyway\n" if $debug;
	    print UNODMADD "odmdelete -o $classname -q \"$keystring\" > /dev/null\n";		
	}

    }
    if ($odmaddOnly == 0)
    {
	print NEWHISTORY "\n";       # blank line after stanza
    }
} # END updOutputFiles

#
# NAME: CleanUp
#
# DESCRIPTION: Clean up files when a signal is received, then exit
#
# PARAMETERS: none
#
# NOTES:
#
# DATA STRUCTURES: none
#
# RETURNS: none
#
sub CleanUp {
    system("$rm -f $odmadd $odmdel $unodmadd tmpodmadd.$$"
           . " tmpodmget.$$ tmpodmdelete.$$ newhistory.$$ stanza.$$ ");
    exit $FATAL;
}

#
# NAME: CountQuotes
#
# DESCRIPTION: Count the number of quotes in the string
#
# PARAMETERS: none
#
# NOTES:
#
# DATA STRUCTURES: none
#
# RETURNS: number of quotes in the string
#
sub CountQuotes {
    # pass parameter junk by name
    local(*junk) = @_;
    local($numquotes);

    # put an X at the end of the string so that the calculation of the 
    # number of quotes will be 0 if the number is even and 1 if the 
    # number of quotes is odd
    $junk = $junk . "X";

    # eliminate escaped backslashes and then escapce quotes
    $junk =~ s/\\\\//g;
    $junk =~ s/\\"//g;

    # count the number of quotes in the string
    (@junk) = split(/"/, $junk);
    $numquotes = $#junk - (int($#junk / 2) * 2);

    $numquotes;
}

#
# NAME: FatalExit
#
# DESCRIPTION: Print out a fatal error message, cleanup and exit.
#
# PARAMETERS: string to print out.
#
# NOTES: Prepends the input string with helpful text like the name
#        of the tool and FATAL ERROR
#
# DATA STRUCTURES: none
#
# RETURNS: none
#
sub FatalExit {
    local($errmsg) = @_;
    print STDERR ("$cmd: FATAL ERROR: $errmsg");
    &CleanUp
}

#
# NAME: PError
#
# DESCRIPTION: Print out an error message.
#
# PARAMETERS: string to print out.
#
# NOTES: Prepends the input string with helpful text like the name
#        of the tool and ERROR
#
# DATA STRUCTURES: none
#
# RETURNS: none
#
sub PError {
    local($errmsg) = @_;
    print STDERR ("$cmd: ERROR: $errmsg");
}
