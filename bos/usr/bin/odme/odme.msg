$ @(#)27	1.8 src/bos/usr/bin/odme/odme.msg, cmdodm, bos411, 9428A410j 4/6/93 12:06:22
$
$  COMPONENT_NAME: (ODME) ODME.MSG - message catalogue for ODME
$
$  FUNCTIONS:
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set 1
$ messages for odme.c
$quote "

$ module = MAIN

5 "Object Data Manager Editor"

$ TRANSLATOR- For messages 15 through 50,
$ Please limit the length of the quoted string
$ to 26 characters (including leading spaces).

15    " Set Default Object Class"
20    " Display relational graphs"
25    " Create "
30    " Selective search"
35    " Retrieve/Edit "
40    " Object Class Management"
45    " Set .Master Path"
50    " Delete object class"


$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

55 "Main menu help\n\
 The Main menu screen allows you to perform functions by\n\
 high-lighting the function using the arrow keys and    \n\
 pressing RETURN.                                       \n\
 - Display relational graphs gives a graphical overview \n\
   of how each object class relates to another.         \n\
 - Create allows you to create an object class.         \n\
 - Selective search allows you to enter search criteria \n\
   for retrieving only the objects you want.            \n\
 - Retrieve/Edit returns all the objects in the class.  \n\
 - Object Class Management allows you to specify list of\n\
   object classes.                                      \n\
 - Delete allows entire object classes to be removed.   \n"

60 "Warning - All objects will be lost. Continue?"
65 "OBJECT CLASS DELETE\n\
 ***** SUCCESSFUL *****\n"

$ module = OBJECT_NAME_EDIT

70 "Object Class Parameters"

$ TRANSLATOR- The words Name, Node, and Path in the following 3 messages
$ should not extend past their current rightmost letters. Adjust left if
$ necessary.

75 "             Name                      "
80 "             Node [%-*.*s]"
85 "             Path                      "

90 "         Open as [Read only] "
95 "         Open as [Read/Write]"

$ module = SET_MASTER_PATH

100 "Path of .Master"

$ TRANSLATOR- The words .Master Path in the follwing message should not
$ extend past the last character of the word Path. Eliminate the word .Master
$ if necessary and just specify Path. Brackets should not be moved.

105 ".Master Path [                    ]"

110 "Class disallowed in odme.\n\
The selected class is disallowed within odme.\n\
The following classes cannot be edited with odme:\n\
        product           Config_Rules        CuVPD\n\
        sm_cmd_opt        sm_name_hdr\n\
        sm_cmd_hdr        sm_menu_opt\n"

$set 2
$ Messages for odmecontrol.c
$ module = CONTROL
$
1 "Link Graph"
2 "Selective Search"
3 "Retrieve/Edit"
4 "Delete"
5 "Return"
10 "OBJECT CLASS MANAGEMENT"
11 "Read Directory?"

20 "Screen %-3d of  %-3d"

22 "WARNING - all objects will be lost. Continue?"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

25 "Object Class Management help \n\
 This function allows you to enter the       \n\
 names of multiple object classes to         \n\
 manipulate, or retrieve them from the       \n\
 default directory.                          \n\
 After entering the names of the object      \n\
 classes you wish to work with, press Enter  \n\
 without entering an object class name to    \n\
 continue. Select the functions key          \n\
 as described at the bottom of the           \n\
 screen to select the desired function       \n\
 to perform on the highlighted object class. \n\
"
30 "OBJECT CLASS DELETE\n\
 ***** SUCCESSFUL *****\n"
35 "*FUNCTIONS*"
40 "Selected Object Class ==> "

$set 3
$ Messages for odmecreate
$ module = CREATE_CLASS
$
5 "Create Object Class"
10 "Object Class : %s"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

15 "Create help \n\
 The Create screen allows you to specify descriptors \n\
 for the object class to be created.                 \n\
                                                    \n\
 Select Add (<Esc>4 or PF4) and enter your descriptor \n\
 name, type, and iterator.  Continue (<Esc>1 or PF1) \n\
 and enter the link object class or other specifed   \n\
 fields.                                             \n\
                                                     \n\
 Upon exit of the create screen (<Esc>3 or PF3) you  \n\
 will be asked if you wish to create this object     \n\
 class.                                              \n\
"
20 "Create Object Class ?"
25 "PERMISSIONS"

$ TRANSLATOR- The words Owner, Group, and Other can be translated but
$ The messages inside the brackets should remain the same for messages
$ 30 through 52.

30 "Owner [Write]    ]"
32 "Owner [Read/Write]"
34 "Owner [None      ]"
36 "Owner [Read      ]"
38 "Group [Write     ]"
40 "Group [Read/Write]"
42 "Group [None      ]"
44 "Group [Read      ]"
46 "Other [Write     ]"
48 "Other [Read/Write]"
50 "Other [None      ]"
52 "Other [Read      ]"

$ edit_descriptor
54 " Permissions for object class"

55 " Descriptor Edit"
56 " Object Link Descriptor Edit"
57 " Link Descriptor Edit"
58 " Character Descriptor Edit"
59 " Binary Descriptor Edit"

$ TRANSLATOR- information inside brackets should not change, and brackets
$ should not be moved. Messages 60 through 70.

60 "Name [%-*.*s]"
65 "Type [%s]"
70 "Iterator [%-3d]"

$ TRANSLATOR- The word Type can be translated but the values inside the
$ brackets should not be translated. Messages 72 through  90.
72 "Type [ODM_LONG]         "
74 "Type [ODM_LONGCHAR]     "
75 "Type [ODM_VCHAR]        "
76 "Type [ODM_CHAR]         "
78 "Type [ODM_REPEAT]       "
80 "Type [ODM_LINK]         "
82 "Type [ODM_VLINKCLASS]   "
84 "Type [ODM_IMBED]        "
86 "Type [ODM_METHOD]       "
88 "Type [ODM_BINARY]       "
90 "Type [ODM_SHORT]        "

$ edit_by_type

$ TRANSLATOR- The information inside the brackets should not change and
$ the brackets should not be moved. Messages 95 and 100.

95 "Object Class        [%-*.*s]"
97 "Link object class [%-*.*s]"
100 "Link     [%-*.*s]"

$ TRANSLATOR- The information inside the brackets should not change.
$ Messages 105 and 110.

105 "Key Descriptor [NO_KEY]"
110 "Key Descriptor [KEY]   "

$ TRANSLATOR- The information inside the brackets should not change and
$ the brackets should not be moved. Truncate message to just Size if
$ required. Message 115.

115 "Field Size [%-4d]"


$set 4
$ messages for ODMECRITERIA.C
$ module = search_criteria

5 "Selective Search"
10 "Object Class : %s"

15 " <Esc>1=Help   <Esc>2=Search   <Esc>3=EXIT   <Esc>7=PgUp   <Esc>8=PgDown"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

20 "Criteria help \n\
 The Criteria screen allows you to enter SQL like  \n\
 syntax for searching the object class.            \n\
                                                   \n\
      What ODME displays        What you type      \n\
                                                   \n\
     Programmer  (char 23) ==>   like 'HAL *'      \n\
     Age         (short 7) ==>   = 23              \n\
                                                   \n\
 Pressing Search (<Esc>2 or PF2) will return all   \n\
 objects where the descriptors satisfy the criteria \n\
 (Programmer like 'Dave *') and (Age = 23).        \n\
                                                   \n\
 Entering no search string returns all objects.    \n\
"

$set 5
$ messages for ODMENEWOBJ.C
$ module = make_new_obj

5 "New Object Class"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

10 "Create New Object Class \n\
 This function will allow you to create a new object \n\
 class from the information currently being edited. \n\
 The new object class will have the same descriptor \n\
 definitions as the object it is being created from. \n\
 Press F2 to create the object class or F3 to exit. \n\
 This function allows you to copy an object         \n\
 class and all of its contents, or create an object \n\
 class from information returned from Selective     \n\
 Search operations.                                 \n\
"
15 "Create Object Class ?"
20 "Create was successful"

$set 6
$ messages for ODMEOBJDISP.C
$ module = object_display

5 "Object Display"
10 "Object Class : %-*.*s  Object: %-5d  Descriptor: %-3d of  %-3d"
15 "LOADING..."

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

20 "Display help \n\
                                                 \n\
 The Display screen allows the user to browse and edit objects  \n\
 in the object class.                                           \n\
 If the object class has been opened in read/write mode the user \n\
 can add objects (<Esc>4 or PF4), copy objects (<Esc>6 or PF6), \n\
 delete objects (<Esc>5 or PF5), and edit the current object.   \n\
 On exit (<Esc>3 or PF3) you will be asked if you want to commit \n\
 any changes you may have made.                                 \n\
 NOTE*  ODM always makes changes at the end of the object class, \n\
        so relative positions of objects may change.            \n\
"
25 "COMMIT changes ?"

$set 7
$ messages for ODMEOBJSTORE.C
2 "DELETE SUCCESSFUL"

$set 8
$ messages for ODMERELGRAPH.C
$ module = CONTROL

2 "Link Graph"
4 "Retrieve/Edit"
6 "Display Descriptors"
8 "Return"

10 "Relational Graph"
15 "Screen %-3d of  %-3d"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

20 "Relational Graphs help \n\
 This function will allow you to see the object \n\
 classes which are linked to the selected       \n\
 Object Class.                                  \n\
                                                \n\
 Additional functions include:                  \n\
  - Display Relational Graphs for the           \n\
      highlighted object class.                 \n\
  - Retrieve/Edit objects of the                \n\
      highlighted object class.                 \n\
  - Display the descriptors of the              \n\
      highlighted object class.                 \n\
"
$ module = disp_my_desc

25 "Display Descriptors"
30 "Selected Object Class ==> "
35 "Screen %-3d of  %-3d"

$ TRANSLATOR- For the following HELP message,
$ Please keep sentence width under 64 characters and
$ the number of lines below 18.

40 "Display Descriptors help \n\
 This function will display the descriptors for \n\
 the selected object class. The NAME field will \n\
 identify the descriptor, the TYPE field will  \n\
 show the descriptors type, and the LINKS TO   \n\
 field will show the object class that this    \n\
 descriptor links to, if any.                  \n\
 Page up or page down will allow you to display \n\
 additional pages of descriptors.              \n\
"


$ TRANSLATOR- The following heading is for column information.
$ Translated names should line up with the start of current words.

45 "DESCRIPTOR NAME           TYPE               LINKS TO"

$set 9
$ messages for ODMEWINDOW.C
5 "Yes or No"
10 "Signal = "
15 "<Esc>3 quit odme"
20 "<Esc>1 Return  <Esc>3 Quit odme"
25 "\n-----Object Data Manager Editor Print-----\n\n"


$set 10
$ These are the keys for the bottom of the screen.
$
$ Keys for ODME.C (main)

$ TRANSLATOR-  The following messages must not exceed a single line.

10 "                      <Esc>1=Help           <Esc>3=QUIT "
15 "                   <Esc>1=Continue         <Esc>3=Discontinue "

16 "           <Esc>1=Help  <Esc>2=Functions  <Esc>3=Exit/Return "

18 " <Esc>1=Help   <Esc>2=Edit   <Esc>3=EXIT/Create   <Esc>4=Add   <Esc>5=Delete "

20 "                        <Esc>1=Help   <Esc>3=EXIT/Return "

22 " <Esc>1=Help   <Esc>2=Search   <Esc>3=EXIT     <Esc>4=Add    <Esc>5=Delete"
24 " <Esc>6=Copy   <Esc>7=PgUp     <Esc>8=PgDown   <Esc>9=Left   <Esc>0=Right "

26 " <Esc>1=Help   <Esc>2=Search   <Esc>3=EXIT   <Esc>7=PgUp   <Esc>8=PgDown"

$set 12
$ These are the error messages for ODME
$
$ Error msgs for ODME.C (main)

2 "ERROR - object class exists. Cannot create"
4 "Delete Failed"

$ Error msgs for ODMECONTROL.C

6 "Open of OBJREPOS failed"
8 "Opened Read_Only. Cannot delete"

$ Error msgs for ODMECREATE.C

10 "Fatal Error - Malloc failed"
12 "CREATE Failed! error = "
14 "Print Screen Error\n"
15 "INPUT ERROR - Link object class and Link to fields required."
16 "Error - More Data Required"
17 "INPUT ERROR - Field size is required."

$ Error msgs for ODMECRITERIA.C

18 "Error - search string too long"

$ Error msgs for ODMENEWOBJ.C

20 "Fatal Error - Malloc failed"
22 "CREATE Failed! error = "
24 "Error - Add failed! error = "

$ Error msgs for ODMEOBJDISP.C

26 "No objects returned"
28 "Fatal Error! Delete malloc failed"
30 "Error - ODM get! error = "
32 "Error - not a link descriptor"
34 "Error - open failed! error = "
36 "Error - close failed! error = "
38 "Error - delete failed! error = "
40 "Error - add failed! error = "
42 "Error - change failed! error = "
44 "Readonly mode"
46 "Fatal Error! delete expand malloc failed"
47 "Error! No descriptors"

$ Error msgs for ODMEOBJDISP.C

48 "Error! unknown descriptor type"
50 "Error! Bad search criteria"
52 "Error! Bad criteria syntax"
54 "Error! Bad ODM return"

56 "Fatal Error! Data malloc failed"
58 "Fatal Error! Descriptor malloc failed"
60 "Error! Invalid class type"

62 "Error - Open failed! error = "
64 "Delete Failed! error = "

$ Error msgs for ODMERELGRAPH.C

66 "Fatal Error! Malloc failed"
68 "Print Screen Error\n"

$ Error msgs for ODMEWINDOW.C

70 "Error! Malloc failed"
