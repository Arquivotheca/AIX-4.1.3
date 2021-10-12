static char sccsid[] = "@(#)49  1.8     src/bldenv/pkgtools/common/processPtfmsg.c, pkgtools, bos41J, 9512A_all 2/17/95 14:55:16";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern char	*commandName;

char	*AllFlagsNotSpecified =
"\t All the flags on the command line should be specified.\n ";

char	*BuildTypeNotSet =
"\tThe BUILD_TYPE environment variable is not set.  It should be set\n\
	to either sandbox, production or area.\n";

char	*CouldNotGetVRMF =
"\tCould not get vrmf number from file %s.\n";

char	*CouldNotOpenForVRMF =
"\tCould not open lpp_info file to get the base vrmf \n\
     for fileset %s.\n";

char	*InvalidFileFormat =
"\tThe \"<@>\" delimiter was not found in the %s\n\
	file.  The format of this file may be invalid.\n";

char	*InvalidFileset =
"\tFileset %s was not listed as a valid fileset\n\
	in the lpp_name files from %s.\n";

char	*InvalidReq =
"\t %s requisite type is not vaild.\n";

char	*InvalidVRMF =
"       fileset %s has an invalid vrmf %s format.\n\
	The correct format is v.r.m.f where\n\
	v is version, r is release, m is mod and f is fix level.\n";

char    *MultipleEntries =
"\tSpecial case ptf types should have only one entry in the ptf_pkg file.\n\
	These types are cum_ptf, enh_ptf, C_ptf, opp_ptf, pmp_ptf and\n\
	pkg_ptf.\n";

char	*MultiplePtfs =
"\tThe wk_ptf_pkg file contains more than one ptf.\n";

char	*MultipleFilesetsForFile =
"\tThe entry for %s\n\
	in input file %s\n\
	contains multiple filesets in the fourth field.  Each entry\n\
	should only have one fileset.\n";

char	*MultipleFilesetForPtf =
"\tThe wk_ptf_pkg file has entries with different filesets.\n\
	The only types of ptfs which should span filesets are enh_ptf, C_ptf,\n\
	opp_ptf, pmp_ptf and pkg_ptf which are special case packaging\n\
	ptf types.\n";

char	*FilesetNotFound = 
"\tCould not get fileset for ptf %s from\n\t%s file.\n";

char	*PtfoptionsMismatch = 
"\tThe entry for ptf %s in %s\n\
	does not match the fileset %s listed in the wk_ptf_pkg file.\n";

char	*ReqNotInPtfOptFile =
"\t Ptf %s was not found in the\n\
	%s file or internalvrmfTable.\n";

char	*SelfixTopNotSet =
"\tThe TOP environment variable is not set.  It should be set\n\
	to the base of the UPDATE tree.\n";

char	*StatFailed =
"\tStat failed on file %s.\n\
	errno = %d\n";

char	*Usage =
"USAGE:\n\t%s -v <version> -r <release> -m <modification level> -f <fix level > \n\
	where all the flags are optional. If one of the flags is specified then all \n\
	should be specified.\n  ";

char	*VRMFNotFound = 
"\tCould not get vrmf number for ptf %s from %s file. \n";


char	*WriteError =
"\tA write error occurred trying to update file\n\
	%s, errno = %d\n";

char	*InvalidPtf =
"\tPTF %s is an invalid ptf.  Could not \n\
	determine the next valid fileset and vrmf number to promote, \n\
	using the %s fileset, from either the ptfsList file, the \n\
	internal vrmf table, or the cumsList file. \n";

