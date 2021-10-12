static char sccsid[] = "@(#)86  1.13  src/bos/usr/ccs/lib/libodm/odmfind.c, libodm, bos411, 9434B411a 8/22/94 17:34:23";
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: breakcrit, raw_find_obj
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <mbstr.h>

#include <odmi.h>
#include "odmlib.h"
#include "odmtrace.h"
#include "odmhkids.h"

extern int odmerrno;
extern char *odmcf_errstr;
extern char repospath[];
extern int odmcf_perms_op;
extern int errno;

#define INVALID -1
#define FALSE 0
#define TRUE 1

#define RELOPS 8

#define RELOP_EQ 0
#define RELOP_NE 1
#define RELOP_LE 2
#define RELOP_GE 3
#define RELOP_LT 4
#define RELOP_GT 5
#define RELOP_LIKE 6
#define RELOP_like 7

struct relop {
    char *string;
    int len;
} relop[RELOPS] =
{
    { "=",1 },
    { "!=",2 },
    { "<=",2 },
    { ">=",2 },
    { "<",1 },
    { ">",1 },
    { "LIKE",4 },
    { "like",4 }
};

#define ODMI_NUMERIC_TYPE(a) (a == ODM_SHORT || a == ODM_LONG || a == ODM_DOUBLE)
#define ODMI_STRING_TYPE(a) (a == ODM_CHAR || a == ODM_LONGCHAR || \
        a == ODM_METHOD || a == ODM_LINK || a == ODM_VCHAR)
#define ODMI_NUMERIC_RELN(a) (a == RELOP_EQ || a == RELOP_NE || a == RELOP_GT \
        || a == RELOP_GE || a == RELOP_LT || a == RELOP_LE)
#define ODMI_STRING_RELN(a) (a == RELOP_EQ || a == RELOP_NE || a == RELOP_LIKE \
        || a == RELOP_like)

/*
long strtol();
double strtod();
*/

/*
 * NAME: breakcrit
 *
 * FUNCTION:
 *
 *       Processes a SQL-like criteria which selects objects in the
 *       database.  Presently only takes "=","!=","like" and
 *       "and" and "AND" conjunctions.
 *
 *        Example:
 *
 *              "device_name like 'printer*' and device_num != 5"
 *
 * RETURNS:
 *
 *      A pointer to an array of 'Crit' structures with describe the
 *      search criteria if successful, -1 otherwise.
 */

struct Crit *
breakcrit(crit,myclass,pngot)
char *crit;              /* Criteria to parse                      */
int *pngot;              /* Number of criteria structures returned */
struct Class *myclass;   /* Pointer to the class description.       */
{
    int i,ngot,relation,valid;
    struct Crit *c;
    struct Crit *base = NULL;
    struct ClassElem *thiselem;
    char *cp,*cpo,*rq,*first_invalid;
    char element[256],value[256],conjunction[256];
    double a;
    int character_index;            /* Used for NLS characters */
    int character_length;

    START_ROUTINE(ODMHKWD_BREAKCRIT);
    TRC("breakcrit","Looking through criteria %s",crit,"","");
    if (verify_class_structure(myclass) < 0)
      {
        TRC("breakcrit","Invalid class structure!","","","");
        STOP_ROUTINE;
        return((struct Crit *) -1);
      } /* endif */

    if (pngot == NULL)
      {
        TRC("breakcrit","NULL pgnot!","","","");
        STOP_ROUTINE;
        return((struct Crit *) -1);

      }
    else
      {
      } /* endif */
    /*
        fprintf(stderr,"class %s : crit %s\n",
                myclass->classname,crit);
*/
    if (crit == NULL)
      {
        TRC("breakcrit","NULL criteria ptr","","","");
        *pngot  = 0;
        STOP_ROUTINE;
        return(NULL);
      } /* endif */

    cp =  crit;
    ngot = 0;
    base = NULL;
loop:
    while(*cp == ' ' || *cp == '\t' )cp++;
    if(!*cp)
      {
        *pngot = ngot;
        TRC("breakcrit","Elements found %d",ngot,"","");
        STOP_ROUTINE;
        return(base);
      }

    if(ngot)
      {
        cpo = conjunction;
        while(*cp != ' ' && *cp != '\t' && *cp)  {
            if ((character_length = mblen(cp,MB_CUR_MAX)) == 1)
              {
                *cpo++ = *cp;
                cp++;
              }
            else if (character_length < 0)
		break;
            else
              {
                for (character_index = 0;
                    character_index < character_length;
                    character_index++ )
                  {

                    *cpo++ = *cp;
                    cp++;
                  } /* endfor */
              } /* endif */
          } /* endwhile */

        *cpo = '\0';
        if(!*cp)
          {
            TRC("breakcrit","Conjunction only!","","","");
            odmcf_errstr = "incomplete crit - conjunction only";
            odmerrno = ODMI_BAD_CRIT;
            if (base)
                free((void *) base);
            STOP_ROUTINE;
            return((struct Crit *)-1);
          }

        while(*cp == ' ' || *cp == '\t' )cp++;
        if(!*cp)
          {
            TRC("breakcrit","Only conjunction after whitespace!",
                "","","");
            odmcf_errstr = "incomplete crit - conjunction only";
            odmerrno = ODMI_BAD_CRIT;
            if (base)
                free((void *) base);
            STOP_ROUTINE;
            return((struct Crit *)-1);
          }

        if(strcmp(conjunction,"and") && strcmp(conjunction,"AND"))
          {
            TRC("breakcrit","Bad conjunction %s!",conjunction,
                "","");

            odmcf_errstr = "bad crit:only and conj'n implemented";
            odmerrno = ODMI_BAD_CRIT;
            if (base)
                free((void *) base);
            STOP_ROUTINE;
            return((struct Crit *)-1);
          }

      } /* endif ngot */

    TRC("breakcrit","Element is %s",element,"","");

    cpo = element;
    while(*cp != ' ' && *cp != '\t' && *cp != '=' && *cp != '!'
        && *cp != '<' && *cp != '>' && *cp)
      {
        if ((character_length = mblen(cp,MB_CUR_MAX)) == 1)
          {
            *cpo++ = *cp;
            cp++;
          }
        else if (character_length < 0)
	     break;
        else
          {
            for (character_index = 0;
                character_index < character_length;
                character_index++ )
              {

                *cpo++ = *cp;
                cp++;
              } /* endfor */
          } /* endif */
      } /* endwhile */

    *cpo = '\0';
    TRC("breakcrit","Element is %s",element,"","");
    /* this is just temporary - need to check while parsng*/
    if(strlen(element)>= (MAX_CRITELEM_LEN - 1))
      {
        TRC("breakcrit","Element too long %s!",element,"","");
        odmcf_errstr = "element too long";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }
    if(!*cp)
      {
        TRC("breakcrit","Element only!","","","");

        odmcf_errstr = "incomplete criterion - element only";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }

    while(*cp == ' ' || *cp == '\t' )cp++;
    if(!*cp)
      {
        TRC("breakcrit","Element only after whitespace!","","","");
        odmcf_errstr = "incomplete criterion - element only";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }

    for(i=0;i<RELOPS;i++)
        if(!strncmp(cp,relop[i].string,relop[i].len))
          {
            TRC("breakcrit","Relationship %s",relop[i].string,
                "index %d",i);
            cp += relop[i].len;
            relation = i;
            break;
          }
    if(i==RELOPS)
      {
        TRC("breakcrit","Unknow relation %s",cp,"","");
        odmcf_errstr = "unknown relation only";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }

    if(!*cp)
      {
        TRC("breakcrit","Relation only!","","","");
        odmcf_errstr = "incomplete criterion - relation only";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }

    while(*cp == ' ' || *cp == '\t' )cp++;
    if(!*cp)
      {
        TRC("breakcrit","Relation only after white!","","","");
        odmcf_errstr = "incomplete criterion - relation only";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }

    if(*cp == '\'')
      {
        rq = strchr(cp+1,'\'');

        if (rq == 0)
          {
            TRC("breakcrit","cannot find matching quote %s",cp,"","");

            odmcf_errstr = "no ending quote";
            odmerrno = ODMI_BAD_CRIT;
            if (base)
                free((void *) base);
            STOP_ROUTINE;
            return((struct Crit *)-1);
          }

        if((rq - cp -1)>= (MAX_CRITELEM_LEN - 1))
          {

            TRC("breakcrit","string value too long %s",cp,"","");

            odmcf_errstr = "value too long";
            odmerrno = ODMI_BAD_CRIT;
            if (base)
                free((void *) base);
            STOP_ROUTINE;
            return((struct Crit *)-1);
          }

        strncpy(value,cp+1,rq-cp -1);
        value[rq-cp-1] = '\0';
        cp = rq + 1;
      } /* end if */
    else
      {
        cpo = value;
        while(*cp != ' ' && *cp != '\t' && *cp)  {
            if ((character_length = mblen(cp,MB_CUR_MAX)) == 1)
              {
                *cpo++ = *cp;
                cp++;
              }
	    else if (character_length < 0)
	         break;
            else
              {
                for (character_index = 0;
                    character_index < character_length;
                    character_index++ )
                  {

                    *cpo++ = *cp;
                    cp++;
                  } /* endfor */
              } /* endif */
          } /* endwhile */
        *cpo = '\0';
      } /* end else */

    ngot++;
    TRC("breakcrit","Value is %s",value,"ngot is %d",ngot);

    if(ngot == 1)
        base =  (struct Crit *)
            malloc( sizeof (struct Crit));
    else
        base=   (struct Crit *)
            realloc(base,ngot * sizeof(struct Crit));

    if (base == NULL)
      {
        TRC("breakcrit","Could not allocate space! %d",errno,"","");
        odmerrno = ODMI_MALLOC_ERR;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *) -1);
      } /* endif */

    c = base + (ngot -1);

    TRC("breakcrit","Checking for column %s",element,"","");

    for(i=0;i<myclass->nelem;i++)
      {
        if(!strcmp((myclass->elem +i)->elemname,element))
          {
            break;
          }
      }

    if(i == myclass->nelem)
      {
        TRC("breakcrit","Invalid column!","","","");
        odmcf_errstr = "No such column";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }


    thiselem = myclass->elem + i;
    c->type = thiselem->type;
    c->offset = thiselem->offset;
    valid = FALSE;

    if(c->type == ODM_LINK)
        c->offset += LINK_VAL_OFFSET;

    c->relation = relation;
    strcpy(c->name,thiselem->elemname);

    if(ODMI_NUMERIC_TYPE(c->type) && ODMI_NUMERIC_RELN(relation))
      {
        TRC("breakcrit","Numeric type and numberic relationship","","","");
        valid = TRUE;
        first_invalid = NULL;
        switch(c->type)
          {
        case ODM_SHORT:
        case ODM_LONG:
            *(long *) c->value = (long) strtol(value,&first_invalid,0);
            if(*first_invalid)
              {
                TRC("breakcrit","Could not strtol! %s",value,"","");
                odmcf_errstr = "invalid numeric value";
                odmerrno = ODMI_BAD_CRIT;
                if (base)
                    free((void *) base);
                STOP_ROUTINE;
                return((struct Crit *) -1 );
                /*ret_err((struct Crit *),ODMI_BAD_CRIT);*/
              }
            break;
        case ODM_ULONG:
            *(unsigned long *) c->value = (unsigned long) strtoul(value,&first_invalid,0);
            if(*first_invalid)
              {
                TRC("breakcrit","Could not strtoul! %s",value,"","");
                odmcf_errstr = "invalid numeric value";
                odmerrno = ODMI_BAD_CRIT;
                if (base)
                    free((void *) base);
                STOP_ROUTINE;
                return((struct Crit *) -1 );
                /*ret_err((struct Crit *),ODMI_BAD_CRIT);*/
              }
            break;
        case ODM_DOUBLE:
            a = strtod(value,&first_invalid);
            *(double *) c->value = a;
            if(*first_invalid)
              {
                TRC("breakcrit","Could not strtod! %s",value,"","");
                odmcf_errstr = "invalid float value";
                odmerrno = ODMI_BAD_CRIT;
                if (base)
                    free((void *) base);
                STOP_ROUTINE;
                return((struct Crit *) -1);
                /*ret_err((struct Crit *),ODMI_BAD_CRIT);*/
              }
            break;
          } /* end switch */
      }
    else if(ODMI_STRING_TYPE(c->type) && ODMI_STRING_RELN(relation))
      {
        TRC("breakcrit","String type and string relation ","","","");
        valid = TRUE;
        strcpy(c->value,value);
      }

    if(!valid)
      {
        TRC("breakcrit","Relation type mismatch! type %d",c->type,
            "relation %d",relation);
        odmcf_errstr = "type - relation mismatch";
        odmerrno = ODMI_BAD_CRIT;
        if (base)
            free((void *) base);
        STOP_ROUTINE;
        return((struct Crit *)-1);
      }
    goto loop;


}
/*
 *    NAME:     raw_find_obj
 *    FUNCTION: find the offset of the (first,next) object meeting the
 *              criterion.
 *
 *    RETURNS: a pointer to the object if sucessful, -1 otherwise.
 */
char *
raw_find_obj(classp,critstr,first)
struct Class *classp;    /* Pointer to the class description */
char *critstr;           /* The selection criteria.          */
int first;              /* Boolean indicating get the first or next object */
{
    int i,type,result,ncrit,expr,relation;
    int offset;
    short id;
    char *p,*pend;
    struct Crit *breakcrit(),*cr;
    char *value;
    long dboffs;
	char *vchar_string;
    short s,z;
    long l,k;
    double a,b;

    START_ROUTINE(ODMHKWD_RAW_FIND_OBJ);
    TRC("raw_find_obj","Finding objects based on %s",critstr,
        "first %d",first);

    if (verify_class_structure(classp) < 0)
      {
        TRC("raw_find_obj","Invalid class structure!","","","");
        STOP_ROUTINE;
        return((char *)-1);
      } /* endif */

    /* if this is a "first" get, ... */
    if(first)
      {
        TRC("raw_find_obj","This is the first","","","");

        classp->current = 0;

        /* if this is the first "first" search for this class,
                           or the last one had a null criterion, preprocess the
                           new criterion and save it in per-class data struct*/

        if(!classp->crit )
          {
            TRC("raw_find_obj","classp->crit is NULL","","","");

            cr = breakcrit(critstr,classp,&classp->ncrit);
            if ( (int) cr == -1 )
              {
                TRC("raw_find_obj","Could not breakcrit! err %d",
                    odmerrno,"","");
                STOP_ROUTINE;
                return((char *) -1);
              }
            /*if_err_ret_err(cr,(char *),0);*/
	    /* if critstr > MAX_ODMI_CRIT, don't save
	       the critstr.  This is done to allow the
	       crit string greater than 255. Can't malloc
	       string because it will break bin compatibility */

	    if ( strlen(critstr) < MAX_ODMI_CRIT )
          	  strcpy(classp->critstring,critstr);
	    else classp->critstring[0] = '\0';
            classp->crit = cr;
          }

        /* otherwise only preprocess and save the criterion if
                            it's different from the last new criterion*/
        else if (classp->crit && strcmp(classp->critstring,critstr))
          {
            TRC("raw_find_obj","Criteria is different than last",
                "","","");
            free((void *) classp->crit);
	    classp->crit = NULL;  /* p43355 */
            cr = breakcrit(critstr,classp,&classp->ncrit);
            if ( (int) cr == -1 )
              {
                TRC("raw_find_obj","Could not breakcrit 2! err %d",
                    odmerrno,"","");
                STOP_ROUTINE;
                return((char *) -1);
              }
            /*if_err_ret_err(cr,(char *),0);*/
	    /* if critstr > MAX_ODMI_CRIT, don't save
	       the critstr.  This is done to allow the
	       crit string greater than 255. Can't malloc
	       string because it will break bin compatibility */
	    if ( strlen(critstr) < MAX_ODMI_CRIT )
          	  strcpy(classp->critstring,critstr);
	    else classp->critstring[0] = '\0';
            classp->crit = cr;
          }

      } /* endif first */
    cr = (struct Crit *)classp->crit;
    ncrit = classp->ncrit;
    TRC("raw_find_obj","Num crit structures %d",ncrit,"","");

    /* last get was valid on last object in db -
                           or objects were deleted since last get
                           so this time return NULL. */


    if(classp->current >= classp->hdr->ndata)
      {
        TRC("raw_find_obj","Current %d greater ",classp->current,
            "than ndata %d, returning",classp->hdr->ndata);
        classp->current = 0;
        STOP_ROUTINE;
        return(NULL);
      }


    /* prepare pointers for stepping through the objects */

    p = classp->data + classp->current * classp->structsize;
    pend = classp->data + classp->hdr->ndata * classp->structsize;
    TRC("raw_find_obj","Looking through objects","","","");

    /* loop which evaluates the crit, one expression */
    /* at a time */
    do
      {
        result = TRUE;
        id =  *(long *) p;

        if(id == (long) -1 )
		continue;

        for(i=0;i<ncrit;i++)
          {
            offset = (cr+i)->offset;
            type =   (cr+i)->type;
            relation =   (cr+i)->relation;

            switch(type)
              {
  	    case ODM_VCHAR:
                value = (cr+i)->value;
	        dboffs = *(long *) (p+offset);
	        vchar_string = classp->clxnp->data + dboffs;

                switch(relation)
                  {
                case RELOP_like:
                case RELOP_LIKE:
                    expr = cmpkmch(value,vchar_string);
                    break;
                case RELOP_EQ:
                    expr = !strcmp(value,vchar_string);
                    break;
                case RELOP_NE:
                    expr = strcmp(value,vchar_string);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  } /* endswitch */
                break;

            case ODM_CHAR:
            case ODM_LONGCHAR:
            case ODM_LINK:
            case ODM_METHOD:
               value = (cr+i)->value;

                switch(relation)
                  {
                case RELOP_like:
                case RELOP_LIKE:
                    expr = cmpkmch(value,p+offset);
                    break;
                case RELOP_EQ:
                    expr = !strcmp(value,p+offset);
                    break;
                case RELOP_NE:
                    expr = strcmp(value,p+offset);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  } /* endswitch */
                break;
            case ODM_SHORT:
                s = *(short *)(p + offset);
                z = (short)(*(long *)((cr+i)->value));
                switch(relation)
                  {
                case RELOP_EQ:
                    expr = (s == z);
                    break;
                case RELOP_NE:
                    expr = (s != z);
                    break;
                case RELOP_LT:
                    expr = (s < z);
                    break;
                case RELOP_GT:
                    expr = (s > z);
                    break;
                case RELOP_LE:
                    expr = (s <= z);
                    break;
                case RELOP_GE:
                    expr = (s >= z);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  } /* endswitch */
                break;
            case ODM_LONG:
                l = *(long *)(p + offset);
                k = *(long *)((cr+i)->value);
                switch(relation)  {
                case RELOP_EQ:
                    expr = (l == k);
                    break;
                case RELOP_NE:
                    expr = (l != k);
                    break;
                case RELOP_LT:
                    expr = (l < k);
                    break;
                case RELOP_GT:
                    expr = (l > k);
                    break;
                case RELOP_LE:
                    expr = (l <= k);
                    break;
                case RELOP_GE:
                    expr = (l >= k);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  }
                break;
            case ODM_ULONG:
                l = *(unsigned long *)(p + offset);
                k = *(unsigned long *)((cr+i)->value);
                switch(relation)  {
                case RELOP_EQ:
                    expr = (l == k);
                    break;
                case RELOP_NE:
                    expr = (l != k);
                    break;
                case RELOP_LT:
                    expr = (l < k);
                    break;
                case RELOP_GT:
                    expr = (l > k);
                    break;
                case RELOP_LE:
                    expr = (l <= k);
                    break;
                case RELOP_GE:
                    expr = (l >= k);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  }
                break;
            case ODM_DOUBLE:
                a = *(double *)(p + offset);
                b = *(double *)((cr+i)->value);
                switch(relation)  {
                case RELOP_EQ:
                    expr = (a == b);
                    break;
                case RELOP_NE:
                    expr = (a != b);
                    break;
                case RELOP_LT:
                    expr = (a < b);
                    break;
                case RELOP_GT:
                    expr = (a > b);
                    break;
                case RELOP_LE:
                    expr = (a <= b);
                    break;
                case RELOP_GE:
                    expr = (a >= b);
                    break;
                default:
                    TRC("raw_find_obj",
                        "Unknown relation! %d",relation,
                        "","");
                    odmerrno = ODMI_INTERNAL_ERR;
                    STOP_ROUTINE;
                    return((char *) -1 );
                    /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                  }
                break;
            default: /*ret_err((char *),ODMI_INTERNAL_ERR);*/
                TRC("raw_find_obj",
                    "Unknown type! %d",type,
                    "","");
                odmerrno = ODMI_INTERNAL_ERR;
                STOP_ROUTINE;
                return((char *) -1 );
              }
            result = result && expr;
   	TRC("raw_find_obj","result found=%d",result,"","");

            /* since and is the only conjunction supported we can */
            /* stop when result for this object is false */
            if(!result)
		break;
   	/* TRC("raw_find_obj","End of for","","",""); */
          } /* end for */

        if(result)
          {
            classp->current++;
            TRC("raw_find_obj","Found a match, current %d",classp->current,
                "","");
            STOP_ROUTINE;
            return(p);
          }
        /* stop when result for this object is false */
      } while(classp->current++,p += classp->structsize ,p != pend);


    if(classp->current == classp->hdr->ndata)
        classp->current = 0;

    TRC("raw_find_obj","No match, returning","","","");
    STOP_ROUTINE;
    return(NULL);
}
