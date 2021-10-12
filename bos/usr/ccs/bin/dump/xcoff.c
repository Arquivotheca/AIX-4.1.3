static char sccsid[] = "@(#)55  1.37  src/bos/usr/ccs/bin/dump/xcoff.c, cmdaout, bos41B, 9505A 1/3/95 13:50:27";

/*
 * COMPONENT_NAME: CMDAOUT (dump command)
 *
 * FUNCTIONS: dump_aux, dump_csect, dump_load_head, dump_load_reloc,
 *	dump_load_symbol, dump_load_symtab, dump_loader_sect,
 *	dump_sect_headers, dump_symbol, dump_xcoff, dump_xcoff_hdr,
 *	dump_xcoff_lines, dump_xcoff_reloc, dump_xcoff_sects,
 *	dump_xcoff_strings, dump_xcoff_syms, get_aux_name,
 *	get_classes, get_debug_offset, get_ldtype, get_load_import_ids,
 *	get_load_sym_name, get_rtypes, get_sclass, get_stype,
 *	get_symbol_name, get_typelist, read_load_symbol,
 *	read_xcoff_hdr, read_xcoff_symbol, xcoff_func_line,
 *	xcoff_reloc
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <xcoff.h>
#include <values.h>
#include <dbxstclass.h>
#include "dump_defs.h"

extern nl_catd catd;
extern void exit(), free(), dump_archive();
extern char *malloc(), *strcpy(), *strncpy(), *strcat();
extern char *dump_alloc(), *read_xcoff_symbol(), *read_load_symbol();
extern long xcoff_func_line(), get_debug_offset();
static void dump_aux(), dump_load_symbol(), dump_symbol();
static char *get_load_sym_name(), *get_symbol_name(), *get_aux_name();
static char *get_classes(), *get_rtypes(), *get_ldtype();
static char *get_typelist(), *get_stype(), *get_sclass();
void	dump_loader_sect(), dump_load_reloc(), dump_load_symtab(),
	dump_xcoff_lines(), dump_xcoff_strings(), dump_xcoff_syms(),
	get_load_import_ids(), xcoff_reloc();

extern char	*Current_file, *Member_name, *Zname;
extern int	Ar_member, Data_sect, Ex_header, Head_sect, Loader_head,
		Loader_reloc, Loader_sect, Loader_symt, Line_nums,
		Obj_flag, Obj_sect,
		Opt_header, Print_header, Reloc, Str_table,
		Sym_end, Sym_start, Sym_table, Symbolic, Zstart, Zend;
		
extern long	File_origin, Member_size;

static long	load_pos;

static LDHDR	lhd;
static FILHDR	hdr;
static AOUTHDR	*opt_ptr;
static SCNHDR	scn_hdr;
static int	Aout_Hdr;
static char	*Import_strings;
struct import_offsets{
	long	path;
	long	base;
	long	member;
};
static struct import_offsets *Import_offsets;
static char	one_k_buf[1024];


static
dump_csect(aux)
	AUXENT *aux;
{
	if (Symbolic)
		(void) printf("a4  0x%-.8lx%8x %4d   %4s %4s %8d %4d\n",
			aux->x_csect.x_scnlen, aux->x_csect.x_parmhash,
			aux->x_csect.x_snhash,
			get_stype(aux->x_csect.x_smtyp&0x07),
			get_sclass(aux->x_csect.x_smclas),
			aux->x_csect.x_stab,
			aux->x_csect.x_snstab);
	else
		(void) printf("a4  0x%-.8lx%8x %4d   %4d %4d %8d %4d\n",
			aux->x_csect.x_scnlen, aux->x_csect.x_parmhash,
			aux->x_csect.x_snhash, aux->x_csect.x_smtyp,
			aux->x_csect.x_smclas, aux->x_csect.x_stab,
			aux->x_csect.x_snstab);

#ifdef	notdef
	if (Hash_tag)
	{
		(void) printf(MSGSTR(PARM_HASH_MSG, PARM_HASH));

		if (hlen > 0)
			for (i = 0; i < hlen; ++i, ++hash)
				(void) printf("%.2x", *hash);
		else
				(void) printf(MSGSTR(NO_HASH));
	}
#endif	/* notdef */
}
	
static void
dump_aux(fp, sym, aux)
	FILE *fp;
	SYMENT *sym;
	AUXENT *aux;
{
	register int	is_ary = ISARY(sym->n_type),
			is_fcn = ISFCN(sym->n_type),
			tagndx = aux->x_sym.x_tagndx;

	if ((sym->n_sclass) == C_FILE)
	{
		(void) printf("a0%56s%s\n", "", get_aux_name(fp, aux));
		return;
	}

	/* formats: 1-misc, 2-function, 3-array, 4-csect, 5-section names */

	/* Print section names */
	if (sym->n_sclass == C_STAT)
	{
		(void) printf("a5  0x%-12.8lx 0x%.4x %3s 0x%.4x\n",
			aux->x_scn.x_scnlen, aux->x_scn.x_nreloc,
			"", aux->x_scn.x_nlinno);
		return;
	}

	(void) printf("a%d%12ld", 1 + is_fcn + (is_ary * 2), tagndx);
	/* Print function entries */
	if (is_fcn)
		(void) printf("%16ld", aux->x_sym.x_misc.x_fsize);
	else
		(void) printf("%10d%6d", aux->x_sym.x_misc.x_lnsz.x_lnno,
				aux->x_sym.x_misc.x_lnsz.x_size);

	if (is_ary)
	{
		register int	i;
		register unsigned short	*p
				= &aux->x_sym.x_fcnary.x_ary.x_dimen[0];

		(void) printf("  %5d", *(p++));

		for (i = 1; i < DIMNUM; ++i)
			if (*p != 0)
				(void) printf(",%5d", *p++);
			else
				(void) printf("       ");
		(void) printf("   ");
	}
	else {
		(void) printf("%8ld%8ld", aux->x_sym.x_fcnary.x_fcn.x_lnnoptr,
				aux->x_sym.x_fcnary.x_fcn.x_endndx);
		(void) printf("            ");
	}

/*
	if (Symbolic && (tagndx > 0) && !is_fcn)
		(void) printf("%s", read_xcoff_symbol(fp, (long) tagndx));
*/

	(void) putchar('\n');
}

/*
 * dump_xcoff()	- process object file
 */
void
dump_xcoff(filep)
	FILE *filep;
{
	if (!Obj_flag)
		return;

	if (Member_name == NULL)
		pr_name();

	if (read_xcoff_hdr(filep) < 0)
	{
		error(MSGSTR(NOT_AN_OBJ_MSG, NOT_AN_OBJ));
		return;
	}

	if (Opt_header)
		dump_xcoff_hdr(filep);	/* -o dump main head + opt head	-o*/

	if (Head_sect)
		dump_sect_headers(filep);/* dump section headers	-h*/

	if (Obj_sect || Data_sect)
		dump_xcoff_sects(filep); /* dump sect data in hex	-s -d*/

	if (Reloc)
		dump_xcoff_reloc(filep); /* dump relocation entries	-r*/

	if (Line_nums)
		dump_xcoff_lines(filep); /* print line # info		+-z -l*/

	if (Sym_table)
		dump_xcoff_syms(filep);  /* dump symbol table info	-t*/

	if (Str_table)
		dump_xcoff_strings(filep); /* dump string table info	-c*/

	if (Loader_sect || Loader_reloc || Loader_symt || Loader_head)
		dump_loader_sect(filep); /* dump all portions of load   -n*/
}

void dump_loader_sect(fp)
	FILE *fp;
{
	if (Aout_Hdr <= 0) 
	{
		(void) printf(MSGSTR(LD_UNAVAIL_MSG, LD_UNAVAIL));
		return;
	}

	if (!fseek_OK(fp, (long) (File_origin + FILHSZ + hdr.f_opthdr +
		(opt_ptr->o_snloader-1) * SCNHSZ)))
	{
		return;
	}

	if (!fread_OK((char *)&scn_hdr, SCNHSZ, fp))
		return;

	load_pos = File_origin + scn_hdr.s_scnptr; /*position of loader data*/

	if (!fseek_OK(fp, load_pos))
		return;

	if (!fread_OK((char *) &lhd, LDHDRSZ, fp))
		return;

	if (Print_header)
		title(MSGSTR(LD_SCN_MSG, LD_SCN));

	if (Loader_sect || Loader_head || Loader_symt)
		get_load_import_ids(fp);

	if (Loader_head || Loader_sect)
		dump_load_head(fp);

	if (Loader_symt || Loader_sect)
		dump_load_symtab(fp);

	if (Loader_reloc || Loader_sect)
		dump_load_reloc(fp);
}

void dump_load_reloc(fp)
	FILE *fp;
{
	struct ldrel 	lrel;
	register int	i;

	if (Print_header)
	{
		title(MSGSTR(LD_RLINFO_MSG, LD_RLINFO));
	(void) printf(MSGSTR(LD_RLHEAD_MSG, LD_RLHEAD));

		if (Symbolic)
			(void) printf(MSGSTR(LD_RLNAME_MSG, LD_RLNAME));
		else
			(void) putchar('\n');
	}

	if (lhd.l_nreloc)
	{
		if (!fseek_OK(fp, (long) (load_pos + LDHDRSZ
			+ (lhd.l_nsyms * LDSYMSZ))))
		{
			return;
		}

		for (i = lhd.l_nreloc; i > 0; --i)
		{
			if (!fread_OK(&lrel, LDRELSZ, fp))
				return;
	
			/*type cast prints a negative value where appropriate */
			(void) printf("\t0x%.8lx  0x%.8lx",
				lrel.l_vaddr, lrel.l_symndx);

			if (Symbolic)
			{
				(void) printf("%10s      0x%.4x",
					get_rtypes(lrel.l_rtype &
					0x00ff), lrel.l_rsecnm);

				(void) printf("     %s\n",
					((lrel.l_symndx < 0) ||
					(lrel.l_symndx > lhd.l_nsyms + 3))
					? "???" : read_load_symbol(fp,
					(long) lrel.l_symndx));
			}
			else
			{
				(void) printf("    0x%.4x", lrel.l_rtype &
					0x00ff);
				(void) printf("      0x%.4x\n", lrel.l_rsecnm);
			}
		}
	}
	else
		(void) printf(MSGSTR(NO_LDREL_MSG, NO_LDREL));
}
	
char *
read_load_symbol(fp, index)
	FILE *fp;
	long index;
{
	static	LDSYM	lsym;
	long	position;

	position = ftell(fp);

	switch (index)
	{
	  case 0:  return(".text");
	  case 1:  return(".data");
	  case 2:  return(".bss");
	  default:
		if (!fseek_OK(fp, (long) (load_pos + LDHDRSZ +
			(LDSYMSZ * (index - 3)))) || !fread_OK((char *)
			&lsym, (long) LDSYMSZ, fp))
		{
			(void) fseek_OK(fp, position);
			return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
		}
		(void) fseek_OK(fp, position);
		return(get_load_sym_name(fp, &lsym));
	}
}

void dump_load_symtab(fp)
	FILE *fp;
{
	register long	start = 0,
			stop = 0;
	struct	ldsym	lsym;

	if (Print_header)
	{
		title(MSGSTR(LD_SYMTAB_MSG, LD_SYMTAB));
		(void) printf(MSGSTR(LD_SYMHEAD_MSG, LD_SYMHEAD));

	}

	if (!fseek_OK(fp, (long) (load_pos + LDHDRSZ)))
		return;

	if (!lhd.l_nsyms)
		return;

	stop = lhd.l_nsyms - 1;

	for (; start <= stop; ++start)
	{
		if (!fread_OK((char *) &lsym, LDSYMSZ, fp))
			return;

		(void) printf("[%ld]\t", start);
		dump_load_symbol(fp, &lsym);
	}
}

static void
dump_load_symbol(fp, lsym)
	FILE *fp;
	register struct	ldsym	*lsym;
{
	register char	*p = "";
	register int	class = lsym->l_smclas;
	char		buffer[9];
	unsigned short	i;
	unsigned short	j;
	long		save_pos;
	static char	scn_name[9];
	static short	sect = 0xfc;

	if (Symbolic && lsym->l_scnum != sect) {
		if (lsym->l_scnum > 0) {
			save_pos = ftell(fp);

    			if (!fseek_OK(fp, (long) (File_origin + FILHSZ
				+ hdr.f_opthdr + (lsym->l_scnum-1) * SCNHSZ)) ||
				!fread_OK(&scn_hdr, (long) (SCNHSZ), fp))
			{
				return;
			}

			(void) fseek_OK(fp, save_pos);
			(void) strncpy(scn_name, scn_hdr.s_name, 8);
			scn_name[8] = '\0';
		}
		else {
			switch(lsym->l_scnum) {
			  case N_UNDEF:
				(void) strcpy(scn_name, "undef");
				break;

			  case N_ABS:
				(void) strcpy(scn_name, "abs");
				break;

			  case N_DEBUG:
				(void) strcpy(scn_name, "debug");
				break;

			  default:
				(void) strcpy(scn_name, "???");
			}
		}
	}

	sect = lsym->l_scnum;
	(void) printf("0x%.8lx", lsym->l_value);

	if (Symbolic)
	{
		p = get_ldtype(lsym->l_smtype & 0x07);

		i = (lsym->l_smtype>>3);
		switch (i & 0x1f)
		{
		  case 0x02:
			(void) strcpy(buffer, "EXP");
			break;

		  case 0x04:
			(void) strcpy(buffer, "ENTpt");
			break;

		  case 0x06:
			(void) strcpy(buffer, "EXPentPT");
			break;

		  case 0x08:
			(void) strcpy(buffer, "IMP");
			break;

		  case 0x0a:
			(void) strcpy(buffer, "ImpExp");
			break;

		  case 0x0c:
			(void) strcpy(buffer, "IMPentPT");
			break;

		  case 0x0e:
			(void) strcpy(buffer, "ImExenPT");
			break;

		  default:
			(void) strcpy(buffer, "????");
		}

		(void) printf("%9s%9s%7s%7s", scn_name, buffer,
				get_sclass(class), p);
	}
	else
	{
		i = ((lsym->l_smtype>>3) & 0x1f);
		j = (lsym->l_smtype) & 0x07;
		(void) printf("   0x%.4x     0x%.2x   0x%.2x    0x%.1x", 
				lsym->l_scnum, i, class, j);
	}

	if (Symbolic)
	{
		if (Import_offsets != NULL &&
		*(Import_strings + Import_offsets[lsym->l_ifile].base) != '\0')
		{
			if (Import_offsets[lsym->l_ifile].member != 0)
			{
				sprintf(one_k_buf, "%s(%s)", Import_strings +
					Import_offsets[lsym->l_ifile].base,
					Import_strings +
					Import_offsets[lsym->l_ifile].member);
				(void) printf(" %15s", one_k_buf);
			}
			else
				(void) printf(" %15s",
					Import_strings +
					Import_offsets[lsym->l_ifile].base);
		}
		else
			(void) printf(" %15s", MSGSTR(NO_IMID_MSG, NO_IMID));
	}
	else
		(void) printf("          0x%.4x", lsym->l_ifile);
		

	if (lsym->l_zeroes == 0 && lsym->l_offset == 0)
		(void) printf(MSGSTR(NO_SYM_MSG, NO_SYM));
	else
		(void) printf(" %s\n", get_load_sym_name(fp, lsym));
}

static char *
get_load_sym_name(fp, lsym)
	FILE *fp;
	struct	ldsym	*lsym;
{
	static char	buf[BUFSIZ];
	register char	*p;
	register int	c = ~0,
			i;
	long		position;

	if (lsym->l_zeroes != 0)
	{
		/* copied only because we can't guarantee null termination */
		(void) strncpy(buf, lsym->l_name, 8);
		buf[8] = '\0';
		return(buf);
	}

	position = ftell(fp);

	if (!fseek_OK(fp, load_pos + lhd.l_stoff + lsym->l_offset))
	{
		(void) fseek_OK(fp, position);
		return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
	}

	for (i = 0, p = buf; i < BUFSIZ && c && (c = getc(fp)) != EOF; ++i, ++p)
		*p = c;

	if (c == EOF)
	{
		(void) fseek_OK(fp, position);
		error(MSGSTR(READ_ERR_MSG, READ_ERR));
		return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
	}

	(void) fseek_OK(fp, position);
	return(buf);
}

void get_load_import_ids(fp)
	FILE *fp;
{
	register char	*p;
	long		position,
			num_imps;

	position = ftell(fp);

	if (Import_strings != NULL)
		free((char *)Import_strings);
	Import_strings = (char *)malloc(lhd.l_istlen);
	if (!fseek_OK(fp, load_pos + lhd.l_impoff) ||
	!fread_OK((char *)Import_strings, (int)lhd.l_istlen, fp))
	{
		(void) fseek_OK(fp, position);
		free((char *)Import_strings);
		free((char *)Import_offsets);
		Import_offsets = NULL;
		return;
	}
	
	if (Import_offsets != NULL)
		free((char *)Import_offsets);
	Import_offsets = (struct import_offsets *)malloc(lhd.l_nimpid *
		sizeof(struct import_offsets));

	p = Import_strings;
	for (num_imps = 0; num_imps < lhd.l_nimpid; )
	{
		Import_offsets[num_imps].path = (long) (p - Import_strings);
		for ( ; *p != '\0'; p++);
		p++;
		Import_offsets[num_imps].base = (long) (p - Import_strings);
		for ( ; *p != '\0'; p++);
		p++;
		Import_offsets[num_imps].member = (long) (p - Import_strings);
		for ( ; *p != '\0'; p++);
		p++;

		num_imps++;
	}

	return;
}

dump_load_head()
{
	int i;

	if (Print_header)
	{
		(void) printf(MSGSTR(LD_HDR_MSG, LD_HDR),"");
		(void) printf(MSGSTR(LD_HDRHEAD1_MSG, LD_HDRHEAD1));
	}

	(void) printf("0x%-15.8lx0x%-15.8lx0x%-15.8lx0x%-15.8lx\n\n",
			lhd.l_version, lhd.l_nsyms, lhd.l_nreloc,
			lhd.l_istlen);
	
	if (Print_header)
		(void) printf(MSGSTR(LD_HDRHEAD2_MSG, LD_HDRHEAD2));

	(void) printf("0x%-15.8lx0x%-15.8lx0x%-15.8lx0x%-15.8lx\n\n",
			lhd.l_nimpid, lhd.l_impoff, lhd.l_stlen,
			lhd.l_stoff);

	if (lhd.l_nimpid)
	{
		if (Print_header)
		{
			title(MSGSTR(IMP_FILE_STRS_MSG, IMP_FILE_STRS));
			(void) printf("%-7s%-30s%-20s%-20s\n",
				MSGSTR(INDEX_STR_MSG, INDEX_STR), 
				MSGSTR(PATH_STR_MSG, PATH_STR),
				MSGSTR(BASE_STR_MSG, BASE_STR),
				MSGSTR(MEMBER_STR_MSG, MEMBER_STR));
		}
		for (i = 0; i < lhd.l_nimpid; i++)
			(void) printf("%-7d%-30s%-20s%-20s\n", i,
				Import_strings + Import_offsets[i].path,
				Import_strings + Import_offsets[i].base,
				Import_strings + Import_offsets[i].member);
	}
}

dump_xcoff_hdr(fp)
	FILE *fp;
{
	if (Print_header)
	{
		title(MSGSTR(OBJ_HDR_MSG, OBJ_HDR));
		(void) printf(MSGSTR(OBJ_HDRHEAD_MSG, OBJ_HDRHEAD));
	}

	(void) printf("%10d\t0x%.8lx\t%9d\t%11d\t0x%.4x\n",
			hdr.f_nscns, hdr.f_symptr, hdr.f_nsyms,
			hdr.f_opthdr, hdr.f_flags);
	if (Symbolic)
	{
		(void) printf(MSGSTR(OBJ_FLAGS_MSG, OBJ_FLAGS));
		if (hdr.f_flags & F_RELFLG)
			(void) printf(" RELFLG");
		if (hdr.f_flags & F_EXEC)
			(void) printf(" EXEC");
		if (hdr.f_flags & F_LNNO)
			(void) printf(" LNNO");
		if (hdr.f_flags & F_LSYMS)
			(void) printf(" LSYMS");
		if (hdr.f_flags & F_AR16WR)
			(void) printf(" AR16WR");
		if (hdr.f_flags & F_AR32WR)
			(void) printf(" AR32WR");
		if (hdr.f_flags & F_AR32W)
			(void) printf(" AR32W");
		if (hdr.f_flags & F_DYNLOAD)
			(void) printf(" DYNLOAD");
		if (hdr.f_flags & F_SHROBJ)
			(void) printf(" SHROBJ");
		if (hdr.f_flags & F_LOADONLY)
			(void) printf(" LOADONLY");
		(void) printf(" )\n");
	}

	if (opt_ptr) {	/* We have some optional header */
		/* Do we understand? */
		if (!Aout_Hdr) {
			(void) printf("\n");
			hex_dump(MSGSTR(BAD_OPTHDR_MSG, BAD_OPTHDR),
				 fp, File_origin +
				(long) FILHSZ, (long) hdr.f_opthdr);
		}
		else {	/* I Understand */
			title(MSGSTR(OPT_HDR_MSG, OPT_HDR));

			if (Print_header)
				(void) printf(MSGSTR(OPT_HDRHEAD1_MSG,
					OPT_HDRHEAD1));

			(void) printf(
				"0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx\n",
				opt_ptr->tsize, opt_ptr->dsize, opt_ptr->bsize,
				opt_ptr->text_start, opt_ptr->data_start);
			if (Aout_Hdr > 0)
			{
				(void) printf(MSGSTR(OPT_HDRHEAD2_MSG,
					OPT_HDRHEAD2));
				(void) printf(
				"0x%-8.4x  0x%-8.4x  0x%-8.4x  0x%-8.4x  0x%-8.4x\n\n",
					opt_ptr->o_snloader, opt_ptr->o_snentry,
					opt_ptr->o_sntext, opt_ptr->o_sntoc,
					opt_ptr->o_sndata);
				(void) printf(MSGSTR(OPT_HDRHEAD3_MSG,
				OPT_HDRHEAD3));
				(void) printf(
				"0x%-8.4x  0x%-8.4x  0x%.8lx  0x%-8.4x  0x%.8lx\n\n",
					opt_ptr->o_algntext, opt_ptr->o_algndata,
					opt_ptr->o_toc, opt_ptr->vstamp,
					opt_ptr->entry);
				(void) printf(MSGSTR(OPT_HDRHEAD4_MSG,
				OPT_HDRHEAD4));
				(void) printf(
				"0x%.8lx  0x%.8lx\n",
					opt_ptr->o_maxstack, 
					opt_ptr->o_maxdata);
			}
		}
	}
}

void dump_xcoff_lines(fp)
	FILE *fp;
{
	register int	start = Zstart ? Zstart : 1,
			stop = (Zname != NULL && Zend > 0) ? Zend : MAXINT,
			i;
	register long	n,
			num_lnums,
			offset;
	LINENO	lnum;

	if (Print_header)
	{
		title(MSGSTR(LN_INFO_MSG, LN_INFO));
		(void) printf(MSGSTR(LN_HEAD_MSG, LN_HEAD));
	}

	/* Try to find some line numbers anywhere */
	if (!fseek_OK(fp, (long) (File_origin + FILHSZ + hdr.f_opthdr)))
		return;

	for (n = hdr.f_nscns; n ; n--) {
		if (!fread_OK((char *) &scn_hdr, SCNHSZ, fp))
			return;

		if ((scn_hdr.s_flags == STYP_TEXT) && scn_hdr.s_nlnno
			&& (offset = scn_hdr.s_lnnoptr))
		{
			if((scn_hdr.s_nreloc == 0xffff) &&
				(scn_hdr.s_nlnno == 0xffff))
			{	/* overflow */
				if (!fseek_OK(fp, (long) (File_origin +
				FILHSZ + hdr.f_opthdr)))
					return;

				for (i = 0; i < hdr.f_nscns; i++)
				{
					if (!fread_OK((char *) &scn_hdr,
					SCNHSZ, fp))
						return;

					if (((scn_hdr.s_flags &
					0x0000ffff) == STYP_OVRFLO) &&
					(scn_hdr.s_nlnno == 
					((hdr.f_nscns - n) + 1)))
					{
						num_lnums =
							scn_hdr.s_vaddr;
						break;
					}
				}
			}
			else
				num_lnums = scn_hdr.s_nlnno;

			break;
		}
	}

	if (!n)
		return;	/* Couldn't find any */

	if (Zname != NULL) {	/* Get start from symbol table */
		if ((offset = xcoff_func_line(fp, Zname)) < 0)
			return;
	}

	/* Now scn_hdr should contain appropiate section header */
	if (!fseek_OK(fp, File_origin + offset))
		return;

	/* Subtract any specified offset from number of entries */
	num_lnums = num_lnums - ((offset - scn_hdr.s_lnnoptr) / LINESZ);

	for (n = 0; n < num_lnums; ++n)
	{
		if (!fread_OK((char *) &lnum, LINESZ, fp))
			return;

		if (lnum.l_lnno == 0)	/* function entry */
		{
			if (Zname != NULL && n > 0)	/* only one function */
				break;

			(void) printf(MSGSTR(LN_FCN_MSG, LN_FCN),
				lnum.l_addr.l_symndx, lnum.l_lnno);

			if (Symbolic)
				(void) printf("  %s\n",
					read_xcoff_symbol(fp,
					(long) lnum.l_addr.l_symndx));
			else
				(void) putchar('\n');
		}
		else if (lnum.l_lnno > stop)
			break;
		else if (lnum.l_lnno >= start)
			(void) printf("\t 0x%.8lx   %5hu\n",
				lnum.l_addr.l_paddr, lnum.l_lnno);
	}
}

dump_xcoff_reloc(fp)
	FILE *fp;
{
	int i;

	if (Print_header)
	{
		title(MSGSTR(RL_INFO_MSG, RL_INFO));
	(void) printf(MSGSTR(RL_HEAD2_MSG, RL_HEAD2));

		if (Symbolic)
			(void) printf(MSGSTR(RL_HDNAME_MSG, RL_HDNAME));
		else
			(void) putchar('\n');
	}

	for (i = 0; i < hdr.f_nscns; i++) {
		if (!fseek_OK(fp, (long) (File_origin + FILHSZ + hdr.f_opthdr
				+ i * SCNHSZ))
			|| !fread_OK(&scn_hdr, (long) SCNHSZ, fp))
		{
			continue;
		}

		/* overflow section entries are printed when the overflown
		   section calls this loop; shouldn't be printed explicitly. */
		if (scn_hdr.s_relptr && scn_hdr.s_nreloc &&
		   ((scn_hdr.s_flags & 0x0000ffff) != STYP_OVRFLO))
			xcoff_reloc(fp, (i + 1));
	}
}

dump_sect_headers(fp)
	FILE *fp;
{
	int i;
	char scn_name[9];

	if (Print_header)
		title(MSGSTR(SH_INFO_MSG, SH_INFO));

	for (i = 0; i < hdr.f_nscns; i++) {
		if (!fseek_OK(fp, (long) (File_origin + FILHSZ + hdr.f_opthdr
				+ i * SCNHSZ))
			|| !fread_OK(&scn_hdr, (long) SCNHSZ, fp))
		{
			continue;
		}

		(void) strncpy(scn_name, scn_hdr.s_name, 8);
		(void) printf(MSGSTR(SH_TITLE_MSG, SH_TITLE), scn_name);

		if (Print_header)
			(void) printf(MSGSTR(SH_HEAD1_MSG, SH_HEAD1));

		(void) printf("0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx  0x%.8lx\n\n",
			scn_hdr.s_paddr, scn_hdr.s_vaddr, scn_hdr.s_size,
			scn_hdr.s_scnptr, scn_hdr.s_relptr);

		if (Print_header)
			(void) printf(MSGSTR(SH_HEAD2_MSG, SH_HEAD2));

		(void) printf("0x%.8lx  0x%-8.4x  0x%-8.4x  0x%.8lx\n\n",
			scn_hdr.s_lnnoptr, scn_hdr.s_nreloc, scn_hdr.s_nlnno,
			scn_hdr.s_flags);
	}		
}

dump_xcoff_sects(fp)
	FILE *fp;
{
	int i;
	char scn_name[9];

	if (Print_header)
		title(MSGSTR(SCN_DATA_MSG, SCN_DATA));

	for (i = 0; i < hdr.f_nscns; i++) {
		if (!fseek_OK(fp, (long) (File_origin + FILHSZ + hdr.f_opthdr
			+ i * SCNHSZ)) ||
			!fread_OK(&scn_hdr, (long) SCNHSZ, fp) ||
			(scn_hdr.s_flags == -1))
		{
			continue;
		}

		if (scn_hdr.s_scnptr && scn_hdr.s_size) {
			(void) strncpy(scn_name, scn_hdr.s_name, 8);
			scn_name[8] = '\0';
			hex_dump(scn_name,fp,scn_hdr.s_scnptr,scn_hdr.s_size);
		}
	}
}

void dump_xcoff_strings(fp)
	FILE *fp;
{
	register int	c;
	register long	offset = 4;
	long		size, seek_offset;

	if (Print_header)
	{
		title(MSGSTR(ST_INFO_MSG, ST_INFO));
		(void) printf(MSGSTR(ST_HEAD_MSG, ST_HEAD));
	}

	seek_offset = hdr.f_symptr + hdr.f_nsyms*SYMESZ ;
	if  ((Member_name != NULL) && (seek_offset >= Member_size))
	{
		(void) printf(MSGSTR(NO_STRINGS_MSG, NO_STRINGS));
		return;
	}

	if (!hdr.f_symptr || !hdr.f_nsyms
		|| !fseek_OK(fp, File_origin + seek_offset))
	{
		return;
	}

	if (fread((void *) &size,(size_t) 1,(size_t) sizeof(long), fp) != sizeof(long)) 
	{
		(void) printf(MSGSTR(NO_STRINGS_MSG, NO_STRINGS));
		return;
	}

	while (offset < size && !feof(fp))
	{
		(void) printf("\t%6ld   ", offset);

		for (c = 1; c && (c = getc(fp)) != EOF; ++offset)
			(void) putchar(c ? c : '\n');
	}

	if (offset < size)
		error(MSGSTR(READ_ERR_MSG, READ_ERR));
}

void dump_xcoff_syms(fp)
	FILE *fp;
{
	register int	num_aux;
	register long	num_syms = hdr.f_nsyms,
			start = Sym_start,
			stop = Sym_end;
	SYMENT	sym;
	AUXENT	aux;

	if (Print_header)
	{
		title(MSGSTR(SYM_INFO_MSG, SYM_INFO));
		(void) printf(MSGSTR(SYM_HEAD_MSG, SYM_HEAD));
	}

	if (num_syms == 0 || start >= num_syms)
		return;

	if (stop == 0)
	{
		if (start)
			stop = start;
		else
			stop = num_syms - 1;
	}
	else if (stop >= num_syms)
		stop = num_syms - 1;

	if (!fseek_OK(fp, File_origin + hdr.f_symptr + (long) SYMESZ * start))
		return;

	for (; start <= stop; ++start)
	{
		if (!fread_OK((char *) &sym, SYMESZ, fp))
			return;

		(void) printf("[%ld]\t", start);
		dump_symbol(fp, &sym);

		for (num_aux = (int)sym.n_numaux & 0xFF; num_aux > 0; --num_aux)
		{
			if (!fread_OK(&aux, AUXESZ, fp))
				return;

			(void) printf("[%ld]\t", ++start);

			/* if last aux entry is a csect then dump it */
			if ((num_aux == 1) && ((sym.n_sclass == C_EXT)
				|| (sym.n_sclass == C_HIDEXT)))
			{
				dump_csect(&aux);
			}
			else
				dump_aux(fp, &sym, &aux);
		}
	}
}

static void
dump_symbol(fp, sym)
	FILE *fp;
	register SYMENT *sym;
{
	register char	*typestr = "";
	register int	class = sym->n_sclass;
	char		buffer[13];
	long		save_pos;
	static char	scn_name[9];
	static short	sect = 0xfc;

	if (Symbolic && sym->n_scnum != sect) {
		if (sym->n_scnum > 0) {
			save_pos = ftell(fp);

			if (!fseek_OK(fp, (long) (File_origin + FILHSZ
				+ hdr.f_opthdr + (sym->n_scnum-1) * SCNHSZ))
				|| !fread_OK(&scn_hdr, (long) SCNHSZ, fp))
			{
				return;
			}

			(void) fseek_OK(fp, save_pos);
			(void) strncpy(scn_name, scn_hdr.s_name, 8);
			scn_name[8] = '\0';
		}
		else {
			switch(sym->n_scnum) {
			  case N_UNDEF:
				(void) strcpy(scn_name, "undef");
				break;

			  case N_ABS:
				(void) strcpy(scn_name, "abs");
				break;

			  case N_DEBUG:
				(void) strcpy(scn_name, "debug");
				break;

			  default:
				(void) strcpy(scn_name, "???");
			}
		}
	}

	sect = sym->n_scnum;
	(void) printf("m   0x%.8lx", sym->n_value);

	if (Symbolic)
	{
		/* Get type, only if C_FILE */
		if ((sym->n_type) && (class == C_FILE))
			typestr = get_typelist(sym->n_type);

		buffer[0] = '\0';

		(void) printf("%10s%6d%8s%15s%-5s", scn_name, sym->n_numaux,
				get_classes(class), typestr, buffer);
	}
	else
		(void) printf("%10d%6d    0x%.2x         0x%.4x     ",
				sect, sym->n_numaux, class, sym->n_type);

	(void) printf("%s\n", get_symbol_name(fp, sym));
}

static char *
get_symbol_name(fp, sym)
	FILE *fp;
	SYMENT *sym;
{
	static char	buf[BUFSIZ];
	register char	*p;
	register int	c = ~0,
			i;
	long		offset,
			position,
			toff;

	if (sym->n_zeroes != 0)
	{
		/* copied only because we can't guarantee null termination */
		(void) strncpy(buf, sym->n_name, 8);
		buf[8] = '\0';
		return(buf);
	}

	if (sym->n_offset == 0)
		return(MSGSTR(NO_SYMBOL_NAME_MSG, NO_SYMBOL_NAME));

	offset = File_origin;

	if (sym->n_sclass & DBXMASK)
	{
		if ((toff = get_debug_offset(fp)) < 0)
			return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));

		offset += toff;
	}
	else
		offset += hdr.f_symptr + (hdr.f_nsyms * SYMESZ);

	position = ftell(fp);

	if (!fseek_OK(fp, offset + sym->n_offset))
	{
		(void) fseek_OK(fp, position);
		return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
	}

	for (i = 0, p = buf; i < BUFSIZ && c && (c = getc(fp)) != EOF; ++i, ++p)
		*p = c;

	if (c == EOF)
	{
		(void) fseek_OK(fp, position);
		error(MSGSTR(READ_ERR_MSG, READ_ERR));
		return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
	}

	(void) fseek_OK(fp, position);
	return(buf);
}

static char *
get_aux_name(fp, aux)
	FILE *fp;
	AUXENT *aux;
{
	static char	buf[BUFSIZ];
	register char	*p;
	register int	c = ~0,
			i;
	long		position;

	if (aux->x_file._x.x_zeroes != 0)
	{
		/* copied because we can't guarantee null termination */
		(void) strncpy(buf, aux->x_file.x_fname, 14);
		buf[14] = '\0';
		return(buf);
	}

	position = ftell(fp);

	if (!fseek_OK(fp, File_origin + hdr.f_symptr + (hdr.f_nsyms * SYMESZ)
		+ aux->x_file._x.x_offset))
	{
		(void) fseek_OK(fp, position);
		return(MSGSTR(BAD_AUX_NAME_MSG, BAD_AUX_NAME));
	}

	for (i = 0, p = buf; i < BUFSIZ && c && (c = getc(fp)) != EOF; ++i, ++p)
		*p = c;

	if (c == EOF)
	{
		(void) fseek_OK(fp, position);
		error(MSGSTR(READ_ERR_MSG, READ_ERR));
		return(MSGSTR(BAD_AUX_NAME_MSG, BAD_AUX_NAME));
	}

	(void) fseek_OK(fp, position);
	return(buf);
}

long
xcoff_func_line(fp, name)
	FILE *fp;
	char *name;
{
	register long	numsyms = hdr.f_nsyms,
			save_pos;
	int	num_aux;
	SYMENT	sym;
	AUXENT	aux, tmp_aux;

	if (!fseek_OK(fp, File_origin + hdr.f_symptr))
		return(-1);

	for (; numsyms > 0; numsyms -= sym.n_numaux + 1)
	{
		if (!fread_OK(&sym, SYMESZ, fp))
			return(-1);

		for (num_aux = 0; num_aux < sym.n_numaux; num_aux++)
		{
			if (num_aux == 0)
			{
				if (!fread_OK(&aux, AUXESZ, fp))
					return(-1);
			}
			else
			{
				if (!fread_OK(&tmp_aux, AUXESZ, fp))
					return(-1);
			}
		}

		/* if symbol is a function and is the name we are looking for */
		if (ISFCN(sym.n_type)
			&& !strcmp(name, get_symbol_name(fp, &sym)))
		{
			if (sym.n_numaux)	/* get the section header */
			{
				save_pos = ftell(fp);

				if (!fseek_OK(fp, (long) (File_origin + FILHSZ
					+ hdr.f_opthdr + (sym.n_scnum-1) *
					SCNHSZ)) || !fread_OK(&scn_hdr,(long)
					SCNHSZ,fp))
				{
					(void) fseek_OK(fp, save_pos);
					return(-1);
				}

				(void) fseek_OK(fp, save_pos);
				break;
			}

			error(MSGSTR(NO_AUX_ENT_MSG, NO_AUX_ENT));
		}
	}

	if (numsyms <= 0)
	{
		error(MSGSTR(FCN_NOT_FOUND_MSG, FCN_NOT_FOUND));
		return(-1);
	}

	return(aux.x_sym.x_fcnary.x_fcn.x_lnnoptr);
}


void xcoff_reloc(fp, pri_scn)
	FILE *fp;
	int pri_scn;
{
	register int	i;
	long		num_reloc;
	RELOC		rel;
	char		prompt[9];

	(void) strncpy(prompt, scn_hdr.s_name, 8); /* Copy and output */
	prompt[8] = '\0';
	output(prompt);

	if((scn_hdr.s_nreloc == 0xffff) && (scn_hdr.s_nlnno == 0xffff))
	{
		/* Overflow section */
		for (i = 0; i < hdr.f_nscns; i++)
		{
			if (!fseek_OK(fp, (long) (File_origin + FILHSZ +
				hdr.f_opthdr + i * SCNHSZ))
				|| !fread_OK(&scn_hdr, (long) SCNHSZ, fp))
			{
				continue;
			}

			if (((scn_hdr.s_flags & 0x0000ffff) == STYP_OVRFLO) &&
			(scn_hdr.s_nreloc == pri_scn))
			{
				num_reloc = scn_hdr.s_paddr;
				break;
			}
		}
	}
	else
		num_reloc = scn_hdr.s_nreloc;

	if (!fseek_OK(fp, File_origin + scn_hdr.s_relptr))
		return;

	for (i = num_reloc; i > 0; --i)
	{
		if (!fread_OK(&rel, RELSZ, fp))
			return;

		/* the type cast prints a negative value where appropriate */
		(void) printf("\t0x%.8lx  0x%.8lx  %4d  %5d  0x%.4x",
				rel.r_vaddr, rel.r_symndx, (rel.r_rsize
				& R_SIGN) >> 7, (rel.r_rsize & R_FIXUP) >> 6,
				rel.r_rsize & R_LEN);

		if (Symbolic)
		{
			(void) printf("%10s  %s\n",
				(rel.r_rtype <= R_RBRC)
					? get_rtypes((unsigned short)
					rel.r_rtype) : "???",
				(rel.r_symndx >= hdr.f_nsyms)
					? "???"
				: read_xcoff_symbol(fp, (long )rel.r_symndx));
		}
		else
			(void) printf("    0x%.4x\n", rel.r_rtype);
	}

}


read_xcoff_hdr(fp)
	FILE *fp;
{
	if (!fread_OK((char *) &hdr, FILHSZ, fp))
		return(-1);

	if ((hdr.f_magic != U802WRMAGIC) && (hdr.f_magic != U802ROMAGIC)
		&& (hdr.f_magic != U800WRMAGIC) && (hdr.f_magic != U800ROMAGIC)
		&& (hdr.f_magic != U802TOCMAGIC)
		&& (hdr.f_magic != U800TOCMAGIC))
	{
		return(-1);
	}

	/* Read in any optional header */
	if (hdr.f_opthdr) {
		if (opt_ptr)
			free(opt_ptr);

		if ((opt_ptr = (AOUTHDR *) malloc(hdr.f_opthdr)) == NULL) {
			error(MSGSTR(NO_MEM_MSG, NO_MEM));
			exit(-1);
		}

		if (!fread_OK((char *) opt_ptr, (int) hdr.f_opthdr, fp))
			return(-1);

		if (((hdr.f_opthdr != sizeof(AOUTHDR)) &&
		(hdr.f_opthdr != 0x1c)) || 
		((opt_ptr->magic != (short)0x0107) &&
		(opt_ptr->magic != (short)0x0108) &&
		(opt_ptr->magic != (short)0x010b)))
			Aout_Hdr = 0;
		else
		{
			if (hdr.f_opthdr == 0x1c)
				Aout_Hdr = -1;
			else
				Aout_Hdr = 1;
		}
	}
	else    /* No optional header */
		Aout_Hdr = 0;

	return(0);
}

char *
read_xcoff_symbol(fp, index)
	FILE *fp;
	long index;
{
	static SYMENT sym;
	long	position;

	position = ftell(fp);

	if (!fseek_OK(fp, File_origin + hdr.f_symptr + (long) (SYMESZ * index))
		|| !fread_OK((char *) &sym, SYMESZ, fp))
	{
		(void) fseek_OK(fp, position);
		return(MSGSTR(BAD_SYMBOL_NAME_MSG, BAD_SYMBOL_NAME));
	}

	(void) fseek_OK(fp, position);
	return(get_symbol_name(fp, &sym));
}

long
get_debug_offset(filep)
	FILE *filep;
{
	register int	i;
	long		position;
	SCNHDR		scnhdr;
	static char	*filename = NULL;
	static long	offset,
			origin = -1;

	/* see if we've already done this deck */
	if (filename == Current_file && origin == File_origin)
		return(offset);

	position = ftell(filep);
	filename = Current_file;
	origin = File_origin;

	if (!fseek_OK(filep, (long) (File_origin + FILHSZ + hdr.f_opthdr)))
	{
		error(MSGSTR(NO_SCN_HDR_MSG, NO_SCN_HDR));
		(void) fseek(filep, position, 0);
		return(-1);
	}

	for (i = 0; i < hdr.f_nscns; ++i)
	{
		if (!fread_OK((char *) &scnhdr, SCNHSZ, filep))
		{
			error(MSGSTR(SCN_HDR_ERR_MSG, SCN_HDR_ERR));
			(void) fseek(filep, position, 0);
			return(-1);
		}

		if (scnhdr.s_flags & STYP_DEBUG)
		{
			offset = scnhdr.s_scnptr;
			(void) fseek(filep, position, 0);
			return(offset);
		}
	}

	(void) fseek(filep, position, 0);
	return(-1);
} /* get_debug_offset */





static char *
get_classes(sc_class)
/*
 * Get the character string representation for the storage class.
 */
char sc_class;
{
	switch(sc_class)
	{
	case C_EFCN:	return("endfcn");
	case C_NULL:	return("???");
	case C_AUTO:	return("auto");
	case C_EXT:	return("extern");
	case C_STAT:	return("static");
	case C_REG:	return("reg");
	case C_EXTDEF:	return("extdef");
	case C_LABEL:	return("label");
	case C_ULABEL:	return("ulabel");
	case C_MOS:	return("strmem");
	case C_ARG:	return("arg");
	case C_STRTAG:	return("strtag");
	case C_MOU:	return("unmem");
	case C_UNTAG:	return("untag");
	case C_TPDEF:	return("typdef");
	case C_USTATIC:	return("ustat");
	case C_ENTAG:	return("entag");
	case C_MOE:	return("enmem");
	case C_REGPARM:	return("regprm");
	case C_FIELD:	return("bitfld");
	case C_BLOCK:	return("block");
	case C_FCN:	return("fcn");
	case C_EOS:	return("endstr");
	case C_FILE:	return("FILE");
	case C_LINE:	return("line");
	case C_ALIAS:	return("duptag");
	case C_HIDDEN:	return("hidden");
	case C_HIDEXT:	return("unamex");
	case C_BINCL:	return("bincl");
	case C_EINCL:	return("eincl");
	case C_INFO:	return("info");
	case C_GSYM:	return("gsym");
	case C_LSYM:	return("lsym");
	case C_PSYM:	return("psym");
	case C_RSYM:	return("rsym");
	case C_RPSYM:	return("rpsym");
	case C_STSYM:	return("stsym");
	case C_TCSYM:	return("tcsym");
	case C_BCOMM:	return("bcomm");
	case C_ECOML:	return("ecoml");
	case C_ECOMM:	return("ecomm");
	case C_DECL:	return("decl");
	case C_ENTRY:	return("entry");
	case C_FUN:	return("fun");
	case C_BSTAT:	return("bstat");
	case C_ESTAT:	return("estat");
	default:	return("???");
	}
}


static char *
get_rtypes(type)
/*
 * Get the character string representation for the relocation type.
 */
unsigned short type;
{
	switch(type)
	{
	case R_POS:	return("Pos_Rel");
	case R_NEG:	return("Neg_Rel");
	case R_REL:	return("Rel_Self");
	case R_TOC:	return("TOC_ILodx");
	case R_TRL:	return("TOC_ALodx");
	case R_TRLA:	return("Pos_ALod");
	case R_GL:	return("Ext_TOC");
	case R_TCL:	return("Loc_TOC");
	case R_RL:	return("Pos_ILod");
	case R_RLA:	return("Pos_Alod");
	case R_REF:	return("Non_Rel");
	case R_BA:	return("Brn_Abs");
	case R_RBA:	return("Brn_Absx");
	case R_RBAC:	return("Brn_AbsCx");
	case R_BR:	return("Brn_Sel");
	case R_RBR:	return("Brn_Selx");
	case R_RBRC:	return("Brn_AbsCx");
	case R_RTB:	return("RT_Brn");
	case R_RRTBI:	return("RT_Brnx");
	case R_RRTBA:	return("RT_ABrnx");
	default:	return("???");
	}
}



static char *
get_ldtype(type)
/*
 * Get the character string representation for the loader type.
 */
char type;
{
	switch(type)
	{
	case XTY_ER:	return("EXTref");
	case XTY_SD:	return("SECdef");
	case XTY_LD:	return("Ldef");
	case XTY_CM:	return("BSS");
	case XTY_EM:	return("LNKedt");
	case XTY_US:	return("unset");
	default:	return("???");
	}
}




static char *
get_typelist(type)
/*
 * Get the character string representation for the symbol type.
 */
unsigned short type;
{
	unsigned short	sl_id = (type >> 8),     /* Source Language ID */
			cpu_id = (type & 0xff);  /* CPU Version ID */
	register char *sl_string = "";

	if (cpu_id == TCPU_INVALID)
		/* PWR cpu assumed; can't know language for certain */
		return("???:PWR");

#define MAX_SL_STRING_LEN 20  /* chars to print source lang and cpu id's */
	sl_string = (char *) malloc(MAX_SL_STRING_LEN);

	switch(sl_id)
	{
	case TB_C:	strcpy(sl_string,"C:");
			break;
	case TB_FORTRAN: strcpy(sl_string,"FORTRAN:");
			break;
	case TB_PASCAL:	strcpy(sl_string,"Pascal:");
			break;
	case TB_ADA:	strcpy(sl_string,"Ada:");
			break;
	case TB_PLI:	strcpy(sl_string,"PL/I:");
			break;
	case TB_BASIC:	strcpy(sl_string,"BASIC:");
			break;
	case TB_LISP:	strcpy(sl_string,"LISP:");
			break;
	case TB_COBOL:	strcpy(sl_string,"COBOL:");
			break;
	case TB_MODULA2: strcpy(sl_string,"MODULA2:");
			break;
	case TB_CPLUSPLUS: strcpy(sl_string,"C++:");
			break;
	case TB_RPG:	strcpy(sl_string,"RPG:");
			break;
	case TB_PL8:	strcpy(sl_string,"PL8:");
			break;
	case TB_ASM:	strcpy(sl_string,"ASM:");
			break;
	case TB_OBJECT:	strcpy(sl_string,"OBJECT:");
			break;
	case TB_FRONT:	strcpy(sl_string,"FRONT:");
			break;
	case TB_BACK:	strcpy(sl_string,"BACK:");
			break;
	default:	strcpy(sl_string,"???:");
	}

	switch(cpu_id)
	{
	case TCPU_PWR:	return (strcat (sl_string,"PWR"));
	case TCPU_PPC:	return (strcat (sl_string,"PPC"));
	case TCPU_PPC64: return (strcat (sl_string,"PPC64"));
	case TCPU_COM:	return (strcat (sl_string,"COM"));
	case TCPU_ANY:	return (strcat (sl_string,"ANY"));
	case TCPU_601:	return (strcat (sl_string,"601"));
	case TCPU_PWR1:	return (strcat (sl_string,"PWR1"));
	case TCPU_PWRX:	return (strcat (sl_string,"PWRX"));
	default:	return (strcat (sl_string,"???"));
	}

}




static char *
get_stype(type)
/*
 * Get the character string representation for the csect type.
 */
unsigned char type;
{
	switch(type)
	{
	case XTY_ER:	return("ER");
	case XTY_SD:	return("SD");
	case XTY_LD:	return("LD");
	case XTY_CM:	return("CM");
	case XTY_EM:	return("EM");
	case XTY_US:	return("US");
	default:	return("??");
	}
}



static char *
get_sclass(class)
/*
 * Get the character string representation for the loaser storage class.
 */
unsigned char class;
{
	switch(class)
	{
	case XMC_PR:	return("PR");
	case XMC_RO:	return("RO");
	case XMC_DB:	return("DB");
	case XMC_GL:	return("GL");
	case XMC_XO:	return("XO");
	case XMC_SV:	return("SV");
	case XMC_TI:	return("TI");
	case XMC_TB:	return("TB");
	case XMC_RW:	return("RW");
	case XMC_TC0:	return("TC0");
	case XMC_TC:	return("TC");
	case XMC_DS:	return("DS");
	case XMC_UA:	return("UA");
	case XMC_BS:	return("BS");
	case XMC_UC:	return("UC");
	case XMC_TD:	return("TD");
	default:	return("??");
	}
}
