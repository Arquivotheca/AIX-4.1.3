/* This file was generated from ed.keys, and should not be directly modified */
/* ../../../../src/tenplus/keys/keys.c sid keywords are src/tenplus/keys/keys.c, tenplus, tenplus410, tenp 7/19/91 14:46:33 */

static struct nstruct {
	char *name;
	unsigned char value;
} names [] = {
	"(1)",			1,	/* U1               */
	"(2)",			2,	/* U2               */
	"(3)",			3,	/* U3               */
	"(4)",			4,	/* U4               */
	"(5)",			5,	/* U5               */
	"(6)",			6,	/* U6               */
	"(7)",			7,	/* U7               */
	"(8)",			8,	/* U8               */
	"+LINE",		43,	/* PLUS_LINE        */
	"+PAGE",		44,	/* PLUS_PAGE        */
	"+SEARCH",		45,	/* PLUS_SEARCH      */
	"-LINE",		40,	/* MINUS_LINE       */
	"-PAGE",		41,	/* MINUS_PAGE       */
	"-SEARCH",		42,	/* MINUS_SEARCH     */
	"-TAB",			19,	/* MINUS_TAB        */
	"ALT_MENU",		83,	/* UNUSED           */
	"BACK-UP",		106,	/* BACK_UP          */
	"BACKSPACE",		18,	/* BACKSPACE        */
	"BADKEY",		65,	/* BADKEY           */
	"BEGIN-LINE",		63,	/* BEGIN_LINE       */
	"BIND",			84,	/* UNUSED           */
	"BOX-MARK",		72,	/* BOX_MARK         */
	"CANCEL",		20,	/* CANCEL           */
	"CENTER",		21,	/* CENTER           */
	"CHANGEFILE",		69,	/* CHANGEFILE       */
	"CHFILE",		96,	/* CHFILE           */
	"CHFORM",		95,	/* CHFORM           */
	"CLEAR",		94,	/* CLEAR            */
	"CMOUSE",		89,	/* UNUSED           */
	"CONCEPT",		101,	/* CONCEPT          */
	"CORRECT",		112,	/* CORRECT          */
	"DELETE",		74,	/* DELETE           */
	"DELETE-CHARACTER",	23,	/* DELETE_CHARACTER */
	"DEL_COPY",		75,	/* UNUSED           */
	"DISPLAY",		92,	/* DISPLAY          */
	"DO",			80,	/* DO               */
	"DOWN-ARROW",		25,	/* DOWN_ARROW       */
	"END-DISPLAY",		93,	/* END_DISPLAY      */
	"END-LINE",		64,	/* END_LINE         */
	"ENTER",		26,	/* ENTER            */
	"EXECUTE",		24,	/* EXECUTE          */
	"EXIT",			28,	/* EXIT             */
	"EXPAND",		85,	/* UNUSED           */
	"FONT",			29,	/* FONT             */
	"FORMAT",		30,	/* FORMAT           */
	"GLOSSARY",		86,	/* UNUSED           */
	"GO-TO",		31,	/* GO_TO            */
	"HELP",			34,	/* HELP             */
	"HELP-LOAD",		103,	/* HELP_LOAD        */
	"HOME",			33,	/* HOME             */
	"INSERT",		10,	/* INSERT           */
	"INSERT-MODE",		35,	/* INSERT_MODE      */
	"LAST-ARG",		78,	/* LAST_ARG         */
	"LEARN",		87,	/* UNUSED           */
	"LEFT",			36,	/* LEFT             */
	"LEFT-ARROW",		37,	/* LEFT_ARROW       */
	"LINE-FEED",		38,	/* LINE_FEED        */
	"LMOUSE",		27,	/* UNUSED           */
	"LOCAL-MENU",		9,	/* LOCAL_MENU       */
	"MACRO",		82,	/* UNUSED           */
	"MARGIN",		39,	/* MARGIN           */
	"MENU",			32,	/* MENU             */
	"MENU-ENTRY",		99,	/* MENU_ENTRY       */
	"MENU-HELP",		100,	/* MENU_HELP        */
	"MENU-LOAD",		98,	/* MENU_LOAD        */
	"MOVIE-MENU",		111,	/* MOVIE_MENU       */
	"NEXT",			17,	/* NEXT             */
	"NEXT-WINDOW",		22,	/* NEXT_WINDOW      */
	"PAUSE",		90,	/* PAUSE            */
	"PICK-COPY",		12,	/* PICK_COPY        */
	"PICK-UP",		11,	/* PICK_UP          */
	"PREV-MENU",		105,	/* PREV_MENU        */
	"PREVIOUS",		79,	/* PREVIOUS         */
	"PRINT",		46,	/* PRINT            */
	"PUT-COPY",		13,	/* PUT_COPY         */
	"PUT-DOWN",		14,	/* PUT_DOWN         */
	"QUOTE",		47,	/* QUOTE            */
	"REFRESH",		48,	/* REFRESH          */
	"REPEAT",		102,	/* REPEAT           */
	"REPLACE",		49,	/* REPLACE          */
	"REQUIRE",		91,	/* REQUIRE          */
	"RESTORE",		76,	/* RESTORE          */
	"RESTORE_COPY",		77,	/* UNUSED           */
	"RETURN",		50,	/* RETURN           */
	"RIGHT",		51,	/* RIGHT            */
	"RIGHT-ARROW",		52,	/* RIGHT_ARROW      */
	"RMOUSE",		81,	/* UNUSED           */
	"ROFIELD",		71,	/* ROFIELD          */
	"ROFILE",		70,	/* ROFILE           */
	"SAVE",			53,	/* SAVE             */
	"SET-TAB",		55,	/* SET_TAB          */
	"SETCURSOR",		62,	/* SETCURSOR        */
	"SHOWMOVIE",		97,	/* SHOWMOVIE        */
	"SKIP",			104,	/* SKIP             */
	"START-SCREEN",		110,	/* START_SCREEN     */
	"TAB",			54,	/* TAB              */
	"TEXT-KEY",		56,	/* TEXT_KEY         */
	"TEXT-MARK",		73,	/* TEXT_MARK        */
	"TOO-MANY",		108,	/* TOO_MANY         */
	"TUT-HELP",		109,	/* TUT_HELP         */
	"UDEL",			68,	/* UDEL             */
	"UINS",			67,	/* UINS             */
	"UMOD",			66,	/* UMOD             */
	"UNREPLACE",		88,	/* UNREPLACE        */
	"UNUSED",		60,	/* UNUSED           */
	"UP-ARROW",		57,	/* UP_ARROW         */
	"USE",			58,	/* USE              */
	"WINDOW",		59,	/* WINDOW           */
	"WRAPKEY",		61,	/* WRAPKEY          */
	"WRONG",		107,	/* WRONG            */
	"XFUNCTIONS",		113,	/* XFUNCTIONS       */
	"ZOOM-IN",		15,	/* ZOOM_IN          */
	"ZOOM-OUT",		16,	/* ZOOM_OUT         */
};

static struct pairs {
	char *extname;
	char *intname;
} namepairs [] = {
	"LOCAL-1",		"(1)",
	"LOCAL-2",		"(2)",
	"LOCAL-3",		"(3)",
	"LOCAL-4",		"(4)",
	"LOCAL-5",		"(5)",
	"LOCAL-6",		"(6)",
	"LOCAL-7",		"(7)",
	"LOCAL-8",		"(8)",
	"MINUS-LINE",		"-LINE",
	"MINUS-PAGE",		"-PAGE",
	"MINUS-SEARCH",		"-SEARCH",
	"PLUS-LINE",		"+LINE",
	"PLUS-PAGE",		"+PAGE",
	"PLUS-SEARCH",		"+SEARCH",
	"U1",			"(1)",
	"U2",			"(2)",
	"U3",			"(3)",
	"U4",			"(4)",
	"U5",			"(5)",
	"U6",			"(6)",
	"U7",			"(7)",
	"U8",			"(8)",
};
