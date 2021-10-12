static char sccsid[] = "@(#)15	1.3  src/bos/usr/ccs/lib/libi18n/i18n.c, libi18n, bos41J, 9512A_all 3/17/95 10:13:18";
/*
 *   COMPONENT_NAME: LIBI18N
 *
 *   FUNCTIONS: layout_object_free
 *		layout_object_create
 *		layout_object_transform
 *		wcslayout_object_transform
 *		layout_object_editshape
 *		wcslayout_object_editshape
 *		layout_object_setvalue
 *		layout_object_getvalue
 *		layout_object_shapeboxchars
 *		instantiate
 *		nextpath
 *		
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

#include <sys/types.h>
#include <sys/lc_layout.h>
#include <sys/lc_layoutP.h>
#include <errno.h>
#include <stdio.h>

#define DEFAULTPATH "/usr/lib/nls/loc"

/*************************************************************************/
#ifdef AIX321
lc_layout *instantiate(char *path, lc_layout *(*inst)())
{
	lc_layout *q;

        if (inst == NULL) {
  	    return NULL;
   	}

	/* 
	** Invoke the instantiate method 
	** this used to be done in __lc_load.
	*/
	q = (*inst)();

	/*
	** Make sure the magic number is valid and that the type is 
	** correct for the object. If it is, return the object.
	*/
	if ((q->lc_hdr.__magic == _LC_MAGIC) && (q->lc_hdr.__type_id == _LC_LAYOUT)) {
	    return q;
	}
	else {
	    return NULL;
	}
}
#endif AIX321
/*************************************************************************/
/*
 * NAME: nextpath
 *
 * FUNCTION:
 *	Get the next path element from the given locpath.
 *	The result followed by '/' is stored in the buffer,
 *	and locpath pointer is advanced.
 *
 * RETURN VALUE DESCRIPTION:
 *	Length of the next path element.
 */
/*************************************************************************/
int nextpath(char **locpath, char *buf)
{
	char	*locptr;
	int	len;


	locptr = *locpath;
	while (*locptr && *locptr != ':') locptr++;
	len = locptr - *locpath;
	if (len > _POSIX_PATH_MAX - 1)
		return -1;
	if (len == 0) {
		locptr++;
		buf[0] = '.';
		len = 1;
	}
	else {
		if (*locptr && !*++locptr)
			locptr--;
		(void)memcpy(buf, *locpath, len);
	}
	buf[len++] = '/';
	*locpath = locptr;
	return len;
}

/***********************************************************************/
/* plh layout_object_create : This functions takes as input the locale name. */
/* It concatinates it to the name of the BIDI module, and loads this   */
/* module. If the load is successful, it executes the entry point to   */
/* the module which in turn sets up the addresses of all the other     */
/* entry points.							*/
/***********************************************************************/

int layout_object_create(locale_name,plh)
const char *locale_name;
LayoutObjectP  *plh;
{
         char *locpath;
         char *defpath;
         char *path;
         char *modulename;
         int pathlen;
         lc_layout *entry;
         int loop;
         long *Address;

       /*Check uid and gid, and set an appropreate search
	 path to find a file which will be loaded.  */
	defpath = locpath = NULL;
	if (!__issetuid())
		locpath = getenv("LOCPATH");
	if (!locpath || !*locpath)
		defpath = locpath = DEFAULTPATH;
        modulename=malloc(strlen(locpath)+strlen(locale_name)+10);
        path=malloc(strlen(locpath));
         while (1)
         {
	   while(*locpath) 
           {
		/* Get the next element in locpath */
		pathlen = nextpath(&locpath, path);
		if (pathlen < 0 || _POSIX_PATH_MAX < pathlen + 
                                                     strlen(locale_name)+6) 
                {
                   free(modulename);
                   free(path);
		   return (EINVAL);  /* failed */
                }

                /* append locale_name to path */
                memcpy(modulename,path,pathlen);
		(void)strcpy(modulename + pathlen, locale_name);

                /* append .layout.o to module name */
                strcat(modulename,".layout.o");
	        /* try to load it.  */
#ifdef AIX321
                entry=(lc_layout *)__lc_load(modulename,instantiate);
#else 
                entry=(lc_layout *)__lc_load(modulename,0);
#endif AIX321

                 /* if successfully loaded */
		 if (entry!=NULL) 
                 {
                    free(modulename);
                    free(path);
                    *plh=(LayoutObjectP)((*(entry)->initialize)());
	            return (0);  /* successfull */
                 }
           }
        
         /* if not found in LOCPATH, try the default */
         if (!defpath)
           defpath = locpath = DEFAULTPATH;
         else
            break;
         }
     /* if not found anywhere, resort to default functions */
       free(modulename);
       free(path);
       *plh=(LayoutObjectP)DefaultOpen();
       return (0); 
}

/***********************************************************************/
int	layout_object_free(plh)
LayoutObjectP plh;

{
        if (plh->methods->Close)
   	  (*plh->methods->Close)(plh);
        else free(plh);
	return;
}

/**********************************************************************/
/* Following are all the Bidi functions, actually these functions here*/
/* do nothing, except call the functions in the loaded object, the     */
/* addresses of these function were filled in structure plh by   */
/* routine layout_object_create.                                       */
/**********************************************************************/

int layout_object_transform (LayoutObjectP plh,
                     const char *InpBuf  ,
		     size_t *InpSize,
                     void *OutBuf  ,
                     size_t  *OutSize   ,
		     size_t *ToTarget,
		     size_t *ToSource,
		     unsigned char *BidiLvl)
{  
   return((int)(*plh->methods->Transform)(plh,InpBuf,InpSize,
                                 OutBuf,OutSize,
                                 ToTarget,ToSource,BidiLvl));
}

/**********************************************************************/

int wcslayout_object_transform (LayoutObjectP plh,
                     const wchar_t *InpBuf  ,
                     size_t *InpSize,
                     void *OutBuf  ,
                     size_t  *OutSize   ,
                     size_t *ToTarget,
                     size_t *ToSource,
                     unsigned char *BidiLvl)
{  
   return((int)(*plh->methods->wcsTransform)(plh,InpBuf,InpSize,
                                 OutBuf,OutSize,
                                 ToTarget,ToSource,BidiLvl));
}

/**********************************************************************/

int layout_object_editshape (LayoutObjectP plh,
             	     BooleanValue EditType,
             	     size_t *index, 
		     const char *InpBuf,
		     size_t *InpSize,
		     void *OutBuf,
		     size_t *OutSize)
{
  return((int)(*plh->methods->EditShape)(plh,EditType,index,
                                InpBuf,InpSize,
                                OutBuf,OutSize));
}

/**********************************************************************/

int wcslayout_object_editshape (LayoutObjectP plh,
                     BooleanValue EditType,
                     size_t *index,
                     const wchar_t *InpBuf,
                     size_t *InpSize,
                     void *OutBuf,
                     size_t *OutSize)
{
  return((int)(*plh->methods->wcsEditShape)(plh,EditType,index,
                                InpBuf,InpSize,
                                OutBuf,OutSize));
}

/**********************************************************************/

int layout_object_shapeboxchars(LayoutObjectP plh,
			const char *InpBuf,
			const size_t InpSize,
			char *OutBuf)
{
  return((int)(*plh->methods->ShapeBoxChars)(plh->core->Values,
              InpBuf, InpSize, OutBuf));
}

/**********************************************************************/

int layout_object_setvalue (LayoutObjectP plh,
		            LayoutValues values,
                            int *index_returned)
{
  return((int)(*plh->methods->SetValues)(plh,values, index_returned));
}

/**********************************************************************/

int layout_object_getvalue (LayoutObjectP plh,
                            LayoutValues values,
                            int *index_returned)
{
return((int)(*plh->methods->GetValues)(plh,values, index_returned));
}

