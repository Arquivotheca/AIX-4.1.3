$ @(#)26        1.13  src/bos/usr/ccs/bin/yacc/yacc.msg, cmdlang, bos411, 9428A410j 10/7/93 12:20:47
$
$ COMPONENT_NAME: (CMDLANG) Language Utilities
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_YACC
$quote "
$len
$ Message numbers have been assigned a range of 700-799
$ Do not translate yacc, %d, %s, %1$d, %1$s, etc ...
$ In this message file only do not translate terminal, nonterminal, token, type.
$ These are very specific yacc technical terms.
NOPARSER        "1285-700 Cannot find parser %s"
NOYACDEBUG      "1285-701 Cannot open yacc.debug"
$ Do not translate yacc.debug
ETEMPF          "1285-702 Cannot open action temporary file."
EITEM           "1285-703 A terminal or nonterminal is too big."
SUMMARY1        "\n%d/%d terminals, %d/%d nonterminals\n"
$ Do not translate.
SUMMARY2        "%d/%d grammar rules, %d/%d states\n"
$ Do not translate.
SUMMARY3        "%d shift/reduce, %d reduce/reduce conflicts reported\n"
$ Do not translate.
SUMMARY4        "%d/%d working sets used\n"
$ Do not translate.
SUMMARY5        "memory: states,etc. %d/%d, parser %d/%d\n"
$ Do not translate.
SUMMARY6        "%d/%d distinct lookahead sets\n"
$ Do not translate.
SUMMARY7        "%d extra closures\n"
$ Do not translate.
SUMMARY8        "%d shift entries, %d exceptions\n"
$ Do not translate.
SUMMARY9        "%d goto entries\n"
$ Do not translate.
SUMMARY10       "%d entries saved by goto default\n"
$ Do not translate.
CONFLSR         "\nconflicts: %d shift/reduce\n"
$ Do not translate.
CONFLRR         "\nconflicts: %d reduce/reduce\n"
$ Do not translate.
CONFLSRR        "\nconflicts:%d shift/reduce, %d reduce/reduce\n"
$ Do not translate.
FATALERR         "\n yacc: %1$s Line %2$d\n"
$ Do not move %1$s
NONTERM         "1285-704 The nonterminal %s is not defined."
YACINTER        "1285-705 Internal error; pyield is %d."
$ Do not translate pyield.
NOLOOK          "1285-706 Cannot look ahead as state requires."
ESTATE          "1285-707 Cannot have more than 5000 states."
PITEM           "putitem(%S), state %d\n"
$ This is a debug message. Do not translate.
ESTATE2         "1285-708 Use the -Nm flag to increase state size."
$ Do not translate -Nm
NONTERM1        "1285-709 The nonterminal %s does not generate a token string."
WSET            "1285-710 The working set is full."
$ working set is a internal array for work in progress.
STATELOOK       "\nState %d, nolook = %d\n"
$ This is a debug message. Do not translate.
FSET            "flag set!\n"
$ This is a debug message. Do not translate.
OFLAG           "yacc: 1285-770 The -o flag is now the default in yacc.\n"
$ Do not translate -o
NORATFOR        "1285-711 Ratfor yacc is not supported."
BADOPT          "1285-712 The %s flag is not valid."
NODEBUG         "1285-713 Cannot open yacc.debug"
$ Do not translate yacc.debug
NOTOPN          "1285-714 Cannot open %s"
NOTEMP          "1285-715 Cannot open temporary file."
NOINPUT         "1285-716 There is no specified input file."
BADSTART        "1285-717 The %%start statement is not correct."
$ Do not translate %%start
BADSYNTAX       "1285-718 The %%type syntax is not correct."
$ Do not translate %%type
EREDECLR        "1285-719 The type of token %s is already declared."
EREDEC2         "1285-720 The type of nonterminal %s is already declared."
EPRECID         "1285-721 The precedence of %s is already declared."
ERETYPE         "1285-722 The type of %s is already declared."
LATETYPE        "1285-723 Define type number of %s earlier."
SYNTAX          "1285-724 There is a syntax error."
PEOF            "1285-725 Reached end of file before %%%%"
$ Do not translate %%%%
SNTAX1          "1285-726 Syntax on first rule is not correct."
BADLHS          "1285-727 Cannot have a token on LHS of grammar rule."
$ LHS is left hand side
BADDRULE        "1285-728 There appears to be a missing ; or | in the rule."
$ Do not translate ; or |
BADPREC         "1285-729 The %%prec syntax is not correct."
$ Do not translate %%prec
NONTERM2        "1285-730 Cannot have nonterminal %s after %%prec"
$ Do not translate %%prec
MANYRULES       "1285-731 Cannot have more than %d rules."
NOVALUE         "1285-732 LHS has a type; rule must return a value."
$ LHS is left hand side
TCLASH          "1285-733 A type conflict may occur because of default action."
MANYNTERMS      "1285-734 Cannot have more than %d nonterminals."
MANYTERMS       "1285-735 Cannot have more than %d terminals."
ESCINVAL        "1285-736 The character escape is not valid."
BADCONSTRT      "1285-737 The \\nnn construction is not valid."
$ \\nnn construction is referring to a coded entity in one of the input lines.
$ Do not translate \\nnn
OZERO           "1285-738 \\000 is not valid."
$ Do not translate \\000
ELIT            "1285-739 The identifiers and literals have too many characters."
UNTERM          "1285-740 A < ... > clause has no end."
$ Do not translate < ... >
MISSQUOTE       "1285-741 There is an extra or a missing ' or \" character."
ILLWORD         "1285-742 The escape is not valid or %s is not a reserved word."
NOTYPE          "1285-743 Must specify a type for %s."
NONAME          "1285-744 Define %s earlier."
FEOF            "1285-745 Reached end of file before %%}"
$ Do not translate %%)
BADCOMNT        "1285-746 The comment is not correct."
BADIDENT        "1285-747 Syntax on the $<identifier> clause is not correct."
$ Must have $<> around the word indentifier, identifier can be translated.
ILLOFF          "1285-748 Use of $%d is not valid."
$ Do not translate $%d
NOTYPE2         "1285-749 Must specify type of $%d"
$ Do not translate $%d
COMTEOF         "1285-750 Reached end of file inside comment."
NLINSTR         "1285-751 Newline found in string or character constant."
EOFINSTR        "1285-752 Reached end of file in string or character constant."
NONTERMACT      "1285-753 Action does not end."
REDRED1         "\n%d: reduce/reduce conflict (red'ns %d and %d ) on %s"
$ Do not translate.
OFF1            "off = %d, k = %d\n"
$ This is a debug message. Do not translate.
ACTOVR          "1285-754 The action table is full."
ACTOVR2         "1285-755 The action table is full."
GOTOON          "%s: gotos on "
$ This is a debug message. Do not translate.
SFTRED1         "\n%d: shift/reduce conflict (shift %d, red'n %d) on %s"
$ Do not translate.
ACCEPT          "accept"
$ Do not translate.
MERROR          "error"
$ Do not translate.
SHIFT           "shift %d"
$ Do not translate.
EREDUCE         "reduce %d"
$ Do not translate.
CRREDUCE        "\n\t.  reduce %d\n\n"
$ Do not translate.
CRERROR         "\n\t.  error\n\n"
$ Do not translate.
NOTRED          "Rule not reduced:   %s\n"
$ Do not translate.
NEVRRED         "yacc: 1285-756 %d rules are never reduced.\n"
BADTEMP         "1285-757 The temporary file is corrupted."
ARYOVR          "1285-758 The internal a array is full."
$ Do not translate a
NONTERM3        "Nonterminal %d, entry at %d\n"
$ Do not translate.
CANTGOTO        "1285-759 Cannot put goto %d in array.\n"
$ Do not translate goto
STENTEQ         "State %d: entry at %d equals state %d\n"
$ Do not translate.
OPTSPC          "1285-760 Out of space in optimizer a array."
$ Do not translate a
ARYCLBR         "1285-761 The a array is corrupted at position %1$d, %2$d."
$ Do not translate a
STATENT         "State %d: entry at %d\n"
$ Do not translate.
STATERR         "1285-762 Cannot put state %d in array.\n"
OPTUSED         "Optimizer space used: input %d/%d, output %d/%d\n"
$ Do not translate.
OPTTAB          "%d table entries, %d zero\n"
$ Do not translate.
OPTSPRED        "maximum spread: %d, maximum offset: %d\n"
$ Do not translate.
OUTOSPC         "1285-763 Use -Nm flag to increase space size."
$ Do not translate -Nm
NOOPT           "1285-764 The optimizer cannot open the temporary file."
PREMIS          "1285-765 Specify a prefix with the -b flag."
OUTMEM          "1285-766 There is not enough memory available now."
NMNUM           "1285-767 Use an integer greater than 40000 with the -N flag."
NOPT            "1285-768 The -N flag format is not correct."
EOFENC          "1285-769 Reached end of file while processing %%union"
$ Do not translate %%union
NNNUM           "1285-770 Use an integer greater than 8000 with the -Nn flag."
$ Do not translate -Nn
NNOPT            "1285-771 The -Nn flag format is not correct."
$ Do not translate -Nn
OUTNMSPC         "1285-772 Use -Nn flag to increase name space size."
$ Do not translate -Nn
OUTBUFSPC        "1285-773 Use -Nr flag to increase buffer size to handle large grammars (default size is %d)."
$ Do not translate -Nr, %d
NRNUM            "1285-774 Use an integer greater than %d with the -Nr flag."
$ Do not translate -Nr, %d
PPREMIS          "1285-775 Specify a prefix with the -p flag."
FATALERR2        "\n yacc: %1$s\n"
$ Do not move %1$s
YPREMIS          "1285-776 Specify a parser with the -y flag."
