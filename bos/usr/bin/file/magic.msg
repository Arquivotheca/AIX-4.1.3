$ @(#)21        1.11  src/bos/usr/bin/file/magic.msg, cmdfiles, bos411, 9428A410j 2/22/94 10:41:59
$  COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$  FUNCTIONS: message file
$
$  ORIGINS: 27
$
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$  Licensed Materials - Property of IBM
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$quote "
$
$ $ Message numbers 950 - 959 reserved for this file.
$
$set MS_MAGIC
$ 
$ TRANSLATORS:  Please note that some of the lines in this file will be
$	appended together to form a single line of output.
$	Those lines which have a greater than sign (>) in the second
$	column might be appended in order to the end of the previous
$	lines.  For example:
$		 8  0       short           0x0103         some message
$		 9  >2      byte            0x50           addition_1
$		10  >28     long            >0             addition_2
$		11  >6      short           >0             addition_3
$	The four lines above can produce the following output:
$		some message
$		some message addition_1
$		some message addition_1 addition_2
$		some message addition_1 addition_2 addition_3
$
$
$ $ The following contains what used to be known as the /etc/magic
$ $ file.  If you are going to add magic entries, you must make sure that
$ $ you follow the current entries as an example.
$ $ ---
$ $              file cmd's magic file
$ $
$ $      Basically, the fields of this file are as follows:
$ $              1 byte offset,
$ $              2 value type,
$ $              3 optional relational operator ("=" by default) and
$ $                value to match (numeric or string constant),
$ $              4 string to be printed.
$ $
$ $      Numeric values may be decimal, octal, or hexadecimal.
$ $      Strings can be entered as hex values by preceding them with
$ $      '0x'.
$ $      Also, the last string may have one printf format specification.
$ $
$ $      The '>' in occasional column 2's is magic: it forces file to
$ $      continue scanning and matching additional lines. The first line
$ $      afterwards not so marked terminates the search.
$ $
     1	0	short		0x01df		executable (RISC System/6000) or object module
     2	>12	long		>0		not stripped
     3	0	short		0x0104		shared library
     4	0	long		505		message catalog
     5	0       string          0xabcdef        message catalog
     6	0       long            0x000001f9      compiled message catalog
     7	257	string		ustar		tar archive
     8	0       long            0x09006bea      backup/restore format file
     9	0	short		070707		cpio archive
    10	0	string		070707		ASCII cpio archive
    11	0	short		017436		packed text
    12	0	string		<aiaff>		archive
    13	0	string		Rast		RST format font raster
    14	0	short		0x0103		executable (RT Version 2) or obj module
    15	>2	byte		0x50		pure
    16	>28	long		>0		not stripped
    17	>6	short		>0		- version %ld
    18	0	short		0x0105		ctab data
    19	0	short		0xfe04		structured file
    20	0	short		0x1f9d		compressed data
    21	>2	byte		&0x80>0		block compressed
    22	>2	byte		&0x1f>0		%d bit
    23	0       string          <!OP		Interleaf "ascii" format
    24	1       string          OPS		Interleaf binary format
    25	1       string          [C		3812 print file
    26	0	string		BEGIN		awk script
    27	0	short		0xffd8		jpeg image file
    28	0	string		%!PS		PostScript file
    29	0	string		StartFontMetrics	Adobe PostScript Type1 font metrics file (afm) (afmbin)
    30	0	string		STARTFONT	Bitmap Distribution Format (bdf) font file (X11)
    31	1	string		fcp		(pcf) font file (X11R5) (LSB)
    32	0	string		pcf		(pcf) font file (X11R5) (MSB)
    33	0	string		D1.0		Speedo scalable font file (spd)
    34	0	string		From		unix mail file
    35	2	string		-lh		Lharc or LArc archive file
    36	2	string		-lz		Lharc or LArc archive file
    37	0	long		0x000001b3	MPEG (.mpg/.mpeg) compressed digital video
    38	0	string		PK		PKZIP (.zip) compressed archive
    39	1	string		PK		PKZIP (.zip) compressed archive
    40	0	string		.snd		audio file (.au)
    41	0	string		FORM		audio file (.iff)
    42	0	string		RIFF		audio file (.wav)
    43	0	string		Creative	Creative Voice File (.voc)
    44	0       string          GIF		Graphics Interchange Format (GIF) file
    45	0	string		ZOO 		Zoo archive file (PC)
    46	0	short		0x1a08		Arc archive file (PC)
    47	0       string          P1		Portable BitMap (PBM) file (ascii)
    48	0       string          P2		Portable GrayMap (PGM) file (ascii)
    49	0       string          P3		Portable PixMap (PPM) file (ascii)
    50	0       string          P4		Portable BitMap (PBM) file (raw)
    51	0       string          P5		Portable GrayMap (PGM) file (raw)
    52	0       string          P6		Portable Pixmap (PPM) file (raw)
    53	0	string		MZ		DOS executable (EXE)
    54	0	byte		0xe9		DOS executable (COM)
    55	0	byte		0xeb		DOS executable (COM)
    56	0	long		0xbeefface	OSF/Rose object
    57	>16	long		0x00000001	MIPS
    58	>16	long		0x00000002	National 32x32
    59	>16	long		0x00000003	Intel [34]86
    60	>16	long		0x00000004	Motorola 68xxx
    61	>28	long		0x00000002	Object file
    62	>28	long		0x0000000c	Program file
    63	>28	long		0x0000001c	Program file, unresolved references
    64	0	string	!<arch>			archive
    65	>8	string	__.SYMDEF		random library
    66	>8	string 	__________E		- MIPS
    67	0	long	0177555			very old pdp11 archive
    68	0	short	0177545			old pdp11 archive/4.0 archive
    69	0	string	<ar>			System V archive
    70	>20	string	U			with mipsucode members
    71	>21	string	L			with mipsel members
    72	>21	string	B			with mipseb members
    73	>19	string	L			and a EL hash table
    74	>19	string	B			and a EB hash table
    75	>22	string	X			-- out of date
    76	0	long	0100554			apl workspace
    77	0	short	017037			packed data
    78	0	short	017777			compacted data (4.1bsd)
    79	0	short	0116437			compressed data 
    80	0	string	begin			uuencoded data
    81	0	string	0x232120		shell script 
    82	>3      string  /bin/sh                 - sh (default shell)
    83	>3      string  /sbin/sh                - sh (default shell)
    84	>3      string  /usr/bin/sh             - sh (default shell)
    85	>3      string  /bin/bsh                - bsh (Bourne shell)
    86	>3      string  /sbin/bsh               - bsh (Bourne shell)
    87	>3      string  /usr/bin/bsh            - bsh (Bourne shell)
    88	>3      string  /bin/csh                - csh (C shell)
    89	>3      string  /usr/bin/csh            - csh (C shell)
    90	>3      string  /bin/ksh                - ksh (Korn shell)
    91	>3      string  /usr/bin/ksh            - ksh (Korn shell)
    92	>3      string  /bin/tcsh               - tcsh
    93	>3      string  /usr/bin/tcsh           - tcsh
    94	>3      string  /bin/rsh                - rsh (remote shell)
    95	>3      string  /usr/bin/rsh            - rsh (remote shell)
    96	>3      string  /bin/Rsh                - bsh (restricted)
    97	>3      string  /usr/bin/Rsh            - bsh (restricted)
    98	>3      string  /bin/psh                - psh (POSIX shell)
    99	>3      string  /usr/bin/psh            - psh (POSIX shell)
   100	0	string	#!			shell script 
   101	>2      string  /bin/sh                 - sh (default shell)
   102	>2      string  /sbin/sh                - sh (default shell)
   103	>2      string  /usr/bin/sh             - sh (default shell)
   104	>2      string  /bin/bsh                - bsh (Bourne shell)
   105	>2      string  /sbin/bsh               - bsh (Bourne shell)
   106	>2      string  /usr/bin/bsh            - bsh (Bourne shell)
   107	>2      string  /bin/csh                - csh (C shell)
   108	>2      string  /usr/bin/csh            - csh (C shell)
   109	>2      string  /bin/ksh                - ksh (Korn shell)
   110	>2      string  /usr/bin/ksh            - ksh (Korn shell)
   111	>2      string  /bin/tcsh               - tcsh
   112	>2      string  /usr/bin/tcsh           - tcsh
   113	>2      string  /bin/rsh                - rsh (remote shell)
   114	>2      string  /usr/bin/rsh            - rsh (remote shell)
   115	>2      string  /bin/Rsh                - bsh (restricted)
   116	>2      string  /usr/bin/Rsh            - bsh (restricted)
   117	>2      string  /bin/psh                - psh (POSIX shell)
   118	>2      string  /usr/bin/psh            - psh (POSIX shell)
   119	0	short	0401		Ultrix-11 Stand-alone or boot executable
   120	0	short	0405		old overlay
   121	0	short	0407		VAX executable (object file)
   122	>8	short	>0		not stripped
   123	0	long	0410		VAX pure executable
   124	>16	long	>0		not stripped
   125	>2	short	>0		- version %d
   126	0	short	0411		pdp-11 separate I&D
   127	>8	short	>0		not stripped
   128	>15	byte	>0		- version %ld
   129	0	short	0413		VAX demand paged pure executable
   130	>2	short	02		POSIX
   131	>2	short	01		SVID
   132	>16	long	>0		not stripped
   133	0	short	0430		Ultrix-11 overlay text kernel executable
   134	0	short	0431		Ultrix-11 user overlay (separated I&D) executable
   135	0	short	0437		pdp11 kernel overlay
   136	0	short	0450		Ultrix-11 overlay kernel executable
   137	0	short	0451		Ultrix-11 overlay kernel (separated I&D) executable
   138	0	short	0510		ns16000
   139	>28	short	0407		unpure text executable
   140	>28	short	0410		pure executable
   141	>28	short	0413		demand-paged executable
   142	>12	long	>0		not stripped
   143	>30	short	>0		- version %ld
   144	>30	short	0		-
   145	0	short	0515		ns16000
   146	>28	short	0407		unpure text executable
   147	>28	short	0410		pure executable
   148	>28	short	0413		demand-paged executable
   149	>12	long	>0		not stripped
   150	>30	short	>0		- version %ld
   151	>30	short	0		-
   152	0	short	0524		ns32000
   153	>18	short	&0004000	apc-II
   154	>18	short	&0010000	xpc532
   155	>18	short	&0020000	xpc w/381 fpu
   156	>18	short	&0040000	xpc w/580 fpu
   157	>20	short	0407		unpure text executable
   158	>20	short	0410		pure executable
   159	>20	short	0413		demand-paged executable
   160	>20	short	0417		unsupported executable
   161	>12	long	>0		not stripped
   162	>22	short	0		-
   163	0	short	0525		ns32000
   164	>18	short	&0004000	apc-II
   165	>18	short	&0010000	xpc532
   166	>18	short	&0020000	xpc w/381 fpu
   167	>18	short	&0040000	xpc w/580 fpu
   168	>20	short	0407		executable
   169	>20	short	0410		pure executable
   170	>20	short	0413		demand-paged executable
   171	>20	short	0417		unsupported executable
   172	>12	long	>0		not stripped
   173	>22	short	0		-
   174	0	short	0540		mipseb
   175	>20	short	0407		executable
   176	>20	short	0410		pure
   177	>20	short	0413		demand paged
   178	>8	long	>0		not stripped
   179	>8	long	<0		not stripped
   180	0	short	0542		mipsel
   181	>20	short	0407		407
   182	>20	short	0410		pure
   183	>20	short	0413		demand paged
   184	>8	long	0		executable
   185	>8	long	>0		not stripped
   186	>8	long	<0		not stripped
   187	0	short	0565		i386 COFF object
   188	0	short	0600		mipseb ucode
   189	0	short	0602		mipsel ucode
   190	0	short	060001		swapped mipseb
   191	>20	short	03401		executable
   192	>20	short	04001		pure
   193	>20	short	05401		demand paged
   194	>8	long	>0		not stripped
   195	>8	long	<0		not stripped
   196	0	short	061001		swapped mipsel
   197	>20	short	03401		407
   198	>20	short	04001		pure
   199	>20	short	05401		demand paged
   200	>8	long	0		executable
   201	>8	long	>0		not stripped
   202	>8	long	<0		not stripped
   203	0	string	head		RCS file
   204	0	long	04553207	X image
   205	0       long    0xdeadbeef      core file
   206	0	short		0x04fe		structured file
   207	0	short		017437		packed text
   208	0	short		0514		386 System V demand paged executable
   209	>12	long		>0		not stripped
   210	0	short		0537		370 XA sysV executable 
   211	>12	long		>0		not stripped
   212	>22	short		>0		- version %d
   213	>30	long		>0		- 5.2 format
   214	0	short		0535		370 sysV executable 
   215	>12	long		>0		not stripped
   216	>22	short		>0		- version %d
   217	>30	long		>0		- 5.2 format
   218	0	short		0532		370 XA sysV pure executable 
   219	>12	long		>0		not stripped
   220	>22	short		>0		- version %d
   221	>30	long		>0		- 5.2 format
   222	0	short		0530		370 sysV pure executable 
   223	>12	long		>0		not stripped
   224	>22	short		>0		- version %d
   225	>30	long		>0		- 5.2 format
   226	0	short		054001		370 sysV pure executable
   227	>12	long		>0		not stripped
   228	0	short		055001		370 XA sysV pure executable
   229	>12	long		>0		not stripped
   230	0	short		056401		370 sysV executable
   231	>12	long		>0		not stripped
   232	0	short		057401		370 XA sysV executable
   233	>12	long		>0		not stripped
   234	0	short		0502		basic-16 executable
   235	>12	long		>0		not stripped
   236	0	short		0503		basic-16 executable (TV)
   237	>12	long		>0		not stripped
   238	0	short		0511		x86 executable (TV)
   239	>12	long		>0		not stripped
   240	0	short		0550		3b20d executable
   241	>12	long		>0		not stripped
   242	0	short		0551		3b20d executable (TV)
   243	>12	long		>0		not stripped
   244	0	short		0560		3b5 executable
   245	>12	long		>0		not stripped
   246	0	short		0561		3b5 executable (TV)
   247	>12	long		>0		not stripped
   248	0	short		0570		vax sysV executable
   249	>12	long		>0		not stripped
   250	>22	short		>0		- version %ld
   251	0	short		0575		vax sysV pure executable
   252	>12	long		>0		not stripped
   253	>22	short		>0		- version %ld
   254	0	short		074001		vax SysV executable
   255	>12	long		>0		not stripped
   256	>22	short		>0		- version %d
   257	0	short		076401		vax SysV pure executable
   258	>12	long		>0		not stripped
   259	>22	short		>0		- version %d
   260	0	short		02401		vax bsd overlay
   261	0	short		03401		vax bsd executable
   262	>16	long		>0		not stripped
   263	>2	short		>0		- version %d
   264	0	short		04001		vax bsd pure executable
   265	>16	long		>0		not stripped
   266	>2	short		>0		- version %d
   267	0	short		04401		vax bsd separate I&D executable
   268	>16	long		>0		not stripped
   269	>2	short		>0		- version %d
   270	0	short		05401		vax bsd demand paged executable
   271	>16	long		>0		not stripped
   272	>2	short		>0		- version %d
   273	0	short		046001		386 System V demand paged executable
   274	>12	long		>0		not stripped
   275	0	short		072401		386 AIX demand paged executable
   276	>12	long		>0		not stripped
   277	0	byte		0x80		8086 relocatable
   278	0	short		0xff65		Xenix archive 
   279	0	short		062777		Xenix archive
   280	1	string		makefile	PBE makefile
   281	0      string          <MakerFile      FrameMaker document
   282	>11    string          3.0              (3.0
   283	>11    string          2.0              (2.0
   284	>11    string          1.0              (1.0
   285	>14    byte            x                 %c)
   286	0      string          <MIFFile        FrameMaker MIF file
   287	>9     string          3.0              (3.0)
   288	>9     string          2.0              (2.0)
   289	>9     string          1.0              (1.x)
   290	0      string          <MakerDictionar FrameMaker Dictionary file
   291	>17    string          2.0              (2.0)
   292	>17    string          1.0              (1.x)
   293	0      string          <MakerScreenFon FrameMaker Font file
   294	>17    string          1.01             (%s)
   295	0      string          <MML            FrameMaker MML file
   296	0      string          <Book           FrameMaker Book file
   297	>10    string          2.0              (2.0
   298	>10    string          1.0              (1.0
   299	>13    byte            x                 %c)
   300	0      string          Maker Intermediate Print File   FrameMaker IPL file
