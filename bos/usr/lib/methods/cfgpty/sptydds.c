#ifndef lint
static char sccsid[] = "@(#)49 1.5 src/bos/usr/lib/methods/cfgpty/sptydds.c, cfgmethods, bos411, 9428A410j 6/2/94 07:23:41";
#endif
/*
 * COMPONENT_NAME: (CFGMETHODS) - PTY Device Dependent Structure build
 *
 * FUNCTIONS: sptydds, generate_minors, generate_devnos
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <cf.h>
#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <sys/devinfo.h>
#include <sys/mode.h>
#include <sys/types.h>

#include "cfgdebug.h"
#include "pparms.h"
#include "ttycfg.h"
#include "ptycfg.h"

/* Header file containing pty definition */
#include "spty.h"

/*
 * ==============================================================================
 * defines and strutures
 * ==============================================================================
 */
#define CLONE_FILE          "/dev/clone"

/* defines for attributes search */
/* These defines MUST be in coherence with ODM database attribute names */
#define MIN_ATTNUM          1
#define MIN_BSDNUM          0

/*
 * ==============================================================================
 * External functions declarations
 * ==============================================================================
 */
extern long * getminor();   /* returns previous minor number */

/*
 * =============================================================================
 *                       LOCAL UTILITY ROUTINES
 * =============================================================================
 */
/*
 * -----------------------------------------------------------------------------
 *                       GENERATE_MINORS
 * -----------------------------------------------------------------------------
 * 
 * Generates the minor numbers for pty device and dummy
 * pty device.
 *
 * For a name and a major, we generate as many
 * minors as required.
 *
 * First, we try to get minors and if not enough minors
 * exist, we try to generate new ones.
 *
 * All minors are generated or retrieved in the ODM database.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int generate_minors(name, majorNo, howManyMinors, firstMinor)
char * name;
dev_t  majorNo;
int    howManyMinors;
long   firstMinor;
{
    extern long * genminor();   /* generates minor number */

    int    how_many;            /* To store getminor result */
    long * minor_ptr;           /* To store getminor result */

    /* ============================== */
    /* set up initial variable values */
    /* ============================== */
    how_many = 1; /* Set to another value than 0 */

    /* =========================== */
    /* Try to find existing minors */
    /* =========================== */
    if ((minor_ptr = getminor(majorNo, &how_many, name))
        == NULL) {
        if (how_many) { /* It means it's a failure */
            DEBUG_1("generate_minors: Error when getting minors for major %d\n",
                    majorNo);
            return(E_MINORNO);
        }
        else { /* It means no existing minors */
            DEBUG_1("generate_minors: No existing minors for major %d\n",
                    majorNo);
            /* Generate required minor numbers */
            if (genminor(name, majorNo, firstMinor, howManyMinors, 1, 1) == NULL) {
                DEBUG_2("generate_minors: Error when generating %d minors for major %d\n",
                        howManyMinors, majorNo);
                return(E_MINORNO);
            }
            else {
                DEBUG_2("generate_minors: %d minors have been generated succesfully for major 0x%x\n",
                        howManyMinors, majorNo);
            } /* End if (genminor(...) == NULL) */
        } /* End if (how_many) */
    }
    else { /* Some minors already exist for this major */
		/* I need to handle specifically ATTDUMMY_MASTER because it's associated */
		/* with the CLONE_FILE major number */
		if (strcmp(name, ATTDUMMY_MASTER)) {
			/* If not enough minor numbers, we try to generate new ones */
			if (how_many < howManyMinors) {
				/* Get the last (upper) existing minor number */
				/* minor_ptr is incremented and this incremented */
				/* value is used */
				minor_ptr += (how_many - 1);
				if (genminor(name, majorNo, (*minor_ptr)+1, (howManyMinors - how_many),
							 1, 1) == NULL) {
					DEBUG_2("generate_minors: Error when generating %d minors for major %d\n",
							(howManyMinors - how_many), majorNo);
					return(E_MINORNO);
				}; /* End if (genminor(...) == NULL) */
			}
			/* If too many minor numbers, we need to reduce minor numbers */
			else if (how_many > howManyMinors) {
				/* All minor numbers are removed but not the major number. */
				/* After, only needed minor numbers are generated again */
				reldevno(name, FALSE);
				/* I need to add this test if the MIN_BSDNUM or MIN_ATTNUM value is 0 */
				/* It means that howManyMinors value can be set to 0 and that, in this case, */
				/* we must not call the genminor routine */
#if (MIN_BSDNUM == 0) || (MIN_ATTNUM == 0)
				if (howManyMinors) {
#endif
					if (genminor(name, majorNo, firstMinor, howManyMinors, 1, 1) == NULL) {
						DEBUG_2("generate_minors: Error when generating %d minors for major %d\n",
								howManyMinors, majorNo);
						return(E_MINORNO);
					}
					else {
						DEBUG_2("generate_minors: %d minors have been generated succesfully for major 0x%x\n",
								howManyMinors, majorNo);
					} /* End if (genminor(...) == NULL) */
#if (MIN_BSDNUM == 0) || (MIN_ATTNUM == 0)
				}; /* End if (howManyMinors) */
#endif
			}
			/* It means that how_many == howManyMinors; no change is needed */
			else {
				DEBUG_1("generate_minors: No change is needed, %d minors already exist\n",
						how_many);
			} /* End if (how_many < howManyMinors) */
		}
		else { /* It is for the ATTDUMMY_MASTER */
			/* Try to find if major_tmp exists already as */
			/* majorNo associated minor */
			while ((how_many--) && (*minor_ptr != firstMinor)) {
				minor_ptr++;
			} /* End while ((how_many--) && (*minor_ptr != firstMinor)) */
			if (how_many >= 0) { /* Minor number has been found */
				DEBUG_2("generate_minors: minor %d has been found for major 0x%x\n",
						*minor_ptr, majorNo);
			}
			else {
				DEBUG_2("generate_minors: No minor %d has been found for major 0x%x\n",
						firstMinor, majorNo);
				/* Generate required minor number which is the ATT master major number */
				if (genminor(ATTDUMMY_MASTER, majorNo, firstMinor, howManyMinors, 1, 1) == NULL) {
                    DEBUG_2("generate_minors: Error when generating %d minors for major %d\n",
                            howManyMinors, majorNo);
                    return(E_MINORNO);
				}
				else {
                    DEBUG_2("generate_minors: %d minors have been generated succesfully for major 0x%x\n",
                            howManyMinors, majorNo);
				} /* End if (genminor(...) == NULL) */
			} /* End if (how_many) */
		} /* End if (strcmp(name, ATTDUMMY_MASTER)) */
	} /* End if ((minor_ptr = getminor(...)) == NULL) */
	
	return(0);
} /* End static int generate_minors(...) */

/*
 * -----------------------------------------------------------------------------
 *                       GENERATE_DEVNOS
 * -----------------------------------------------------------------------------
 * 
 * Generates the major numbers for pty device and dummy
 * pty device.
 * The ATT master is a clone driver ===> Major of the clone driver and
 * minor is the ATT master major number are used to create special file.
 * The ATT slave major comes from a dummy driver (ATTDUMMY_SLAVE).
 * The BSD master major comes from the effective PTY driver.
 * The BSD slave major comes from a dummy driver (BSDDUMMY_SLAVE).
 *
 * All minors are generated or retrieved in the ODM database.
 * All DDS devno fields are set with only major number.
 *
 * Return code:   Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
static int generate_devnos(lname, driver, ptyDdsPtr, cloneDevnoPtr)
char *  lname;                   /* logical name of the device */
char *  driver;                  /* device driver for the device */
struct  spty_dds * ptyDdsPtr;    /* DDS pointer */
dev_t * cloneDevnoPtr;           /* which clone devno */
{
    extern long   genmajor();   /* generates major number */

    long   major_tmp;           /* To store major */
    int    return_code;

    struct stat stbuf;          /* To get major of the clone driver */

    /* ========== */
    /* ATT master */
    /* ========== */
    if (stat(CLONE_FILE, &stbuf)) {
        DEBUG_0("generate_devnos: Error getting clone device majorno\n");
        return(E_MAJORNO);
    }
    else {
        /* To generate one major number */
        if ((major_tmp = genmajor(ATTDUMMY_MASTER)) < 0) {
            DEBUG_0("generate_devnos: Error generating ATT master majorno\n");
            return(E_MAJORNO);
        }
        else {
            ptyDdsPtr->ptc_dev = makedev(major_tmp, 0);
            *cloneDevnoPtr = stbuf.st_rdev;
            DEBUG_1("generate_devnos: The clone file devno is 0x%x\n",
                    *cloneDevnoPtr);
            /* Only one special file will be created for the ATT master */
            /* So, only one minor is generated */
			if (return_code = generate_minors(ATTDUMMY_MASTER, major(*cloneDevnoPtr),
											  1, major_tmp)) {
				DEBUG_0("generate_devnos: Error generating ATT master minorno\n");
				relmajor(ATTDUMMY_MASTER);
				return(return_code);
			}
			else {
				DEBUG_0("generate_devnos: ATT master minorno generated succesfully\n");
			} /* End if (return_code = generate_minors(...)) */
        } /* End if ((major_tmp = genmajor(...)) < 0) */
    } /* End if (stat(CLONE_FILE, &stbuf)) */

    /* ========= */
    /* ATT slave */
    /* ========= */
    if ((major_tmp = genmajor(ATTDUMMY_SLAVE)) < 0) {
        DEBUG_0("generate_devnos: Error generating ATT slave majorno\n");
        reldevno(ATTDUMMY_MASTER, TRUE);
        return(E_MAJORNO);
    }
    else {
        ptyDdsPtr->pts_dev = makedev(major_tmp, 0);
        if (return_code = generate_minors(ATTDUMMY_SLAVE, major_tmp,
                                          ptyDdsPtr->max_pts, 0)) {
            DEBUG_0("generate_devnos: Error generating ATT slave minornos\n");
            relmajor(ATTDUMMY_SLAVE);
            reldevno(ATTDUMMY_MASTER, TRUE);
            return(return_code);
        }
        else {
            DEBUG_0("generate_devnos: ATT slave minornos generated succesfully\n");
        } /* End if (return_code = generate_minors(...)) */
    } /* End if ((major_tmp = genmajor(ATTDUMMY_SLAVE)) < 0) */

    /* ========== */
    /* BSD master */
    /* ========== */
    if ((major_tmp = genmajor(driver)) < 0) {
        DEBUG_0("generate_devnos: Error generating BSD master majorno\n");
        /* The ATT slave is removed */
        reldevno(ATTDUMMY_MASTER, TRUE);
        reldevno(ATTDUMMY_SLAVE, TRUE);
        return(E_MAJORNO);
    }
    else {
        ptyDdsPtr->ptyp_dev = makedev(major_tmp, 0);
        if (return_code = generate_minors(lname, major_tmp,
                                          ptyDdsPtr->max_ttyp, 0)) {
            DEBUG_0("generate_devnos: Error generating BSD master minornos\n");
            relmajor(lname);
            reldevno(ATTDUMMY_MASTER, TRUE);
            reldevno(ATTDUMMY_SLAVE, TRUE);
            return(return_code);
        }
        else {
            DEBUG_0("generate_devnos: BSD master minornos generated succesfully\n");
        } /* End if (return_code = generate_minors(...)) */
    } /* End if ((major_tmp = genmajor(driver)) < 0) */

    /* ========= */
    /* BSD slave */
    /* ========= */
    if ((major_tmp = genmajor(BSDDUMMY_SLAVE)) < 0) {
        DEBUG_0("generate_devnos: Error generating BSD slave majorno\n");
        reldevno(ATTDUMMY_MASTER, TRUE);
        reldevno(ATTDUMMY_SLAVE, TRUE);
        reldevno(lname, TRUE);
        return(E_MAJORNO);
    }
    else {
        ptyDdsPtr->ttyp_dev = makedev(major_tmp, 0);
        if (return_code = generate_minors(BSDDUMMY_SLAVE, major_tmp,
                                          ptyDdsPtr->max_ttyp, 0)) {
            DEBUG_0("generate_devnos: Error generating BSD slave minornos\n");
            relmajor(BSDDUMMY_SLAVE);
            reldevno(ATTDUMMY_MASTER, TRUE);
            reldevno(ATTDUMMY_SLAVE, TRUE);
            reldevno(lname, TRUE);
            return(return_code);
        }
        else {
            DEBUG_0("generate_devnos: BSD slave minornos generated successfully\n");
        } /* End if (return_code = generate_minors(...)) */
    } /* End if ((major_tmp = genmajor(BSDDUMMY_SLAVE)) < 0) */

    return(0);
} /* End static int generate_devnos(...) */


/*
 * =============================================================================
 *                       SPTYDDS
 * =============================================================================
 * 
 * This function is used to build the PTY DDS structure.
 *
 * The PTY DDS is updated in the "generate_devnos" function.
 * 
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * =============================================================================
 */

int sptydds(cusDevPtr, sptyDdsPtr, driverName, attList, cloneDevnoPtr)
struct CuDv *      cusDevPtr;  /* Customized device instance */
struct spty_dds ** sptyDdsPtr; /* To store PTY DDS pointer */
char *             driverName; /* Driver name */
struct attr *      attList;    /* attributes list */
dev_t *            cloneDevnoPtr; /* To store clone devno */
{
    int    return_code;               /* return codes go here */
    long   ATT_count;                 /* no of ATT slaves */
    long   BSD_count;                 /* no of BSD naming files (master/slave) */
        
   /* ODM structures declarations */
    struct Class * cus_att_class;     /* customized attribute class */
    struct Class * pre_att_class;     /* predefined attribute class */
        
    /* ======================== */
    /* Build DDS for pty device */
    /* ======================== */
    DEBUG_0("sptydds: Building dds\n");
    if (((*sptyDdsPtr) = (struct spty_dds *) malloc(sizeof(struct spty_dds)))
        == NULL) {
        DEBUG_0("sptydds: Malloc of dds failed\n");
        return(E_MALLOC);
    }
    else {
        bzero((char *)(*sptyDdsPtr), sizeof(struct spty_dds));
    }
    
    /* ======================================== */
    /*  Get ATTNUM_ATT and BSDNUM_ATT attributes */
    /* ======================================== */
    if ((int)(pre_att_class = odm_open_class(PdAt_CLASS)) == -1) {
        DEBUG_0("sptydds: open class PdAt failed\n");
        return(E_ODMOPEN);
    };
    
    if ((int)(cus_att_class = odm_open_class(CuAt_CLASS)) == -1) {
        DEBUG_0("sptydds: open class CuAt failed\n");
        return(E_ODMOPEN);
    };
    
    if ((return_code = getatt(&ATT_count, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue,
                              ATTNUM_ATT, attList)) != 0)  {
        DEBUG_3("sptydds: getatt '%s' for %s fails with error %x\n",
                ATTNUM_ATT, cusDevPtr->name, return_code);
        return(return_code);
    }
    else {
        if (ATT_count >= MIN_ATTNUM) {
            (*sptyDdsPtr)->max_pts = ATT_count;
        }
        else {
            DEBUG_2("sptydds: %d is invalid for %s attribute\n",
                    ATT_count, ATTNUM_ATT);
            return(E_ATTRVAL);
        } /* End if (ATT_count >= MIN_ATTNUM) */
    } /* End if ((return_code = getatt(&ATT_count, ...)) != 0) */
    
    if ((return_code = getatt(&BSD_count, 'i', cus_att_class,
                              pre_att_class, cusDevPtr->name,
                              cusDevPtr->PdDvLn_Lvalue,
                              BSDNUM_ATT, attList)) != 0)  {
        DEBUG_3("sptydds: getatt '%s' for %s fails with error %x\n",
                BSDNUM_ATT, cusDevPtr->name, return_code);
        return(return_code);
    }
    else {
        if (BSD_count >= MIN_BSDNUM) {
            (*sptyDdsPtr)->max_ttyp = BSD_count;
        }
        else {
            DEBUG_2("sptydds: %d is invalid for %s attribute\n",
                    BSD_count, BSDNUM_ATT);
            return(E_ATTRVAL);
        } /* End if (BSD_count >= MIN_BSDNUM) */
    } /* End if ((return_code = getatt(&BSD_count, ...)) != 0) */
    
    /* Close ODM object classes */
    if (odm_close_class(pre_att_class) < 0) {
        DEBUG_0("sptydds: close object class PdAt failed\n");
        return(E_ODMCLOSE);
    };
    
    if (odm_close_class(cus_att_class) < 0) {
        DEBUG_0("sptydds: close object class CuAt failed\n");
        return(E_ODMCLOSE);
    };
    
    /* ================================= */
    /* Generate major, minor numbers for */
    /* pty device and pty dummy device */
    /* The PTY DDS devnos fields are */
    /* updated in this function call */
    /* ================================= */
    if ((return_code = generate_devnos(cusDevPtr->name, driverName,
                                       (*sptyDdsPtr), cloneDevnoPtr)) != 0) {
        DEBUG_0("sptydds: Generate of majors failed.\n");
        return(return_code);
    };
    
    /* ============= */
    /* For debug ... */
    /* ============= */
    DEBUG_0("sptydds: PTY dds:\n");
    DEBUG_2("\tptc_dev   = (%d,%d)\n", major((*sptyDdsPtr)->ptc_dev),minor((*sptyDdsPtr)->ptc_dev));
    DEBUG_2("\tpts_dev   = (%d,%d)\n", major((*sptyDdsPtr)->pts_dev),minor((*sptyDdsPtr)->pts_dev));
    DEBUG_1("\tmax_pts   = %d\n", (*sptyDdsPtr)->max_pts);
    DEBUG_2("\tptyp_dev  = (%d,%d)\n", major((*sptyDdsPtr)->ptyp_dev),minor((*sptyDdsPtr)->ptyp_dev));
    DEBUG_2("\tttyp_dev  = (%d,%d)\n", major((*sptyDdsPtr)->ttyp_dev),minor((*sptyDdsPtr)->ttyp_dev));
    DEBUG_1("\tmax_ttyp  = %d\n", (*sptyDdsPtr)->max_ttyp);
    
    return(0);
} /* End sptydds(...) */
