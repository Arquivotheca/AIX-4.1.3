static char sccsid[] = "@(#)46	1.2  src/bos/usr/ccs/lib/libpse/strconfig.c, cmdpse, bos411, 9436C411a 9/7/94 17:48:45";
/*
 *   COMPONENT_NAME: cmdpse
 *
 *   FUNCTIONS: basename
 *		cfg_dd
 *		cfg_kmod
 *		iskernext
 *		load_dev
 *		load_mod
 *		loadit
 *		logerr
 *		makenode
 *		mkdevice
 *		mkdevname
 *		mkpath
 *		newerrlog
 *		queryload
 *		rmdevice
 *		rmnode
 *		strconferr
 *		strconfig
 *		unload_dev
 *		unload_mod
 *		unloadit
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * strconfig - streams extension configuration routine
 * strconferr - report errors that occurred during strconfig
 *
 * SYNOPSIS:
 *	int strconfig(int cmd, ustrconf_t *conf);
 *	void strconferr(FILE *fp, char *prefix, int verbose);
 *	int mkdevice(node_t *nodes, int maj);
 *	int rmdevice(node_t *nodes, int maj);
 *	char *mkdevname(char *path, char *suffix);
 *
 * HISTORY:
 *	94/08/05 dgb	created for defect 151742
 */

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <sys/strconf.h>
#include "strload_msg.h"

/* device perms and mode for mknod, chmod */
#define	PERMS	0666
#define	MODE	(_S_IFCHR | PERMS)
char PSEDIR[] =	"/usr/lib/drivers/pse";	/* base for kernel exts */

/* lint hacks */
#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf
#define	Printf	(void)printf
#define	Strcpy	(void)strcpy
#define	Strcat	(void)strcat
#define	Strncat	(void)strncat

static void newerrlog(), logerr();
static mid_t queryload();
static char *basename();
char *mkdevname();

/*
 * private error logging/reporting data structures
 */

static char Errbuf[512], *Errloc;
static int Doloaderr;

static struct {
	int code;
	char *text;
} msgs[] = {
	{ 0,		"system error"					},
	{ USAGE,
	"usage: %s [-u|-q] [-h <size>] [-f <file>] [-d <list>] [-m <list>]\n" },
	{ MSG1,		"cannot open %s"				},
	{ MSG2,		"unknown PSE type '%s'\n"			},
	{ MSG3,		"cannot load %s"				},
	{ MSG4,		"cannot initialize %s"				},
	{ MSG5,		"cannot terminate %s"				},
	{ MSG6,		"cannot unload %s"				},
	{ MSG7,		"cannot initialize %s"				},
	{ MSG8,		"cannot initialize %s"				},
	{ MSG9,		"cannot unload: %s not present"			},
	{ MSG10,	"cannot terminate %s"				},
	{ MSG11,	"cannot unload %s"				},
	{ MSG12,	"cannot unload: %s not present"			},
	{ MSG13,	"cannot terminate %s"				},
	{ MSG14,	"cannot unload %s"				},
	{ MSG15,	"yes"						},
	{ MSG16,	"no"						},
	{ MSG17,	"cannot query %s"				},
	{ MSG18,	"'%s' already loaded\n"				},
	{ MSG19,	"cannot load %s"				},
	{ MSG20,	"cannot get major for %s"			},
	{ MSG21,	"cannot unlink %s"				},
	{ MSG22,	"cannot release major for %s - odmerr %d"	},
	{ MSG23,	"cannot generate major for %s - odmerr %d"	},
	{ MSG24,	"cannot get major for /dev/clone"		},
	{ MSG25,	"cannot generate major for %s - odmerr %d"	},
	{ MSG26,	"cannot unlink %s"				},
	{ MSG27,	"cannot create %s"				},
	{ MSG28,	"cannot create %s"				},
	{ MSG30,	"line %d: only first %d minors used"		},
	{ MSG31,	"line %d: unknown attribute '%c'"		},
	{ MSG32,	"line %d: missing extension type"		},
	{ MSG33,	"line %d: no filename present"			},
	{ 0, NULL }
};

/*
 ***************************************************
 * External Configuration Routines
 *
 * Each routine in this section should return either
 * success (0) or failure (-1).  Failures should
 * produce a diagnostic.  Nothing is fatal at this level.
 ***************************************************
 */

/*
 * strconfig - load, unload, init, term, or query a streams extension
 */

int
strconfig(int cmd, ustrconf_t *conf)
{
	struct cfg_load load;
	node_t nodes[2];
	int newmaj = 0;
	int rc = -1;
	int err = 0;
	mid_t kmid;

	/* start a new error logging "session" */
	newerrlog();

	if (getuid() != 0)			/* check authorization */
		err = EACCES;
	else if (!conf ||			/* check basic arguments */
		 conf->version != STRCONF_V1 || !conf->extname)
		err = EINVAL;
	if (err) {
		logerr(0, 0, err);
		return -1;
	}

	kmid = queryload(&load, conf->extname);

	if (cmd == STR_QUERY)
		return (int)kmid;

	/* fill in defaults for devices */
	if (conf->flags & SF_DEVICE) {
		if (!conf->nodes) {
			nodes[0].name = basename(conf->extname);
			nodes[0].dev = -1;
			nodes[1].name = NULL;
			conf->nodes = nodes;
		}
		if (!conf->odmkey)
			conf->odmkey = mkdevname(conf->nodes[0].name, 0);
		if (conf->maj == -1) {
			newmaj = conf->maj = genmajor(conf->odmkey);
			if (conf->maj == -1) {
				/* "cannot get major for %s" */
				logerr(MSG20, conf->odmkey, 0);
				return -1;
			}
		}
	}

	/* switch out to operation handler */
	switch (cmd) {
	case STR_LOAD:
		if (conf->flags & SF_DEVICE)
			rc = load_dev(cmd, &load, conf);
		else
			rc = load_mod(cmd, &load, conf);
		break;

	case STR_UNLOAD:
		if (conf->flags & SF_DEVICE)
			rc = unload_dev(cmd, &load, conf);
		else
			rc = unload_mod(cmd, &load, conf);
		break;

	case STR_INIT:
	case STR_TERM:
		cmd = (cmd == STR_INIT) ? CFG_INIT : CFG_TERM;
		if (conf->flags & SF_DEVICE)
			rc = cfg_dd(cmd, &load, conf);
		else
			rc = cfg_kmod(cmd, &load, conf);
		break;

	default:
		logerr(0, 0, EINVAL);
		break;
	}

	/*
	 * Prevent ODM pollution:
	 * If the operation failed, and we used genmajor, and the extension
	 * was not loaded before the operation, release the major.  Cannot
	 * just release it on failure, since it may have been previously
	 * allocated, and the operation simply failed with EEXIST or similar.
	 * This hack brought to you by genmajor semantics.
	 */
	if (rc && newmaj && !kmid)
		(void)relmajor(conf->odmkey);

	return rc;
}

/*
 * strconferr - report all the errors that occur during operation
 *
 * This routine is only valid if strconfig() has just failed (returned -1).
 *
 * Usage:
 *	if (strconfig(op, &conf)) {
 *		fprintf(stderr, "%s: cannot %s %s",prog,name[op],conf.extname);
 *		strconferr(stderr, prog, 1);
 *	}
 */

void
strconferr(FILE *fp, char *prefix, int verbose)
{
	char *cp, *cp1, *arg, *fmt;
	int code, err;
	nl_catd xcatd;

	xcatd = catopen(MF_STRLOAD, NL_CAT_LOCALE);
	for (cp1 = cp = Errbuf; *cp1; cp = cp1) {
		cp1 = cp + strlen(cp) + 1;
		code = atoi(strtok(cp, " "));
		err = atoi(strtok(0, " "));
		arg = strtok(0, " ");
		if (code != 0) {
			fmt = catgets(xcatd, MS_STRLOAD, code, msgs[code].text);
			if (cp = strchr(fmt, '\n'))
				*cp = 0;
		} else
			fmt = "strconfig";
		if (prefix)
			Fprintf(fp, "%s: ", prefix);
		Fprintf(fp, fmt, arg);
		if (err)
			Fprintf(fp, ": %s", strerror(err));
		Fprintf(fp, "\n");
	}
	(void)catclose(xcatd);
	if (verbose && Doloaderr)
		loaderr(fp);
}

/*
 ***************************************************
 * private, internal functions
 ***************************************************
 */

static void
newerrlog()
{
	Errloc = Errbuf;
	Errloc[0] = 0;
	Doloaderr = 0;
	errno = 0;
}

/*
 * logerr - log a strconfig error message
 */

static void
logerr(msgno, a, err)
{
	Sprintf(Errloc, "%d %d %s", msgno, err, a);
	Errloc += (strlen(Errloc) + 1);
	Errloc[0] = 0;
}

/*
 * load_dev - load and/or init a driver kernel extension
 *
 * returns 0 or -1
 */

static int
load_dev(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	int donodes = !load->kmid || !(conf->flags & SF_KEEPNODES);

	if (loadit(load, conf->extname, conf->flags & SF_DUP))
		return -1;

	if (cfg_dd(CFG_INIT, load, conf)) {
		(void)unloadit(load, conf->extname);
		return -1;
	}
	
	/*
	 * create the nodes last so EEXIST or similar init errors
	 * do not cause us to remove valid nodes
	 */
	if (donodes) {
		if (mkdevice(conf->nodes, conf->maj)) {
			(void)cfg_dd(CFG_TERM, load, conf);
			(void)unloadit(load, conf->extname);
			return -1;
		}
	}

	return 0;
}

/*
 * load_mod - load and/or initialize kernel extension
 *
 * returns 0 or -1
 */

static int
load_mod(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	if (loadit(load, conf->extname, conf->flags & SF_DUP))
		return -1;

	if (cfg_kmod(CFG_INIT, load, conf)) {
		(void)unloadit(load, conf->extname);
		return -1;
	}
	
	return 0;
}

/*
 * unload_dev - terminate and/or unload driver kernel extension
 *
 * returns 0 or -1
 */

static int
unload_dev(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	if (!load->kmid) {
		logerr(MSG12, conf->extname, 0); /* "%s not present" */
		return -1;
	}

	if (cfg_dd(CFG_TERM, load, conf))
		return -1;

	if (unloadit(load, conf->extname))
		return -1;

	if (!(conf->flags & SF_KEEPNODES) || !queryload(load, conf->extname)) {
		(void)relmajor(conf->odmkey);
		return rmdevice(conf->nodes, conf->maj);
	}

	return 0;
}

/*
 * unload_mod - terminate and/or unload kernel extension
 *
 * returns 0 or -1
 */

static int
unload_mod(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	if (!load->kmid) {
		logerr(MSG12, conf->extname,0 ); /* "%s not present" */
		return -1;
	}

	if (cfg_kmod(CFG_TERM, load, conf))
		return -1;

	return unloadit(load, conf->extname);
}

/*
 ***************************************************
 * Configuration Support Routines
 *
 * Each routine in this section should return either
 * success or failure.  Failures should produce a
 * diagnostic.  Nothing is fatal at this level.
 ***************************************************
 */

/*
 * queryload - query if extension is loaded
 *
 * returns non-zero mid_t if loaded, 0 if not loaded
 */

static mid_t
queryload(struct cfg_load *load, char *name)
{
	static char path[128];

	bzero(load, sizeof(*load));
	load->path = path;
	if (strchr(name, '/') || iskernext(name))
		Strcpy(path, name);
	else
		Sprintf(path, "%s/%s", PSEDIR, name);

	if (sysconfig(SYS_QUERYLOAD, load, sizeof(load)) < 0) {
		logerr(MSG17, name, errno);	/* "cannot query %s" */
		load->kmid = (mid_t)0;	/* look like not loaded */
	}

	errno = 0;	/* already reported any errors */
	return load->kmid;
}

/*
 * loadit - load a kernel extension
 */

static int
loadit(struct cfg_load *load, char *name, int dupload)
{
	if (load->kmid && !dupload) {
		logerr(MSG18, name, 0);	/* "'%s' already loaded\n" */
		return -1;
	}

	/* only load if not already loaded */
	if (sysconfig(SYS_SINGLELOAD, load, sizeof(*load)) < 0) {
		logerr(MSG3, name, errno);	/* "cannot load %s" */
		Doloaderr = 1;
		return -1;	/* look like not loaded */
	}
	
	return 0;
}

/*
 * unloadit - unload a kernel extension
 */

static int
unloadit(struct cfg_load *load, char *name)
{
	if (sysconfig(SYS_KULOAD, load, sizeof(*load)) < 0) {
		logerr(MSG11, name, errno);	/* "cannot unload %s" */
		return -1;
	}
	return 0;
}

/*
 * cfg_dd - tell device to initiate/terminate processing
 *
 * NB: if cfg_dd() fails, then device doesn't want to (un)load
 * NB: requires coordination with str_install(2)
 */

static int
cfg_dd(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	struct cfg_dd cfg;
	int rc;

	cfg.cmd = cmd;
	cfg.kmid = load->kmid;
	cfg.devno = makedev(conf->maj, 0);
	cfg.ddsptr = conf->dds;
	cfg.ddslen = conf->ddslen;

	if (rc = sysconfig(SYS_CFGDD, &cfg, sizeof(cfg))) {
		if (cmd == CFG_INIT)
			logerr(MSG7, conf->extname, errno); /*"cannot init %s"*/
		else if (cmd == CFG_TERM)
			logerr(MSG13, conf->extname, errno);/*"cannot term %s"*/
		else
			logerr(0, 0, errno);
	}
	return rc;
}

/*
 * cfg_kmod - tell module to initiate/terminate processing
 *
 * NB: if cfg_kmod() fails, then module doesn't want to (un)load
 * NB: requires coordination with str_install(2)
 */

static int
cfg_kmod(int cmd, struct cfg_load *load, ustrconf_t *conf)
{
	struct cfg_kmod cfg;
	int rc;

	cfg.cmd = cmd;
	cfg.kmid = load->kmid;
	cfg.mdiptr = conf->dds;
	cfg.mdilen = conf->ddslen;

	if (rc = sysconfig(SYS_CFGKMOD, &cfg, sizeof(cfg))) {
		if (cmd == CFG_INIT)
			logerr(MSG7, conf->extname, errno); /*"cannot init %s"*/
		else if (cmd == CFG_TERM)
			logerr(MSG13, conf->extname, errno);/*"cannot term %s"*/
		else
			logerr(0, 0, errno);
	}
	return rc;
}

/*
 * makenode - ensures existance of desired node
 *
 * returns 0 on success, else -1
 */

static int
makenode(char *path, dev_t dev)
{
	struct stat stbuf;
	int i;

	/* if no node or devno's differ, (re)create node */
	if ((i = stat(path, &stbuf)) || stbuf.st_rdev != dev) {
		if (i == 0 && rmnode(path)) {
			logerr(MSG21, path, errno); /* "cannot unlink %s" */
			return -1;
		} else if (mknod(path, MODE, dev)) {
			/* if failed, see if need subdirs */
			if (errno != ENOENT ||
			    mkpath(path) ||
			    mknod(path, MODE, dev)) {
				/* "cannot create %s" */
				logerr(MSG27, path, errno);
				return -1;
			}
		}
		(void)chmod(path, PERMS);	/* negate effect of umask */
	}
	return 0;
}

/*
 * mkdevice - make the nodes requested in nodes
 *
 * device number rules:
 *	if nodes->dev == -1
 *		make cloning node with major as minor
 *	else if major(nodes->dev) == -1
 *		make std node with major but use minor(nodes->dev) as minor
 *	else use nodes->dev as given
 *
 * returns -1 on failure to create first node, 0 otherwise
 */

int
mkdevice(node_t *nodes, int maj)
{
	static int clone = -1;
	caddr_t path;
	dev_t dev;
	node_t *p;
	int rc;

	if (clone == -1) {
		clone = genmajor("/dev/clone");
		if (clone == -1)
			return -1;
	}

	for (p = nodes; p && p->name; ++p) {
		if (!(path = mkdevname(p->name, 0))) {
			logerr(MSG27, p->name, errno); /* "cannot create %s" */
			return -1;
		}
		if (p->dev == -1)
			dev = makedev(clone, maj);
		else if (major(p->dev) == (unsigned short)-1)
			dev = makedev(maj, minor(p->dev));
		else
			dev = p->dev;
		rc = makenode(path, dev);
		if (rc && p == nodes)
			return -1;
	}
	return 0;
}

/*
 * rmdevice - remove a device entry and /dev node
 *
 * returns -1 on any failure, else 0 (but removes all nodes it can)
 */

int
rmdevice(node_t *nodes, int maj)
{
	caddr_t path;
	node_t *p;
	int rc;

	rc = 0;
	for (p = nodes; p && p->name; ++p) {
		if (!(path = mkdevname(p->name, 0))) {
			logerr(MSG21, p->name, errno); /* "cannot unlink %s" */
			return -1;
		}
		if (rmnode(path)) {
			logerr(MSG21, path, errno);	/* "cannot unlink %s" */
			rc = -1;
		}
	}
	return rc;
}

/*
 ***************************************************
 * Miscellaneous Support Routines
 *
 * Each routine in this section should return either
 * success or failure, ptr or null.  If they fail,
 * they should set errno.  None should produce output.
 ***************************************************
 */

/*
 * basename - return the base name of a path
 */

static char *
basename(path)
	char *path;
{
	char *cp;
	return (cp = strrchr(path, '/')) ? ++cp : path;
}

/*
 * rmnode - only unlink character devices
 *
 * behaves like a restricted unlink(2)
 *
 * This protects the user who tries to unlink, say, /dev.
 */

static int
rmnode(path)
	char *path;
{
	struct stat stbuf;
	if (stat(path, &stbuf) == 0) {
		if (stbuf.st_mode & S_IFCHR)
			return unlink(path);
		errno = ENODEV;
	}
	return -1;
}

/*
 * mkpath - make whatever dirs are needed to create a path
 *
 * returns 0 or -1
 *
 * path is assumed to be "/dir/[dir/...]file", only the dirparts are created
 */

static int
mkpath(path)
	char *path;
{
	char *cp = path+1;
	int rc;
	errno = 0;
	while (cp = strchr(cp, '/')) {
		*cp = 0;
		rc = mkdir(path, 0755);
		*cp++ = '/';
		if (rc && errno != EEXIST)
			return -1;
	}
	return 0;
}

/*
 * mkdevname - make a /dev/node name out of name
 *
 * returns a pointer to a malloc'd area, or nil if out of memory
 */

char *
mkdevname(name, suffix)
	char *name;
	char *suffix;
{
	char *path;
	char *cp;
	char *dev = "";
	int len;

	if (!suffix)
		suffix = "";
	len = sizeof("/dev/")+strlen(name)+strlen(suffix)+1;
	if (!(path = (char *)malloc(len))) {
		errno = ENOMEM;
		return NULL;
	}

	/* setup for /dev/name unless user requests /foo/bar */
	if (name[0] != '/') {
		dev = "/dev/";
#ifdef	XXX
		/* why this restriction?  this disallows "dlpi/et" */
		if (cp = strrchr(name, '/'))
			name = ++cp;
#endif
	}
	Sprintf(path, "%s%s%s", dev, name, suffix);
	return path;
}

/*
 * iskernext - is path a plausible kernel extension?
 *
 * return true or false
 */

#include <filehdr.h>

static int
iskernext(path)
	char *path;
{
	struct stat st;
	int be_silly = 1;

	if (stat(path, &st) < 0)
		return 0;
	
	if (!S_ISREG(st.st_mode))
		return 0;
	
	if (be_silly) {
		struct filehdr hdr;
		int fd;

		/*
		 * if you ask me, this is silly;
		 * what's more, it's not even foolproof:
		 *	aix 3.2 user executables meet the same criteria(!)
		 * but, it does prevent attempts on text files
		 */

		if ((fd = open(path, O_RDONLY)) < 0)
			return 0;
		if (read(fd, &hdr, sizeof hdr) != sizeof hdr) {
			close(fd);
			return 0;
		}
		(void)close(fd);
		if (hdr.f_magic != U802TOCMAGIC || !(hdr.f_flags & F_DYNLOAD))
			return 0;
	}

	return 1;
}
