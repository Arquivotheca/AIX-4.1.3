static char sccsid[] = "@(#)12        1.7  src/bos/kernext/pse/str_config.c, sysxpse, bos411, 9440A411d 10/5/94 15:48:05";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      strdev_print
 *                 strmod_print
 *                 strdev_print_term
 *                 strmod_print_term
 *                 str_config
 *                 str_init
 *                 str_term
 *                 
 * 
 * ORIGINS: 27, 63, 83
 * 
 */
/*
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 **/

#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <sys/systemcfg.h>
#include <sys/strinfo.h>
#include <sys/strconf.h>

extern int	bufcall_configure();

extern int	clone_configure();
extern int	sad_configure();
extern int	log_configure();

void
strdev_print(sc, name, retval)
	str_config_t *sc;

	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: '%s' configure failed (%d)\n", name, retval);
#ifdef	STREAMS_DEBUG
	else
		printf("STREAMS: '%s' is configured\n", name);
#endif /* STREAMS_DEBUG */
}

void
strmod_print(sc, name, retval)
	str_config_t *sc;
	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: module '%s' configure failed (%d)\n",
			name, retval);
#ifdef	STREAMS_DEBUG
	else
		printf("STREAMS: module '%s' is configured\n", name);
#endif /* STREAMS_DEBUG */
}

void
strdev_print_term(sc, name, retval)
	str_config_t *sc;
	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: '%s' unconfigure failed (%d)\n", name, retval);
#ifdef	STREAMS_DEBUG
	else
		printf("STREAMS: '%s' is unconfigured\n", name);
#endif /* STREAMS_DEBUG */
}

void
strmod_print_term(sc, name, retval)
	str_config_t *sc;
	char *name;
	int retval;
{
	if (retval)
		printf("STREAMS: module '%s' unconfigure failed (%d)\n",
			name, retval);
#ifdef	STREAMS_DEBUG
	else
		printf("STREAMS: module '%s' is unconfigured\n", name);
#endif /* STREAMS_DEBUG */
}

static simple_lock_data_t str_conf_lock = {SIMPLE_LOCK_AVAIL};

#define CONF_LOCKINIT()	{						 \
	lock_alloc((&str_conf_lock), LOCK_ALLOC_PIN, PSE_CONF_LOCK, -1); \
	simple_lock_init(&str_conf_lock);				 \
}

int init_state = 0;
int str_count = 0;

int
str_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
        int err = 0;

	ENTER_FUNC(str_config, cmd, uiop, 0, 0, 0, 0);
	
	CONF_LOCKINIT();

	SIMPLE_LOCK(&str_conf_lock);

        switch (cmd) {
        case CFG_INIT:  err = str_init(uiop); break;
        case CFG_TERM:  err = str_term(uiop); break;
        default:        err = EINVAL;
        }

	SIMPLE_UNLOCK(&str_conf_lock);

	lock_free(&str_conf_lock);

	LEAVE_FUNC(str_config, err);

        return err;
}

int
str_init(uiop)
        struct uio *uiop;
{
	str_config_t	sc;
#define sc_size	sizeof(str_config_t)
	extern void funnelq_init();
	
	int		retval;
	strmdi_t	mdi;
	int		error = 0;

	ENTER_FUNC(str_init, uiop, 0, 0, 0, 0, 0);

	if (retval = uiomove((char *)&mdi, sizeof(mdi), UIO_WRITE, uiop))
	{
                error = EFAULT;
		goto out;
	}
	switch (init_state) {
	case 0:
	    if (error = pincode(str_init)) goto out;
	    if (error = pse_init ())
		goto out;
	    init_state ++;

	case 1:
	    retval = bufcall_configure(CFG_INIT , NULL, 0, &sc, sc_size);
	    strmod_print(&sc, "bufcall", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    if (__power_mp()) funnelq_init();
 	    init_state ++;

	case 2:
	    sc.sc_devnum = makedev(mdi.clonemaj, 0);
	    retval = clone_configure(CFG_INIT, &sc, sc_size, &sc, sc_size);
	    strdev_print(&sc, "clone", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state ++;

	case 3:
	    sc.sc_devnum = makedev(mdi.sadmaj, 0);
	    retval = sad_configure(CFG_INIT, &sc, sc_size, &sc, sc_size);
	    strdev_print(&sc, "sad", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state ++;

	case 4:
	    sc.sc_devnum = makedev(mdi.logmaj, 0);
	    retval = log_configure(CFG_INIT, &sc, sc_size, &sc, sc_size);
	    strdev_print(&sc, "slog", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state ++;

	case 5:
	    init_state |= INITDONE;
	    break;

	default:
	    error = ENXIO;
	    break;
	}

#undef sc_size

out:
	LEAVE_FUNC(str_init, error);
	return error;
}

int 
str_term(uiop)
	struct uio *uiop;
{
	str_config_t	sc;
#define sc_size		sizeof(sc)
	int		retval;
	int error = 0;
	extern void funnelq_term();

	ENTER_FUNC(str_term, uiop, 0, 0, 0, 0, 0);

	if ((str_count != 0) || ((str_count == 0) && (error = dmodsw_search())))
	{
	    if (!error) error = EEXIST;
	    goto out;
	}

	init_state &= ~INITDONE;

	switch (init_state) {
	case 5:
	    retval = log_configure(CFG_TERM, NULL, 0, &sc, sc_size);
	    strdev_print_term(&sc, "slog", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state --;

	case 4:
	    retval = sad_configure(CFG_TERM, NULL, 0, &sc, sc_size);
	    strdev_print_term(&sc, "sad", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state --;

	case 3:
	    retval = clone_configure(CFG_TERM, NULL, 0, &sc, sc_size);
	    strdev_print_term(&sc, "clone", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state --;

	case 2:
	    if (__power_mp()) funnelq_term();
	    retval = bufcall_configure(CFG_TERM , NULL, 0, &sc, sc_size);
            strmod_print_term(&sc, "bufcall", retval);
	    if (retval) {
		error = retval;
		goto out;
	    }
	    init_state --;

	case 1:
	    pse_term();
	    (void)unpincode(str_init);
	    init_state --;
	    break;
	default:
		error = ENXIO;
	}
	/*
	 * nothing we could do about this anyway...
	 */

out:
	LEAVE_FUNC(str_term, error);
	return error;
}
