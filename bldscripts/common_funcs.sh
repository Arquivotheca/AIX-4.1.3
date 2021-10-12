#!/bin/sh
# @(#)39        1.5  src/bldscripts/common_funcs.sh, ade_build, bos412, GOLDA411a 6/11/94 19:15:06
#
#   COMPONENT_NAME: bldprocess
#
#   ORIGINS: 27
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#-----------------------------------------------------------------------------
#
# Function to create the list of backing tree directories.  This function
# uses the links contained in the directory above src.
#
backing_tree_links()
{
    #
    # Contiune to traverse until no more links can be found.
    #
    while [ -L ../link ]
    do
        #
        # Try to cd to the link to find out if the link point to a vaild
        # directory.
        #
        cd ../link/src 2>/dev/null

        #
        # Make sure the link is valid and a src directory is present
        # at the location that the link points to.
        #
        [ "$?" = 0 ] || break

        #
        # Get the absolute path of the link.
        #
        echo -n "`/bin/pwd` "
    done
}

#-----------------------------------------------------------------------------
#
# Function to create the srcpath script.  This script is used to find source
# files that may resides elsewhere then locally.  This script is by other
# scripts (i.e. setup.sh) that need to determine the source path for files.
# Since this script is the starting point script, it is essential that
# finding the script and building it be easy.  If the script were to reside
# in the source tree (as srcpath.sh) and the source tree was being accessed
# through the backing tree link, then it would become a problem to find the
# script since we can't use the script to find the script.  This is the
# reason why the script is produced here.
#
# The script uses the environment variable SOURCEDIR and SOURCEBASE.
# SOURCEDIR conatins all the paths to the backing trees whereas SOURCEBASE
# conatins the path to the local source tree.
#
srcpath_script()
{
    cat <<END_SRCPATH_SCRIPT
    #
    # This script was produced by the $0 script.
    #

    #
    # Search where source file resides.
    #
    echo \$1 |
    awk '
    {
        #
        # Retrieve the value of SOURCEDIR and SOURCEBASE from the system
        # environment.
        #
        sourcedir    = ENVIRON["SOURCEDIR"]
        sourcebase   = ENVIRON["SOURCEBASE"]
        source_paths = sourcebase

        if (sourcedir != "")
        {
            #
            # Use SOURCEDIR if it has been set to a value.  SOURCEDIR
            # points to backing trees.
            #
            source_paths = source_paths":"sourcedir
        }

        #
        # Spilt the search paths into separate components.
        #
        npath = split(source_paths, source_dir, ":")

        #
        # The name of the file to perform the search on.
        #
        file = \$1

        #
        # Search each source path component for the file.
        #
        for (i = next_path; next_path <= npath; next_path++)
        {
            #
            # Piece the next search path together using the next search
            # path component prepended to the file name.
            #
            search_dir = source_dir[next_path]
            found_path = search_dir"/"file

            #
            # Has the file been found?
            #
            if (! system("test -r " found_path))
            {
                #
                # File has been found so return the full path for the
                # file.
                #
                print found_path
                break
            }
        }
    }'
END_SRCPATH_SCRIPT
}

#-----------------------------------------------------------------------------
#
# Function to read the value of a symbol from the tool symbol file.  The
# symbol value can span more than one line using the continuation character
# (\).  It is assumed that the symbol being read follows standard OSF
# conventions.  The symbol read is always exported to the current shell
# environment.
#
get_tool_symbol()
{
    #
    # Define where to find the file that contains the tool symbols.
    #
    [ -n "$symbol_file" ] || symbol_file=`srcpath bldenv/mk/osf.aix.tools.mk`

    #
    # Retrieve the current set of invocation parameters.  This will be
    # used to reset the environment back to a known state.
    #
    current_sets=`echo $- | sed -e "s|.|-& |g"`

    #
    # Turn off debugging.  This will eliminate a great deal of output.
    #
    set +x

    symbol=$1              # The name of the symbol to retrieve.

    sep="-_-"              # Seperator used to strip the lead equal (=).

    #
    # Read the symbol and its value.
    #
    symbol_value=`
    awk '
    {
        #
        # Search for the symbol using the following criteria:
        #
        #    1) Symbol name must start at the beginning of the line.
        #    2) Symbol name can be followed by either =, ?=, <space> or
        #       <tab>.  <space> represents a blank space and <tab> is
        #       a hortizontal tab.
        #
        if (match($0, "^'"$symbol"'=") || match($0, "^'"$symbol"'\\\\?=") ||
            match($0, "^'"$symbol"' ") || match($0, "^'"$symbol"'	"))
        {
            #
            # Keep reading more lines for the found symbol until it is
            # determined that all of the symbol value has been read.
            #
            do
            {
                #
                # Determine if the symbol value spans more than one line.
                # If the symbol value is continued, strip the continuation
                # (\) from the end of the line.
                #
                symbol_line = $0
                done        = (! sub(/\\\\$/, "", symbol_line))

                #
                # Replace:
                #
                #    1) " (quote) with \" (blackslash-quote).  This is
                #       done so that when the symbol value is expanded
                #       the quote will not disappear.
                #    2) ODE_TOOLS with BLDENV_TOOLS.  ODE_TOOLS will only
                #       be defined outside the build environment.
                #    3) :U with :-.  :U is the equivalent to :- for the
                #       shell.
                #
                gsub(/"/, "\\\\\"", symbol_line)
                gsub(/ODE_TOOLS/, "BLDENV_TOOLS", symbol_line)
                gsub(/:U/, ":-", symbol_line)

                #
                # Print the parsed symbol line.
                #
                print symbol_line
            }
            while ((! done) && (getline > 0))
        }
    #
    # Strip the leading symbol assignment.  This is done because
    # OSF uses ?=, +=, :=, and != which may not be compatiable
    # with the shell.
    #
    }' $symbol_file | sed -e "s|=|${sep}|" -e "s|^.*${sep}||"`

    #
    # Was a symbol found?  If one was found, export the symbol value so
    # that the next tool can use it.
    #
    if [ -n "$symbol_value" ]
    then
        #
        # Restore the invocation parameters to the state before debug was
        # turned off.
        #
        [ -z "$current_sets" ] || set $current_sets

        #
        # Export the symbol and its value.
        #
        eval $symbol=\"$symbol_value\"; export $symbol
    else
        #
        # Restore the invocation parameters to the state before debug was
        # turned off.
        #
        [ -z "$current_sets" ] || set $current_sets
    fi
}

#-----------------------------------------------------------------------------
#
# Function to build and install a set of sub-directories defined by the
# variable SUBDIRS.  When a tools has been installed, the symbol value for
# the tool is read from the tool symbol file (symbol_file) and exported
# so that the next tool can use the tool just built.
#
walk_subdirs()
{
    #
    # Walk each directory specified.
    #
    for next_dir in $SUBDIRS
    do
        #
        # Make sure that there is a directory to build under.  If the source
        # for this directory is accessed through the backing tree link,
        # then the directory to build under may not exist, so create it if
        # it does not.
        #
        if [ ! -d $next_dir ]
        then
            #
            # Note that the build directory was created.  The directory
            # will be removed after the building has finished.
            #
            dir_created="yes"

            #
            # Create the directory.
            #
            mkdir -p $next_dir
        fi

        #
        # Build the next directory.
        #
        for next_pass in $*
        do
            (cd $next_dir; make -r ${next_pass}_all)
            status=$?

            [ "$status" = 0 ] || break

            if [ "$next_pass" = "install" ]
            then
                #
                # Note that the install has completed.  This will trigger
                # the exporting of the tool symbol.
                #
                install_performed=yes
            fi
        done

        #
        # Removed the directory that was previously created.  This will
        # only occur when the source for this directory is accessed
        # through the backing tree link.
        #
        if [ -n "$dir_created" ]
        then
            #
            # Reset the indication that the directory was previously created.
            #
            unset dir_created

            #
            # Remove the previously created directory.
            #
            rm -rf $next_dir
        fi

        #
        # Did the directory successfully build.  If not, exit and set an
        # invalid status.
        #
        [ "$status" = 0 ] || return $status

        if [ -n "$install_performed" ]
        then
            unset install_performed

            #
            # Find out if the tool symbol to export needs another symbol
            # exported before the tool symbol is exported.  Generally
            # this is the case when a tool has a support file
            # (i.e. BRAND has BRANDDICT).
            #
            tool=`basename $next_dir`
            tool=`echo $tool | tr '[a-z]' '[A-Z]'`
            other_symbol=`eval echo $\`echo ${tool}_OTHER_SYMBOL\``

            if [ -n "$other_symbol" ]
            then
                #
                # Read and export the support symbol for this tool.
                #
                get_tool_symbol $other_symbol
            fi

            #
            # Find out if the tool symbol to export needs a second
            # symbol exported as in the case of LD
            #
            other_symbol2=`eval echo $\`echo ${tool}_OTHER_SYMBOL2\``

            if [ -n "$other_symbol2" ]
            then
                #
                # Read and export the support symbol for this tool.
                #
                get_tool_symbol $other_symbol2
            fi

            #
            # Read and export the tool symbol.
            #
            get_tool_symbol $tool
        fi
    done

    return 0
}

exit_function()
{
	/usr/bin/echo "$lpplabel ended at `/usr/bin/date`"
	exit 1
}

conv_bld_cycle ()
{
#----------------------------------
#
# This function will translate the BLDCYCLE variable into
# the lpp level which will be stored as part of each
# lpp.  The translation is:
#
#   9415B            translates to              4151
#
#     where:                                  where:
#         94 : last two digits of the year       4: last digit of the year
#         15 : # week in the year               15: # week in the year
#          B : 2nd build of the week             2: 2nd build of week2
#
#----------------------------------

build_cycle=$1
index1=0
char_found=1

#   Get the alpha char of build (last char in BLDCYCLE)
char_value=`echo $build_cycle | /usr/bin/cut -c5`

case ${char_value} in
      "A") index1=1;;
      "B") index1=2;;
      "C") index1=3;;
      "D") index1=4;;
      "E") index1=5;;
      "F") index1=6;;
      "G") index1=7;;
      "H") index1=8;;
      "I") index1=9;;
esac

#   Get 2nd digit of year and two digits that are week from BLDCYCLE
#   append number conversion of alpha.
#   assign to LPPTESTLEVEL
return_val=`echo $build_cycle | /usr/bin/cut -c2-4`
return_val=${return_val}${index1}
eval LPPTESTLEVEL=\"$return_val\"; export LPPTESTLEVEL
}

#-----------------------------------------------------------------------------
#
#  Copy the locales into the build environment prior to building compilers.
#
#-----------------------------------------------------------------------------
bootlocales()
{
#
# Create the directory for the tools
#
if [ ! -d ${BLDENV_TOOLS}${BLDENV_LOCPATH} ]
then
    mkdir -p ${BLDENV_TOOLS}${BLDENV_LOCPATH}
fi

#
# Copy the files from the source tree into the
# ode_tools tree.  Set the link as each locale
# is copied.
#

save_path=`pwd`
cd `srcpath bldenv/locales/${LOCALES_BUILD_DIR}`
for x in *
  do
    if [ $x != "Makefile" ]
    then
      /usr/bin/cp $x ${BLDENV_TOOLS}${BLDENV_LOCPATH}
      link_val=`echo $x | /usr/bin/cut -f1 -d\.`
      save_path1=`pwd`
      cd ${BLDENV_TOOLS}${BLDENV_LOCPATH}
      /usr/bin/ln -s $x $link_val
      cd $save_path1

    fi
  done
cd $save_path

#
# Set the correct permissions for the locales
#
/usr/bin/chmod 755 ${BLDENV_TOOLS}${BLDENV_LOCPATH}
}

