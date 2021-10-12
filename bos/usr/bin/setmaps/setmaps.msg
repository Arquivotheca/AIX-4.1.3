$ @(#)31 1.7 src/bos/usr/bin/setmaps/setmaps.msg, cmdtty, bos411, 9428A410j 2/23/94 05:34:31
$
$ COMPONENT_NAME: CMDTTY terminal control commands
$
$ FUNCTIONS: msg file for setmaps
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$

$set MS_SETMAPS

$quote "

ROOT "%s: must be root to reload a map\n"
CANTUSE "%s: can't use -k with -t\n"
INTERN "%s: internal error, unexpected verb=%d\n"
RMBOTH "%s: both mappings have been removed\n"
RMINPUT "%s: input mappings have been removed\n"
RMOUTPUT "%s: output mappings have been removed\n"
INT2 "%s: internal error, bad verb in use_map: %d\n"
ATTEMPT "%s: attempting to use map %s\n"
LOADED "%s: %s already loaded, now using it\n"
NOTLOADED "%s: %s is not presently loaded\n"
RLOAD "%s: must be root to load a new map\n"
ATTEMPT2 "%s: will attempt to load %s\n"
LOADOK "%s: %s loaded ok, will try again to use it\n"
PATH "%s: map pathname is: %s\n"
NOMEM "%s: can't allocate memory to build map\n"
CREATE "%s: unable to create map from %s\n"
ALOADED "%s: NLS map %s is already loaded\n"
NONEI "%s map: none installed\n"
MAP "%s map: %s\n"
UNKNOWN "<UNKNOWN?>"
FULL "%s: internal error, fullpath botch\n"
CSMAPPATH "%s: csmap pathname is: %s\n"
CSMAPINTL "%s: internal error, csmap_path botch\n"
CSMAPLARGE "%s: improper file format: file too large\n"
CSMAPSMALL "%s: improper file format: file too small\n"
CSMAPFORMAT "%s: improper file format at line %d\n"

USAGE "to use a map: setmaps [-v] -{i | o | t} mapname\n\
use one of -i, -o, or -t\n\
-i = use mapname.in as your input map\n\
-o = use mapname.out as your output map\n\
-t = two-way mapping, use both mapname.in and mapname.out\n\
the special name NOMAP will clear the corresponding mapping:\n\
setmaps -i NOMAP\n\
\n\
to clear all mappings on this terminal: setmaps -c\n\
\n\
to use a code set map: setmaps [-v] -s -{i | o} mapname\n\
\n\
-h = help, print this message(all other options ignored)\n\
-v = verbose\n\
\n\
Type: setmaps -v -h for advanced usage information\n"

VUSAGE "\n\
advanced usage(administrators only):\n\
-l mapname Load a map for later use(must include suffix if any)\n\
-k keyname Overrides mapname, normally filename is used\n\
-d dirpath Overrides directory where map files are assumed to be\n\
-I, -O, -L Same as -i, -o, -l, except full path is given by arg\n\
-r Force reload of a map even if already loaded\n\
-D Produce debug dump of map on stdout before loading\n\
(useful for making new maps, don't run as root until\n\
map is fully debugged to prevent actual load)\n\
\n\
All maps loaded must have unique names, use -k to eliminate conflicts\n\
Only -i, -o and -t implicitly add a suffix, other options specifying\n\
mapnames should include a suffix if appropriate.\n\
If a requested mapname is already loaded in the kernel, it will be\n\
used, even if the path info implies a different map. Administrator\n\
may force a reload with the -r option\n"

IGNORED "%s: null mapping ignored, pat=%s, rep=%s\n"
CANTADD "%s: can't add rule, pat=%s, rep=%s\n"
MAKEMAP "%s: makemap, no rules loaded\n"
NULLMAP "%s: %d null mappings ignored\n"
BADRULE "%s: %d bad rules rejected\n"
TOOLONG "%s: mapping too long, pat=%s, replace=%s\n"
XLATE "%s: xlate, illegal $ spec(%c%c) in replacement: %s\n"
OVERRAN "%s: xlate, overran translation buffer: %s\n"
NOCLOSE "%s: xlate, no closing ']': %s\n"
MISSEND "%s: xlate, missing end-of-range: %s\n"
REPBUF "%s: xlate, overran replacement buf: %s\n"
MAL "%s: malformed NLS pattern: %s\n"
NAME "\nMap dump, name: %s\n"
TMAP " total map length:\t%d\n"
RULES " rules in map:\t%d\n"
DEF " default rule:\t%d\n"
WILD " first wildcard:\t%d\n\n"
HASH "Hash table:\n"
RULE2 "\nRules:\n"
STATS " %d\tpat=%s\trep=%s\tnext=%d\n"
NLSDD "%s: can't determine if NLS module on the stream\n"
NLSLOAD "%s: can't load NLS module into the kernel\n"
NLSCFG "%s: can't configure NLS module\n"
NLSPOP "%s: unable to pop module from stream\n"
NLSPUSH "%s: unable to push module %s on stream\n"
NLSROOT "%s: must be root to load NLS module\n"
LONGLN "%s: input line too long, ignored."
