#!/usr/local/bin/perl
# @(#)88	1.2  src/bldenv/pkgtools/fsCnt.pl, pkgtools, bos41B, 412_41B_sync 12/16/94 13:53:29#
#   COMPONENT_NAME: pkgtools
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

#*******************************************************************
# NAME     : Fileset Counts
# FUNCTION : To count the number of filesets required to fix
#            the APAR given in the fix data stanza.
# EXECUTION ENVIRONMENT :
#            AIX user level (ie. nothing special).
# INPUTS   :
#    STDIN - FIX data stanza.
# OUTPUTS  :
#    STDOUT- Decimal number indicating number of filesets in 
#            the stanza.
# PRECONDITIONS   : none
# POST CONDITIONS : none
# NOTES    :
#    - If the specified APAR is an ML type (ie. has "_ML"
#      on the end), then count is obtained from the file
#      $TOP/HISTORY/MLcnts.  This file is currently updated
#      by hand!
#*******************************************************************

require "getopts.pl"    ;		# for cmd line parm parsing

#------------------------------------------
# Some global variables
#------------------------------------------
$cmd = substr($0, rindex($0, "/") + 1) ;
$version = "v1.0"  ;


#----------------------------------------------------------------
# NAME        : Cleanup
# DESCRIPTION :
#     General cleanup on exit.  
#----------------------------------------------------------------
sub cleanup
{
    system("rm -f $tmpStanza") ;
    return(0) ;
}


#----------------------------------------------------------------
# NAME        : Signal Handler
# DESCRIPTION :
#     General Signal Handler so that we can cleanup
#     appropriately
# INPUTS      :
#     $sig    - the name of the signal we caught.
#----------------------------------------------------------------
sub handler
{
    local($sig) = @_ ;

    print (STDERR "$cmd: Caught signal 'SIG$sig' - exiting\n") ;
    &cleanup ;

    exit(0) ;
}


#----------------------------------------------------------------
# NAME        : Usage
# DESCRIPTION :
#     Generate the usage statment for this utility
#----------------------------------------------------------------
sub Usage
{
    print STDERR <<END_USAGE ;

$cmd $version

$cmd - Usage: $cmd -a APAR_Nmbr [-d FixDataDBfile]
    where
      -a   identifies the APAR (or keyword) to search for in the 
           given FixDataDB file.  Only 1 APAR (or keyword) can
           be given.
           This flag is required.
      - d  identifies an alternative FixDataDB file to use to determine
           the number of filesets fixed by the given APAR.
           If this parameter is ommitted the default is to look for
           the file in the "selfix" tree using build environment
           variables to determine the location of that tree.
           The default location is determined via \$TOP/HISTORY/fixdataDB.
	   Currently, \$TOP is set to '$TOP'.

END_USAGE

    exit 1 ;
} # END Usage


#========================================================================
# Function Name : Parse Command Line
# Purpose       : Set variables based on command line
#                 options specifications
# Inputs        :
#       @ARGV   - command line arguments
# Outputs       : 
#    The following are global variables that are set by this routine.
#       $apar  - the value to search for in the "name" attribute.
#       $fixDB - the name of the fixdataDB file to search.
#       $debug - indicates that debug messages should be generated.
#========================================================================
sub parseCmdLine
{
    &Getopts("Da:d:") || &Usage ;
    if (defined($opt_D))
    {
	$debug = 1 ;
    }
    if (defined($opt_a))
    {
	$apar = $opt_a ;
	print (STDERR "'-a' detected; value = $opt_a\n") if ($debug) ;
	print (SDTERR "apar = $apar\n") if ($debug) ;
    }
    if (defined($opt_d))
    {
	$fixDB = $opt_d ;
	print (STDERR "'-d' detected; value = $opt_d\n") if ($debug) ;
	print (SDTERR "fixDB = $fixDB\n") if ($debug) ;
    }

    #-------------------------------------------------------------
    # Make sure all required parameters are set and
    # initialize any optional ones that can be.
    #-------------------------------------------------------------
    if (length($apar) == 0)
    {
	print STDERR "ERROR: $cmd- You must specify an APAR via -a flag\n" ;
	&Usage() ;
    }
    if (length($fixDB) == 0)
    {
	$fixDB = "$ENV{TOP}/HISTORY/fixdataDB" ;
	print STDERR "initialized fixDB to '$fixDB'\n" if $debug ;
    }
} # END parseCmdLine


#========================================================================
# Function Name : get Fix Record
# Purpose       : locate the fix record for the given keyword
#                 in the given fixdataDB.
# Inputs        :
#       $apar       - the apar/keyword stanza to locate
#       $fixDB      - the name of the fixdataDB to search.
#       $stanzaFile - name of the temp file to put the stanza in.
# Outputs       : 
#    Returns    - 0 on success, 1 otherwise.
#    $stanzaFile- contents are overwritten. 
#========================================================================
sub getFixRecord
{
    local($apar, $fixDB, $stanzaFile) = @_ ;

    local($matched, $rc) = 0 ;



    if (!open(DB , "<$fixDB"))
    {
	print STDERR "ERROR: $cmd- Can't read the fixdata DB '$fixDB'\n" ;
	print STDERR "ERROR: $cmd- reason = $!\n" ;
    }
    elsif (!open(STANZA, ">$stanzaFile"))
    {
	print STDERR 
	    "ERROR: $cmd- Can't write the temp stanza file '$stanzaFile'\n" ;
	print STDERR "ERROR: $cmd- reason = $!\n" ;
    }
    else # all files opened, so go to work.
    {
	print STDERR "@@ All files opened\n" if $debug ;
	while(<DB>)
	{
	    chop ;
	    #-------------------------------------------------
	    # Only want to look for the APAR if we haven't
	    # already found it.  Need to do something else
	    # if we have found it.
	    #-------------------------------------------------
	    if ($matched == 0)
	    {
		if (/^\s+name\s+=\s+.*/)
		{
		    ($attr, $value) = split(/\s+=\s+/) ;
		    $value =~ s/"//g ;    #"

                    print STDERR "@@ value of name = $value\n" if $debug ;

		    if ($value eq $apar)
		    {
			$matched++ ;
			printf(STANZA "fix:\n$_\n") ;

			print STDERR "@@@ Found match = '$_'\n" if $debug ;
		    }
		}
	    }
	    #-----------------------------------------------
	    # Once we have a match, a new stanza ends
	    # the current one.
	    #-----------------------------------------------
	    elsif (m/^fix:/) 
	    {
		last ;		# exit the loop
	    }
	    else		# otherwise, print the line.
	    {
		printf(STANZA "$_\n") ;
		print STDERR "@@@@ processing match; line = '$_'\n" if $debug ;
	    }
	} # END while loop
    }

    if (! $matched)
    {
	$rc = 1 ;
	
	print STDERR "@@ Never found a match for '$apar'\n" if $debug ;
    }
    return($rc) ;
} # END getFixRecord


#========================================================================
# Function Name : count Filesets
# Purpose       : Counts the number of filesets said to fix
#                 the problem represented by the given fix record.
# Inputs        :
#       $stanza - string containing the name of the tmp file
#                 containing the stanza.
# Outputs       : 
#       Returns - the number of filesets in the stanza.
#                 A negative number indicates an error.
# Notes         :
#       - This function expects that only 1 stanza exist in the
#         temp file.  Multiple stanza errors should have already
#         been detected and corrected.
#========================================================================
sub countFilesets
{
    local($stanza) = @_ ;

    local($attr)  = 'unknown' ;
    local($fsCnt) = 0 ;

    if (!open(STANZA, "<$stanza"))
    {
	print STDERR "ERROR: $cmd- could not open stanza file '$stanza'\n" ;
	print STDERR "ERROR: $cmd- reason = $!\n" ;
	$fsCnt = -1 ;
    }
    else
    {
	while(<STANZA>)
	{
	    chop ;
	    print STDERR "@@$_\n" if $debug ;
	    
	    if ($attr eq 'unknown')
	    {
		if (/\s+filesets\s+=/)         # look for fileset attr
		{
		    $attr = 'filesets' ; 
		    ($junk, $fs) = split(/\s+=\s+"/) ;	    #"
		    $fs =~ s/\\n\\// ;         # strip continuation stuff

		    print STDERR "@@@@ fs = $fs; attr = $attr\n" if $debug ;

		    #---------------------------------------------
		    # Make sure that we have a fileset VRMF
		    # entry before bumping the count.  
                    # If we have 2 parts and the 2nd part is
                    # a valid VRMF format, then we conclude
                    # that this is a filset VRMF entry to
                    # be counted.
		    #---------------------------------------------
	            ($fs, $vrmf) = split(/[\s:]+/, $fs) ;
	            if ($vrmf =~ m/\d+\.\d+\.\d+\.\d+/)   
	            {
			$fsCnt++ ;
			if ($debug)
			{
			    printf STDERR 
				"@@@@ bumped fsCnt; fsCnt = $fsCnt\n"; 
			}
		    }
                }
	    }
	    elsif ($attr eq 'filesets')
	    {
		print STDERR "@@@@ in attr = 'filesets' mode\n" if $debug ;

		$fs = $_ ;
		$fs =~ s/\\n\\// ;             # strip continuation stuff
 
		print STDERR "@@@@ fs = $fs\n" if $debug ;

		#--------------------------------------------------
		# See comment above for description of the
		# following check.
		#--------------------------------------------------
		($fs, $vrmf) = split(/[\s:]+/, $fs) ;
		if ($vrmf =~ m/\d+\.\d+\.\d+\.\d+/)
		{
		    $fsCnt++ ;
		    if ($debug)
		    {
			printf STDERR 
			    "@@@@ bumped fsCnt; fsCnt = $fsCnt\n"; 
		    }
		}
	    }
	} # END while loop
    }
    return($fsCnt) ;
} # END countFilesets


#***************************************************************
#  B E G I N    M A I N
#***************************************************************

$SIG{'INT'}  = 'handler' ;
$SIG{'QUIT'} = 'handler' ;

&parseCmdLine ;

$tmpStanza = "/tmp/fixStanza.$apar.$$" ;

if ($apar =~ /_ML$/)
{
    $mlCntFile = "$ENV{TOP}/HISTORY/MLcnts" ;
    $rc = 2 ;
    if (!open(MLCNT, "<$mlCntFile"))
    {
	print STDERR 
	    "ERROR: $cmd- could not open ML Count file '$mlCntFile'\n" ;
	print STDERR "ERROR: $cmd- reason = $!\n" ;
    }
    else
    {
	while(<MLCNT>)
	{
	    next if (! /^$apar /) ;
	    
	    chop ;
	    ($junk, $fsCnt) = split(/\s+/) ;
	    $rc = 0 ;
	}
	if ($rc != 0)
	{
	    print STDERR 
       "ERROR: $cmd- Could not find apar '$apar' in count file '$mlCntFile\n" ;
	}
    }
}
elsif (&getFixRecord($apar, $fixDB, $tmpStanza) == 0)
{
    $fsCnt = &countFilesets($tmpStanza) ;
    if ($fsCnt >= 0)
    { 
	$rc = 0 ;
    }
    else
    {
	$rc = 2 ;
    }
}
else # could not get fix record
{
    $rc = 2 ;
}

if (($rc == 0) && ($fsCnt >= 0))
{
    print "$fsCnt\n" ;
}
&cleanup() ;
exit $rc ;
