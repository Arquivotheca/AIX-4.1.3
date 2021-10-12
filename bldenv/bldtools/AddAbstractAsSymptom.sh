#!/bin/ksh
# @(#) 81 1.1  src/bldenv/bldtools/AddAbstractAsSymptom.sh, bldtools, bos412, GOLDA411a 6/27/94 19:13:37
#
# COMPONENT_NAME: (bldtools) BAI Build Tools
#
# FUNCTIONS:    
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: AddAbstractAsSymptom
#
# FUNCTION:  Given a list of defects, it will add a START_SYMPTOM
#       STOP_SYMPTOM note to the defect which contains the abstract
#       of the defect as the symptom text.  It will not add the
#       note if a START_SYMPTOM string is already in the defect.
#
# INPUT: Filename which is a list of defects to process.
#
# OUTPUT: none
#
# SIDE EFFECTS: Adds symptom strings to CMVC defects.  Implies that
#        CheckSymptoms or ChangeSymptoms needs to run after this
#        command.
#


#
# NAME: Check_Buildable_State
#
# FUNCTION: Add START_SYMPTOM\nabstract\nSTOP_SYMPTOM to the
#        given defect if the defect doesn't have such a string to
#        begin with.
#
# INPUT: Defect to update with symptom text ($1)
#
# OUTPUT: none.
#
# SIDE EFFECTS: Attaches START_SYMPTOM\nabstract\nSTOP_SYMPTOM to the
#        given defect if the defect doesn't have such a string to
#        begin with.
#
# RETURNS: 0
#
function AddSymptomString
{
    defect=$1
    Defect -view $defect -long |
    awk 'BEGIN           { start=0; stop=0; }
        /^abstract/     { abstract=$0}
	/^name/         { name=$2 }
	/START_SYMPTOM/ { start=1 }
	/STOP_SYMPTOM/  { if ( start ) {stop=1; start=0} }
	END             { abstract="START_SYMPTOM\n" abstract \
		                   "\nSTOP_SYMPTOM";
	                  if ( stop==0 ) {
			      rc=system("Defect -note " name \
			             " -rem \"" abstract "\"");
			      if ( rc != 0 ) exit 1; else exit 0;
                          }
			  else exit 1;
	                 }
     '
     if [[ $? = 0 ]]
     then
	 log -b "Symptom string attached to defect, $defect."
     else
	 log -b "Defect, $defect, already contains a symptom string."
     fi
}

. initchanges

if [[ -n "$infile" ]]
then
    cat $infile |
    while read defect
    do
	AddSymptomString $defect
    done
else
    while read defect
    do
	AddSymptomString $defect
    done
fi
