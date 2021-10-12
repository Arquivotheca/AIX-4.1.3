#
# @(#)26	1.8  src/bldenv/pkgtools/subptf/bldoptlib.pl, pkgtools, bos412, GOLDA411a 1/14/94 13:14:51
# COMPONENT_NAME: (BLDTOOLS) BAI Build Tools
#
# FUNCTIONS:    OptimizeLibDeps
# 		GetLibraryData
# 		SaveLibraryData
# 		GetCommandsLibs
# 		GetLibraryObjects
# 		GetLinkObjects
# 		ReplaceTDlinks
# 		AddDepTarget
# 		UniqNewTargets
# 		LogLibData
#               EncodeArray
#               DecodeArray
# 		IsShipFile
#		IsObjectFile
#		IsSharedObject
#		IsLibrary
#		IsIgnoreFile
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# NAME: OptimizeLibDeps
#
# FUNCTION: Replace command-library relationships (target and dependency) 
#           with command-object relationships.
#
#	    The target-dependency relationships indirectly have commands
#           dependent on all object modules within a library.  For efficiency,
#           commands should only be dependent on those object modules within
#           a library which they are actually linked with.  This function
#           makes these target-dependency modifications as illustrated below
#           (assume a.o was the only object actually linked with):
#
#                                   command
#                                      |_____________
#                                      |             |
#                                   lib.a           ...
#                                      |
#                         _____________|_______________________
#                        |             |             |         |
#                       a.o           b.o           c.o      interm
#                        |             |             |         |
#                   _____|_____       ...           ...      auxdep1
#                  |           |
#                 a.c        auxdep2
#
#                                      || modified to
#                                      \/
#
#                                   command
#                                      |_____________
#                                      |             |
#                                      |            ...
#                         _____________|_______________________
#                        |                                     |
#                       a.o                                    |
#                        |                                     |
#                   _____|_____                              auxdep1
#                  |           |
#                 a.c        auxdep2
#
#
# INPUT: release (parameter) - release name
#        targets (parameter) - array mapping new dependents to their targets
#        depends (parameter) - array mapping new targets to their dependents
#        alldepends (parameter) - array mapping all targets to their dependents
#
# OUTPUT: targets (parameter) - updated array mapping new dependents to targets
#         depends (parameter) - updated array mapping new targets to dependents 
#         alldepends (parameter) - updated array mapping all targets to 
#             dependents 
#         envlist (parameter) - a list of files bldtd data must be kept on
#         envlistplus (parameter) - the envlist plus ALL objects for each lib
#             in the list
#
# SIDE EFFECTS: internal files are updated with library information (see
#               SaveLibraryData)
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTES: 
#  -  the target and dependency input arrays contain only NEW makelist entries
#
#  -  All path names (i.e. targets, dependencies, commands, libraries) have to
#     be encoded to numeric values in order to save memory space
#
#  -  Main Variables
#        cmd -  command
#        lib - library
#        objsbybase - maps library and object base name to full object name
#        objects - maps library to all object files within the library
#        extradeps - maps library to its non-objects dependencies (only leafs)
#        cmdlibs - maps command to its library dependencies 
#        ldobjects - maps command to the libraries actually linked with
#        auxdeps - temp/modifiable copy of ldobjects 
#        targetsToUnique - indexes to targets which have been added to
#        nulllist - a null array
#
# RETURNS: nothing
#
sub OptimizeLibDeps {
     local($release,*targets,*depends,*alldepends,*envlist,*envlistplus) = @_;
     local(%objsbybase,%objects,%allobjects,%extradeps,$cmd,$lib,%cmdlibs);
     local(@auxdeps,$dep,%targetsToUnique,%null,@nulllist,$newaux);
     local(%mapcmds,$ldo,$libo,$dep);

     ## gather all command/library data contained in dependencies relationships
     &GetCommandsLibs (*alldepends,*cmdlibs);  ## only need new commands
     &GetLibraryData ("new",*depends,*null,*objects,*extradeps,*thisrel); 
         (%null)=();
     &GetLibraryData ("all",*alldepends,*objsbybase,*allobjects,*allextradeps,
         *thisrel);
     &GetLinkData (*linkobjs,*mapcmds);

     foreach $cmd (keys %cmdlibs) {  ## for each cmd which links with lib
          next if ($depends[$cmd] eq undef);  ## skip command if it's not NEW
          foreach $lib (split(/$SEP/,$cmdlibs{$cmd})) {  ## for each lib linked
               ## get libraries the command actually linked with
               if(&GetLinkObjects($cmd,$lib,*linkobjs,*mapcmds,
                                       *objsbybase,*ldobjects)){
                    ## modify target-dependency relationships for cmds' 
                    ## library; basically, modify targets/depends to reflect 
                    ## relationships only with libraries actually linked with

                    ## first do the new data
                    (@newldobjects)=();
                    foreach $ldo (@ldobjects) {
                         foreach $libo (split(/$SEP/,$objects{$lib})) {
                              if ($ldo eq $libo) {
                                   push(@newldobjects,($ldo));
                    }    }    }
                    (@newaux)=(split(/$SEP/,$extradeps{$lib}),@newldobjects); 
                    &ReplaceTDLinks(*targets,$lib,$cmd,*nulllist);
                    &ReplaceTDLinks(*depends,$cmd,$lib,*newaux);
                    foreach $dep (@newaux) {
                         &AddDepTarget(*targetsToUnique,$cmd,$dep,*targets);
                    }

                    ## now do the "all" data
                    (@auxdeps)=(split(/$SEP/,$allextradeps{$lib}),@ldobjects); 
                    &ReplaceTDLinks(*alldepends,$cmd,$lib,*auxdeps);

                    ## may need to save object data for ref from other releases
                    ## - it's easier to do this for all new libs than to figure
                    ## out which ones will go into the build environment
                    %envlistplus=(%envlist);
                    if (defined $thisrel{$lib}) {
                         foreach $dep (split(/$SEP/,$objects{$lib})) {
                              $envlist{&Decodepath($dep)} = $DEFINED;
                         }
                         foreach $dep (split(/$SEP/,$allobjects{$lib})) {
                              $envlistplus{&Decodepath($dep)} = $DEFINED;
                         }
		    }

                    ## keep a record of the library substitutions
                    &LogLibData($release,$cmd,$lib,*newaux,*auxdeps);
               }
          }
     }
     &UniqNewTargets(*targetsToUnique,*targets);  ## remove duplicates 
     &SaveLibraryData("new",*null,*objects,*extradeps);
     &SaveLibraryData("all",*objsbybase,*allobjects,*allextradeps);
}

#
# NAME: GetLibraryData
#
# FUNCTION: Returns all available library data from dependencies.  The current
#           release's data is combined with the accumulation of previous 
#           releases' data.
#
# INPUT:  type (parameter) - indicated if "all" or "new" data requested
#         depends (parameter) - the dependencies for every target file
#
# OUTPUT: objsbybase (parameter) - object file referenced by library and 
#             base name of object file
#         objects (parameter) - objects referenced by library
#         extradeps (parameter) - non-object dependents for a library (only
#             leaf nodes)s
#         thisrel (parameter) - defines lib data coming from this release
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTE: If a fatal error occurs, the command will direct exit.
#
# RETURNS: nothing
#
sub GetLibraryData {
     local($type,*depends,*objsbybase,*objects,*extradeps,*thisrel)=@_;
     local($historypath,$globalpath,$datapath,$lib,%libobjectsdone);
     local(@moreobjs,@moredeps,$d_obj);

     if ($type eq "all") {
     	chop($historypath=`ksh bldhistorypath`);
        $datapath="$historypath/all";
     }
     elsif ($type eq "new") {
     	chop($globalpath=`ksh bldglobalpath`);
        $datapath="$globalpath/new";
     }
     else {
	&log("-x","illegal GetLibraryData input type");
     }

     ## get the library data accumulated from other releases
     &Getdata($datapath . "libobjsbybase",*objsbybase) ||
          &log("$lvar -x","unable to get objsbybase data");
     &Getdata($datapath . "libobjects",*objects) ||
          &log("$lvar -x","unable to get objects data");
     &Getdata($datapath . "libextradeps",*extradeps) ||
          &log("$lvar -x","unable to get extradeps data");

     # the inter-release library data must be stored in decoded form to 
     # insure naming consistency across releases and build cycles (e.g.
     # this week's code 37 will not be the same as next week's code 37).
     &EncodeArray(*objsbybase);
     &EncodeArray(*objects);
     &EncodeArray(*extradeps);

     ## get library data from current release and add to accumulated data
     foreach $lib (1..$#depends) {  ## depends encodings start at 1
          next if (! &IsLibrary($lib));  ## skip if not a library
          if (! $libobjectsdone{$lib}) {  ## if lib not already processed
               &GetLibraryObjects ($lib,*depends,*objects,*extradeps,
                                                 *moredeps,*moreobjs);
               if ($#moreobjs > -1 || $#moredeps > -1) {  ## if data returned 
                    ## save all the lib data for easy access when needed;
                    ## this data is accumulated for each release processed
                    $objects{$lib}=join("$SEP",@moreobjs);
                    $objects{$lib}=&Uniquesort($SEP,$objects{$lib});
                    $extradeps{$lib}=join("$SEP",@moredeps);
                    $extradeps{$lib}=&Uniquesort($SEP,$extradeps{$lib});
                    foreach $obj (@moreobjs) {
                         $d_obj=&Decodepath($obj);
                         $objsbybase{$lib . $SEP . &Basename($d_obj)}=
                              $obj . $SEP;  ## add SEP to match data from file
                    }
                    $thisrel{$lib}=DEFINED;
               }
               $libobjectsdone{$lib}=$TRUE;  ## mark the lib as processed
     }    }
}

#
# NAME: SaveLibraryData
#
# FUNCTION: Save library data in files.
#
# INPUT : objsbybase (parameter) - object file referenced by library and base 
#             name of object file
#         objects (parameter) - objects referenced by library
#         extradeps (parameter) - non-object dependents for a library (only
#             leaf nodes)
#
# OUTPUT: none
#
# SIDE EFFECTS: library data is saved in files
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTES: Mapped associative arrays could not be used to store this data
#        because the 1K record size limitation is exceeded.
#
#        If a fatal error occurs, the command will directly exit.
#
# RETURNS: nothing
#
sub SaveLibraryData {
     local($type,*objsbybase,*objects,*extradeps) = @_;
     local($historypath,$globalpath,$datapath);
     
     if ($type eq "all") {
     	chop($historypath=`ksh bldhistorypath`);
        $datapath="$historypath/all";
     }
     elsif ($type eq "new") {
     	chop($globalpath=`ksh bldglobalpath`);
        $datapath="$globalpath/new";
     }
     else {
	&log("-x","illegal SaveLibraryData input type");
     }

     # the inter-release library data must be stored in decoded form to 
     # insure naming consistency across releases and build cycles (e.g.
     # this week's code 37 will not be the same as next week's code 37).
     &DecodeArray(*objsbybase);
     &DecodeArray(*objects);
     &DecodeArray(*extradeps);

     &Savedata($datapath . "libobjsbybase",*objsbybase) ||
          &log("$lvar -x","unable to save objsbybase");
     &Savedata($datapath . "libobjects",*objects) ||
          &log("$lvar -x","unable to save objects");
     &Savedata($datapath . "libextradeps",*extradeps) ||
          &log("$lvar -x","unable to save extradeps");
}

#
# NAME: GetCommandsLibs
#
# FUNCTION: Return list of all commands (which depend on a library) and the
#           libraries each command depends on.
#
# INPUT: depends (parameter) - the dependencies for every target file
#
# OUTPUT: cmdlibs (parameter) - new commands with associated libraries
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTES: There is not a way to definitely determine which targets are commands
#        and which are not.  To get a few extras is okay; to miss a few
#        is not okay.  What really needs to be identified is every dependency
#        on a library which might represent a link with the library.
#
# RETURNS: nothing
#
sub GetCommandsLibs {
     local(*depends,*cmdlibs) = @_;
     local($target,$depend);
     foreach $target (1..$#depends) {
          next if (&IsLibrary($target));
          next if (&IsObjectFile($target));
          next if (&IsIgnoreFile($target));
          foreach $depend (split(/$SEP/,$depends[$target])) {
               if (&IsLibrary($depend)) {
                    next if (&IsShipFile($depend));
                    $cmdlibs{$target} .= $depend . $SEP;
     }    }    }
}

#
# NAME: GetLinkData
#
# FUNCTION: Return the link data generated when the commands are linked
#           with the libraries.  This data indirectly comes from the .m
#           files.
#
# INPUT: none
#
# OUTPUT: linkobjs - objects each command binds with, indexed by cmd and lib
#         mapcmds - the command for which link data was found
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTE: If a fatal error occurs, the command will directly exit.
#
# RETURNS: nothing
#
sub GetLinkData {
     local(*linkobjs,*mapcmds) = @_;
     local($linkdir,$linkline,$cmd,$lib,$objects,$e_cmd,$e_lib);
     chop ($linkdir=`ksh bldglobalpath`);
     open(LINKDATA,"<$linkdir/linkdata") ||
          &log("$lvar -x","unable to open link file $linkdir/linkdata ($!)");
     while ($linkline=<LINKDATA>) {
          chop $linkline;
          ($cmd,$lib,$objects)=split(/\|/,$linkline);
          $e_cmd=&Encodepath($cmd); $e_lib=&Encodepath($lib);
          $linkobjs{$e_cmd,$e_lib}=$objects;
          $mapcmds{$e_cmd}=$DEFINED;
     }
}

#
# NAME: GetLibraryObjects
#
# FUNCTION: Return all of a library's object files and leaf dependencies.
#           The data returned may cross release boundaries.  Leaf
#           dependencies are children in the library's dependency tree
#           which are not libraries themselves, not object files, and
#           have no dependents (thus called leafs).
#
# INPUT: lib (parameter) - a library 
#        depends (parameter) - the dependencies for every target file
#        objects (parameter) - all library objects thus far gathered from 
#            other releases
#        extradeps (parameter) - all non-lib/obj leafs from other releases
#            (ref'd by lib)
#
# OUTPUT: moredeps (parameter) - all library objects gathered for lib
#         moreobjs (parameter) - all non-lib/obj leafs gathered for lib
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTE: If a fatal error occurs, this command will directly exit.
#       
# RETURNS: nothing
#
sub GetLibraryObjects {
     local($lib,*depends,*objects,*extradeps,*moredeps,*moreobjs) = @_;
     local($item,@stack,$loopcount,$tdep,@dependents);

     push(@stack,($lib));
     (@moreobjs)=(); (@moredeps)=();
     while (($tdep=pop(@stack)) ne undef) {  ## walk all children nodes of lib
          @dependents=(split(/$SEP/,$depends[$tdep]));## current nodes' children
          foreach $item (@dependents) {  
               if (&IsObjectFile($item)) 
                    { push(@moreobjs,($item)); }  ## save lib's object files
               else { 
                    if ($depends[$item] ne undef)  ## if not leaf node 
                         { push(@stack,($item)); }  ## put in to-be-done stack
                    else {  ## leaf node
                         if (&IsLibrary($item)) {
			      ## put any lib data on the stack which comes from
                              ## another release (this occurs when a lib is
                              ## referenced locally but built in another release
                              push(@moreobjs,split("$SEP",$objects{$item}));
                              push(@moredeps,split("$SEP",$extradeps{$item}));
                         }
                         else { 
                              ## save the non-library/non-object leaf node 
                              push(@moredeps,($item)); 
          }    }    }    }
          if (++$loopcount > $MAXITERATIONS) {  ## if much more than should be
               &log ("$lvar -x","probably circular dependency ($lib)");
     }  }
}

#
# NAME: GetLinkObjects
#
# FUNCTION: Return the object modules from a library which were actually
#           linked into a specified command.
#
# INPUT: cmd (parameter) - command
#        lib (parameter) - library
#        linkobjs (parameter) - link data from map file
#        mapcmds (parameter) - commands for which map data was found
#        objsbybase (parameter) - object files referenced by lib and object
#            base name
#
# OUTPUT: ldobjects (parameter) - object files the library actually linked with
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (success) or !0 (failure)
#
sub GetLinkObjects {
     local($cmd,$lib,*linkobjs,*mapcmds,*objsbybase,*ldobjects) = @_;
     local($linksfound,$bname,$d_lib);
     (@ldobjects)=();
     $linksfound=$FALSE;
     if (defined $mapcmds{$cmd}) {
          $linksfound=$TRUE;
          foreach $bname (split(/$SEP/,$linkobjs{$cmd,$lib})) {
               if ($objsbybase{$lib . $SEP . $bname} ne undef) {
                    push(@ldobjects,($objsbybase{$lib . $SEP . $bname}));
                    chop $ldobjects[$#ldobjects];  ## remove SEP
               }
               else {
                    $d_lib=&Decodepath($lib);
                    &log("$lvar -w","base name not found for $bname in $d_lib");
                    $linksfound=$FALSE;
                    last;
               }
          }
     }
     return $linksfound;
}

#
# NAME: ReplaceTDlinks
#
# FUNCTION: Replace a set of target-dependency relationships (links) with a 
#           new set of relationships.
#
# INPUT: TD (parameter) - associative array holding the target-dependency 
#            relationships
#        key (parameter) - the assoc. array key whose values are to be replaced
#        oldvalue (parameter) - the value to be replace
#        replacementitems (parameter) - the new items (replaces oldvalue)
#
# OUTPUT: TD (parameter) - updated target-dependency relationships
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub ReplaceTDLinks {
     local(*TD,$key,$oldvalue,*replacementitems) = @_;
     local($item,@newitems);
     foreach $item (split(/$SEP/,$TD[$key])) {
          if ($item eq $oldvalue) {  
               push(@newitems,@replacementitems);
          }
          else {
               push(@newitems,($item));
          }
     }
     $TD[$key] = &Uniquesort($SEP,join($SEP,@newitems)) . "$SEP";
}

#
# NAME: AddDepTarget
#
# FUNCTION: Add a command to the target list of a specified dependent
#
# INPUT: targetsToUnique (parameter) - list of targets which should later be 
#            uniqued
#        cmd (parameter) - new target value to be added
#        dep (parameter) - dep for which to add target
#        targets (parameter) - associative array of dependencies-targets
#
# OUTPUT: targetsToUnique (parameter) - updated list of targets which need to 
#             be uniqued
#         targets (parameter) - updated dependencies-targets array
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub AddDepTarget {
     local(*targetsToUnique,$cmd,$dep,*targets) = @_;
     $targets[$dep] = $targets[$dep] . $cmd . $SEP;
     $targetsToUnique[$dep]=$DEFINED;
}

#
# NAME: UniqNewTargets
#
# FUNCTION: Unique the target values for a set of dependents.
#
# INPUT: targetsToUnique (parameter) - list of dependents whose targets are 
#            to be uniqued
#        targets (parameter) - associative array of targets
#
# OUTPUT: targets (parameter) - updated associated array of targets
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTES: This command is used for an optimization.  It takes much less
#        time to uniq the dependents once at the end instead of each time
#        a modification is made.
#
# RETURNS: nothing
#
sub UniqNewTargets {
     local(*targetsToUnique,*targets) = @_;
     local($dep1);
     foreach $dep1 (@targetsToUnique) {
          next if ($targets[$dep1] != $DEFINED);
          $targets[$dep1] = &Uniquesort($SEP,$targets[$dep1]) . $SEP;
     }
}

#
# NAME: LogLibData
#
# FUNCTION: Log modifications to library dependencies
#
# INPUT: cmd (parameter) - command
#        lib (parameter) - library
#        newobjects (parameter) - the new dependency relationships
#        allobjects (parameter) - all (but pruned) the dependency relationships
#
# OUTPUT: a message is "logged"
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: nothing
#
sub LogLibData {
     local($release,$cmd,$lib,*newobjects,*allobjects) = @_;
     local($releasepath,$obj,$d_cmd,$d_lib,$d_obj);
     if ($loginit ne "DONE") {
          $loginit="DONE";
          chop($releasepath=`ksh bldreleasepath $release`);
          open (LOGFILE,">$releasepath/libopts") ||
                &log("$lvar -x","$releasepath/libopts open error ($!)");
          &log("$lvar -b","optimizations recorded in $releasepath/libopts");
     }
     $d_cmd=&Decodepath($cmd); $d_lib=&Decodepath($lib);
     print LOGFILE "COMMAND=$d_cmd\n";
     print LOGFILE "\tLIBRARY=$d_lib\n";
     print LOGFILE "\tNEW REPLACEMENTS:\n";
     foreach $obj (@newobjects) {
          $d_obj=&Decodepath($obj);
          print LOGFILE "\t\t$d_obj\n";
     }
     print LOGFILE "\tALL REPLACEMENTS:\n";
     foreach $obj (@allobjects) {
          $d_obj=&Decodepath($obj);
          print LOGFILE "\t\t$d_obj\n";
     }
}

#
# NAME: EncodeArray
#
# FUNCTION: Encode an array of values (but not object base name in key)
#
# INPUT: dataarray (parameter) - the array of values
#
# OUTPUT: dataarray (parameter) - the encoded array of values
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTE: This function is not general purpose; it is customized to encode
#       %objects, %extradeps, and %objsbybase.
#
# RETURNS: nothing
#
sub EncodeArray {
     local(*dataarray) = @_;
     local($newlist,$value,$lib,$baseobject,$newkey);

     foreach $key (keys %dataarray) {
          $newlist=undef;
          foreach $value (split(/$SEP/,$dataarray{$key})) {
               $newlist .= &Encodepath($value) . $SEP;
          }
          delete $dataarray{$key};
          if ($key !~ /$SEP/) {  ## if SEP then object base in second field
              $newkey=&Encodepath($key); 
          }
          else {
              ($lib,$baseobject) = split(/$SEP/,$key);
              $newkey = &Encodepath($lib) . $SEP . $baseobject;
          }
          $dataarray{$newkey}=$newlist;
     }
}

#
#
# NAME: DecodeArray
#
# FUNCTION: Decode an array of values (by not object base name in key)
#
# INPUT: dataarray (parameter) - the array of values
#
# OUTPUT: dataarray (parameter) - the decoded array of values
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# NOTE: This function is not general purpose; it is customized to decode
#       %objects, %extradeps, and %objsbybase.
#
# RETURNS: nothing
#
sub DecodeArray {
     local(*dataarray) = @_;
     local($newlist,$value,$lib,$baseobject,$newkey);

     foreach $key (keys %dataarray) {
          $newlist=undef;
          foreach $value (split(/$SEP/,$dataarray{$key})) {
               $newlist .= &Decodepath($value) . $SEP;
          }
          delete $dataarray{$key};
          if ($key !~ /$SEP/) {  ## if SEP then object base in second field
              $newkey=&Decodepath($key); 
          }
          else {
              ($lib,$baseobject) = split(/$SEP/,$key);
              $newkey = &Decodepath($lib) . $SEP . $baseobject;
          }
          $dataarray{$newkey}=$newlist;
     }
}

#
# NAME: IsShipFile, IsObjectFile, IsSharedObject, IsLibrary, IsIgnoreFile
#
# FUNCTION: 
#       IsShipFile - returns true if input file is a ship file
#       IsObjectFile - returns true if input file is an object module
#       IsSharedObject - returns true if input file is a shared object module
#       IsLibrary - returns true if the input file is a library
#       IsIgnoreFile - returns true if the input file should be ignored 
#
# INPUT: file (parameter) - the input file name
#
# OUTPUT: none
#
# SIDE EFFECTS: none
#
# EXECUTION ENVIRONMENT: the build process environment
#
# RETURNS: 0 (success/meets criterion) or !0 (failure)
#
sub IsShipFile {
     local($file)=&Decodepath(@_);
     return ($file =~ m#/ship/#);
}
sub IsObjectFile {
     local($file)=&Decodepath(@_);
     return ($file =~ /\.o$/);
}
sub IsLibrary {
     local($file)=@_;
     local($datafile,$lib);
     if ($initialization ne "DONE") {
          $initialization="DONE";
          $datafile="$ENV{'LIBNAMES'}";
          open (LIBNAMES,"<$datafile") || 
                &log("$lvar -x","Cannot find/open LIBNAMES file : $datafile ($!)");
          while ($lib=<LIBNAMES>) {
                chop $lib;
                $libnames{$lib}=$DEFINED;
          }
          close (LIBNAMES);
     }
     $file=&Decodepath($file);
     return ($file =~ /\.a$/ || $libnames{$file} ne undef);
}
sub IsIgnoreFile {
     local($file)=&Decodepath(@_);
     return ($file =~ m#/lpp_name$# ||
             $file =~ m#/lpp_name$# ||
             $file =~ m#/lpp_name$# 
            );
}
