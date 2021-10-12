/* @(#)62	1.8  src/bos/usr/ccs/lib/libim/mapping.c, libim, bos411, 9428A410j 3/24/94 05:41:05 */
/*
 * COMPONENT_NAME :	LIBIM - AIX Input Method
 *
 * FUNCTIONS :		AIX Input Method library
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include "im.h"
#include "imP.h"
#include "deadtable.h"
#include <ctype.h>

extern int	imerrno;

#define	BUF_ALLOC_UNIT	256	/* has to be larger than 256 */

static void	placebyte(IMBuffer *imb, unsigned char c)
{
	if (imb->len >= imb->siz) {
		imb->siz += BUF_ALLOC_UNIT;
		imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
	}
	imb->data[imb->len++] = c;
}

static void	placestr(IMBuffer *imb, unsigned char *str, int len)
{
	if (imb->len + len >= imb->siz) {
		imb->siz = ((imb->len + len) / BUF_ALLOC_UNIT + 1) *
			BUF_ALLOC_UNIT;
		imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
	}
	memcpy(&imb->data[imb->len], str, len);
	imb->len += len;
}

static int	getkeysymindex(IMKeymap *immap, unsigned int keysym)
{
	int	high, mid, low;
	unsigned int	*ksym;

	ksym = immap->ksym;
	high = immap->nksym;
	low = 0;
	while (high >= low) {
		mid = (high + low) / 2;
		if (ksym[mid] < keysym)
			low = mid + 1;
		else if (ksym[mid] > keysym)
			high = mid - 1;
		else 
			return mid;
	}
	return -1;
}

static KeyMapElement	*imlookuprealkeymap(IMKeymap *immap, 
	unsigned int keysym, unsigned int state)
{
	int	keysymindex;
	int	stateindex;

	if (state >= immap->maxstat)
		return NULL;
	keysymindex = getkeysymindex(immap, keysym);
	if (keysymindex == -1)
		return NULL;
	stateindex = immap->stat[state];
	if (stateindex == -1)
		return NULL;
	return &immap->elmt[stateindex * immap->nksym + keysymindex];
}

KeyMapElement	*_IMLookupKeymap(IMKeymap *immap, 
	unsigned int keysym, unsigned int state)
{
	KeyMapElement	*kme;
	static KeyMapElement	unboundkme = {KME_UNBOUND};

	kme = imlookuprealkeymap(immap, keysym, state);
	if (!kme)
		kme = &unboundkme;
	return kme;
}

/*
 *	Compatibility with AIX3.1 
 *	Create a new in-core imkeymap data structure from an old
 *	version imkeymap file.
 *	fill_func(), addtooksym(), placekeysym(), placedead(),
 *	fill_sstr(), addtostr(), convert_to_new()
 */

static int	fill_func(KeyMapElement *kme, unsigned int func_id)
{
	switch (func_id) {
	case 0x101:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'A', '\0');
		break;
	case 0x102:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'B', '\0');
		break;
	case 0x103:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'C', '\0');
		break;
	case 0x104:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'D', '\0');
		break;
	case 0x105:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'Z', '\0');
		break;
	case 0x106:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'Y', '\0');
		break;
	case 0x107:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'I', '\0');
		break;
	case 0x108:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'H', '\0');
		break;
	case 0x10b:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'F', '\0');
		break;
	case 0x10c:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'E', '\0');
		break;
	case 0x151:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'P', '\0');
		break;
	case 0x152:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'L', '\0');
		break;
	case 0x153:	kme->type = KME_ESEQ2;
		PUT3BYTES(kme->data, '[', 'M', '\0');
		break;
	case 0x154:	kme->type = KME_ESEQ3;
		PUT3BYTES(kme->data, '[', '0', 'K');
		break;
	case 0x155:	kme->type = KME_ESEQ3;
		PUT3BYTES(kme->data, '[', '0', 'N');
		break;
	case 0x156:	kme->type = KME_ESEQ3;
		PUT3BYTES(kme->data, '[', '2', 'J');
		break;
	case 0x157:	kme->type = KME_ESEQ1;
		PUT3BYTES(kme->data, 'c', '\0', '\0');
		break;
	case 0x162:	kme->type = KME_ESEQ1;
		PUT3BYTES(kme->data, 'L', '\0', '\0');
		break;
	case 0x163:	kme->type = KME_ESEQ1;
		PUT3BYTES(kme->data, 'D', '\0', '\0');
		break;
	default:
		return False;
	}
	return True;
}

static int	addtooksym(IMKeymap *immap, unsigned int keysym, int idx)
{
#define	OKSYM_ALLOC_UNIT	512
	if (idx >= immap->oksymsiz) {
		immap->oksymsiz += OKSYM_ALLOC_UNIT;
		immap->oksym = REALLOC(immap->oksym, immap->oksymsiz);
	}
	immap->oksym[idx++] = keysym;
	return idx;
}

static int	placekeysym(IMKeymap *immap,
	KeyMapElement *kme, unsigned int keysym, int idx)
{
	if ((keysym & 0xff000000) == AIX_PRIVKEYSYM) {
		kme->type = KME_PRIVKEYSYM;
		PUT3DIGITS(kme->data, keysym);
	}
	else if ((keysym & 0xff000000)) {
		kme->type = KME_OTHERKEYSYM;
		idx = addtooksym(immap, keysym, idx);
		PUT3DIGITS(kme->data, idx);
	}
	else {
		kme->type = KME_KEYSYM;
		PUT3DIGITS(kme->data, keysym);
	}
	return idx;
}

static int	placedead(IMKeymap *immap,
	KeyMapElement *kme, OldKeyMapElt *okme, int idx)
{
	unsigned int	keysym;

	if (okme->key.page == P0) {
		switch (okme->key.point) {
		case 0xef: keysym = XK_dead_acute; break;
		case 0x27: keysym = XK_dead_acute; break; /* apostrophe */
		case 0x60: keysym = XK_dead_grave; break;
		case 0x5e: keysym = XK_dead_circumflex; break;
		case 0xf9: keysym = XK_dead_diaeresis; break;
		case 0x7e: keysym = XK_dead_tilde; break;
		case 0xf8: keysym = XK_dead_degree; break;
		case 0xf7: keysym = XK_dead_cedilla; break;
		}
		return placekeysym(immap, kme, keysym, idx);
	}
	else if (okme->key.page == P1) {
		switch (okme->key.point) {
		case 0x73: keysym = XK_dead_caron; break;
		case 0x9d: keysym = XK_dead_breve; break;
		case 0x9e: keysym = XK_dead_doubleacute; break;
		case 0x85: keysym = XK_dead_abovedot; break;
		case 0xa3: keysym = XK_dead_macron; break;
		case 0x87: keysym = XK_dead_ogonek; break;
		}
		return placekeysym(immap, kme, keysym, idx);
	}
	kme->type = KME_UNBOUND;
	PUT3BYTES(kme->data, '\0', '\0', '\0');
	return idx;
}

/*
 *	make a sstr (short string) if possible, otherwise return False.
 */
static int	fill_sstr(KeyMapElement *kme,
	unsigned char *ptr, unsigned int len)
{
	switch (len) {
	case 0:
		kme->type = KME_UNBOUND;
		PUT3BYTES(kme->data, '\0', '\0', '\0');
		return True;
	case 1:
		kme->type = KME_SSTR1;
		PUT3BYTES(kme->data, ptr[0], '\0', '\0');
		return True;
	case 2:
		if (ptr[1] == 0x1b) {
			kme->type = KME_ESEQ1;
			PUT3BYTES(kme->data, ptr[1], '\0', '\0');
		}
		else {
			kme->type = KME_SSTR2;
			PUT3BYTES(kme->data, ptr[0], ptr[1], '\0');
		}
		return True;
	case 3:
		if (ptr[1] == 0x1b) {
			kme->type = KME_ESEQ2;
			PUT3BYTES(kme->data, ptr[1], ptr[2], '\0');
		}
		else {
			kme->type = KME_SSTR3;
			PUT3BYTES(kme->data, ptr[0], ptr[1], ptr[2]);
		}
		return True;
	case 4:
		if (ptr[1] == 0x1b) {
			kme->type = KME_ESEQ3;
			PUT3BYTES(kme->data, ptr[1], ptr[2], ptr[3]);
			return True;
		}
		break;
	case 6:
		if (ptr[0] == 0x1b && ptr[1] == '[' &&
			isdigit(ptr[2]) && isdigit(ptr[3]) && isdigit(ptr[4])) {
			if (ptr[5] == 'q') {
				kme->type = KME_QPFK;
				PUT3BYTES(kme->data, ptr[2], ptr[3], ptr[4]);
				return True;
			}
			if (ptr[5] == 'z') {
				kme->type = KME_ZPFK;
				PUT3BYTES(kme->data, ptr[2], ptr[3], ptr[4]);
				return True;
			}
		}
		break;
	}
	return False;
}

static int	addtostr(IMBuffer *imb, unsigned char *ptr, unsigned int len)
{
#define	KSTR_ALLOC_UNIT	512
	int	idx;

	if (imb->len + len >= imb->siz) {
		imb->siz += ((len / KSTR_ALLOC_UNIT) + 1) * KSTR_ALLOC_UNIT;
		if (!(imb->data = REALLOC(imb->data, imb->siz)))
			return -1;
	}
	imb->data[imb->len] = len;
	memcpy(&imb->data[imb->len + 1], ptr, len);
	idx = imb->len;
	imb->len += len + 1;
	return idx;
}

static IMKeymap	*convert_to_new(OldIMKeymapFile *oldmap)
{
	IMKeymap	*immap;
	int	hsize, ssize, ksize, esize;
	unsigned int	maxstat;
	unsigned int	nstat;
	int	nksym;
	short	*stat;
	OldKeyMapElt	*elmt, *okme;
	unsigned char	*kstr;
	unsigned char	*ptr;
	KeyMapElement	*kme;
	int	idx;
	int	i, j;
	int	oksymidx;

	if (oldmap->Max_States == 0)
		maxstat = 32;
	else
		maxstat = oldmap->Max_States + 1;
	nstat = oldmap->max_state;
	stat = (short *)((char *)oldmap + sizeof (OldIMKeymapFile));
	nksym = oldmap->max_pos - oldmap->min_pos;
	elmt = (OldKeyMapElt *)((char *)oldmap + sizeof (OldIMKeymapFile) +
		(maxstat + maxstat % 2) * sizeof (short));
	kstr = (unsigned char *)elmt + nstat * nksym * sizeof (OldKeyMapElt);

	hsize = sizeof (IMKeymapFile) + sizeof (IMKeymap);
	ssize = (maxstat + maxstat % 2) * sizeof (short);
	ksize = nksym * sizeof (unsigned int);
	esize = nstat * nksym * sizeof (KeyMapElement);
	if (!(immap = (IMKeymap *)malloc(hsize + ssize + ksize + esize)))
		return NULL;
	immap->file = (IMKeymapFile *)((char *)immap + sizeof (IMKeymap));
	immap->stat = (short *)((char *)immap + hsize);
	immap->maxstat = maxstat;
	immap->ksym = (unsigned int *)((char *)immap->stat + ssize);
	immap->nksym = nksym;
	immap->elmt = (KeyMapElement *)((char *)immap->ksym + ksize);
	immap->nstat = nstat;
	immap->oksym = NULL;
	immap->oksymsiz = 0;
	immap->kstr.data = NULL;
	immap->kstr.len = 0;
	immap->kstr.siz = 0;
	immap->bstr.data = NULL;
	immap->bstr.len = 0;
	immap->bstr.siz = 0;
	immap->file->magic = oldmap->magic;
	/*
	 *	Make state mapping table.
	 */
	for (i = 0; i < maxstat; i++)
		immap->stat[i] = stat[i];
	/*
	 *	Make keysym table.
	 */
	for (i = oldmap->min_pos, j = 0; i < 256; i++, j++)
		immap->ksym[j] = i;
	for (; i < oldmap->max_pos; i++, j++)
		immap->ksym[j] = 0xff00 | i;
	/*
	 *	Make KeyMapElement table.
	 */
	oksymidx = 0;
	for (i = 0; i < nstat; i++) {
		kme = &immap->elmt[i * nksym];
		okme = &elmt[i * nksym];
		for (j = 0; j < nksym; j++) {
			switch (okme[j].key.type) {
			case GRAPHIC:
				if (okme[j].key.page != P0) {
					kme[j].type = KME_UNBOUND;
					PUT3BYTES(kme[j].data,
						'\0', '\0', '\0');
					break;
				}
				if (okme[j].key.stat == DEAD) {
					/* needs to be corrected */
					oksymidx = placedead(immap, &kme[j],
						&okme[j], oksymidx);
					break;
				}
				else {
					kme[j].type = KME_SSTR1;
					PUT3BYTES(kme[j].data,
						okme[j].key.point, '\0', '\0');
				}
				break;
			case SGL_CTL:
				kme[j].type = KME_SSTR1;
				PUT3BYTES(kme[j].data,
					okme[j].key.point, '\0', '\0');
				break;
			case CHAR_STR:
				ptr = kstr + okme[j].str.offset;
				if (fill_sstr(&kme[j], ptr + 1, *ptr))
					break;
				idx = addtostr(&immap->kstr, ptr + 1, *ptr);
				if (idx == -1) {
					kme->type = KME_UNBOUND;
					PUT3BYTES(kme->data, '\0', '\0', '\0');
					break;
				}
				kme->type = KME_LSTR;
				PUT3DIGITS(kme->data, idx);
				break;
			case ES_FUNC:
				if (!fill_func(&kme[j], okme[j].func.id)) {
					kme->type = KME_UNBOUND;
					PUT3BYTES(kme->data, '\0', '\0', '\0');
				}
				break;
			case CTLFUNC:
				if (okme[j].key.stat == NORM) {
					if (!fill_func(&kme[j],
						okme[j].func.id)) {
						kme->type = KME_UNBOUND;
						PUT3BYTES(kme->data,
							'\0', '\0', '\0');
					}
				}
				else {		/* CPFK */
					kme[j].type = KME_QPFK;
					PUT3DECIMAL(kme[j].data,
						okme[j].func.id + 1);
				}
				break;
			default:
				kme[j].type = KME_UNBOUND;
				PUT3BYTES(kme[j].data, '\0', '\0', '\0');
				break;
			}
		}
	}
	return immap;
}

/*
 *	_IMOpenKeymap
 */

IMKeymap	*_IMOpenKeymap(char *fname)
{
	int	fd;
	IMKeymap	*immap, *t_immap;
	struct stat	st;
	IMKeymapFile	*imfile;

	if ((fd = open(fname, O_RDONLY)) < 0)
		return NULL;
	(void)fstat(fd, &st);
	immap = (IMKeymap *)
		malloc(sizeof (IMKeymap) + (unsigned int)st.st_size);
	if (!immap) {
		close(fd);
		return NULL;
	}
	imfile = immap->file =
		(IMKeymapFile *)((char *)immap + sizeof (IMKeymap));
	if (read(fd, (char *)imfile, st.st_size) != st.st_size) {
		free(immap);
		close(fd);
		return NULL;
	}
	close(fd);
	switch (imfile->magic) {
	case RT256_KEYMAP_MAGIC:
	case AIX_KEYMAP_MAGIC:
		t_immap = convert_to_new((OldIMKeymapFile *)imfile);
		free(immap);
		immap = t_immap;
		break;

	case ILS_KEYMAP_MAGIC:
		immap->stat = (short *)((char *)imfile + imfile->stat);
		immap->maxstat = imfile->statsiz / sizeof (short);
		immap->ksym = (unsigned int *)((char *)imfile + imfile->ksym);
		immap->nksym = imfile->ksymsiz / sizeof (unsigned int);
		immap->elmt = (KeyMapElement *)((char *)imfile + imfile->elmt);
		immap->nstat = imfile->elmtsiz / imfile->ksymsiz;
		immap->oksym = (unsigned int *)((char *)imfile + imfile->oksym);
		immap->oksymsiz = imfile->oksymsiz / sizeof (unsigned int);
		immap->kstr.data = (char *)imfile + imfile->kstr;
		immap->kstr.len = imfile->kstrsiz;
		immap->kstr.siz = imfile->kstrsiz;
		immap->bstr.data = NULL;
		immap->bstr.len = 0;
		immap->bstr.siz = 0;
		break;

	default:
		/*
		 *	Unknown type.
		 */
		free(immap);
		return NULL;
	}
	return immap;
}

/*
 *	copy from 'f' to 't'.  remove "im=" if "@im=" is found in 'f'.
 */
static void	strcpyremoveequal(unsigned char *t, unsigned char *f)
{
	while (*t++ = *f)
		if (*f == '@' && !memcmp(f, "@im=", 4))
			f += 4;
		else
			f++;
}

/*
 *	_IMAreYouThere()
 */
char	*_IMAreYouThere(IMLanguage lang, char *postfix)
{
	char	*pathname;
	char	*locpath, *locptr;
	int	len;

	if (!lang) {
		imerrno	= IMNoSuchLanguage;
		return NULL;
	}

	locpath = NULL;
	if (!__issetuid())
		locpath = getenv("LOCPATH");
	if (!locpath || !*locpath)
		locpath = IMDIRECTORY;
	pathname = malloc(strlen(locpath) + strlen(lang) + strlen(postfix) +
		sizeof (IMDIRECTORY) + 3);
	do {
		locptr = locpath;
		while (*locptr && *locptr != ':') locptr++;
		len = locptr - locpath;
		if (len == 0) {
			locptr++;
			len = sizeof (IMDIRECTORY) - 1;
			locpath = IMDIRECTORY;
		}
		else {
			if (*locptr && !*++locptr)
				locptr--;
		}
		memcpy(pathname, locpath, len);
		pathname[len++] = '/';
		strcpyremoveequal(&pathname[len], lang);
		strcat(pathname, postfix);
		if (access(pathname, R_OK) >= 0)	/* have read access ? */
			return pathname;
		locpath = locptr;
	} while (*locpath);
	imerrno	= IMNoSuchLanguage;
	free(pathname);
	return NULL;
}

/*
 *	_IMInitializeKeymap()
 */
IMKeymap	*_IMInitializeKeymap(IMLanguage lang)
{
	static char	*paths[] = { NULL, NULL };
	IMKeymap	*immap = 0;
	char	*fullpath;
	caddr_t	*path;
	char	*getenv(char *);

	if (!lang)
		return NULL;

	if (!paths[0]) {
		paths[0] = getenv("IMKEYMAPPATH");
		paths[1] = getenv("HOME");
	}

	for (path = paths; path - paths != sizeof (paths) / sizeof (paths[0]);
		++path) {
		if (!*path) continue;
		fullpath = (char *)malloc((unsigned int)(strlen((char *)*path) +
			strlen((char *)lang) + sizeof (IMKMPOSTFIX) + 2));
		(void)sprintf(fullpath, "%s/%s/%s", *path, lang, IMKMPOSTFIX);
		immap = _IMOpenKeymap(fullpath);
		free((char *)fullpath);
		if (immap)
			break;
	}
	if (!immap) {
		fullpath = _IMAreYouThere(lang, IMKMPOSTFIX);
		if (fullpath)
			immap = _IMOpenKeymap(fullpath);
		free((char *)fullpath);
	}
	return immap;
}

/*
 *	_IMCloseKeymap
 */

void	_IMCloseKeymap(IMKeymap *immap)
{
	if (immap->bstr.data)
		free(immap->bstr.data);
	if (immap->file->magic == RT256_KEYMAP_MAGIC ||
		immap->file->magic == AIX_KEYMAP_MAGIC) {
		if (immap->kstr.data)
			free(immap->kstr.data);
		if (immap->oksym)
			free(immap->oksym);
	}
	free(immap);
}

/*
 *	_IMMapKeysym()
 */
void	_IMMapKeysym(IMKeymap *immap, unsigned int *keysym, unsigned int *state)
{
	KeyMapElement	*kme;

	/*
	 *	If input state is not BASE (i.e., not 0),
	 *	look up the imkeymap.
	 *	If the look-up'ed entry is a keysym, use it
	 *	as a BASE state keysym.
	 */
	kme = _IMLookupKeymap(immap, *keysym, *state);
	switch (kme->type) {
	case KME_KEYSYM:
		*keysym = GET3DIGITS(kme->data);
		*state = 0;
		break;
	case KME_PRIVKEYSYM:
		*keysym = AIX_PRIVKEYSYM + GET3DIGITS(kme->data);
		*state = 0;
		break;
	case KME_OTHERKEYSYM:
		*keysym = immap->ksym[GET3DIGITS(kme->data)];
		*state = 0;
		break;
	}
}

/*
 *
 *	_IMAIXMapping stuff
 *
 */

static int	NumPad(unsigned int keysym)
{
	switch (keysym) {
	case XK_KP_0: return 0; case XK_KP_1: return 1;
	case XK_KP_2: return 2; case XK_KP_3: return 3;
	case XK_KP_4: return 4; case XK_KP_5: return 5;
	case XK_KP_6: return 6; case XK_KP_7: return 7;
	case XK_KP_8: return 8; case XK_KP_9: return 9;
	}
	return -1;
}

static int	sweep_out(IMKeymap *immap, IMBuffer *imb, unsigned int keysym)
{
	KeyMapElement	*kme;

	/*
	 *	Here we try to translate a keysym to an encoded
	 *	character.   However, we don't know the encoded value
	 *	since it may be IBM-850 or ISO8859-1.
	 *	So lookup the current imkeymap.  It must have a correct
	 *	value (i.e, type must be KME_SSTR1).
	 *	If it can be translated to an encoded value, return True.
	 */
	kme = _IMLookupKeymap(immap, keysym, 0);
	if (kme->type == KME_SSTR1) {
		placebyte(imb, kme->data[0]);
		return True;
	}
	return False;
}

static int	IsDeadKeysym(unsigned int keysym)
{
	switch (keysym) {
	case XK_dead_acute:
	/* case XK_dead_apostrophe: */
	case XK_dead_grave:
	case XK_dead_circumflex:
	case XK_dead_diaeresis:
	case XK_dead_tilde:
	case XK_dead_caron:
	case XK_dead_breve:
	case XK_dead_doubleacute:
	case XK_dead_degree:
	case XK_dead_abovedot:
	case XK_dead_macron:
	case XK_dead_cedilla:
	case XK_dead_ogonek:
		return True;
	}
	return False;
}

static int	try_to_compose_keysym(IMKeymap *immap, IMBuffer *imb,
	unsigned int *dead_state, unsigned int keysym)
{
	const DeadTable	*cur;
	KeyMapElement	*kme;
	int	high, mid, low;

	switch (*dead_state) {
	case XK_dead_acute:
	/* case XK_dead_apostrophe: */
		cur = &acute[0];
		high = sizeof (acute) / sizeof (acute[0]);
		break;

	case XK_dead_grave:
		cur = &grave[0];
		high = sizeof (grave) / sizeof (grave[0]);
		break;

	case XK_dead_circumflex:
		cur = &circumflex[0];
		high = sizeof (circumflex) / sizeof (circumflex[0]);
		break;

	case XK_dead_diaeresis:
		cur = &diaeresis[0];
		high = sizeof (diaeresis) / sizeof (diaeresis[0]);
		break;

	case XK_dead_tilde:
		cur = &tilde[0];
		high = sizeof (tilde) / sizeof (tilde[0]);
		break;

	case XK_dead_caron:
		cur = &caron[0];
		high = sizeof (caron) / sizeof (caron[0]);
		break;

	case XK_dead_breve:
		cur = &breve[0];
		high = sizeof (breve) / sizeof (breve[0]);
		break;

	case XK_dead_doubleacute:
		cur = &doubleacute[0];
		high = sizeof (doubleacute) / sizeof (doubleacute[0]);
		break;

	case XK_dead_degree:
		cur = &degree[0];
		high = sizeof (degree) / sizeof (degree[0]);
		break;

	case XK_dead_abovedot:
		cur = &abovedot[0];
		high = sizeof (abovedot) / sizeof (abovedot[0]);
		break;

	case XK_dead_macron:
		cur = &macron[0];
		high = sizeof (macron) / sizeof (macron[0]);
		break;

	case XK_dead_cedilla:
		cur = &cedilla[0];
		high = sizeof (cedilla) / sizeof (cedilla[0]);
		break;

	case XK_dead_ogonek:
		cur = &ogonek[0];
		high = sizeof (ogonek) / sizeof (ogonek[0]);
		break;

	default: return False;
	}
	if (keysym == XK_space) {
		sweep_out(immap, imb, *dead_state);
		*dead_state = XK_VoidSymbol;
		return True;
	}
	low = 0;
	while (high >= low) {
		mid = (high + low) / 2;
		if (keysym > cur[mid].alpha)
			low = mid + 1;
		else if (keysym < cur[mid].alpha)
			high = mid - 1;
		else {
			if (sweep_out(immap, imb, cur[mid].diac)) {
				*dead_state = XK_VoidSymbol;
				return True;
			}
			/*
			 *	may fail to compose.   E.g., n-acute
			 *	is not in 88591 nor IBM-850, though
			 *	it's in 88592.
			 *	sweep_out() returns False in this case.
			 *	This logic needs to be reviewed for
			 *	future version.
			 */
			break;
		}
	}
	return False;
}

static int	try_to_compose_char(IMKeymap *immap, IMBuffer *imb,
	unsigned int *dead_state, unsigned char c)
{
	/*
	 *	Here we assume [ A-Za-z] are portable characters.
	 *	I.e., they all have the same code points in any
	 *	code sets we support (e.g., IBM-850, ISO8859-1).
	 *	Also we assume, only those alphabet character can
	 *	be a subject of the dead key processing.
	 *	So we can make the following assmuption.
	 *	    XK_A == (unsigned int)'A'
	 */
	if (c == ' ' || 'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z')
		return try_to_compose_keysym(immap, imb, dead_state,
			(unsigned int)c);
	return False;
}

/* ARGSUSED */
int	_IMAIXMapping(IMKeymap *immap,
	unsigned int keysym, unsigned int state, IMBuffer *imb,
	unsigned int *dead_state, int *accum_state, unsigned char *accumulation)
{
	KeyMapElement	*kme;
	int	add;

	/*
	 *	Check to see if the input is ALT-NumPad.
	 */
	if (state & (MetaMask | AltGraphMask) && (add = NumPad(keysym)) != -1) {
		*accumulation = 10 * *accumulation + add;
		if (++*accum_state >= 3) {
			/*
			 *	Saturated.
			 *	If it's in Dead state, try to compose.
			 */
			if (*dead_state == XK_VoidSymbol) {
				placebyte(imb, *accumulation);
				*accumulation = 0;
				*accum_state = 0;
			}
			else if (!try_to_compose_char(immap, imb, dead_state,
				*accumulation)) {
				/*
				 *	Couldn't be composed.
				 *	Sweep out the dead key as a character.
				 *	Then a character generated by
				 *	ALT-NumPad.
				 */
				sweep_out(immap, imb, *dead_state);
				*dead_state = XK_VoidSymbol;
				placebyte(imb, *accumulation);
				*accumulation = 0;
				*accum_state = 0;
			}
		}

		/*
		 *	In any cases above, the input is eaten by the SIM.
		 */
		return IMInputUsed;
	}

	/*
	 *	Here we don't need to care about ALT-NumPad Input.
	 *	It is handled in the above.
	 *	However, we need to generate a character if already
	 *	some ALT-NumPad input were made.
	 */
	if (*accum_state != 0) {
		if (*dead_state != XK_VoidSymbol) {
			/*
			 *	A dead key is stacked.
			 *	Also ALT-NumPad data are stacked.
			 *	Try to compose them.
			 */
			if (!try_to_compose_char(immap, imb, dead_state,
				*accumulation)) {
				sweep_out(immap, imb, *dead_state);
				*dead_state = XK_VoidSymbol;
				placebyte(imb, *accumulation);
				*accumulation = 0;
				*accum_state = 0;
			}
		}
		else {
			placebyte(imb, *accumulation);
			*accumulation = 0;
			*accum_state = 0;
		}

		/*
		 *	Handle the current input.
		 *	If it's a BASE keysym and is also a dead keysym,
		 *	go into the dead state.  Otherwise, we don't
		 *	use the input.  We're not interested in.
		 *	Remember, another interests, i.e., ALT-NumPad,
		 *	is already handled in the above.
		 */
		if (state == 0 && IsDeadKeysym(keysym)) {
			*dead_state = keysym;
			return IMInputUsed;
		}
		return IMInputNotUsed;
	}

	/*
	 *	Reaching here, we know,
	 *	1) the input is not ALT-NumPad.
	 *	2) it's not in ALT-NumPad state.
	 *	So we can concentrate on only the dead state.
	 */
	if (*dead_state != XK_VoidSymbol) {
		if (state == 0) {
			/*
			 *	In this case, the input is a BASE keysym.
			 *	Try to perform Keysym base dead key
			 *	processing.
			 */
			if (try_to_compose_keysym(immap, imb, dead_state,
				keysym)) {
				/*
				 *	A character is generated.
				 */
				return IMInputUsed;
			}

			/*
			 *	Failed to compose.
			 *	Sweep out the current dead key.
			 */
			sweep_out(immap, imb, *dead_state);
			*dead_state = XK_VoidSymbol;

			/*
			 *	The current input may be another dead key.
			 */
			if (IsDeadKeysym(keysym)) {
				*dead_state = keysym;
				return IMInputUsed;
			}
			return IMInputNotUsed;
		}

		/*
		 *	The current input is not a BASE keysym.
		 *	We try to compose a character if the current
		 *	input is corresponding to an alphabet.
		 *	KME_SSTR1 entry means a 1-byte character is
		 *	mapped for this input.  We try only this case
		 *	and it's enough.
		 */
		kme = _IMLookupKeymap(immap, keysym, state);
		if (kme->type == KME_SSTR1 &&
			try_to_compose_char(immap, imb, dead_state,
				kme->data[0])) {
			return IMInputUsed;
		}

		/*
		 *	Finally we come here.  We couldn't compose a
		 *	character.  The stacked dead key is swept out.
		 *	The current input is not interested, so return
		 *	NotUsed.
		 */
		sweep_out(immap, imb, *dead_state);
		*dead_state = XK_VoidSymbol;
		return IMInputNotUsed;
	}

	/*
	 *	Not in dead state, not in ALT-NumPad state and the input
	 *	is not ALT-NumPad.
	 *	One thing left is to check to see if the current input
	 *	is a dead key.
	 */
	if (state == 0 && IsDeadKeysym(keysym)) {
		*dead_state = keysym;
		return IMInputUsed;
	}
	return IMInputNotUsed;
}

/*
 *	_IMSimpleMapping
 */

/* ARGSUSED */
void	_IMSimpleMapping(IMKeymap *immap,
	unsigned int keysym, unsigned int state, IMBuffer *imb)
{
	KeyMapElement	*kme;
	unsigned char	*ptr;

	/*
	 *	Lookup the imkeymap.
	 */
	kme = _IMLookupKeymap(immap, keysym, state);
	switch (kme->type) {
	case KME_SSTR3:				/* 3-byte string */
		placestr(imb, &kme->data[0], 3);
		break;

	case KME_SSTR2:				/* 2-byte string */
		placestr(imb, &kme->data[0], 2);
		break;

	case KME_SSTR1:				/* 1-byte string, e.g., char */
		placebyte(imb, kme->data[0]);
		break;

	case KME_ESEQ3:				/* ESC X X X */
		placebyte(imb, 0x1b);		/* ESC */
		placestr(imb, &kme->data[0], 3);
		break;

	case KME_ESEQ2:				/* ESC X X */
		placebyte(imb, 0x1b);		/* ESC */
		placestr(imb, &kme->data[0], 2);
		break;

	case KME_ESEQ1:				/* ESC X */
		placebyte(imb, 0x1b);		/* ESC */
		placebyte(imb, kme->data[0]);
		break;

	case KME_QPFK:				/* ESC [ d d d q */
	case KME_ZPFK:				/* ESC [ d d d z */
		placebyte(imb, 0x1b);		/* ESC */
		placebyte(imb, 0x5b);		/* '[' */
		placestr(imb, &kme->data[0], 3);
		if (kme->type == KME_QPFK)
			placebyte(imb, 'q');
		else
			placebyte(imb, 'z');
		break;

	case KME_LSTR:				/* a long string */
		ptr = &immap->kstr.data[GET3DIGITS(kme->data)];
		placestr(imb, ptr + 1, *ptr);
		break;

	case KME_BSTR:				/* a bound string */
		ptr = &immap->bstr.data[GET3DIGITS(kme->data)];
		placestr(imb, ptr + 1, *ptr);
		break;

	default:	/* KME_KEYSYM, KME_PRIVKEYSYM, KME_UNBOUND */
		break;
	}
}

/*
 *	Rebind the specified string to the specified keysym/state.
 */

int	_IMRebindCode(IMKeymap *immap, unsigned int keysym, unsigned int state,
	unsigned char *str, int len)
{
	KeyMapElement	*kme, *tkme;
	int	i;
	int	idx;

	kme = imlookuprealkeymap(immap, keysym, state);
	if (!kme)
		return False;

	/*
	 * 1 byte character. majority of the cases.
	 */
	if (len == 1 && str) {
		kme->type = KME_SSTR1;
		PUT3BYTES(kme->data, *str, '\0', '\0');
		return True;
	}

	/*
	 * UNBOUND if neither str nor len is given.
	 */
	if (!len && !str) {
		kme->type = KME_UNBOUND;
		PUT3BYTES(kme->data, '\0', '\0', '\0');
		return True;
	}

	/*
	 * Dead key definition if len == 0.
	 */
	if (!len) {
		static unsigned int	dead_keysyms[] = {
			XK_dead_acute,		/* XK_dead_apostrophe, */
			XK_dead_grave,		XK_dead_circumflex,
			XK_dead_diaeresis,	XK_dead_tilde,
			XK_dead_caron,		XK_dead_breve,
			XK_dead_doubleacute,	XK_dead_degree,
			XK_dead_abovedot,	XK_dead_macron,
			XK_dead_cedilla,	XK_dead_ogonek,
		};
#define	DEADKEYTBLSIZ	(sizeof (dead_keysyms) / sizeof (dead_keysyms[0]))

		for (i = 0; i < DEADKEYTBLSIZ; i++) {
			tkme = _IMLookupKeymap(immap, dead_keysyms[i], 0);
			if (tkme->type == KME_SSTR1 && tkme->data[0] == *str) {
				kme->type = KME_PRIVKEYSYM;
				PUT3DIGITS(kme->data, dead_keysyms[i]);
				return True;
			}
		}
#undef	DEADKEYTBLSIZ
		return False;
	}

	/*
	 * Function key definition if str == 0.
	 * len holds function id.
	 * See AIX OSTR, hft for information about function id.
	 * NOTE : There is one design error. According to AIX X-Windows
	 *        User's Guide, len can not be zero ( and this routine
	 *	  does so ). So it's not possible to rebind to PF1 key
	 *	  since its function id == 0.
	 */

	if (!str) {
		/*
		 *	PF Key.  0x0000 < len < 0x00ff
		 */
		if (len < 0xff) {
			kme->type = KME_QPFK;
			PUT3DECIMAL(kme->data, len + 1);
			return True;
		}
		/*
		 *	ESC sequences.
		 */
		return fill_func(kme, len);
	}

	/*
	 * Otherwise (len > 1 && str > 0), bind the string.
	 */

	if (len > 256)
		return False;
	if (fill_sstr(kme, str, len))
		return True;
	if ((idx = addtostr(&immap->bstr, str, len)) == -1)
		return False;
	kme->type = KME_BSTR;
	PUT3DIGITS(kme->data, idx);
	return True;
}
