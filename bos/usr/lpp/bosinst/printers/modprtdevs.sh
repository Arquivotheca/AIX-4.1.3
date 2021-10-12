#!/usr/bin/ksh
# @(#)59        1.2  src/bos/usr/lpp/bosinst/printers/modprtdevs.sh, cmdpios, bos411, 9428A410j 3/23/94 11:19:40
#
# COMPONENT_NAME: (CMDPIOS)
#
# FUNCTIONS: (Modifies 3.2 printer device attributes to 4.1 format)
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	Script to convert printer device attributes from 3.2 to 4.1 format.

set -o nounset
typeset -r	pgnm=${0##*/}
typeset -r	USAGE="Usage:\t$pgnm -d CuDvStanzaFile -a CuAtStanzaFile"
typeset		flag
typeset -i	dflag=0
typeset -i	aflag=0
typeset		dvfl			# ODM CuDv file
typeset		atfl			# ODM CuAt file
typeset		ptype			# printer type
typeset -r	tdvfl=/tmp/CuDv_prt_$$
typeset -r	tatfl=/tmp/CuAt_prt_$$

# Process the flags
while getopts :d:a: flag
do
   case $flag in
      d)   dflag=1;dvfl="$OPTARG";;
      a)   aflag=1;atfl="$OPTARG";;
      :)   print -u2 "$pgnm: -$OPTARG needs an argument\n$USAGE";exit 1;;
      ?)   print -u2 "$pgnm: -$OPTARG is an invalid flag\n$USAGE";exit 1;;
   esac
done
[[ $dflag = 1 && $aflag = 1 ]] || { print -u2 "$USAGE";exit 1;}
[[ -r $dvfl ]] || { print -u2 "$pgnm: $dvfl doesnt exist or unreadable";exit 2;}
[[ -r $atfl ]] || { print -u2 "$pgnm: $atfl doesnt exist or unreadable";exit 2;}

# Convert the CuDv and CuAt stanza files.
awk -v atfl=$atfl -v oatfl=$tatfl -v pgnm=$pgnm '
     # Process the cudv file
     {
	#gsub(/"/,"",$0);
	if ($0 ~ /^[ 	]*[^:]*:[ 	]*$/)
	   devnm = substr($1,1,((i = index($1,":"))?i:length($1))-1);
	if ($1 == "PdDvLn" && match($3,"printer/[^/]*/"))
	{
	   dnms[devnm] = devnm;
	   prttype = substr($3,RSTART+RLENGTH);
	   if (prttype == "2380" ||
	       prttype == "2380-2" ||
	       prttype == "2381" ||
	       prttype == "2381-2" ||
	       prttype == "2390" ||
	       prttype == "2390-2" ||
	       prttype == "2391" ||
	       prttype == "2391-2" ||
	       prttype == "3812-2" ||
	       prttype == "3816" ||
	       prttype == "4019" ||
	       prttype == "4029" ||
	       prttype == "4037" ||
	       prttype == "4039" ||
	       prttype == "4070" ||
	       prttype == "4072" ||
	       prttype == "4076" ||
	       prttype == "4079" ||
	       prttype == "4201-2" ||
	       prttype == "4201-3" ||
	       prttype == "4202-2" ||
	       prttype == "4202-3" ||
	       prttype == "4207-2" ||
	       prttype == "4208-2" ||
	       prttype == "4212" ||
	       prttype == "4216-31" ||
	       prttype == "4224" ||
	       prttype == "4226" ||
	       prttype == "4234" ||
	       prttype == "5202" ||
	       prttype == "5204" ||
	       prttype == "5575" ||
	       prttype == "5577" ||
	       prttype == "5585" ||
	       prttype == "6180" ||
	       prttype == "6182" ||
	       prttype == "6184" ||
	       prttype == "6185-1" ||
	       prttype == "6185-2" ||
	       prttype == "6186" ||
	       prttype == "6252" ||
	       prttype == "6262" ||
	       prttype == "7372")
	      prttype = "ibm" prttype;
	   else if (prttype == "prt1021")
	      prttype = "bull1021";
	   else if (prttype == "prt1072")
	      prttype = "bull1070";
	   else if (prttype == "prt0201")
	      prttype = "bull201";
	   else if (prttype == "prt0411")
	      prttype = "bull411";
	   else if (prttype == "prt0422")
	      prttype = "bull422";
	   else if (prttype == "prt4512" || prttype == "prt4513")
	      prttype = "bull451";
	   else if (prttype == "prt4542" || prttype == "prt4543")
	      prttype = "bull454";
	   else if (prttype == "prt0721")
	      prttype = "bull721";
	   else if (prttype == "prt9222")
	      prttype = "bull922";
	   else if (prttype == "prt9232")
	      prttype = "bull923";
	   else if (prttype == "prt9242")
	      prttype = "bull924";
	   else if (prttype == "prt9282")
	      prttype = "bull924N";
	   else if (prttype == "prt9702")
	      prttype = "bull970";
	   else if (prttype == "pr88")
	      prttype = "bullpr88";

	   $3 = substr($3,1,RSTART+RLENGTH-1) prttype;
	   print "	" $0;
	}
	else
	   print;
     }

     # Process the cuat file.
     END {
	while ((retcode = getline < atfl) > 0)
	{
	   #gsub(/"/,"",$0);
	   if ($0 ~ /^[ 	]*[^:]*:[ 	]*$/)
	   {
	      devnm = substr($1,1,((i = index($1,":"))?i:length($1))-1);
	      pflg = (devnm in dnms)?1:0;
	      print_rec($0,oatfl);
	   }
	   else if (pflg)
	   {
	      if ($1 == "tbc")
	      {
		  print_rec(sprintf("	tbc16 = %s",$3),oatfl);
		  print_rec(sprintf("	tbc64 = %s",$3),oatfl);
		  #print_rec(sprintf("	tbc128 = %s",$3),oatfl);
	      }
	      else if ($1 == "line_disp" || $1 == "map_disp" ||
		       $1 == "discipline" || $1 == "sttyval" ||
		       $1 == "xon" || $1 == "dtr" ||
		       $1 == "sticky_xonxoff")
	      {
		 # skip this line
	      }
	      else
		 print_rec($0,oatfl);
	   }
	   else
	      print_rec($0,oatfl);
	}				# end - while getline
	if (retcode < 0)
	{
	   print_error(sprintf("%s: Error in reading the file \"%s\"",pgnm,\
			       atfl));
	   exit(4);
	}

	close(atfl);
	close(oatfl);
     }

     # Function to output a message to a file.
     function print_rec(rec,ofl)
     {
	print rec > ofl;
	return;
     }

     # Function to output an error message.
     function print_error(errormsg)
     {
	 system("echo " errormsg " >&2");
	 return;
     }
     ' $dvfl>|$tdvfl ||
       { print "Error in creating $tdvfl and/or $tatfl"; exit 4;}

# Save the new converted files.
mv -f $tdvfl $dvfl || { print "Error in moving $tdvfl to $dvfl";exit 3;}
mv -f $tatfl $atfl || { print "Error in moving $tatfl to $atfl";exit 3;}

exit 0
