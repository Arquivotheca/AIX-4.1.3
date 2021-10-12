static char sccsid[] = "@(#)17        1.7  src/bos/usr/bin/trcrpt/builtins.c, cmdtrace, bos411, 9428A410j 6/24/94 07:51:03";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: builtin_init, fntobuiltin, fntoname, fntodesc
 *            finstall, flookup
 *            Tsetdelim, Tflih, Tresume, Tbuftofilename, Tsidtofilename
 *            Tvnodetofilename, Tvpagetofilename, Tfdtofilename, Tslihlookup
 *            Tkexit, Tdevtoname, Tapproxlookup
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

#include <stdio.h>
#include "rpt.h"
#include "td.h"

extern char *devicelookup();			/* major/minor to device name */
extern char *buflookup();				/* sid to filename */
extern char *sidlookup();				/* sid to filename */
extern char *vpagelookup();				/* vpage to filename */
extern char *vnodelookup();				/* vnode to filename */
extern char *fdlookup();				/* fd to filename */
extern char *exec_lookup();				/* pid to exec_pathname */
extern char *rptsym_nmlookup();			/* symbol value to symbol name */
extern char *loaderlookup();			/* loaded symbol value to name */

extern Pid;
extern Intr_depth;
extern Nodelimflg;

#define NBUILTINS 310
static struct builtins bl[NBUILTINS];
static Nbuiltins;

extern Tpidinstall();
extern Tloaderinstall();
extern Tslihlookup();
extern Tflih();
extern Tresume();
extern Tlookuppninstall1();
extern Tlookuppninstall2();
extern Tfdinstall();
extern Tdebug();
extern Tvmmbufinstall();
extern Tbufinstall();
extern Tsidtofilename();
extern Tbuftofilename();
extern Tvpagetofilename();
extern Tvnodetofilename();
extern Tfdtofilename();
extern Tpfsrdwrinstall1();
extern Tpfsrdwrinstall2();
extern Tkexit();
extern Tstarttimer();
extern Tendtimer();
extern Tdeviceinstall();
extern Tdevtoname();
extern Tsetdelim();
extern Tapproxlookup();
extern Tprargs();
extern Tdoesnothing();

char *fntoname(n)
unsigned n;
{

	if(n < Nbuiltins)
		return(bl[n].bl_name);
	return("?");
}


#ifdef TRCRPT

union td *fntodesc(n)
{

	if(n >= Nbuiltins)
		return(0);
	return(bl[n].bl_desc);
}

#endif


builtin_init()
{

	finstall("pidinstall",       Tpidinstall,      0);
	finstall("loaderinstall",    Tloaderinstall,   0);
	finstall("slihlookup",       Tslihlookup,      0);
	finstall("flih",             Tflih,            0);
	finstall("kexit",            Tkexit,           0);
	finstall("resume",           Tresume,          0);
	finstall("lookuppninstall1", Tlookuppninstall1,0);
	finstall("lookuppninstall2", Tlookuppninstall2,0);
	finstall("fdinstall",        Tfdinstall,       0);
	finstall("fdtofilename",     Tfdtofilename,    0);
	finstall("sidtofilename",    Tsidtofilename,   0);
	finstall("debug",            Tdebug,           0);
	finstall("pfsrdwrinstall1",  Tpfsrdwrinstall1, 0);
	finstall("pfsrdwrinstall2",  Tpfsrdwrinstall2, 0);
	finstall("bufinstall",       Tbufinstall,      0);
	finstall("vmmbufinstall",    Tvmmbufinstall,   0);
	finstall("vnodetofilename",  Tvnodetofilename, 0);
	finstall("vpagetofilename",  Tvpagetofilename, 0);
	finstall("buftofilename",    Tbuftofilename,   0);
	finstall("starttimer",       Tstarttimer,      0);
	finstall("endtimer",         Tendtimer,        0);
	finstall("deviceinstall",    Tdeviceinstall,   0);
	finstall("devtoname",        Tdevtoname,       0);
	finstall("setdelim",         Tsetdelim,        0);
	finstall("approxlookup",     Tapproxlookup,    0);
	finstall("prargs",           Tprargs,          0);
	finstall("doesnothing",      Tdoesnothing,     0);
}

struct builtins *finstall(name,func,desc)
char *name;
int (*func)();
union td *desc;
{
	int n;

	Debug("finstall(%s,%x,%x)\n",name,func,desc);
	if(Nbuiltins >= NBUILTINS)
		return(0);
	if((n = flookup(name)) < 0) {
		n = Nbuiltins++;
		bl[n].bl_name = stracpy(name);
	}
	bl[n].bl_func = func;
	bl[n].bl_desc = desc;
	return(&bl[n]);
}

flookup(name)
char *name;
{
	int i;
	struct builtins *blp;

	for(i = 0, blp = bl; i < Nbuiltins; i++, blp++)
		if(STREQ(blp->bl_name,name))
			return(i);
	return(-1);
}

struct builtins *fntobuiltin(n)
{

	if(n >= Nbuiltins)
		return(0);
	return(&bl[n]);
}

static Tdebug(n)
{
	static initflg;
	static debugbase;

	if(!initflg)
		debugbase = Debugflg;
	if(n && !initflg++)
		debuginit("Btrace");
	Debugflg = n ? n : debugbase;
}

Tsetdelim(n)
{

	Nodelimflg = n ? 0 : 1;
}

/*
 * template function
 */
Tflih(type)
{

	if(Intr_depth < 0)
		Intr_depth = 0;
	Debug("flih(%d)\n",type);
	switch(type) {
	case 5:
		Intr_depth++;
		break;
	}
}

/*
 * template function
 */
Tresume(procflg)
{

	if(Intr_depth < 0)
		Intr_depth = 0;
	Debug("resume(%d)\n",procflg);
	if(procflg == 0) {
		Intr_depth = 0;
		return;
	}
	if(Intr_depth)
		Intr_depth--;
}

/*
 * template function
 */
Tbuftofilename(buf)
{
	char *cp;

	Debug("Tbuftofilename(%x)\n",buf);
	if(cp = buflookup(buf))
		pr_string(cp);
}

/*
 * template function
 */
Tsidtofilename(sid)
{
	char *cp;

	Debug("Tsidtofilename(%x)\n",sid);
	if(cp = sidlookup(sid))
		pr_string(cp);
}

/*
 * template function
 */
Tvnodetofilename(vnode)
{
	char *cp;

	Debug("Tvnodetofilename(%x)\n",vnode);
	if(cp = vnodelookup(vnode))
		pr_string(cp);
}

/*
 * template function
 */
Tvpagetofilename(vpage,sid)
{
	char *cp;

	Debug("Tvpagetofilename(%x,%x)\n",vpage,sid);
	if(cp = vpagelookup(vpage,sid))
		pr_string(cp);
}

/*
 * template function
 */
Tfdtofilename(fd)
{
	char *cp;

	Debug("Tfdlookup(%d)\n",fd);
	if(cp = fdlookup(fd))
		pr_string(cp);
}

/*
 * template function
 */
Tslihlookup(value)
{
	char *cp;
	char buf[16];

	Debug("slihlookup(%x)\n",value);
	if((cp = rptsym_nmlookup(value,0)) == 0 &&
	   (cp = loaderlookup(value)) == 0) {
		sprintf(buf,"%08X",value);
		cp = buf;
	}
	pr_string(cp);
}

Tkexit()
{
}

/*
 * Hard coded because of incompatibility between V3 and V2 (non-ansi)
 * header files.
 */
#define _major(mmdev) (mmdev >> 16)
#define _minor(mmdev) (mmdev  & 0xFFFF)

Tdevtoname(rdev)
{
	char *cp;
	char buf[32];

	if(cp = devicelookup(rdev)) {
		pr_string(cp);
	} else {
		sprintf(buf,"(%d.%d)",_major(rdev),_minor(rdev));
		pr_string(buf);
	}
}

Tapproxlookup(value)
{
	char *cp;
	char buf[16];

	Debug("approxlookup(%x)\n",value);
	if((cp = rptsym_nmlookup(value,1)) == 0) {
		sprintf(buf,"%08X",value);
		cp = buf;
	}
	pr_string(cp);
}

int Tdoesnothing ()
{
	pr_string("?");
}
