#!/usr/local/bin/perl
#
#   COMPONENT_NAME: pkgtools
#
#   FUNCTIONS:  openfiles
#		getparms
#		scanfile
#		orderprereqList
#		addoptionimage
#		usage
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
#  Read the command line parameters and verify them
#
#------------------------------------------------------------------------------
&getparms;
 
#------------------------------------------------------------------------------
#
#  open the files for reading and writing
#
#------------------------------------------------------------------------------
&openfiles; 

#------------------------------------------------------------------------------
#
#  for each of the images listed in the stack list, restore the lpp_name 
#  file and strip out the options and their prereqs and coreqs
#
#------------------------------------------------------------------------------
print "restoring lpp_name file for each image file\n\n";
&scanfile;

#------------------------------------------------------------------------------
#
#  close the files so they get flushed
#
#------------------------------------------------------------------------------
close(PREREQS);
close(OPTIONS);

#------------------------------------------------------------------------------
#
#  order the options with prereqs in prereq order
#
#------------------------------------------------------------------------------

#
#  intitialize the ordered list of options (bos.rte always goes first)
#
$orderedprereqs{0} = "bos.rte";
$orderedList{"bos.rte"} = "bos.rte";
$prereq_index = 0;

#
#  put the list of options in prereq order
#
foreach $mainoption (keys %optionprereqs) {
    &orderprereqList ($mainoption);
}

#
#  put the options that have prereqs, but nothing prereqs them
#  into the COREQS file
#
foreach $opt (keys %optionprereqs) {
    if ($orderedList{$opt} eq undef) {
        print COREQS ":$opt\n";
    }
}

# 
#  put the options that have coreqs associated with them into the COREQS file
#
foreach $opt_name (keys %optioncoreqs) {
    print COREQS ":$opt_name\n";
}

#
#  close the file so it gets flushed
#
close(COREQS);

#------------------------------------------------------------------------------
# 
# sort the coreqs file with sort -u to eliminate duplicates
# then, sort the coreqs file with sort -f to ignore case
#
#------------------------------------------------------------------------------
system("/usr/bin/sort -u -t: +1 /tmp/coreqs.$$ | /usr/bin/sort -f -t: -o /tmp/sortedcoreqs.$$ +1");

# 
#  create an associative array out of the sorted coreqs
#
open(SORTED_COREQS,"</tmp/sortedcoreqs.$$") || die "cannot open sorted coreq file for reading:$!";

$coreq_index = -1;

foreach $line (<SORTED_COREQS>) {
    chop $line;
    ($x, $coreq) = split(":", $line);
    $coreq_index = $coreq_index + 1;
    $sortedcoreqs{$coreq_index} = $coreq;
}

#------------------------------------------------------------------------------
# 
# sort the options file with sort -f to ignore case
#
#------------------------------------------------------------------------------
system ("/usr/bin/sort -f -o /tmp/sortedoptions.$$ /tmp/options.$$");

# 
#  create an associative array out of the ordered options that don't have
#  any prereqs or coreqs
#
open(SORTED_OPTIONS,"</tmp/sortedoptions.$$") || die "cannot open sorted options file for reading:$!";

$option_index = -1;

foreach $option (<SORTED_OPTIONS>) {
    chop $option;
    $option_index = $option_index + 1;
    $sortedoptions{$option_index} = $option;
}

#-------------------------------------------------------------------------------
#  
#  For each option in the master file, add it to the master_list associative
#  array
#  
#-------------------------------------------------------------------------------
while ($master_option=<MASTER_FILE>) {
    chop $master_option;
    
    #
    #  ignore comments and blank lines
    #
    next if ($master_option =~ m/ *#/);
    next if (($master_option eq undef) || ($master_option =~ m/^\s*$/));

    # substitute an asterisk for anything in square brackets
    $master_option =~ s/\[.*\]/*/;

    # 
    #  split the option name on the wild card (the master file may contain
    #  an asterisk as a wild card in option names)
    #
    @fld = split(/\*/, $master_option);
  
    #
    #  if the option name contains a wild card, the second field will be set or 
    #  the first field will not equal the option name
    #
    if ((defined $fld[1]) || ($master_option ne $fld[0])) {
        #
        #  search through the list of options to find all matches
        #
        foreach $opt (keys %option_names) {
            #
            #  if the option name matches the wild card
            #
            if (($opt =~ m/^$fld[0](.*)$fld[1]\.(.*$)/) ||
               ($opt =~ m/^$fld[0](.*)$fld[1]$/)) {
                #
                #  Options with 'msg' in their names require special processing.
                #  Only care about msg options that have a-z or J as the first
                #  letter following 'msg.'
                #
                #  if this in not an option with msg in its name, or if
                #  it is an option with msg in its name and the first
                #  letter in the wild card is a-z or J, then add the option
                #  to the list. 
                if ($opt !~ m/(\.msg\.)[A-IK-Z]/) {
                    #
                    #  add the option to the list
                    #
                    $master_list{$opt} = $opt;
                }
                else
                {
                    #
                    #  ignore this option because it is a msg option we
                    #  don't care about
                    #
                    next;
                }
            }
        }
    }
    else  # no wild card - assume there is a .* at the end of the option name
    {
        #
        #  search through the list of options to find all matches
        #
        foreach $opt (keys %option_names) {
            #
            #  if the option name matches the option in the master file or
            #  the option matches the option with .* following the option
            #  name
            #
            if (($opt =~ m/^$master_option$/) ||
                ($opt =~ m/^$master_option\.(.*$)/)) {
                #  
                #  store the option in an associative array
                #
                $master_list{$master_option} = $opt;
            }
        }
    }
    
} # end of while loop

#-------------------------------------------------------------------------------
#  
#  For each option in the master file, add its prereqs and coreqs
#  
#-------------------------------------------------------------------------------
@options = keys %master_list;
$numinlist = $#options;

while ($prevnuminlist != $numinlist) {
    $prevnuminlist = $numinlist;
    foreach $option (keys %master_list) {
        if (defined $optionprereqs{$option}) {
            #
            #  put prereqs in associative array
            #
            foreach $prereq (split (" ",$optionprereqs{$option})) {
                $master_list{$prereq} = $prereq;
            }
        }

        #
        #  if the option has coreqs
        #
        if (defined $optioncoreqs{$option}) {
            #
            #  put the coreqs in the associative array
            #
            foreach $coreq (split(" ", $optioncoreqs{$option}, 9999)) {
                $master_list{$coreq} = $coreq;
            }
        }
    }

    @options = keys %master_list;
    $numinlist = $#options;

}

print "ordering the image files\n\n";
print "errors found during processing are also recorded in stacklist.errors.$$\n\n";


#-------------------------------------------------------------------------------
#  
#  The images associated with the options in the master file are ordered
#  by prereqs, coreqs, then the rest of the options.  This set of images
#  comprise the stack.list.short.  The entire list of images are ordered
#  by the images in the stack.list.short, followed by the images associated 
#  with the remaining prereqs, coreqs, and the rest of the options.
#   
#-------------------------------------------------------------------------------

#  
#  Order the options in the master list.  The images associated with the 
#  options that are prereqed go first, followed by the images associated with 
#  the options that are coreqed, followed by the images associated with the 
#  rest of the options
#  

$image_index = -1;

#
#  always put bos.rte at the top of the list if it is in the list of images
#
if (defined $option_file{"bos.rte"})  {
    $image_index = $image_index + 1;
    $image{$image_index} = $option_file{"bos.rte"};
    $image_file{$option_file{"bos.rte"}} = $option_file{"bos.rte"};
}

# 
#  put prereqs first
#  go through the list of options in prereq order and determine if the option
#  is in the master list.  If the option is in the master list, add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$prereq_index) {
    if (defined $master_list{$orderedprereqs{$indx}}) {
        &addoptionimage($orderedprereqs{$indx});
    }
}

# 
#  put coreqs next 
#  go through the list of sorted coreqs and determine if the option
#  is in the master list. If the option is in the master list, add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$coreq_index) {
    if (defined $master_list{$sortedcoreqs{$indx}}) {
        &addoptionimage($sortedcoreqs{$indx});
    }
}

# 
#  put the rest of the options next
#  go through the list of sorted options and determine if the option
#  is in the master list. If the option is in the master list, add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$option_index) {
    if (defined $master_list{$sortedoptions{$indx}}) {
        &addoptionimage($sortedoptions{$indx});
    }
}

# 
#  output stack list
#
open(STACKLIST,">stack.list");
foreach $x (0..$image_index) {
   print STACKLIST "$image{$x}\n";
}

#
#  if a short stack list was requested
#
if ($shortstack == 1) {
    # 
    #  output short stack list
    #
    close(STACKLIST);
    system("/usr/bin/cp stack.list stack.list.short");
}

# 
#  output the rest of the stack.list 
#
#  save index of where stack.list.short ends
$next_image_index = $image_index + 1;

# 
#  put prereqs first
#  go through the list of options in prereq order and add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$prereq_index) {
    &addoptionimage($orderedprereqs{$indx});
}

# 
#  put coreqs next 
#  go through the list of sorted coreqs and add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$coreq_index) {
    &addoptionimage($sortedcoreqs{$indx});
}

# 
#  put the rest of the options
#  go through the list of sorted options and add the name
#  of the image file the option appears in to the list
#
foreach $indx (0..$option_index) {
    &addoptionimage($sortedoptions{$indx});
}

open(STACKLIST,">>stack.list");
foreach $x ($next_image_index..$image_index) {
   print STACKLIST "$image{$x}\n";
}

#
#  delete unnecessary files from the current directory
#
system("/usr/bin/rm -f /tmp/prereqs.$$ /tmp/coreqs.$$ /tmp/options.$$ /tmp/sortedcoreqs.$$ /tmp/sortedoptions.$$ /tmp/lpp_name.$$");



################################################################################
#
#   SUBROUTINES START HERE
#
################################################################################

#-------------------------------------------------------------------------------
#
#  Subroutine to open the files for reading and writing
#
#-------------------------------------------------------------------------------
sub openfiles {
    open(MASTER_FILE,"<$masterfile") || die "cannot open masterfile $masterfile for reading:$!";

    open(STACK_LIST,"<$stack_list") || die "cannot open stack list file for reading:$!";

    open(PREREQS,">/tmp/prereqs.$$") || die "cannot open prereq file for writing:$!";

    open(COREQS,">/tmp/coreqs.$$") || die "cannot open coreq file for writing:$!";

    open(OPTIONS,">/tmp/options.$$") || die "cannot open options file for writing:$!";

    open(ERRORS,">stacklist.errors.$$") || die "cannot open options file for writing:$!";

} 


#-------------------------------------------------------------------------------
#  
#  Subroutine to read the command line parameters and verify them
#
#-------------------------------------------------------------------------------
sub getparms {
    require "getopts.pl";
    &Getopts ('a:s') || &usage;

    #
    #  if an alternate master file was specified
    #
    if (defined $opt_a) {
        #
        #  use the specified file instead of the defined one
        #
        $masterfile = $opt_a;
    }
    else
    { 
        $masterfile = "$ENV{'ODE_TOOLS'}/usr/lib/masterfile";
    }

    #
    #  if short stack list was requested 
    #
    if ($opt_s == 1) {
        #
        #  set a flag to generate the short stack list
        #
        $shortstack = 1;
    }
    else
    {
        #
        #  set a flag to not generate the short stack list
        #
        $shortstack = 0;
    }
 
    #
    #  if the number of command line parameters does not equal 1 ($#ARGV = 0
    #  if one parameter was specified)
    #
    if ($#ARGV != 0) {
        &usage;
    }
    else
    {
        #
        #  get the stacklist file name
        #
        $stack_list = $ARGV[0];
    }   
}
 
#------------------------------------------------------------------------------
#
#  Subroutine to restore the lpp_name file for each image file 
#  and strip out the options and their prereqs and coreqs and 
#  store the information in associative arrays
#
#------------------------------------------------------------------------------
sub scanfile {
    while ($image_name=<STACK_LIST>)  {
        chop $image_name;
        
        #
        #  ignore blank lines
        #
        next if ($image_name eq undef);

        #  
        #  if the file has length greater than zero
        #
        if (-s $image_name) {
            #
            #  Open the image file and convert the first four bytes of the file
            #  to hex
            #
            open(IMAGEFILE,"<$image_name");
            sysread(IMAGEFILE, $firstword, 4);
            @a =  unpack("H*",$firstword);

            #
            # if the first four bytes of the file do not equal the constant
            #
            if ($a[0] ne "09006bea") {
                 print "file $image_name is not an image\n";
                 print ERRORS "file $image_name is not an image\n";

                 #
                 # get the next image file name
                 #
                 next;
            }
            else
            {
                @fld = split(/\//, $image_name);
                if (!defined $fld[1]) {
                   $image_file_name = $ENV{'PWD'} . "/$image_name";
                }
                else
                {
                   $image_file_name = $image_name;
                }
            
                #
                #  restore the lpp_name file for this image 
                #
                system("cd /tmp; /usr/bin/rm -f lpp_name lpp_name.$$; echo | restore -xqf $image_file_name ./lpp_name > /dev/null; cp lpp_name lpp_name.$$");
            }
        }
        else
        {
            print "image $image_name does not exist or has length of zero\n";
            print ERRORS "image $image_name does not exist or has length of zero\n";
            
            next;
        }

        #
        #  only save the basename of the image
        #
        $image_name =~ s#^.*/(.*)$#$1#;

        #
        #  open the lpp_name (in the current directory) file for reading
        #
        open(LPP_NAME,"</tmp/lpp_name.$$") || die "cannot open lpp_name file associated with image $image_name for reading:$!";

        #
        #  initialize flags
        #
        $pre_co_req_flag = 0;
        $associated = 0;
        undef %listofprereqs;

        #  
        #  read the first line of the file 
        # 
        $line=<LPP_NAME>;

        #
        #  set the option flag so that the first option will be processed
        #
        $option_next = 1;

        #
        #  read each line of the lpp_name file
        #
        while($line=<LPP_NAME>) {
            chop $line;

            #
            #  ignore blank lines
            #
            next if (($line eq undef) || ($line eq ">0 {") || ($line eq "}"));

            #
            #  if option flag is set, then this line contains the option name
            #
            if ($option_next == 1) {
                #
                #  reset the option flag
                # 
                $option_next = 0;

                #  
                #  split the line into fields, option name is the first field
                #
                @fld = split(" ",$line,999);

                # 
                #  save the option name
                #
                $option_name =  $fld[0];
 
                #  
                #  get the next line
                #
                next;
            }

            #
            #  if the prereq/coreq flag is set, then this line may contain
            #  a prereq, coreq, instreq, or % (% designates the end of the list
            #  of prereqs, coreqs, and instreqs)
            #
            if ($pre_co_req_flag == 1) {
                #
                #  split the line into fields(prereq option is the second field)
                #
                @fld = split(" ",$line,999);

                if (($fld[0] eq "*prereq") || ($fld[0] eq "*instreq")) {
                    #
                    #  set a flag to indicate there is a prereq associated with
                    #  this option
                    #
                    $associated = 1;

                    #
                    #  save the file name the option is found in (for later use)
                    #
                    $option_file{$option_name} = $image_name;

                    #  
                    #  write the option name and prereq name to the 
                    #  prereq file
                    #
                    print PREREQS "$option_name:$fld[1]\n";
      
                    # for the option that was prereqed or instreqed, create a 
                    # list of options that prereqed or instreqed it
                    $req_options{$fld[1]} .= " " . $option_name;

                    #  
                    #  put the prereq in the associated array
                    #
                    $optionprereqs{$option_name} .= " " . $fld[1];
                 
                    if (!defined $listofprereqs{$fld[1]}) {
                         $listofprereqs{$fld[1]} = $fld[1];
                         $fileprereqs{$image_name} .= " " . $fld[1];
                    }

                    # 
                    #  read the next line
                    #
                    next;
                }
                else  
                {
                    if ($fld[0] eq "*coreq") {
                        #
                        #  set a flag to indicate there is a coreq associated
                        #  with this option
                        #
                        $associated = 1;

                        #
                        #  save the name of the file the option is found in 
                        #
                        $option_file{$option_name} = $image_name;

                        #  
                        #  put the coreq in the associative array
                        #
                        $optioncoreqs{$option_name} .= " " . $fld[1];

                        #  
                        #  write the option name and coreq name to the 
                        #  coreq file
                        #
                        print COREQS "$option_name:$fld[1]\n";

                        # for the option that was coreqed, create a 
                        # list of options that coreqed it
                        $req_options{$fld[1]} .= " " . $option_name;

                        # 
                        #  read the next line
                        #
                        next;
                     }
                     else
                     {
                         if ($fld[0] eq "%") {
                             #
                             #  check if the option already appeared in another
                             #  image
                             #
                             if ((defined $option_names{$option_name}) && ($image_name ne $option_file{$option_name})) {
                                 print "option $option_name appeared in images $option_file{$option_name} and $image_name\n";
                                 print ERRORS "option $option_name appeared in images $option_file{$option_name} and $image_name\n";
                             }
                             
                             #
                             #  save the name of the file the option is found in
                             #
                             $option_file{$option_name} = $image_name;

                             $option_names{$option_name} = $option_name;

                             #
                             #  if the option doesn't have any prereqs or coreqs
                             #
                             if ($associated == 0) {
                                 #  
                                 #  write the option name to the options file
                                 #
                                 print OPTIONS "$option_name\n";
                             }
                             else
                             {
                                 #
                                 #  reset the flag to indicate there is a prereq
                                 #  or coreq associated with this option
                                 #
                                 $associated = 0;
                             }
 
                             #
                             #  reset the flag
                             # 
                             $pre_co_req_flag = 0;

                             # 
                             #  read the next line
                             #
                             next;
                         } 
                     }
                }
            }

            #
            #  if the line is a [
            #
            if ($line eq "[") {
               #
               #  set a flag to read the prereq or coreq or % on the next line
               #
               $pre_co_req_flag = 1;

               # 
               #  the next line will be the prereqs or coreqs
               #
               next;
            }
            else
            {
                #
                #  if the last character on the line is ]
                # 
                if  ($line eq "]") {
                   #
                   #  set a flag to read the option on the next line
                   #
                   $option_next = 1;

                   #
                   #  the the next line is the option name
                   #
                   next;
                }
                else
                {
                    # 
                    #  read the next line
                    #
                    next;
                } 
            }
        }
    } # end of while loop for images
}

#-------------------------------------------------------------------------------
#
#  recursive subroutine to order options in prereq order
#
#-------------------------------------------------------------------------------
sub orderprereqList {
    #
    # get input parameter
    #
    local($option_name) = @_;

    #
    #  if the option has prereqs
    #
    if (defined $optionprereqs{$option_name}) {
        # go through the list of prereqs
        foreach $prereq (split(" ", $optionprereqs{$option_name})) {
            &orderprereqList ($prereq);
        }
    }
    else # option doesn't have any prereqs
    {
        #
        #  if the option is not in the list
        #
        if (!defined $orderedList{$option_name}) {
            # make sure all the options for the image file the option
            # appears in are in the prereq list
            foreach $req (split(" ",$fileprereqs{$option_file{$option_name}})) {
                if (!defined $orderedList{$req}) {
                    #
                    #  if the option has prereqs
                    #
                    #if (defined $optionprereqs{$req}) {
                    #    &orderprereqList ($req);
                    #}
                    #else
                    #{
                        #
                        #  add the option image file prereqs to the ordered list
                        #
                        $prereq_index = $prereq_index + 1;
                        $orderedprereqs{$prereq_index} = $req;
                        $orderedList{$req} = $req;
                    #}
                }
            }
            #
            #  add the option to the ordered list
            #
            $prereq_index = $prereq_index + 1;
            $orderedprereqs{$prereq_index} = $option_name;
            $orderedList{$option_name} = $option_name;

        }
    }

    #
    #  if the option is not in the list
    #
    if ((!defined $orderedList{$option_name}) && ($option_name ne $mainoption))
    {
        # make sure all the options for the image file the option
        # appears in are in the prereq list
        foreach $preq (split(" ",$fileprereqs{$option_file{$option_name}})) {
            if (!defined $orderedList{$preq}) {
                #
                #  if the option has prereqs
                #
                #if (defined $optionprereqs{$preq}) {
                #    &orderprereqList ($preq);
                #}
                #else
                #{
                    #
                    #  add the option's image file prereqs to the ordered list
                    #
                    $prereq_index = $prereq_index + 1;
                    $orderedprereqs{$prereq_index} = $preq;
                    $orderedList{$preq} = $preq;
                #}
            }
        }

        #
        #  add the option to the ordered list
        #
        $prereq_index = $prereq_index + 1;
        $orderedprereqs{$prereq_index} = $option_name;
        $orderedList{$option_name} = $option_name;
    }
}
 
#-------------------------------------------------------------------------------
# 
#  subroutine which determines the image file for an option and adds it to
#  the stack.list
#
#-------------------------------------------------------------------------------
sub addoptionimage {
    local($option) = @_;
    if (!defined $option_file{$option}) {
        foreach $opt (split(" ",$req_options{$option})) {

            print "option $opt has a requisite on a non-existent option $option\n";
            print ERRORS "option $opt has a requisite on a non-existent option $option\n";
        }
    }
    else
    {
        #
        #  if the image file is not on the list
        #
        if (!defined $image_file{$option_file{$option}}) {

            # debug 
            # print "image file for option $option = $option_file{$option}\n";

            #
            #  put image name in the list
            #
            $image_index = $image_index + 1;
            $image{$image_index} = $option_file{$option};
            $image_file{$image{$image_index}} = $image{$image_index};
         
        }
    }  
}

#-------------------------------------------------------------------------------
#
# usage
#
#-------------------------------------------------------------------------------
sub usage {

    print "Usage: OrderImages [-s] [-a File] File \n";
    print "           where -s will generate stack.list.short\n";
    print "                 -a File will use File instead of the predefined master file\n";
    print "                    File is the file containing the list of images to be ordered\n";
    exit;
}
