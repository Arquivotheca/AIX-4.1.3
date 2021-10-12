$ @(#)96        1.134.2.7  src/bos/usr/lib/nim/objrepos/cmdnim.msg, cmdnim, bos41J, 9523C_all 6/9/95 20:55:20
$ ******************************************************************************
$ 
$ COMPONENT_NAME: (CMDNIM) Network Install Manager
$ 
$ FUNCTIONS: cmdnim.msg
$ 
$ ORIGINS: 27
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ ******************************************************************************
$
$                             cmdnim.msg
$
$ this file contains all of the PII for cmdnim
$ *****************************************************************************/
$
$quote "
$
$ DO NOT terminate strings with a "\n"; many of these strings get logged using
$		the nim_log function; this function logs by invoking the "alog" command;
$		if there is a "\n" within the message getting logged, the shell will barf
$		because the "\n" is a special shell metacharacter
$
$ ----------------------------  ERR_SET       ----------------------------------
$set ERR_SET
$ ------------ system errors
ERR_METHOD				"0042-001 %s: processing error encountered on \"%s\":\n\
   %s"
ERR_SIGNAL				"0042-002 %s: error signal number %s received -\n\
\tprocessing aborted"
ERR_SYS					"0042-003 %s: error on system call; info follows:\n%s\n"
ERR_ERRNO				"0042-004 %s: error on system call\n%s\n"
ERR_ODM					"0042-005 %s: ODM error encountered; odmerrno = %d"
ERR_ERRNO_LOG			"0042-006 %s: (%s) %s %s \n"
ERR_BUFF_OVER			"0042-007 %s: internal error (buffer overflow) in \
function %s"
ERR_REFUSED				"0042-008 %s: Request denied - %s"
ERR_INCOMPLETE			"0042-009 %s: Insufficient data for requested operation \
-\n\t%s"
ERR_DIR_NOT_LOCAL		"0042-010 %s: the contents of the %s directory are\n\
\tnot local to the machine %s and they\n\
\tmust be in order for this to succeed"
ERR_NIM_ENV_FILE		"0042-011 %s: unable to access the /etc/niminfo file"
ERR_NOT_MASTER			"0042-012 %s: this command may only be executed on a\n\
\tNIM master"
ERR_CANT_GLOCK			"0042-013 %s: unable to obtain the global NIM lock"
ERR_BAD_OBJECT			"0042-014 %s: an internal error has occurred resulting\n\
\tin an invalid %s object named \"%s\""
$ ------------ user errors
ERR_SYNTAX				"0042-015 %s: syntax error\n%s"
ERR_BAD_OPT				"0042-016 %s: \"-%c\" is not a valid option for this \
operation\n\t%s"
ERR_OPT_ARG				"0042-017 %s: invalid option-argument for \"-%c\"\n%s"
ERR_MISSING_OPT		"0042-018 %s: \"-%c\" is a required option for this \
operation\n\t%s"
ERR_MISSING_OPT_ARG	"0042-019 %s: option \"-%c\" requires an argument\n\t%s"
ERR_MISSING_OPERAND	"0042-020 %s: \"%s\" must be supplied for this\n\
\toperation %s"
ERR_MISSING_ATTR		"0042-021 %s: the \"%s\" attribute is required for this\n\
\toperation"
ERR_MAX_VALUE			"0042-022 %s: \"%s\" exceeds the maximum limit of %d"
ERR_VALUE				"0042-023 %s: \"%s\" is not a valid %s"
ERR_BAD_TYPE_FOR		"0042-024 %s: \"%s\" is not a valid object type for \"%s\""
ERR_OP_NOT_ALLOWED	"0042-025 %s: the \"%s\" operation cannot be applied to\n\
\t\"%s\" object types"
ERR_OBJ_CLASS			"0042-026 %s: \"%s\" is not a \"%s\" object"
ERR_OBJ_MISSING_ATTR	"0042-027 %s: \"%s\" is required to complete the\n\
\tdefinition of the \"%s\" object.  This attribute is either\n\
\tcurrently missing or the current operation would cause\n\
\tit to become missing."
ERR_CONTEXT				"0042-028 %s: \"%s\" cannot be used for this operation"
ERR_NO_CH_ATTR			"0042-029 %s: the \"%s\" attribute cannot be supplied\n\
\tfor this operation"
ERR_NO_SEQNO			"0042-030 %s: a sequence number may not be specified with\
\n\tthe \"%s\" attribute"
ERR_CANT_ID				"0042-031 %s: unable to generate a unique object id"
ERR_ONLY_ONE			"0042-032 %s: %s must be unique and \"%s\" already exists"
ERR_SEQNO				"0042-033 %s: \"%s\" attr already exists with seqno of \
\"%d\""
ERR_GEN_SEQNO			"0042-034 %s: unable to generate a new seqno for \"%s\""
ERR_ATTR_NOT_FOUND	"0042-035 %s: unable to find attr which matches this \
query:\n\t\"%s\""
ERR_SERVER_TYPE		"0042-036 %s: \"%s\" cannot serve a resource because of\n\
\tit's type (\"%s\")"
ERR_STATE				"0042-037 %s: the state of \"%s\" prevents this operation\
\n\tfrom succeeding"
ERR_ALREADY_LOCKED	"0042-038 %s: unable to continue because the \"%s\"\n\
\tobject is locked"
ERR_RELEASE_LEVEL		"0042-039 %s: unacceptable release level of %s.%s for\n\
\t\"%s\""
ERR_REFERENCED			"0042-040 %s: unable to remove the \"%s\" object because\n\
\tit is currently referenced by other objects"
ERR_EXISTS    			"0042-041 %s: %s already exists"
ERR_NO_PING    		"0042-042 %s: unable to communicate with %s"
ERR_SERVING				"0042-043 %s: unable to remove \"%s\" because one or\n\
\tmore NIM resources are still defined as being served by that machine"
ERR_MISSING_VALUE		"0042-044 %s: <value> missing in attribute assignment\n\
\tfor \"%s\""
ERR_MISSING_SEQNO		"0042-045 %s: a sequence number must be specified with\n\
\tall \"%s\" attributes for this operation"
ERR_FILE_MODE			"0042-046 %s: unable to %s the file \"%s\"\n"
ERR_TYPE_OF_RES		"0042-047 %s: unable to allocate the \"%s\"\n\
\tresource because it's type cannot be used by \"%s\" objects"
ERR_ROUTE_TO_RES		"0042-048 %s: \"%s\" is unable to access the \"%s\"\n\
\tresource due to network routing"
ERR_ALREADY_ALLOC		"0042-049 %s: either \"%s\" or another resource of\n\
\tthis type has already been allocated to \"%s\""
ERR_COMMAND				"0042-050 %s: operation failed for the following reason:\n\
\tt%s %s %s"
ERR_IP_RESOLVE			"0042-051 %s: unable to resolve \"%s\" to a \"%s\""
ERR_USES					"0042-052 %s: unable to %s \"%s\" because resources\n\
\thave been allocated to it"
ERR_DNE					"0042-053 %s: there is no NIM object named \"%s\""
ERR_RM_LPP_SOURCE		"0042-054 %s: unable to remove \"%s\" because it is part\n\
\tof the \"%s\" installp_bundle"
ERR_SOURCE				"0042-055 %s: \"%s\" is not a valid \"source\" for\n\
\tthis operation"
ERR_DUP_ATTR_ASS		"0042-056 %s: duplicate attribute assignment for \"%s\""
ERR_CKROS_MISSING		"0042-057 %s: \"%s\" is a required attribute"
ERR_IF_NS_BY_SPOT		"0042-058 %s: unable to allocate \"%s\" to \"%s\"\n\
\tbecause it does not support the network interface type\n\
\tof that client"
ERR_TYPE_CONFLICT		"0042-059 %s: conflict between the supplied type (\"%s\")\
\n\tand the real type (\"%s\") for \"%s\""
ERR_ONLY_ONE_ATTR		"0042-060 %s: multiple \"%s\" attributes were\n\
\tspecified and only one is allowed"
ERR_RES_BUSY			"0042-061 %s: the \"%s\" resource is currently\n\
\tallocated for client use"
ERR_MISSING				"0042-062 %s: \"%s\" is missing something which is\n\
\trequired"
ERR_MISSING_RES		"0042-063 %s: the following resource types are required\n\
\tfor this operation but have not yet been allocated\n\
\tto \"%s\":"
ERR_SERVER_BUSY		"0042-064 %s: \"%s\" is currently serving a resource\n\
\t(\"%s\") that is allocated for use"
ERR_RESERVED_WORD		"0042-065 %s: \"%s\" is a reserved word and cannot be\n\
\tused to name objects"
ERR_BAD_CHARS			"0042-066 %s: the string \"%s\" contains one\n\
\tor more characters which are not allowed to be used in strings in the\n\
\tNIM environment"
ERR_RESERVED_OBJ		"0042-067 %s: this operation cannot be applied to \"%s\"\n\
\tbecause it is a reserved NIM object"
ERR_NOT_ALLOC			"0042-068 %s: \"%s\" is not allocated to \"%s\""
ERR_OP_NOT_NOW			"0042-069 %s: the %s operation cannot be performed when\n\
\tthe target has a %s of \"%s\""
ERR_OBJ_NOT_FOUND		"0042-070 %s: there is no NIM object which has the\n\
\tfollowing characteristics:\n\t%s"
ERR_AT_ALREADY			"0042-071 %s: an operation has already been scheduled\n\
\tfor \"%s\""
ERR_MISSING_AT			"0042-072 %s: unable to perform a scheduled operation\n\
\tbecause the \"%s\" attribute is missing from\n\
\tthe definition of \"%s\""
ERR_NO_CUST				"0042-073 %s: unable to create a customization script\n\
\tfor \"%s\" because it does not have any customization\n\
\tresources allocated to it"
ERR_PATHNAME			"0042-074 %s: \"%s\" is not a valid pathname because it \n\
\thas been specified with a relative path and pathnames\n\
\tmust be absolute"
ERR_NFSEXP				"0042-075 %s: error encountered when exporting \"%s\"\n\
\ton \"%s\":%s"
ERR_ON_PORT				"0042-076 %s: port %d is already used by the \"%s\"\n\
\tservice"
ERR_ADD_ROUTE			"0042-077 %s: unable to add routes from the NIM Master"
ERR_DEST_MATCH			"0042-078 %s: unable to change the \"%s\" NIM route\n\
\tbecause the destination network specified in the\n\
\tnew value does not match the destination network\n\
\tspecified in the current value"
ERR_DEALLOC_DEPEND	"0042-079 %s: unable to deallocate \"%s\" because\n\
\t\"%s\" must be deallocated first"
ERR_CMD_OLD             	"0042-080 %s: fatal error returned by the\n\
\t\"%s\" command:\n\t%s"
ERR_RES_LOCATION		"0042-081 %s: a resource already exists on \"%s\" at\n\
location \"%s\"; due to NFS export restrictions, the\n\
\tnew location \"%s\" may not be used"
ERR_UNSUPPORTED_DEVICE 	"0042-082 %s: \"%s\" is not a supported device"
ERR_HARD_ADDR			"0042-083 %s: network hardware addresses must be unique\n\
\tand \"%s\" is already being used by the\n\
\tmachine \"%s\""
ERR_ALREADY_MASTER	"0042-084 %s: This machine is already configured to be\n\
\ta NIM Master"
ERR_NIMINIT         	"0042-085 %s: unable to continue due to fatal error\n\
\tencountered by the niminit command"
ERR_ROUTE_ALREADY		"0042-086 %s: a NIM route already exists between \"%s\"\n\
\tand \"%s\" (only 1 is allowed)"
ERR_OBSOLETE_2			"0042-087 %s: obsolete error message"
ERR_OBSOLETE_3      	"0042-088 %s: obsolete error message"
ERR_OBSOLETE_4	     	"0042-089 %s: obsolete error message"
ERR_OBSOLETE_5     	"0042-090 %s: obsolete error message"
ERR_MALLOC          	"0042-091 %s: error returned from malloc"
ERR_ABSOLUTE_PATH   	"0042-092 %s: SPOT location must be an absolute path\n\
\tname; \"%s\" is not"
ERR_MKDIR         	"0042-093 %s: unable to create directory \"%s\""
ERR_POPULATE      	"0042-094 %s: unable to populate SPOT from \"%s\""
ERR_PREPARE_SOURCE 	"0042-095 %s: unable to prepare the source %s"
ERR_SOURCE_UNKNOWN 	"0042-096 %s: unable to prepare the source %s,\n\
\ttype unknown"
ERR_CANT_START_DAEMON	"0042-097 %s: unable to start daemon:%s"
ERR_CANT_REFRESH_DAEMON	"0042-098 %s: unable to refresh daemon:%s"
ERR_CANT_OPEN  		"0042-099 %s: unable to open %s"
ERR_CANT_CLOSE      	"0042-100 %s: unable to close %s"
ERR_CANT_REWIND     	"0042-101 %s: unable to rewind %s"
ERR_CANT_POSITION   	"0042-102 %s: unable to position %s"
ERR_CANT_RESTORE 		"0042-103 %s: unable to restore from %s"
ERR_PIPE         		"0042-104 %s: pipe system call failed"
ERR_DUP          		"0042-105 %s: dup system call failed"
ERR_EXEC         		"0042-106 %s: unable to execute the \"%s\" program"
ERR_WAITPID       	"0042-107 %s: waitpid system call failed"
ERR_INVALID_DEVICE 	"0042-108 %s: \"%s\" is not a valid device name"
ERR_FS_INFO				"0042-109 %s: unable to get filesystem information\n\
\tabout \"%s\""
ERR_ALREADY_EXISTS  	"0042-110 %s: /usr SPOT already exists"
ERR_BAD_SEQNO			"0042-111 %s: invalid sequence number in \"%s\";\n\
\tit must be between 1 and %d"
ERR_INVALID_BOSRTE 	"0042-112 %s: \"%s\" does not contain a valid\n\
\tbos.rte image"
ERR_SIZE          	"0042-113 %s: unable get size requirements for %s"
ERR_FS_EXPAND     	"0042-114 %s: unable to expand file system %s by %d"
ERR_UPD_BOSINST_DATA	"0042-115 %s: unable to update %s"
ERR_USE_AUTOEXPAND  	"0042-116 %s: obsolete error message"
ERR_MKBOOTI         	"0042-117 %s: operation failed due to error encountered\n\
\tby c_mkbooti"
ERR_CHNET      	  	"0042-118 %s: a NIM operation is currently being\n\
\tperformed on the \"%s\" machine or it has\n\
\tresources allocated to it"
ERR_CH_REFERENCED		"0042-119 %s: unable to change the \"%s\" object\n\
\tbecause it is currently referenced by other objects"
ERR_USAGE_GET_LEVEL	"0042-120 %s: Usage: c_get_level <path>" 
ERR_CH_NFSEXP_NBA		"0042-121 %s: Specify %s OR %s"
ERR_CH_NFSEXP_AE		"0042-122 %s: %s is already exported"
ERR_CH_NFSEXP_NE 		"0042-123 %s: %s is NOT exported"
ERR_CH_NFSEXP_UO		"0042-124 %s: NFS option %s is NOT supported "
ERR_USAGE_UPD_IMAGEDATA "0042-125 %s: Usage: c_upd_imagedata <path>"
ERR_UPD_IMAGEDATA		"0042-126 %s: Could not update %s/image.data"
ERR_INVALID_IMAGEDATA   "0042-127 %s: image.data file %s is invalid"
ERR_UPD_COPY_BACKUP	"0042-128 %s: copy original image.data file is in %s"
ERR_BAD_RESOURCE_TYPE   "0042-129 %s: \"%s\" is an invalid resource type for\n\
\tthis operation"
ERR_INVALID_ATTR		"0042-130 %s: \"%s\" cannot be specified as an attribute\n\
\tfor this operation"
ERR_ATTR_CONFLICT		"0042-131 %s: the \"%s\" attribute conflicts with\n\
\t\"%s\": one or the other may be specified, but not both"
ERR_VALUE_CONTEXT		"0042-132 %s: \"%s\" cannot be specified as a value for\n\
\t\"%s\" for this operation"
ERR_RMRES				"0042-133 %s: unable to remove the \"%s\" at\n\
\tlocation \"%s\""
ERR_INITIATE_BOOTP	"0042-134 %s: unable to initiate network boot on \"%s\""
ERR_CONTROL				"0042-135 %s: unable to perform the requested operation\n\
\tbecause the \"%s\" object is currently controlled by\n\
\tthe machine \"%s\""
ERR_NO_ROUTE			"0042-136 %s: there is no NIM route from the \"%s\"\n\
\tnetwork to the machine \"%s\""
ERR_NIM_ENV_MISSING	"0042-137 %s: the /etc/niminfo file is missing some\n\
\trequired environment varibles"
ERR_NO_CHRHOST			"0042-138 %s: unable to update the rhost file with %s\n\
\thostname"
ERR_INSTALL_SELF		"0042-139 %s: the resource \"%s\" is served by \"%s\"\n\
\tand cannot be used to install \"%s\""
ERR_RM_NIMINFO			"0042-140 %s: unable to remove the /etc/niminfo file on\n\
\t\"%s\""
ERR_ATTR_RM_DNE		"0042-141 %s: unable to remove the \"%s\" attribute\n\
\tbecause \"%s\" does not have one"
ERR_ATTR_MK_VALUE		"0042-142 %s: unable to add the \"%s\" attribute because\n\
\tattribute values must be unique and \"%s\" already exists\n\
\twith the same value"
ERR_ATTR_MK				"0042-143 %s: only one \"%s\" attribute can be specified\n\
\tfor an object and one already exists for \"%s\""
ERR_ATTR_SAME_SEQNO	"0042-144 %s: sequence numbers must be unique and \"%d\"\n\
\tis already being used with the \"%s\" attribute"
ERR_ATTR_NAME			"0042-145 %s: \"%s\" is not a valid attribute name"
ERR_OBJ_TYPE			"0042-146 %s: \"%s\" is not a valid object type"
ERR_IS_MASTER			"0042-147 %s: this command may not be executed on a NIM\n\
\tmaster"
ERR_NET_INFO			"0042-148 %s: invalid information in \"%s\" for the \"%s\" object"
ERR_DENIMBUN			"0042-149 %s: unable to deallocate \"%s\" because it is \
part\n\tof the allocated installp_bundle \"%s\""
ERR_VFSTYPE        	"0042-150 %s: the file \"%s\" has a vfstype of %s.  This \
conflicts with\n\tthe expected vfstype, which is one of the following:\
\n\t\t%s"
ERR_FILE_TYPE			"0042-151 %s: the file \"%s\" has a file type of %s.\
\n\tThis conflicts with the expected type of %s."
ERR_ROOT_SYNC      	"0042-152 %s: an error occurred while performing root sync\
\n\toperations on the following clients:\n"
ERR_FILE_PERMS	     	"0042-153 %s: the file \"%s\" has file permissions of %s.\
\n\tThis conflicts with the expected permissions of %s."
ERR_ENOENT				"0042-154 %s: the file \"%s\" does not exist"
ERR_ROOT_SPOT_SERVER	"0042-155 %s: unable to perform requested operation \
because the\n\tserver of the root resource must be the same as the server of \
\n\tthe spot resource and your request\n\tviolates this requirement."
ERR_DIR_DNE				"0042-156 %s: there is no %s at location \"%s\""
ERR_FILE_ACCESS		"0042-157 %s: unable to access the \"%s\" file"
ERR_FILE_MOD			"0042-158 %s: unable to modify the \"%s\" file"
ERR_MISSING_OPTIONS_OLD	"0042-159 %s: %s is missing one or more of the\
\n\tfollowing options:\n%s"
$ NOTE: The previous message is used by the code in spite of its "_OLD" label
$ and should be translated.
ERR_NO_BOOTI			"0042-160 %s: unable to create any type of network boot \
image"
ERR_SPACE_OLD				"0042-161 %s: there is insufficient free disk space in\n\
\t\"%s\" to complete the requested operation; data follows:\n\
\t\trequired\t= %s\n\t\tavailable\t= %s"
ERR_SIMAGES				"0042-162 %s: an lpp_source which has the \"simages\"\n\
\tattribute is required for this operation"
ERR_SERVER_IS_GATEWAY "0042-163 %s: machine \"%s\" serves a resource which\n\
\tis being allocated, however, NIM is unable to determine\n\
\twhich interface on this machine should be used by the\n\
\tclient because this machine is also functioning as the\n\
\tgateway between the \"%s\" and\n\
\t\"%s\" networks."
ERR_CDROM_USE			"0042-164 %s: \"%s\" cannot be used as a source for\n\
\tinstallable images in this context because it is a CDROM."
ERR_NETWORK_TYPE		"0042-165 %s: \"%s\" cannot be associated with the type\n\
\tof network referenced by the \"%s\" attribute"
ERR_INCOMPLETE_IF		"0042-166 %s: \"%s\" requires a corresponding\n\
\t\"%s\" attribute"
ERR_NOT_IPLEMU			"0042-167 %s: \"%s\" does not contain a valid\n\
\tIPL ROM emulation image"
ERR_ROUTE_TO_SELF		"0042-168 %s: the destination network (\"%s\") specified\n\
\tin \"%s\" is the same as the originating network"
ERR_NO_FILESETS		"0042-169 %s: you have allocated an lpp_source, but\n\
\thave not specified which filesets are to be installed\n\
\tusing that lpp_source"
ERR_PLATFORM			"0042-170 %s: \"%s\" is not a valid platform type"
ERR_TYPE_PLATFORM		"0042-171 %s: the \"%s\" configuration type is not\n\
\tsupported for the \"%s\" platform type"
ERR_BAD_CPUID		"0042-172 %s: This machine's CPU ID does not match the CPU ID\n\
\tstored in the NIM database."

ERR_NO_X        "\
0042-173 %s: \"X\" cannot be specified as a value for installp_flags\n\
\twhen the auto_expand attribute has a value of \"no\".\n"

ERR_ATTR_YES_NO "\
0042-174 %s: The %s attribute may only be assigned a value of \"yes\"\n\
\tor \"no\".\n"

ERR_CMD "\
0042-175 %s: An unexpected result was returned by the\n\
\t\"%s\" command:\n\t%s"

ERR_MISSING_OPTIONS     "\
0042-176 %s: The resource located in\n\
\t%s\n\
\tcannot serve as a \"support images\" (or \"simages\") lpp_source\n\
\tbecause one or more of the following filesets are missing:\n%s"

ERR_SPACE "\
0042-177 %s: There is insufficient free disk space in\n\
\t\"%s\" to complete the requested operation; data follows:\n\
\t(All sizes displayed are in 512-byte blocks.)\n\
\t\trequired\t= %s\n\t\tavailable\t= %s"

ERR_ADAPTER_REQ	"0042-178 %s: the logical device name of the network\n\
\tadapter must be supplied when connecting to the \"%s\"\n\
\tnetwork because that network is composed of more than one network\n\
\tinterface type."

ERR_RM_NET_TYPE	"0042-179 %s: the network type specified by the \"%s\"\n\
\tattribute cannot be removed because it is currently being used by\n\
\tone or more clients which are attached to the \"%s\" network."

ERR_WRONG_NETWORK "0042-180 %s: The ip address %s does not belong to\n\
\tthe network %s which is represented by\n\
\tthe nim object %s.\n"

ERR_BAD_FIXQ_FLAG "0042-181 %s: The \"fix_query_flags\" attribute has an\n\
\tillegal value. Use \"lsnim -Pa fix_query_flags\" for\n\
\ta list of legal values.\n"

ERR_RES_CONFLICT "0042-182 %s: A resource of type \"%s\" cannot be allocated\n\
\tfor the current operation at the same time as a resource\n\
\tof type \"%s\". Allocate one or the other but not both.\n"

ERR_ATTR_RES_CONFLICT "0042-183 %s: The \"%s\" attribute cannot be specified\n\
\tfor the current operation when a resource of type\n\
\t\"%s\" is allocated. Use one or the other but not both.\n"

ERR_NET_INUSE 	"0042-184 %s: The network address (net_addr) or subnet mask (snm)\n\
\tcannot be changed for network \"%s\" because NIM clients\n\
\tare currently defined as being connected to that network.\n\
\tRemove the client definitions before changing the network object.\n"

ERR_LN_COPY 	"0042-185 %s: Failed to link or copy %s to %s \n\
\tCheck permissions and filesystem space"

ERR_CP_FAILED	"0042-186 %s: Failed to copy setup programs to %s \n\
\tEither start NFS on %s or free 1000 512-byte blocks in the\n\
\t%s filesystem." 

ERR_EXP_FAILED	"0042-187 %s: Failed to expand filesystem %s on %s" 

ERR_NFS_MNT	"0042-188 %s: Failed to NFS mount %s on %s" 

ERR_BOOT_INFO  "0042-189 %s: Failed saving existing boot image. \n\
\tCheck space in the %s filesystem." 

ERR_KEY_POSN	"0042-190 %s: The key is NOT in the NORMAL position.\n\
\tUnattended installtion cannot complete unless the key\n\
\tis in the NORMAL position."

ERR_IPLR_WRITE	"0042-191 %s: Unable to write the IPLROM emulation to %s"

ERR_BLV 	"0042-192 %s: Unable to find boot logical volume %s"

ERR_NO_RHOST	"0042-193 %s: %s has no entry for %s in .rhosts or cannot resolve host id"

ERR_NO_PUSH	"0042-194 %s: %s does not allow NIM push operations.\n\
\tRemove /etc/nimstop on %s if push operation is necessary."

ERR_SET_BOOTLIST "0042-195 %s: Unable to order boot device list on \"%s\""

ERR_SET_BL_NO_CL_BT "0042-196 %s: The \"set_bootlist\" attribute is only valid\n\
\twhen used in combination with the \"no_client_boot\" attribute."

ERR_MULT_ADPTS_FPUSH "0042-197 %s: The network adapter's logical device name\n\
\tmust be specified in the \"if1\" attribute of the target machine's\n\
\tNIM object when using the force_push attribute if the target\n\
\tmachine has more than one interface for a given network type.\n\
\tAdd one of the following to the \"if1\" attribute of the target's\n\
\tNIM object: %s"
 
$ ----------------------------  MSG_SET       ----------------------------------
$set MSG_SET
$ note that "<...>" is used in syntax messages, so if you translate this, you
$    should also translate it everywhere it's used
MSG_NIM_OBJECT 		"NIM object"
MSG_NIM_CLASS 		"NIM object class"
MSG_NIM_SUBCLASS 	"NIM object subclass"
MSG_NIM_TYPE 		"NIM object type"
MSG_NIM_OPERATION 	"NIM operation"
MSG_NIM_IF_STANZA 	"NIM interface stanza"
MSG_NIM_ROUTING_STANZA 	"NIM routing stanza"
MSG_ATTR_NAME 		"attribute name"
MSG_ATTR_ASS 		"attribute assignment"
MSG_OBJ_NAME 		"object name"
MSG_RES_LOCATION 	"location of resource"
MSG_NET_DEFINITION 	"network definition"
MSG_NAME 		"<object name>"
MSG_ATTR 		"<attr>"
MSG_VALUE 		"<value>"
MSG_NET_ADDR 		"IP address"
MSG_SNM 		"subnetmask"
MSG_NETNAME 		"<network object name>"
MSG_HOSTNAME	"hostname"
MSG_NET_HARD_ADDR 	"<net hard addr>"
MSG_DEFINE 		"define"
MSG_ALLOC 		"allocation"
MSG_NO_ROUTE_TO_MASTER 	"route to the NIM master"
MSG_EXECUTE 		"execute"
MSG_READ 		"read"
MSG_WRITE 		"write"
MSG_WARNING 		"warning"
MSG_RES_ALLOC 		"resource allocation"
MSG_SERVER_INST 	"an install operation is occurring on the server of this resource"
MSG_REQUIRED_RES 	"the following resources are required:"
MSG_OPTIONAL_RES 	"the following resources are optional:"
MSG_REQUIRED_ATTR       "the following attributes are required:"
MSG_OPTIONAL_ATTR       "the following attributes are optional:"
MSG_CHECK_NSTATE "the Cstate has been set based upon the Nstate of \"%s\""
$
$ ---------- nimkid.c
MSG_SEC_PORT		"error getting secondary port"
MSG_CONNECT_PORT	"error connecting to port"
MSG_NIMESIS_READ    	"nimesis unable to read from socket"
MSG_NIMESIS_FP_CREAT	"nimesis can not create file pointer"
MSG_DUP_STDERR		"dup2 stderr"
MSG_DUP_STDOUT		"dup2 stdout"
$
$ ------------ mstr_gen.c
MSG_NIMC_INIT		"# NIM client initialization\n"
MSG_CHG_CONTROL_STATE	"# change the control state\n"
MSG_FLAGS		"flags"
MSG_BUNDLE		"bundle"
MSG_SOURCE		"source"
MSG_OPTIONS		"options"
$
$ -R---------- nim_err.c
MSG_NIM_MAIL		"** Mail message from NIM command %s, process id %d **\n\nText of message:\n\n %s\n\nEnd of Mail:\n"
MSG_NIMERR_REPORT	"NIM Error report"
$
$ ------------ niminit.c
MSG_INIT_TO_MSTR	"init to master"
MSG_RCMD		"rcmd"
MSG_SEND_ATTR_COUNT	"sending attr_count"
MSG_SEND_TO_MASTER	"sending to master"
MSG_FOPEN		"fopen"
MSG_NEW_NIMINFO		"New niminfo"
$
$ ------------ nim_net.c
MSG_GET_ADAPT_ADDR	"get_adpt_addr"
MSG_CANT_GET_ADAPT_ADDR	"Can not get adapter address."
MSG_ADAPT_NOT_FOUND	"Adapter not found ?"
MSG_NO_ADPT_DEVICE_NAME	"Adapter device name not found. "
$
$ ------------ nimconfig.c
MSG_NIMCONF_CLASS   	"Initialized ODM and mounted NIM classes."
MSG_NIMCONF_CPU   	"Saving machines CPU id."
MSG_NIMCONF_OK_IP   	"Checking input attributes."
MSG_NIMCONF_VALID    	"Making sure the NIM Master package is OK."
MSG_NIMCONF_NIMINFO    	"Built NIM infomation file."
MSG_NIMCONF_MSTRNET    	"Adding NIM masters network to NIM class."
MSG_NIMCONF_CONNECT    	"Connecting NIM master to master network."
MSG_NIMCONF_SIMAGE    	"Building support images for NIM."
MSG_NIMCONF_SRC     	"Adding NIM deamons to SRC and starting...."
$
$ ------------------------ syntax messages
$
$ I recommend that the text not pass this column--------------------------|
$
MSG_NIMCLIENT_SYNTAX_OLD "\nusage: nimclient -d\n\
       nimclient -p | -P\n\
       nimclient -o <operation> [-a <attr=value>]...]\n\
       nimclient -l <lsnim information>\n"
$
MSG_LSNIM_SYNTAX_OLD "\nusage: lsnim -p|-P [-S]\n\
       lsnim -p|-P -c <object class>|-s <object subclass>|-t <object type>\n\
             [-l]|[-o]|[-O] [-Z]\n\
       lsnim -p|-P -a <attr name>...\n\
       lsnim -t <object type> -q <operation>\n\
       lsnim [-c <object class>]|[-s <object subclass>]|[-t <object type>]\n\
             [-l]|[-o]|[-O] [-Z] [<object name>]\n\
       lsnim [-a <attr name>] [-Z] [<object name>]\n\
       lsnim -q <operation> <object name>\n\
       lsnim -L [-s <subclass>]|[-t <object type>] <object name>\n"
$
MSG_M_MKMAC_SYNTAX "\nusage:\n\
\tm_mkmac -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_M_RMMAC_SYNTAX "\nusage: m_rmmac <object name>\n"
$
MSG_M_MKRES_SYNTAX "\nusage:\n\
\tm_mkres -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_M_MKBOSI_SYNTAX "\nusage:\n\
\tm_mkbosi -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_M_MKNET_SYNTAX "\nusage:\n\
\tm_mknet -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_M_RMNET_SYNTAX "\nusage:\n\
\tm_rmnet <object name>\n"
$
$ I recommend that the text not pass this column--------------------------|
$
MSG_NIMINIT_SYNTAX_OLD "\nusage: niminit -q\n\
       niminit -a <attr=value>...\n"
$
MSG_NIM_SYNTAX_OLD "\nusage:\n\
\tnim -o <operation> [-t <type>] [-a <attr=value>]... <object name>\n"
$
MSG_M_MKPDIR_SYNTAX 	"\nusage:\n\
\tm_mkres -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_M_RMPDIR_SYNTAX 	"\nusage: m_rmpdir <object name>\n"
$
MSG_C_CKROS_SYNTAX  	"\nusage: c_ckros_emu -a ros_emu=<value>\n"
$
MSG_GENERIC_MKSYNTAX "\nusage:\n\
\t<method> -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_GENERIC_CHSYNTAX "\nusage: <method> -a <attr=value>...<object name>\n"
$
MSG_GENERIC_RMSYNTAX "\nusage: <method> <object name>\n"
$
MSG_GENERIC_ALSYNTAX "\nusage: <method> -a <attr=value>... <object name>\n"
$
$ I recommend that the text not pass this column--------------------------|
$
MSG_COP_SYNTAX "\nusage: <method> -a <attr=value>... <object name>\n"
$
MSG_SOP_SYNTAX "\nusage:\n\
\tm_schedule <\"at\" time> <method> [<args>] <object name>\n"
$
MSG_NIMCONFIG_SYNTAX_OLD	"\nusage: nimconfig -q\n\
       nimconfig -a <attr=value>...\n\
       nimconfig -r \n"
$
MSG_M_CHNET_SYNTAX "\nusage:\n\
\tm_chnet -t <object type> [-a <attr=value>]... <object name>\n"
$
MSG_C_CH_NFSEXP_SYNTAX "\nusage: c_ch_nfsexp -a <attr=value>...\n"
$
MSG_MISSING_RES "unable to perform %s due to missing resources"

MSG_NIMCLIENT_SYNTAX "\n\
usage:\n\
\tnimclient -d\n\
\t   sets client date and time to that of master.\n\
\tnimclient -l <lsnim information>\n\
\t   invokes the lsnim command on the master.\n\
\tnimclient -p | -P\n\
\t   enables/disables the NIM master's push permissions on client.\n\
\tnimclient -o <operation> [-a <attr=value>]...\n\
\t   executes NIM operations on the client.\n"

MSG_LSNIM_SYNTAX "\n\
usage:\n\
\tTo display predefined and help information about NIM objects,\n\
\ttypes, classes and subclasses:\n\
\t  lsnim -p|-P [-S]\n\
\t  lsnim -p|-P -c <object class>|-s <object subclass>|-t <object type>\n\
\t        [-l]|[-o]|[-O] [-Z]\n\
\t  lsnim -p|-P -a <attr name>...\n\n\
\tTo list required and optional attributes for an operation:\n\
\t  lsnim -q <operation> <object name>\n\
\t  lsnim -q <operation> -t <object type>\n\n\
\tTo list information by class, subclass, type or attribute:\n\
\t  lsnim [-c <object class>]|[-s <object subclass>]|[-t <object type>]\n\
\t        [-l]|[-o]|[-O] [-Z] [<object name>]\n\
\t  lsnim [-a <attr name>] [-Z] [<object name>]\n\n\
\tTo list information that can be accessed by a client machine:\n\
\t  lsnim -L [-s <subclass>]|[-t <object type>] <object name>\n\n\
\tMiscellaneous options:\n\
\t  -l displays detailed information\n\
\t  -O lists operations NIM supports\n\
\t  -o used by NIM's SMIT interface\n\
\t  -Z produces colon-separated output\n"

MSG_NIMCONFIG_SYNTAX "\n\
usage:\n\
\tnimconfig -r [-v]\n\
\t   rebuilds /etc/niminfo file (not valid with -a)\n\
\tnimconfig -a <attr=value>...[-v]\n\
\t   specifies attribute assignments for nimconfig operation\n\
\t-v displays detailed output for nimconfig operations.\n"

MSG_NIMINIT_SYNTAX "\n\
usage:\n\
\tniminit -a <attr=value>...[-v]\n\
\t   -v displays detailed output for the niminit operation.\n"

MSG_VALID_OPS "\
\tvalid operations:\n"

MSG_VALID_TYPES "\
\tvalid types:\n"

MSG_USG_HINT1 "\
\t** Use lsnim -O <object name> or lsnim -POt <type> to list valid\n\
\t   operations for a particular object or object type.\n"

MSG_USG_HINT1_CL "\
\t** Use nimclient -l -O <object name> or nimclient -l -POt <object type>\n\
\t   to list valid operations for a particular object or object type.\n"

MSG_USG_HINT2 "\
\t** Use lsnim -q <operation> <object name> or\n\
\t   lsnim -q <operation> -t <type> to list required and optional\n\
\t   resources and attributes for a particular NIM operation.\n"

MSG_USG_HINT2_CL "\
\t** Use nimclient -l -q <operation> <client name> to list required and\n\
\t   optional resources and attributes for a particular NIM operation.\n"

MSG_NIM_SYNTAX "\nusage:\n\
\tnim -o <operation> [-F] [-t <type>] [-a <attr=value>]... <object name>\n"

MSG_REBOOT_WARNING "\
*******************************************************************\n\
*******************************************************************\n\
*******************************************************************\n\n\
   NIM has initiated a bos installation operation on this machine.\n\
   Automatic reboot and reinstallation will follow shortly...\n\n\
*******************************************************************\n\
*******************************************************************\n\
*******************************************************************\n\n"

$
$ ----------------------------  STATE        -----------------------------------
$
$ WARNING:
$ 		tags beginning with STATE_ are NOT translatable; they are for NIM
$			internal use only
$		tags beginning with H_STATE are translatable and that's what will be
$			displayed to the users
$
$ used to represent object states
$set STATE_SET
$ -------- generic
STATE_ANY "any"
STATE_INCOMPLETE "incomplete"
$ -------- mstate
STATE_SHUTDOWN "shutdown"
STATE_BOOTING "booting"
STATE_RUNNING "running"
$ -------- cstate
STATE_CREADY "ready"
STATE_SCHEDULED "scheduled"
STATE_DD_BOOT "dd_boot"
STATE_DKLS_INIT "dkls_init"
STATE_DTLS_INIT "dtls_init"
STATE_DIAG "diag"
STATE_CUST "cust"
STATE_BOS_INST "bos_inst_ready"
STATE_BOS_INST_1 "bos_inst_phase_1"
STATE_BOS_INST_2 "bos_inst_phase_2"
STATE_BOS_INST_3 "bos_inst_phase_3"
STATE_MAINT "maint"
$ -------- rstate
STATE_AVAILABLE "available"
STATE_UNAVAILABLE "unavailable"
$ -------- nstate
STATE_NREADY "ready"
$ -------- helps
H_STATE_ANY "NIM internal use only"
H_STATE_INCOMPLETE "information is missing from this object's definition"
H_STATE_SHUTDOWN "not running"
H_STATE_BOOTING "in the process of booting"
H_STATE_RUNNING "currently running"
H_STATE_CREADY "ready for a NIM operation"
H_STATE_SCHEDULED "a NIM operation has been scheduled for this object"
H_STATE_DD_BOOT "diskless or dataless boot is enabled"
H_STATE_DKLS_INIT "diskless resources are being initialized"
H_STATE_DTLS_INIT "dataless install has been enabled"
H_STATE_DIAG "diagnostic boot has been enabled"
H_STATE_CUST "customization is being performed"
H_STATE_BOS_INST "BOS installation has been enabled"
H_STATE_BOS_INST_1 "Base Operating System installation is being performed"
H_STATE_BOS_INST_2 "customization is being performed"
H_STATE_BOS_INST_3 "post install processing is being performed"
H_STATE_MAINT "software maintenance is being performed"
H_STATE_AVAILABLE "ready for use"
H_STATE_UNAVAILABLE "unavailable for use"
H_STATE_NREADY "ready for use"
$
$ ----------------------------  RESULT       -----------------------------------
$
$ WARNING:
$ 		tags beginning with RESULT_ are NOT translatable; they are for NIM
$			internal use only
$		tags beginning with H_RESULT are translatable and that's what will be
$			displayed to the users
$
$ used to store the results from NIM operations
$set RESULT_SET
$ -------- generic
RESULT_SUCCESS "success"
$ the result of the operation was successful
RESULT_FAILURE "failure"
$ an fatal error was encountered and the operation failed
RESULT_ABORT "reset"
$ the operation was aborted by a user
$ -------- helps
H_RESULT_SUCCESS "the NIM operation specified by the p_Cstate attribute was successful"
H_RESULT_FAILURE "the NIM operation specified by the p_Cstate attribute encountered a fatal error and did not complete"
H_RESULT_ABORT "a user has aborted a NIM operation by executing the reset operation"
$
$ ----------------------------   ATTR             ------------------------------
$
$ !!!!!!!!!!!!!!!!!!!!!!!!!!! WARNING to developers !!!!!!!!!!!!!!!!!!!!!!!!!!
$	in order to for NIM to use attributes correctly the following rules MUST
$		be adhered to:
$			1) all attribute names must be unique; there are many places in the
$				NIM code which query based on an attribute's name
$			2) obsolete attributes MUST NOT BE DELETED because the position they
$				occupy in this file gets translated into an attribute number which
$				is included in the nim_pdattr class; if you disturb those numbers,
$				you're in for TROUBLE!
$			3) new attributes MUST BE ADDED AT THE END of this set for the same
$				reason specified in #2 above
$			4) if you add a new attribute, you must also add a H_ATTR_... and
$				VERBOSE_ATTR_... entry
$
$ !!!!!!!!!!!!!!!!!!!!!!!!!!! WARNING to translators !!!!!!!!!!!!!!!!!!!!!!!!!!
$ 		-> tags beginning with ATTR_ are NOT translatable; they are for NIM
$			internal use only
$		-> tags beginning with H_ATTR and VERBOSE_ATTR are translatable
$
$set ATTR_SET
$ ------------------------------ object classes
ATTR_CLASS_MACHINES "machines"
ATTR_CLASS_RESOURCES "resources"
ATTR_CLASS_NETWORKS "networks"
ATTR_CLASS_FLAGS "flag_attrs"
$ internal use only - used to identify attributes which are used as flags only
ATTR_CLASS_STATES "states"
$ internal use only - used to identify object states
ATTR_CLASS_RESULTS "results"
$ internal use only - used to identify object state results
$
$ ------------------------------ object subclasses
ATTR_SUBCLASS "subclass"
$ subclass attr
ATTR_SUBCLASS_CLASS "class"
$ identifies a class definition
ATTR_SUBCLASS_TYPE "type"
$ identifies object types
ATTR_SUBCLASS_OPERATION "operations"
$ identifies NIM operations
ATTR_SUBCLASS_CONTROL "control_ops"
$ identifies operations which can actually control processing on a client
ATTR_SUBCLASS_INFO "info_attrs"
$ identifies information which can be customized for an object
ATTR_SUBCLASS_BOS "bos_res"
$ groups types of BOS images together
ATTR_SUBCLASS_CUST "cust_res"
$ groups customization resources together
ATTR_SUBCLASS_ROS_SUPPORT "iplrom_subclass"
$ identifies information about ROS support
ATTR_SUBCLASS_IF "if_attrs"
$ groups network interface attributes together
ATTR_SUBCLASS_ONLY_1 "only_1"
$ identifies those resources which a machine can have only 1 of
ATTR_SUBCLASS_PDIR_TYPE "pdir"
$ identifies resource types which are parent directories for client directories
ATTR_SUBCLASS_BOOT "boot_res"
$ identifies a resource types which are network boot resources
$
$ ------------------------------ object types
$ ---------- machine
ATTR_DISKLESS "diskless"
ATTR_DATALESS "dataless"
ATTR_STANDALONE "standalone"
ATTR_MASTER "master"
$ ---------- resource
ATTR_SPOT "spot"
ATTR_BOOT "boot"
ATTR_ROOT "root"
ATTR_PAGING "paging"
ATTR_DUMP "dump"
ATTR_HOME "home"
ATTR_SHARED_HOME "shared_home"
ATTR_TMP "tmp"
ATTR_LOG "log"
ATTR_LPP_SOURCE "lpp_source"
ATTR_SIMAGES "simages"
ATTR_INSTALLP_BUNDLE "installp_bundle"
ATTR_BOSINST_DATA "bosinst_data"
ATTR_IMAGE_DATA "image_data"
ATTR_RTE "rte"
ATTR_MKSYSB "mksysb"
ATTR_SCRIPT "script"
ATTR_NIM_SCRIPT "nim_script"
$ ---------- network
ATTR_TOK "tok"
ATTR_ENT "ent"
ATTR_FDDI "fddi"
$ ------------------------------ common attrs
ATTR_CLASS "class"
ATTR_TYPE "type"
ATTR_NAME "name"
ATTR_AT "at"
ATTR_AT_ARG "at_arg"
ATTR_AT_JOB "at_job"
ATTR_LOCKED "locked"
ATTR_LEVEL "level"
ATTR_VERSION "version"
ATTR_RELEASE "release"
ATTR_TRANS "trans"
ATTR_SIZE "size"
ATTR_COMMENTS "comments"
ATTR_MISSING "missing"
ATTR_INFO "info"
ATTR_RESERVED "reserved"
ATTR_CONTROL "control"
$ ------------------------------ context attrs
ATTR_CONTEXT "context"
ATTR_MSERVED "mserved"
ATTR_MUSED "mused"
ATTR_RUSED "rused"
ATTR_MRES "mres"
ATTR_NMEMBER "NMEMBER"
ATTR_NETNAME "netname"
$ ------------------------------ operations
ATTR_DEFINE "define"
ATTR_CHANGE "change"
ATTR_REMOVE "remove"
ATTR_ALLOCATE "allocate"
ATTR_DEALLOCATE "deallocate"
ATTR_SCHEDULE "schedule"
ATTR_DD_BOOT "dd_boot"
ATTR_DKLS_INIT "dkls_init"
ATTR_DTLS_INIT "dtls_init"
ATTR_DIAG "diag"
ATTR_CUST "cust"
ATTR_BOS_INST "bos_inst"
ATTR_MAINT "maint"
ATTR_ABORT "reset"
ATTR_LSLPP "lslpp"
ATTR_CHECK "check"
$ ------------------------------ machine attributes
ATTR_MSTATE "Mstate"
ATTR_CSTATE "Cstate"
ATTR_PREV_STATE "prev_state"
ATTR_CRESULT "Cstate_result"
ATTR_INT_ETH "int_eth"
ATTR_MCA_TOK "mca_tok"
ATTR_MCA_ETH "mca_eth"
ATTR_MCA_FDDI "mca_fddi"
ATTR_PIF "pif"
ATTR_IF "if"
ATTR_ROS_EMU "ros_emu"
ATTR_SERVES "serves"
ATTR_CPUID "cpuid"
ATTR_PUSHABLE "pushable"
$ ------------------------------ resource attributes
ATTR_RSTATE "Rstate"
ATTR_SERVER "server"
ATTR_LOCATION "location"
ATTR_IF_SUPPORTED "if_supported"
ATTR_ALLOC_COUNT "alloc_count"
ATTR_SOURCE "source"
ATTR_SPOT_OPTIONS "spot_options"
ATTR_SIMAGES_OPTIONS "simages_options"
ATTR_EXPORTED "exported"
ATTR_FIRST_DIR "first_dir"
$ ------------------------------ network attributes
ATTR_NSTATE "Nstate"
ATTR_NET_ADDR "net_addr"
ATTR_SNM "snm"
ATTR_ROUTING "routing"
ATTR_RING_SPEED "ring_speed"
$ ------------------------------ attrs for passing NIM information between
$ ------------------------------ 	NIM methods
ATTR_PIF_NAME "pif_name"
ATTR_MASTER_IP "master_ip"
ATTR_MASTER_PORT "master_port"
ATTR_MASTER_UID "master_uid"
ATTR_ADPT_ADDR "adpt_addr"
ATTR_ADPT_NAME "adpt_name"
ATTR_MKNFSEXP_PARAMS "mknfsexp_params"
ATTR_WARNING "warning"
ATTR_BUNDLE "bundle"
ATTR_HOSTNAME "hostname"
ATTR_IP "ip"
ATTR_HA "ha"
ATTR_HT "ht"
ATTR_BF "bf"
ATTR_GW "gw"
ATTR_SM "sm"
ATTR_SA "sa"
ATTR_COMMAND "command"
ATTR_PERMS "perms"
ATTR_BOOT_INFO "boot_info"
$
ATTR_NFS_PERMS "nfs_perms"
ATTR_NFS_ROOT "root"
ATTR_NFS_ACCESS "access"
ATTR_NFS_RO "ro"
ATTR_NFS_RW "rw"
ATTR_NFS_SECURE "secure"
$
ATTR_GRANT	"grant"
ATTR_REVOKE	"revoke"
ATTR_DEVICE "device"
ATTR_CDROM "cdrom"
ATTR_TAPE "tape"
$
$ ------------------------------ flag attrs
ATTR_NO_NIM_CLIENT "no_nim_client"
ATTR_PULL "pull_interface"
ATTR_PULL_REQUEST "pull_request"
ATTR_DISP_MNTPNT "disp_mntpnt"
ATTR_MOUNT_CTRL "mount_ctrl"
ATTR_ASYNC "async"
ATTR_IGNORE_STATE "ignore_state"
ATTR_INSTALLP_FLAGS "installp_flags"
ATTR_INSTALLP_OPTIONS "filesets"
ATTR_LSLPP_FLAGS "lslpp_flags"
ATTR_FORCE "force"
ATTR_VERBOSE "verbose"
ATTR_ALL "all"
ATTR_OPTIONS "packages"
ATTR_DEBUG "debug"
$
$ ------------------------------ attr helps
$
$ I recommend that the text not pass this column--------------------------|
$
H_ATTR_CLASS_MACHINES "class of NIM objects which represent machines"
H_ATTR_CLASS_RESOURCES "NIM object class which represent install resources"
H_ATTR_CLASS_NETWORKS "class of NIM objects which represent networks"
H_ATTR_SUBCLASS_CLASS "used to group NIM predefined object classes"
H_ATTR_SUBCLASS_TYPE "used to group NIM predefined object types"
H_ATTR_SUBCLASS_OPERATION "groups NIM operations"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_SUBCLASS_CONTROL "subset of operations which effect client Cstate"
H_ATTR_SUBCLASS_INFO "groups info which can be customized for an object"
H_ATTR_SUBCLASS_BOS "groups BOS image types together"
H_ATTR_SUBCLASS_CUST "groups customization resource types together"
H_ATTR_SUBCLASS_ROS_SUPPORT "identifies information about ROS support"
H_ATTR_SUBCLASS_IF "groups network interface attributes together"
H_ATTR_DISKLESS "all filesystems & resources remote"
H_ATTR_DATALESS "local paging,dump; remote /,/usr; others remote or local"
H_ATTR_STANDALONE "local filesystems & resources"
H_ATTR_MASTER "machine which controls the NIM environment"
$
$
$ ------------------------------------------------------------------------|
$
H_ATTR_SPOT "Shared Product Object Tree - equivalent to /usr filesystem"
H_ATTR_BOOT "represents the network boot resource"
H_ATTR_ROOT "parent directory for client / (root) directories"
H_ATTR_PAGING "parent directory for client paging files"
H_ATTR_DUMP "parent directory for client dump files"
H_ATTR_HOME "parent directory for client /home directories"
H_ATTR_SHARED_HOME "/home directory shared by clients"
H_ATTR_SPOT_OPTIONS "list of optional packages required by all SPOTs"
H_ATTR_SIMAGES_OPTIONS "list of optional packages required by NIM"
H_ATTR_EXPORTED "location of a directory which NIM has exported"
H_ATTR_FIRST_DIR "pathname of first directory which NIM created"
H_ATTR_TMP "parent directory for client /tmp directories"
H_ATTR_LOG "parent directory for client /var/adm/ras directories"
H_ATTR_LPP_SOURCE "source device for optional product images"
H_ATTR_INSTALLP_BUNDLE "an installp bundle file"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_SIMAGES "directory where required optional product images are stored"
H_ATTR_BOSINST_DATA "config file used during base system installation"
H_ATTR_IMAGE_DATA "config file used during base system installation"
H_ATTR_RTE "an AIX BOS rte image"
H_ATTR_MKSYSB "an AIX mksysb image"
H_ATTR_SCRIPT "an executable file which is executed on a client"
H_ATTR_NIM_SCRIPT "directory containing customization scripts created by NIM"
H_ATTR_PULL "obsolete"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_PULL_REQUEST "indicates that client initiated the operation"
H_ATTR_DISP_MNTPNT "flag that causes resource access point to be displayed"
H_ATTR_MOUNT_CTRL "causes resource to be mounted at specified location"
H_ATTR_ASYNC "indicates that operation will update state asynchronously"
H_ATTR_IGNORE_STATE "used internally by NIM to bypass state checking"
H_ATTR_CLASS "identifies which NIM class an object belongs to"
H_ATTR_TYPE "identifies the NIM type for an object"
H_ATTR_NAME "unique object identifier which is specified by the user"
H_ATTR_AT "the time when a scheduled NIM operation will occur"
H_ATTR_AT_ARG "arguments for the command specified in the ATTR_AT attribute"
H_ATTR_AT_JOB "the job number associated with the scheduled operation"
H_ATTR_LOCKED "object is locked for update"
H_ATTR_LEVEL "AIX release level"
H_ATTR_VERSION "AIX version level"
H_ATTR_RELEASE "AIX release level"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_TRANS "defines cmd to execute when specific state transition occurs"
H_ATTR_SIZE "size information"
H_ATTR_COMMENTS "user entered comments"
H_ATTR_INSTALLP_FLAGS "flags to use with the installp command"
H_ATTR_INSTALLP_OPTIONS "fileset names to use with the installp command"
H_ATTR_LSLPP_FLAGS "options & option arguments to use with lslpp command"
H_ATTR_FORCE "flag used to override NIM state checking"
H_ATTR_VERBOSE "flag used to obtain verbose output from NIM operations"
H_ATTR_ALL "flag used internally by NIM"
H_ATTR_OPTIONS "flag used to pass optional package names"
H_ATTR_DEBUG "flag used to indicate that debug output is requested"
H_ATTR_SUBCLASS "flag used internally to specify a specific NIM subclass"
H_ATTR_MISSING "information missing from an object's definition"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_NO_NIM_CLIENT "indicates that client will not remain a NIM client"
H_ATTR_INFO "used to relate state specific information to the user"
H_ATTR_RESERVED "indicates that the object is for NIM internal use only"
H_ATTR_CONTROL "name of machine which has control over the object"
H_ATTR_PREV_STATE "store the object's previous NIM state"
H_ATTR_CRESULT "result of control action"
H_ATTR_CONTEXT "list of"
H_ATTR_MSERVED "machines which use a resource served by specified machine"
H_ATTR_MUSED "machines which serve a resource used by specified machine"
H_ATTR_RUSED "machines which use the specified resource"
H_ATTR_MRES "resources which may be used by the specified machine"
H_ATTR_USES "resources used by specified machine"
H_ATTR_NMEMBER "machines attached to specified network"
H_ATTR_DEFINE "define an object"
H_ATTR_CHANGE "change an object's attributes"
H_ATTR_REMOVE "remove an object"
H_ATTR_ALLOCATE "allocate a resource for use"
H_ATTR_DEALLOCATE "deallocate a resource"
H_ATTR_SCHEDULE "schedule an operation to occur at a specified time"
H_ATTR_DD_BOOT "enable a diskless/dataless boot"
H_ATTR_DKLS_INIT "initialize a diskless machines's environment"
H_ATTR_DTLS_INIT "initialize a dataless machines's environment"
H_ATTR_DIAG "enable a machine to boot a diagnostic image"
H_ATTR_CUST "perform software customization"
H_ATTR_BOS_INST "perform a BOS installation"
H_ATTR_MAINT "perform software maintenance"
H_ATTR_ABORT "reset an object's NIM state"
H_ATTR_LSLPP "list LPP information about an object"
H_ATTR_CHECK "check the status of a NIM object"
H_ATTR_INT_ETH "models which support booting from integrated ethernet"
H_ATTR_MCA_TOK "models which support booting from MCA token ring adaptors"
H_ATTR_MCA_ETH "models which support booting from MCA ethernet adaptors"
H_ATTR_MCA_FDDI "models which support booting from MCA FDDI adaptors"
H_ATTR_PIF "primary network interface (stanza format)"
H_ATTR_IF "network interface (stanza format)"
H_ATTR_ROS_EMU "identifies the device containing the ROS emulation"
H_ATTR_SERVES "identifies a resource which the machine serves"
H_ATTR_MSTATE "NIM state"
H_ATTR_CSTATE "NIM control state"
H_ATTR_CPUID "machine CPU id"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_PUSHABLE "obsolete"
H_ATTR_RSTATE "NIM state"
H_ATTR_SERVER "name of machine object which serves the resource"
H_ATTR_LOCATION "location of the resource"
H_ATTR_IF_SUPPORTED "specifies network interface types supported by a SPOT"
H_ATTR_ALLOC_COUNT "number of times a specific resource has been allocated"
H_ATTR_SOURCE "location of files needed to create a resource"
H_ATTR_TOK "token ring network"
H_ATTR_ENT "ethernet network"
H_ATTR_FDDI "FDDI network"
H_ATTR_NSTATE "NIM state"
H_ATTR_NET_ADDR "network address for a network"
H_ATTR_SNM "subnetmask for a network"
H_ATTR_ROUTING "routing information for a network"
H_ATTR_RING_SPEED "ring speed for token-ring networks"
H_ATTR_NFS_PERMS "NFS export permissions"
ATTR_BOOT_ENV "boot_env"
$
$ ------------------------------------------------------------------------|
$
H_ATTR_BOOT_ENV "indicates that command executing in boot environment"
ATTR_DIR_CREATED "dir_created"
H_ATTR_DIR_CREATED "pathname of a directory which NIM has created"
ATTR_PRESERVE_RES "preserve_res"
H_ATTR_PRESERVE_RES "used to retain NIM resources objects after install"
ATTR_IGNORE_LOCK "ignore_lock"
H_ATTR_IGNORE_LOCK "NIM internal use only"
ATTR_ENTER_DBG_UP "enter_dbg_up"
H_ATTR_ENTER_DBG_UP "address of the enter_dbg symbol within the UP kernel"
ATTR_ENTER_DBG_MP "enter_dbg_mp"
H_ATTR_ENTER_DBG_MP "address of the enter_dbg symbol within the MP kernel"
ATTR_MODE "mode"
H_ATTR_MODE "defines the acceptable mask for the stat.st_mode of a file"
$
$ ------------------------------------------------------------------------|
$
ATTR_VFSTYPE "vfstype"
H_ATTR_VFSTYPE "defines acceptable values for the stat.st_vfstype of a file"
ATTR_NUM_PARALLEL_SYNCS "num_parallel_syncs"
H_ATTR_NUM_PARALLEL_SYNCS "num diskless/dataless roots to sync in parallel"
ATTR_NO_MKBOOTI "no_mkbooti"
H_ATTR_NO_MKBOOTI "bypasses rebuilding of network boot images"
ATTR_ST_COMMITTED "st_committed"
H_ATTR_ST_COMMITTED "value of ST_COMMITTED in /usr/include/swvpd.h file"
ATTR_ST_APPLIED "st_applied"
H_ATTR_ST_APPLIED "valud of ST_APPLIED in /usr/include/swvpd.h file"
ATTR_IPLROM_EMU "iplrom_emu"
H_ATTR_IPLROM_EMU "defines the device where the IPL ROM emulation resides"
$
$ ------------------------------------------------------------------------|
$
ATTR_ROOT_INITIALIZED "root_initialized"
H_ATTR_ROOT_INITIALIZED "indicates that root directory already initialized"
ATTR_CABLE_TYPE "cable_type"
H_ATTR_CABLE_TYPE "defines ethernet cable type for the client's interface"
H_ATTR_CDROM "used to signify that a resource is a CDROM"
ATTR_DESTROY "destroy"
H_ATTR_DESTROY "an operation for NIM internal use only"
ATTR_NFS_ANON "anon"
H_ATTR_NFS_ANON "NIM interal use only"
ATTR_CLASS_ATTRS "attributes"
H_ATTR_CLASS_ATTRS "used to store characteristics of individual objects"
H_ATTR_CLASS_FLAGS "attributes only used to pass info to NIM operations"
H_ATTR_CLASS_STATES "the set of possible NIM states"
H_ATTR_CLASS_RESULTS "the set of possible results from NIM operations"
$
$ ------------------------------------------------------------------------|
$
ATTR_SUBCLASS_SEQNO "seqno"
H_ATTR_SUBCLASS_SEQNO "subset of attributes which require a sequence number"
ATTR_SUBCLASS_DKLS_RES "dkls_res"
H_ATTR_SUBCLASS_DKLS_RES "subset of resource types which diskless can use"
ATTR_SUBCLASS_DTLS_RES "dtls_res"
H_ATTR_SUBCLASS_DTLS_RES "subset of resource types which dataless can use"
ATTR_SUBCLASS_PULL_OPS "pull_ops"
H_ATTR_SUBCLASS_PULL_OPS "subclass of operations which may be pulled"
ATTR_AUTO_EXPAND "auto_expand"
H_ATTR_AUTO_EXPAND "flag to automatically expand filesystems"
ATTR_SUBCLASS_STD_RES "std_res"
H_ATTR_SUBCLASS_STD_RES "subset of resource types which standalone can use"
ATTR_SUBCLASS_MSTR_RES "mstr_res"
H_ATTR_SUBCLASS_MSTR_RES "subset of resource types which master can use"
H_ATTR_SUBCLASS_ONLY_1 "subset of resources of which only 1 can be used"
H_ATTR_SUBCLASS_PDIR_TYPE "subset of resources that are parent directories"
H_ATTR_SUBCLASS_BOOT "subset of resources which form network boot support"
H_ATTR_SUBCLASS_FLAGS "subset of attributes which are used as flags only"
ATTR_UNCONFIG "unconfig"
H_ATTR_UNCONFIG "unconfigure the NIM master fileset"
H_ATTR_MASTER_PORT "the port number used for master/client socket communication"
ATTR_PLATFORM "platform"
H_ATTR_PLATFORM "specifies the type of hardware platform a machine uses"
ATTR_SUBCLASS_PLATFORM "platform_subclass"
H_ATTR_SUBCLASS_PLATFORM "attributes which define the platform choices"
ATTR_RS6K "rs6k"
H_ATTR_RS6K "RISC System 6000"
ATTR_RS6KSMP "rs6ksmp"
H_ATTR_RS6KSMP "RISC System 6000 Symmetric Multiprocessor"
ATTR_RSPC "rspc"
H_ATTR_RSPC_OLD "RISC Personal Computer System"
ATTR_ENTER_DBG "enter_dbg"
H_ATTR_ENTER_DBG "address of the enter_dbg for the specified platform type"
ATTR_SUBCLASS_RS6K "rs6k_subclass"
H_ATTR_SUBCLASS_RS6K "machine configuration types which are supported for rs6k"
ATTR_SUBCLASS_RS6KSMP "rs6ksmp_subclass"
H_ATTR_SUBCLASS_RS6KSMP "machine configuration types which are supported for \
rs6ksmp"
ATTR_SUBCLASS_RSPC "rspc_subclass"
H_ATTR_SUBCLASS_RSPC "machine configuration types which are supported for rspc"
ATTR_DEFAULT "default"
H_ATTR_DEFAULT "designates a resource as a default resource"
ATTR_OTHER_NET_TYPE "other_net_type"
H_ATTR_OTHER_NET_TYPE "other network interface types which are represented by \
the same network object"
ATTR_MK_NETBOOT "mk_netboot"
H_ATTR_MK_NETBOOT "informs the master that network boot images need to be rebuilt"
H_ATTR_RSPC_OLD1 "IBM Personal Computer Power Series"
H_ATTR_ADPT_ADDR "specifies network adapter hardware address"
$
ATTR_SYNC_ROOTS "sync_roots"
H_ATTR_SYNC_ROOTS "synchronize roots for all clients using specified SPOT"
H_ATTR_RSPC "PowerPC Personal Computer"
ATTR_FIXES "fixes"
H_ATTR_FIXES "fix keywords to specify to the cust or fix_query operation"
ATTR_FIX_BUNDLE "fix_bundle"
H_ATTR_FIX_BUNDLE "fix (keyword) input file for the cust or fix_query operation"
ATTR_FIXQUERY "fix_query"
H_ATTR_FIXQUERY "perform queries on installed fixes"
ATTR_FIXQUERY_FLAGS "fix_query_flags"
H_ATTR_FIXQUERY_FLAGS "optional flags for modifying output of fix_query operation"
ATTR_FORCE_PUSH "force_push"
H_ATTR_FORCE_PUSH "client should be modified to accept NIM push installs"
ATTR_NO_CLIENT_BOOT "no_client_boot"
H_ATTR_NO_CLIENT_BOOT "indicates that client should not be rebooted"
ATTR_REBOOT "reboot"
H_ATTR_REBOOT "reboot specified machines"
ATTR_SET_BOOTLIST "set_bootlist"
H_ATTR_SET_BOOTLIST "set normal mode boot list if no_client_boot=yes"
ATTR_INST_WARNING "inst_warning"
H_ATTR_INST_WARNING "display re-installation warning to users on target machine"
$
$ ------------------------------ SMIT_SET -------------------------------
$ this set contains strings which are used in the NIM smit screens
$set 10
SMIT_1 "Network Installation Management"
SMIT_2 "Configure Network Installation Management Master Fileset"
SMIT_3 "Manage Network Objects"
SMIT_4 "Manage Machine Objects"
SMIT_5 "Manage Resource Objects"
SMIT_6 "Define a Network Object"
SMIT_7 "Change/Show Characteristics of a Network Object"
SMIT_8 "Remove a Network Object"
SMIT_9 "Manage Network Install Routing"
SMIT_10 "List Network Install Routes"
SMIT_11 "Define a Network Install Route"
SMIT_12 "Change/Show Characteristics of a Network Install Route"
SMIT_13 "Remove a Network Install Route"
SMIT_14 "List All Machine Objects"
SMIT_15 "Define a Machine Object"
SMIT_16 "Change/Show Characteristics of a Machine Object"
SMIT_17 "Remove a Machine Object"
SMIT_18 "Manage Network Install Interfaces"
SMIT_19 "Manage Network Install Resource Allocation"
SMIT_20 "Perform Operations on Machine Objects"
SMIT_21 "List Network Install Interfaces"
SMIT_22 "Define a Network Install Interface"
SMIT_23 "Change/Show Characteristics of a Network Install Interface"
SMIT_24 "Remove a Network Install Interface"
SMIT_25 "List Allocated Network Install Resources"
SMIT_26 "Allocate Network Install Resources"
SMIT_27 "Deallocate Network Install Resources"
SMIT_28 "List All Network Install Resource Objects"
SMIT_29 "Define a Resource Object"
SMIT_30 "Change/Show Characteristics of a Resource Object"
SMIT_31 "Remove a Resource Object"
SMIT_32 "Perform Operations on Resource Objects"
SMIT_33 "Network Object Name"
SMIT_34 "  Ring Speed"
SMIT_35 "Primary Network Install Interface"
SMIT_36 "Port Number for Network Install Communications"
SMIT_37 "Required Network Install Support Images (simages)"
SMIT_38 "Object Name"
SMIT_39 "Object Type"
SMIT_40 "Network IP Address"
SMIT_41 "Subnetmask"
SMIT_42 "Comments"
SMIT_43 "4,16"
SMIT_44 "Network Object Type"
SMIT_45 "Network Install State"
SMIT_46 "Destination Network Object Name"
SMIT_47 "Gateway Used by Originating Network"
SMIT_48 "Gateway Used by Destination Network"
SMIT_49 "Force"
SMIT_50 "no,yes"
SMIT_51 "Network Install Route"
SMIT_52 "   Destination Network Object Name"
SMIT_53 "   Gateway Used by Originating Network"
SMIT_54 "List All Machines Objects"
SMIT_55 "Machine Object Type"
SMIT_56 "Machine Object Name"
SMIT_57 "Primary Install Interface"
SMIT_58 "  Network Object Name"
SMIT_59 "  Host Name"
SMIT_60 "  Network Adapter Hardware Address"
SMIT_61 "  Network Adaptor Logical Device Name"
SMIT_62 "CPU Id"
SMIT_63 "Network Install Machine State"
SMIT_64 "Network Install Control State"
SMIT_65 "  Network Adapter Hardware Addr"
SMIT_66 "Network Install Interface"
SMIT_67 "Network Install Resources to Allocate"
SMIT_68 "Allocated Network Install Resources"
SMIT_69 "Diskless Initialization"
SMIT_70 "Size of Paging File"
SMIT_71 "Dataless Initialization"
SMIT_72 "Size of Paging Space"
SMIT_73 "Enable Diagnostic Boot"
SMIT_74 "Reset the NIM State of a Machine"
SMIT_75 "Perform Network Install Customization"
SMIT_76 "installp Flags"
SMIT_77 "Fileset Names"
SMIT_78 "Perform a Network Install"
SMIT_79 "Remain a NIM Client After Install"
SMIT_80 "yes,no"
SMIT_81 "Display Pull Interface during Install"
SMIT_82 "Perform Software Maintenance"
SMIT_83 "When to Perform Operation"
SMIT_84 "Resource Object Type"
SMIT_85 "Source of BOS Files"
SMIT_86 "Source of Install Images"
SMIT_87 "Resource Object Name"
SMIT_88 "Resource Type"
SMIT_89 "Server of Resource"
SMIT_90 "Location of Resource"
SMIT_91 "Customize a SPOT"
SMIT_92 "Source for Installable Images"
SMIT_93 "   installp_bundle"
SMIT_94 "   lpp_source"
SMIT_95 "Perform SPOT installp Maintenance"
SMIT_96 "List SPOT Filesets"
SMIT_97 "lslpp Flags"
SMIT_98 "Check the Status of a SPOT"
SMIT_99 "Network Type Object"
SMIT_100 "Available Network Install Resources"
SMIT_101 "Operation to Perform"
SMIT_102 "Network Install Operation to Perform"
SMIT_103 "Manage Network Install Permissions"
SMIT_104 "Host Name of Network Install Master"
SMIT_105 "IPL ROM Emulation Device"
SMIT_106 "Network Install Master Push Permissions"
SMIT_107 "enable,disable"
SMIT_108 "   Source Device for Creation of simages"
SMIT_109 "   Location of simages"
SMIT_110 "Names of Option Packages"
SMIT_111 "Create IPL ROM Emulation Media"
SMIT_112 "Target DEVICE"
SMIT_113 "Path Name of IPL ROM Emulation Image"
SMIT_114 "Boot MODE"
SMIT_115 "Source for BOS Runtime Files"
SMIT_116 "  Cable Type"
SMIT_117 "Configure Network Installation Management Client Fileset"
SMIT_118 "List All Network Objects"
SMIT_119 "List lpp_source Filesets"
SMIT_120 "Check the Status of an lpp_source"
SMIT_121 "Primary Network"
SMIT_122 "Check the NIM State of a Machine"
SMIT_123 "Originating Network Object Name"
SMIT_124 "Expand file systems if space needed?"
SMIT_125 "Hardware Platform Type"
SMIT_126 "Remain NIM client after install?"
SMIT_127 "installp Bundle"
SMIT_128 "Fixes (Keywords)"
SMIT_129 "Display Symptom Text?"
SMIT_130 "Only Consider Currently Installed Filesets?"
SMIT_131 "Verbose output?"
SMIT_132 "Perform Query on Installed Fixes"
$
$ ----------------------------  VERBOSE_SET   ----------------------------------
$ this set contains detailed information about NIM attributes which is displayed
$		using lsnim (eg, "lsnim -p -a <attr>...")
$ this information should be limited to approx. 70 characters per line
$		so that, when the information is displayed, it doesn't wrap around
$
$set VERBOSE_SET
$
$ I recommend that the text not pass this column----------------------|
$
VERBOSE_ATTR_CLASS_MACHINES "\
NIM functions by modeling some aspects of the physical\n\
environment in which it is operating. To do this, NIM requires that\n\
each physical entity that is going to participate be\n\
represented by a unique object in the NIM database. Each NIM object\n\
is organized into a specific object class and type, and each is\n\
identified by a unique name that is assigned by the user.\n\
This attribute defines the class of NIM objects that is used to\n\
represent the machines that exist in your NIM environment. Each\n\
machine that is going to participate in the NIM environment must be\n\
represented by its own, unique machine object. This object can be\n\
created in two ways: \n\
   -> Use the \"define\" operation on the NIM master. This\n\
      operation requires you to supply the type of object being\n\
      defined and, in this case, type refers to the machine's\n\
      configuration type. Use the lsnim command to display a list\n\
      of supported machine types: \n\
         # lsnim -p -c machines\n\
   -> From a running, standalone machine on which the\n\
      bos.sysmgt.nim.client package has been installed, you can run\n\
      the niminit command. This command contacts the NIM master\n\
      and asks it to create a new machine object for that machine.\n\
Once a machine object has been defined to represent a machine, you\n\
can perform operations on it. Use the lsnim command to display a list\n\
of supported operations: \n\
   -> To display supported operations based on the object type, enter:\n\
         # lsnim -p -O -t <NIM object type>\n\
   -> To display supported operations based on the object name, enter:\n\
         # lsnim -O <NIM object name>\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_RESOURCES "\
NIM functions by modeling some aspects of the physical\n\
environment in which it is operating. To do this, NIM requires that\n\
each physical entity that is going to participate be\n\
represented by a unique object in the NIM database. Each NIM object\n\
is organized into a specific object class and type, and each is\n\
identified by a unique name which is assigned by the user.\n\
This attribute defines the class of NIM objects that is used to\n\
represent the resources that exist in your NIM environment. Each\n\
resource that you want to use in the NIM environment must be\n\
represented by a unique resource object: use the \"define\" operation\n\
to do this. Note: This operation requires you to supply the\n\
type of resource being defined. You can use the lsnim command to\n\
display a list of supported NIM resource types: \n\
   # lsnim -p -c resources\n\
Once this kind of object has been defined, it can be used to support\n\
other NIM operations. To use a resource during a NIM\n\
operation that is performed on a machine object, NIM requires you to\n\
first perform the \"allocate\" operation, which is a preparatory step\n\
that: \n\
   -> Verifies the machine which is going to use the resource\n\
      (the client) can access the machine that is serving the\n\
      resource (the server). NIM uses its connectivity algorithm to\n\
      determine whether this communication is possible.\n\
   -> Verifies the client's configuration type is allowed to use a\n\
      the specified resource type.\n\
   -> Verifies that the specified resource is available for use.\n\
If all of these conditions are met, then NIM grants the client\n\
access to the resource by using NFS to export it for the client's use.\n\
To indicate that a resource has been allocated to a machine,\n\
NIM adds an attribute of the resource's type to the machine's\n\
definition. The value of this attribute will be the NIM name of the\n\
allocated resource. Once a resource has been allocated to a client,\n\
it remains allocated until the deallocate operation is performed.\n\
For deallocation, NIM removes the client's export permissions to\n\
access the resource and removes the attribute that was added to\n\
the client's definition to indicate that the resource was allocated\n\
for use.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_NETWORKS "\
NIM functions by modeling some aspects of the physical\n\
environment in which it is operating. To do this, NIM requires that\n\
each physical entity that is going to participate be\n\
represented by a unique object in the NIM database. Each NIM object\n\
is organized into a specific object class and type, and each \n\
is identified by a unique name that is assigned by the user.\n\
This attribute defines the class of NIM objects that is used to\n\
represent the networks that exist in your NIM environment.  Each\n\
network that is part of your NIM environment must be represented by\n\
a NIM network object. Use the \"define\" operation to create a new\n\
network object. Note: This operation requires you to supply the\n\
type of network being defined. Use the lsnim command to\n\
display a list of supported NIM network types: \n\
   # lsnim -p -c networks\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_FLAGS "\
not used - the text for VERBOSE_ATTR_CLASS_ATTRS is used\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_STATES "\
NIM operates in a distributed environment in which NIM operations can\n\
occur synchronously or asynchronously. To coordinate these\n\
operations, NIM enforces concurrency control among\n\
NIM objects by maintaining state information for each\n\
NIM object. This information is managed entirely by NIM and is updated\n\
in response to the NIM operations that you perform and other conditions\n\
that NIM is able to detect. NIM states are stored in attributes that\n\
are part of every object's definition. The specific attribute \n\
used is dependent upon the object's class: \n\
   -> \"Mstate\" and \"Cstate\" are used for objects in the machines\n\
      class.\n\
   -> \"Rstate\" is used for objects in the resources class.\n\
   -> \"Nstate\" is used for objects in the networks class.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_RESULTS "\
This attribute is maintained as part of an object's definition, and\n\
the attribute is used to indicate the result of the previously\n\
executed NIM operation. When the result indicates that a failure has\n\
occurred, NIM may have added other attributes to the object's\n\
definition that provide you with more information about why the\n\
operation failed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS "\
NIM classifies its management information into classes and types.\n\
Sometimes, NIM also needs to group objects within the same class in\n\
different ways. NIM does this using subclasses: Each subclass\n\
represents a grouping of object types within that object class. Most\n\
NIM subclasses are for internal NIM use only; however, some subclasses\n\
may be used by the user. Use the lsnim command to list the subclasses\n\
you can specify.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_CLASS "\
NIM functions by modeling some aspects of the physical\n\
environment in which it is operating. To do this, NIM requires that\n\
each physical entity that is going to participate be\n\
represented by a unique object in the NIM database. To\n\
organize this information, NIM classifies each object into a specific\n\
class and type. You can display the list of object classes which NIM\n\
supports using the lsnim command: \n\n\
   # lsnim -p\n\n\
To display a list of object types for a specific object class, enter: \n\n\
   # lsnim -p -c <NIM object class>\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_TYPE "\
NIM functions by modeling some aspects of the physical\n\
environment in which it is operating. To do this, NIM requires that\n\
each physical entity that is going to participate be\n\
represented by a unique object in the NIM database. \n\
Each object is classified into a specific class and type.\n\
Use lsnim to display the list of supported object classes.\n\n\
   # lsnim -p\n\n\
To display list of object types for a specific object class, enter: \n\n\
   # lsnim -p -c <NIM object class>\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_OPERATION "\
This subclass identifies predefined NIM attributes that\n\
are NIM operations.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_CONTROL "\
This subclass identifies NIM predefined attributes that\n\
are NIM control operations. NIM control operations represent the\n\
subset of NIM operations that control processing on a target machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_INFO "\
This subclass identifies NIM predefined attributes that\n\
can be added to an object's definition (either internally by NIM or\n\
by the user) and relate specific information about\n\
the object they are associated with.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_BOS "\
This subclass identifies NIM objects that represent types\n\
of BOS images that can be installed by NIM.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_CUST "\
This subclass identifies NIM objects that represent types\n\
of resources that can be used to perform customization\n\
on a target.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_ROS_SUPPORT "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_IF "\
This subclass identifies NIM attributes that store NIM\n\
network interface information for a machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_ONLY_1 "\
This subclass identifies NIM resources of which only one\n\
of that resource type may be allocated to a specific client. That\n\
is, if a NIM client has a resource allocated to it which belongs\n\
to this subclass, NIM will not allow any other resources\n\
in this subclass to be allocated to the client at this time.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_PDIR_TYPE "\
This subclass identifies NIM resource object types that\n\
represent the location of \"parent\" directories in which NIM creates\n\
client subdirectories for each client the resource is allocated to.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_BOOT "\
The network boot resource is a complex resource type consisting of\n\
several parts. This subclass groups the parts together.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DISKLESS "\
This attribute represents a type of machine configuration in which\n\
all of the machine's required resources are accessed remotely.\n\
The required resources for any machine are those resources the machine\n\
needs to achieve the running execution state and consist of: \n\
   -> The standard file systems (/, /home, /tmp, and /usr)\n\
   -> A boot image\n\
   -> A paging device\n\
   -> A dump device\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DATALESS "\
This attribute represents a type of machine configuration in which\n\
some of the machine's required resources are local and some are\n\
remote. The required resources for any machine are those resources\n\
the machine needs to achieve the running execution state and\n\
consist of:\n\
   -> The standard file systems (/, /home, /tmp, and /usr)\n\
   -> A boot image\n\
   -> A paging device\n\
   -> A dump device\n\
For this machine configuration, the following resources are remote:\n\
   -> /, /usr\n\
   -> Boot image\n\
the following resources are local:\n\
   -> Paging device\n\
   -> Dump device\n\
and the following resources may be local or remote:\n\
   -> /home, /tmp\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_STANDALONE "\
This attribute represents a type of machine configuration in which\n\
all of the machine's required resources are accessed locally.  \n\
The required resources for any machine are those resources the machine \n\
needs in order to achieve the running execution state and consist of:\n\
   -> The standard file systems (/, /home, /tmp, and /usr)\n\
   -> A boot image\n\
   -> A paging device\n\
   -> A dump device\n\
For this machine configuration, all these resources are accessed\n\
locally.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MASTER "\
This attribute represents a special type of machine configuration\n\
within the NIM environment. it is the only machine \n\
allowed to push operations on other machines.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SPOT "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a Shared Product\n\
Object Tree (SPOT). SPOTs are important resources in the NIM\n\
environment because they are used to support all NIM operations that\n\
require a machine to boot using a network interface. SPOTs are\n\
created and managed by NIM.\n\
There are two ways to construct a SPOT:\n\
   -> A /usr file system on a running NIM standalone client\n\
      can be converted into a SPOT\n\
   -> A SPOT may be created from scratch in a parent directory that \n\
      you specify. \n\
In either case, SPOTs are large and occupy approximately \n\
130MB of space in the file system where they are created. \n\
Additionally, the network boot images that are created\n\
with the SPOT are stored in the /tftpboot directory on the server of\n\
the SPOT and occupy approximately 30 megabytes of space.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BOOT "\
This attribute is the NIM resource type that represents the\n\
network boot resource, which is one of resources required to\n\
support a network boot operation for a NIM client.  NIM\n\
automatically defines a resource of this type when the\n\
bos.sysmgt.nim.master package is configured for use.  NIM also\n\
manages the use of this resource in response to those NIM\n\
operations you perform that require a network boot; that is, NIM\n\
automatically performs allocation and deallocation of this resource\n\
in response to other NIM operations that you perform.\n\
The network boot resource is composed of the following parts, all of\n\
which reside on the server of the SPOT used to\n\
support the network boot operation: \n\
   -> client specific stanza in the /etc/bootptab file\n\
   -> /tftpboot/<client host name> file\n\
   -> /tftpboot/<client host name>.info file\n\
When NIM allocates this resource to a client in response to a\n\
NIM operation you've performed that requires a network boot, NIM\n\
adds this attribute to the client's definition to indicate\n\
that this resource has been allocated to the client; conversely,\n\
NIM removes this attribute from the client's definition when NIM\n\
automatically deallocates the network boot resource. Therefore, when\n\
you use the lsnim command to display information about a client \n\
and this attribute is displayed, it indicates that the NIM environment \n\
has been enabled to support a network boot for that client.\n\
Note: Having this attribute does not guarantee that the client\n\
will actually boot from a network interface: it only means\n\
that NIM has set up the environment based on the information you\n\
provided. If you experience problems booting a client using\n\
a network interface, consult the troubleshooting section in the\n\
Network Installation Management Guide and Reference.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ROOT "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client root directories are maintained. When this type of\n\
resource is allocated to a client, NIM creates a subdirectory for\n\
the client's exclusive use. This allocated subdirectory is subsequently\n\
initialized when you perform the dkls_init or dtls_init operation.\n\
After initialization, anytime the client performs a network boot,\n\
the client NFS mounts this subdirectory over / to gain\n\
access to the root directory that has been setup for its use. This\n\
subdirectory remains mounted over / on the client as long as the\n\
client is running.\n\
Note: Whenever this resource is deallocated, NIM removes the\n\
subdirectory that was created for the client's use. Therefore, any\n\
files you want to save in the client's subdirectory should be\n\
backed up before you deallocate a resource of this type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PAGING "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client paging files are maintained. When this type of resource\n\
is allocated to a client, NIM creates a subdirectory for the\n\
client's exclusive use. This allocated subdirectory is initialized\n\
by the dkls_init or dtls_init operation, \n\
which creates a file in this subdirectory that the client\n\
configures as a paging device when it performs a network boot.\n\
By default, 32 megabytes is reserved for this file: you may\n\
specify a different value using the \"size\" flag when you perform\n\
the dkls_init or dtls_init operation.\n\
After this resource has been initialized for a client, it is\n\
configured as a paging device by the client each time the client\n\
performs a network boot. If you subsequently deallocate this\n\
resource, NIM removes the paging file and the subdirectory it\n\
created for the client's use.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DUMP "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client dump directories are maintained. When this type of\n\
resource is allocated to a client, NIM creates a subdirectory for\n\
the client's exclusive use. This allocated subdirectory is initialized\n\
by the dkls_init or dtls_init operation, \n\
which creates an empty file in this subdirectory. After\n\
initialization, the client uses this file to store any dump\n\
images it creates. Note: If you subsequently deallocate this\n\
resource, NIM removes the dump file and the subdirectory NIM\n\
created for the client's use.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_HOME "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client /home directories are maintained. When this type of\n\
resource is allocated to a client, NIM creates a subdirectory for\n\
the client's exclusive use. This allocated subdirectory is subsequently\n\
initialized when you perform the dkls_init or dtls_init operation.\n\
After initialization, anytime the client performs a network boot,\n\
the client NFS mounts this subdirectory over /home to\n\
gain access to the home directory that has been setup for its use.\n\
This subdirectory remains mounted over /home on the client as long\n\
as the client is running.\n\
Note: Whenever this resource is deallocated, NIM removes the\n\
subdirectory that was created for the client's use. Therefore, back up\n\
any files you want to save in the client's subdirectory \n\
before you deallocate a resource of this type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SHARED_HOME "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory\n\
which can be used as a common /home directory by one or more clients.\n\
When this type of resource is allocated to a client, and when the\n\
dkls_init or dtls_init operation is performed, NIM configures\n\
the client's configuration to use this common directory. After\n\
initialization, anytime the client performs a network boot, the\n\
client NFS mounts this common directory over its /home\n\
directory. This common directory remains mounted as long as\n\
the client is running.\n\
Note: Whenever this resource is deallocated, NIM only changes\n\
the client's configuration so that this directory is no longer used\n\
by the client; NIM does not remove the common directory.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_TMP "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client /tmp directories are maintained. When is type of\n\
resource is allocated to a client, NIM creates a subdirectory for\n\
the client's exclusive use. This allocated subdirectory is subsequently\n\
initialized when you perform the dkls_init or dtls_init operation.\n\
After initialization, anytime the client performs a network boot,\n\
the client NFS mounts this subdirectory over /tmp to\n\
gain access to /tmp directory that has been setup for its use.\n\
This subdirectory remains mounted over /tmp on the client as long\n\
as the client is running.\n\
Note: Whenever this resource is deallocated, NIM removes the\n\
subdirectory that was created for the client's use. Therefore, back up\n\
any files you want to save in the client's subdirectory \n\
before you deallocate a resource of this type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LOG "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which client RAS directories are maintained. When this type of\n\
resource is allocated to a client and the bos_inst, dkls_init, or\n\
dtls_init operation is then performed on the client's object, NIM\n\
initializes the client's configuration so that the client's\n\
/var/adm/ras directory is overmounted by this resource when\n\
the client performs a network boot.\n\
Note: Whenever this resource is deallocated, NIM removes the\n\
subdirectory that was created for the client's use. Therefore, back up\n\
any files which you wish to save in the client's subdirectory \n\
before you deallocate a resource of this type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LPP_SOURCE "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a directory in\n\
which optional product images are stored. This type of resource\n\
can be defined in several ways: \n\
   -> If you already have a directory that contains optional\n\
      images, no special attributes need to be supplied.\n\
   -> If you want to create a directory and have NIM fill\n\
      it with the set of support images (\"simages\"), \n\
      use the \"source\" attribute.  This attribute specifies the\n\
      name of the device that contains the optional images. NIM\n\
      copies the \"simages\" from this device into the specified\n\
      location of this resource.\n\
   -> If you want to create a directory, but you want\n\
      specific options to be copied rather than the default set of\n\
      \"simages\", supply the \"options\" attribute.\n\
      This attribute tells NIM which packages you want copied.\n\
This type of resource is used to support NIM install operations.\n\
When you perform a NIM install operation on a client and have previously\n\
allocated a resource of this type for the client to use, NIM uses\n\
this resource from the client by first NFS mounting the directory\n\
where the resource is located, then invoking the installp command\n\
with the location of the local mount point where NIM has mounted\n\
this resource. When installp finishes, NIM automatically unmounts\n\
the resource.\n\
To execute successfully, installp requires some information. installp\n\
needs the location of the installable images to be installed, and\n\
installp needs to know \"how\" to perform the install; that is,\n\
should prereqs be installed automatically?, should file systems be \n\
expanded automatically?, and so on. This type of information is \n\
passed to installp as flags to the command. By default, NIM uses the \n\
\"-agX\" flags when calling installp. You can override this behavior \n\
by supplying the \"installp_flags\" attribute when you perform the \n\
NIM install operation. When you do so, NIM uses those flags when it \n\
executes the installp command.\n\
installp also needs to know the list of filesets that are to be \n\
installed. By default, NIM does not\n\
automatically assume that \"all\" products should be installed. So,\n\
in the majority of cases, NIM only installs the filesets it\n\
needs to function. You can override this behavior by\n\
specifying the filesets you want installed. These filesets can be\n\
specified in two ways: \n\
   -> Supply the \"filesets\" attribute when performing the\n\
      NIM install operation. When you do so, NIM uses the filesets\n\
      you specify when NIM executes installp.\n\
   -> Use a previously defined installp_bundle resource. When\n\
      a resource of this type is allocated, and when a NIM install \n\
      operation is performed on a client, NIM NFS mounts this file \n\
      and passes the local access point to installp using the \n\
      installp \"-f\" option.\n\
Note: If you add or remove an installable image from the\n\
the lpp_source, perform the check operation on that object\n\
so that NIM rebuilds the .toc (table of contents) file, which\n\
resides in the lpp_source directory. This is important, as the\n\
installp command uses the .toc to determine which images are available.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SIMAGES "\
This attribute is used to indicate that an lpp_source type resource\n\
contains the set of installable images to which NIM requires access\n\
to perform its basic functionality. This basic set of\n\
images is referred to as \"support images\" or \"simages\".\n\
NIM automatically manages the use of this attribute as part of the\n\
management of an lpp_source object. NIM adds this attribute to\n\
the definition of an lpp_source object when it provides the required\n\
simages, and NIM removes this attribute from the object's\n\
definition if a required image becomes unavailable.\n\
Some NIM operations require access to an lpp_source that has this\n\
attribute as part of its definition, so having this attribute can be\n\
important. Perform the check operation on the lpp_source to\n\
have NIM to check to see if the simages requirement has been\n\
fulfilled. if it has, NIM adds this attribute to the lpp_source\n\
definition.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INSTALLP_BUNDLE "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
contains the names of optional filesets. Once defined, it can be used\n\
in NIM install operations by first allocating it to the target, then\n\
performing the desired NIM install operation on the target. When this\n\
is done, NIM uses this file when it executes the installp command to: \n\
   1) NFS mount this resource so that the client has local access\n\
      to it\n\
   2) Pass the local access point to installp by using the\n\
      \"-f\" installp option\n\
   3) Unmount this resource when installp finishes\n\
Note: To install optional software, the installp command\n\
requires access to the installable images for those products.\n\
This resource should be used in conjunction with a NIM lpp_source type\n\
resource.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BOSINST_DATA "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
contains information which can be used to drive the BOS install\n\
program. When a resource of this type is allocated to a client and\n\
the bos_inst operation is then performed, NIM causes the BOS\n\
install program to use this file during base system installation.\n\
You can affect certain aspects of your base system installation by\n\
modifying the settings in this file. For example, you can avoid\n\
BOS install prompting. However, if you do not use this type of\n\
resource for the bos_inst operation, BOS install will use the \n\
default data file that resides in the SPOT being used to support the \n\
network install operation.  The path name of the default data file is\n\
<SPOT offset>/usr/lpp/bosinst/bosinst.template on the server of the\n\
SPOT.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IMAGE_DATA "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
contains information which can be used to drive the BOS install\n\
program. When a resource of this type is allocated to a client and\n\
the bos_inst operation is then performed, NIM causes the BOS\n\
install program to use this file during base system installation.\n\
You can affect certain aspects of your base system installation by\n\
modifying the settings in this file. For example, you can specify the\n\
size of the paging device which BOS install creates. However,\n\
if you do not use this type of resource for the NIM bos_inst\n\
operation, when NIM invokes the BOS install program, BOS install \n\
uses the default data file that resides in the SPOT being used\n\
to support the network install operation. The path name of the default\n\
data file is <SPOT offset>/usr/lpp/bosinst/image.template on the\n\
server of the SPOT.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RTE "\
This string is one of the possible values for the \"source\" flag when\n\
this flag is specified for a NIM bos_inst operation. \n\
When the string \"rte\" is used as the value of this flag, NIM\n\
uses the BOS run-time image in the lpp_source resource \n\
being used to support the install. Other possible\n\
values are \"spot\", indicating that the spot that is supporting the\n\
install should be copied into the client's /usr file system, or \"mksysb\",\n\
indicating that an allocated mksysb image should be used.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MKSYSB "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
is a system backup image created using the mksysb\n\
command. This type of resource can be used as the source of the\n\
BOS run-time files when a base system installation is performed, by\n\
first allocating a resource of this type to the target, then\n\
performing the bos_inst operation on the target. Note: When you\n\
perform the bos_inst operation, the \"source\" flag should be used\n\
with a value of \"mksysb\". This tells NIM that you want to use the\n\
allocated mksysb image. NIM sets up the network boot\n\
environment such that BOS install will use this image to populate the\n\
client's /usr file system.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SCRIPT "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
is a user-defined shell script. Once defined, this type of resource\n\
can be used to perform user-defined processing by first allocating\n\
a resource of this type to the target, then performing either the\n\
NIM cust or bos_inst operation on the target. When you do this, NIM\n\
configures the environment to cause the target to execute the shell\n\
script you have defined.\n\
Note: When this type of resource is used for a bos_inst operation,\n\
your script is executed during the third step in the NIM\n\
customization phase. During a bos_inst operation, NIM customization\n\
occurs after BOS install performs the base system installation\n\
and before BOS install performs post-install processing. During this\n\
phase, NIM will enforce the following order: \n\
   1) NIM client customization is performed, in which NIM configures\n\
      the target's primary interface.\n\
   2) NIM uses installp to install the optional software you have\n\
      specified to be installed.\n\
   3) NIM executes any user-defined scripts that you have allocated.\n\
NIM enforces this order because, by installing optional software\n\
first then executing your script, NIM enables you to\n\
configure the optional software that NIM installed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NIM_SCRIPT "\
This attribute is the NIM resource type that represents executable\n\
shell scripts which NIM creates to perform customization\n\
operations. NIM automatically defines a resource of this type when \n\
you configure the bos.sysmgt.nim.master package. NIM also\n\
manages the use of this resource in response to those NIM\n\
operations you perform that require customization; that is, NIM \n\
automatically performs allocation and deallocation of this resource in\n\
response to other NIM operations you perform.\n\
NIM performs customization on a target by creating a shell script\n\
which NIM then causes the target to execute. NIM constructs this\n\
script by examining the types of resources that are already\n\
allocated to the target and by using information \n\
passed to the NIM operation to perform the customization.\n\
You can use the lsnim command to display the attributes that spawn\n\
construction of a NIM customization script: \n\
   # lsnim -p -s cust_subclass\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_TOK "\
This attribute is a NIM network type. When a network object of\n\
this type is defined, it represents a Token-Ring network.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ENT "\
This attribute is a NIM network type. When a network object of\n\
this type is defined, it represents an Ethernet network.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FDDI "\
This attribute is a NIM network type. When a network object of\n\
this type is defined, it represents a FDDI network.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_TYPE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NAME "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_AT "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_AT_ARG "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_AT_JOB "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LOCKED "\
This attribute is added to an object's definition to \"lock\"\n\
the object. Because NIM operations can occur synchronously or\n\
asynchronously, NIM requires a mechanism to control access to each\n\
NIM object. NIM does this in two ways:\n\
   -> NIM manages persistent states using state attributes.\n\
   -> NIM provides temporary locks using the \"locked\" attribute.\n\
When a NIM operation begins executing, it accesses information about\n\
the object that it is operating on. To prevent this\n\
information from being changed when a NIM operation is executing, NIM\n\
locks the object it is operating on by adding a \"locked\"\n\
attribute to the object's definition. The value of\n\
the \"locked\" attribute is the process ID of the process that\n\
has locked the object. When the operation is done with the object,\n\
the \"locked\" attribute is removed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LEVEL "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_VERSION "\
This attribute is used to store information about the operating\n\
system version number for machine objects and the spot resource object.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RELEASE "\
This attribute stores information about the operating\n\
system release number for machine objects and the spot resource object.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_TRANS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SIZE "\
This attribute passes information to NIM operations and is\n\
therefore classified as a \"flag\". For operations that accept\n\
this flag, it specifies how big to make a resource that\n\
the operation will create.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_COMMENTS "\
This attribute specifies user-defined comments to be\n\
associated with a NIM object. When specified, this attribute\n\
becomes part of the object's definition.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MISSING "\
NIM manages this attribute in response to NIM operations that\n\
you perform. If you perform a NIM operation that causes the\n\
definition of an object to become incomplete, NIM adds this\n\
attribute to the object's definition to inform you what is\n\
missing from that definition. When you correct the error condition\n\
NIM removes this attribute from the object's definition.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INFO "\
NIM manages this attribute in response to NIM operations that\n\
you perform. NIM uses this attribute to relate miscellaneous \n\
information about a NIM object by adding this attribute to the object's\n\
definition. For example, while a base system installation is\n\
occurring on a NIM client, this attribute is updated to reflect the\n\
BOS install processing occurring on that client.\n\
This enables you to monitor a bos_inst operation as it is\n\
being performed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RESERVED "\
NIM manages this attribute to mark certain objects as\n\
reserved for NIM internal use only.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CONTROL "\
NIM manages this attribute, which indicates which machine\n\
is currently in control of this object. Because NIM operates in\n\
a distributed environment in which multiple NIM operations can be active\n\
at any given time, NIM must coordinate access to each NIM object. \n\
NIM needs to control operations that are performed on a\n\
machine object. Because the allocate operation is a preparatory\n\
step for NIM install operations, when you allocate a resource, NIM\n\
assumes that you are going to perform a NIM install operation soon.\n\
Because the allocate operation can be performed from the NIM master or\n\
a running NIM client, NIM assumes that the machine requesting\n\
the allocation is also going to perform the NIM install operation.\n\
To prevent one machine from allocating resources\n\
to a specific target while another machine is performs an operation\n\
on that same target, NIM adds the \"control\" attribute to the target's\n\
definition when the first resource is allocated to the target. When\n\
this attribute is added by NIM, its value is\n\
the name of the machine that executed the allocate operation. This\n\
is the machine that remains in control of the target until\n\
all resources have been deallocated for the target.\n\
Having control over a NIM object means that only the machine which is\n\
in control of the object may perform operations on it.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CONTEXT "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MSERVED "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MUSED "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RUSED "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MRES "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NMEMBER "\
obsolete\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NETNAME "\
This attribute is only used to pass information to the nimconfig\n\
command and is, therefore, classified as a \"flag\". It \n\
specifies the name of the network object that\n\
nimconfig will create. This network object represents the network \n\
that the master's primary interface connects to.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DEFINE "\
This NIM operation creates a new NIM object to\n\
represent a physical entity in your environment. Before NIM can be\n\
used to perform an operation on a physical entity, the entity must\n\
be represented by a NIM object. When you use this operation to\n\
create a new object, you must always supply the object's type and the\n\
name you want to call the object. Other information may be required \n\
depending on the type of object you are creating.  Use the\n\
following lsnim command to display the attributes that are required \n\
to perform this operation on a specific object type: \n\
   # lsnim -q define -t <NIM object type>\n\
The name that you give an object will be used in all future operations\n\
involving that object.  This name must be unique among NIM objects; it\n\
must adhere to certain restrictions: \n\
   -> It must have between 1 and 20 characters.\n\
   -> It cannot be composed of any shell metacharacters.\n\
   -> It cannot be composed of any file system metacharacters.\n\
   -> It cannot be composed of any regular expression metacharacters.\n\
Note: A NIM object's name is a \"global\" name, as each object\n\
is only known to NIM by that one name.  This is beneficial for\n\
multihomed machines. Multihomed machines have more than one network\n\
interface. Because a host name must be given to each network interface a\n\
machine has, multihomed machines could be known by more\n\
than one name. However, in the NIM environment, each machine is\n\
identified by its unique NIM object name, so it doesn't matter how\n\
many network interfaces the machine has.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CHANGE "\
This NIM operation modifies attribute\n\
information for an existing NIM object. NIM objects are composed of a\n\
set of attributes that store specific information about that object.\n\
Use the following lsnim command to display the attributes that currently\n\
make up the object's definition: \n\
   # lsnim -l <NIM object name>\n\
Not all of the attributes that comprise an object's definition can\n\
be modified. Use the following lsnim command to determine which \n\
ones can be modified: \n\
   # lsnim -q change <NIM object name>\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_REMOVE "\
This NIM operation removes a NIM object from\n\
the NIM environment. NIM operations can only be performed on those\n\
entities that are represented by a NIM object; therefore,\n\
when you remove a NIM object, you remove your ability to perform\n\
operations on the entity represented by that object.\n\
When this operation is performed on a NIM resource object, NIM may\n\
actually remove the physical resource which this object represents.\n\
This is determined by whether NIM actually created the physical\n\
resource when the resource object was defined. If NIM created the\n\
resource, then NIM removes the physical resource; if not, NIM does\n\
not remove the physical resource.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ALLOCATE "\
This NIM operation grants a NIM client permission to\n\
access a NIM resource. The following factors are used when determining\n\
whether a client should be granted permissions: \n\
   -> The NIM connectivity algorithm is run to verify that the\n\
      client has the potential to communicate with the server of the\n\
      resource. This algorithm uses the network interface information\n\
      you entered for the client and server, and the NIM routing\n\
      information you entered for the networks that these machines\n\
      connect to. When this algorithm determines that the client has\n\
      the potential to communicate with the server, it also determines\n\
      which network interface on the server should be used by the client.\n\
      (Note: NIM clients only access NIM resources through one network\n\
      interface, their primary network interface, which is defined by the\n\
      if1 attribute.) If the connectivity algorithm determines that\n\
      the client cannot communicate with the server, permission to access\n\
      the resource is denied.\n\
   -> NIM verifies that the client's object type is permitted to access\n\
      a resource of the type being allocated; if not, access permission\n\
      is denied.\n\
   -> NIM verifies that the resource being allocated is available for use.\n\
      This is determined by the resource's state; if it is unavailable for\n\
      use, access permission is denied.\n\
When NIM grants a client permission to access a resource, NIM \n\
exports that resource for the client's use and performs the\n\
management of the NIM objects to keep track of this allocation. NIM\n\
performs this management in two ways. First, a count is kept of how many\n\
times a resource has been allocated. This information is stored in the\n\
\"alloc_count\" attribute of the resource's object and NIM increments this\n\
count each time the resource is allocated. Secondly, NIM adds an\n\
attribute to the client's object to indicate that the resource has been\n\
allocated to it. This attribute is added in the form of: \n\
   <resource object type> = <resource object name>\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DEALLOCATE "\
This is the NIM operation that removes a NIM client's permission to\n\
access a NIM resource. NIM first verifies that the specified\n\
resource has been allocated to the client. If so, NIM removes the\n\
client's permission to access the resource by first unexporting the\n\
resource, then removing the attribute from the client's definition that\n\
NIM added to indicate that the resource was allocated to the client.\n\
NIM then decrements the \"alloc_count\" attribute associated with\n\
the resource object.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SCHEDULE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DD_BOOT "\
This attribute represents the machine state in which the NIM\n\
environment has been enabled to support a network boot for a diskless\n\
or dataless machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DKLS_INIT "\
This NIM operation initializes the resources\n\
that have been allocated to a machine with the diskless configuration\n\
type. This processing includes: \n\
   -> Populating the root directory that has been allocated for the\n\
      client's use with the prototype root files from the client's SPOT\n\
   -> Configuring the appropriate client root files so that it can\n\
      achieve the running execution state\n\
   -> Creating a paging file of the appropriate size for the client to\n\
      use as its paging device\n\
   -> Allocating the network boot resource for the client's use\n\
When this operation returns successfully, the NIM environment is\n\
enabled to support the dataless client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DTLS_INIT "\
This NIM operation initializes the resources\n\
that have been allocated to a machine with the dataless configuration\n\
type. This processing includes: \n\
   -> Populating the root directory that has been allocated for the\n\
      client's use with the prototype root files from the client's SPOT\n\
   -> Configuring the appropriate client root files so that it can\n\
      achieve the running execution state\n\
   -> Allocating the network boot resource for the client's use\n\
When this operation returns successfully, the NIM environment has been\n\
enabled to support the dataless client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DIAG "\
This NIM operation enables the NIM environment\n\
to support a network based, diagnostic boot for a machine. Because this\n\
operation only enables the environment, it is up to the user to\n\
cause the client to begin the network boot process by issuing a BOOTP\n\
request. Refer to the Network Installation Management Guide\n\
and Reference for procedures on how to initiate the network boot\n\
process.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CUST "\
This NIM operation performs software customization on a running\n\
NIM client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BOS_INST "\
This NIM operation performs base system installation on a machine.\n\
In addition to base system installation,\n\
customization may also be performed by allocating the appropriate\n\
resources before the bos_inst operation is performed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MAINT "\
This NIM operation performs software maintenance operations.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ABORT "\
This NIM operation resets an object's NIM\n\
state by stopping any current NIM operation being performed on it.\n\
In some cases, NIM is not be able to stop an active operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LSLPP "\
This NIM operation uses the lslpp command to display optional software\n\
product information for a NIM object.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CHECK "\
This NIM operation performs verification on a NIM object. The type\n\
of verification performed depends on the object's type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MSTATE "\
NIM manages this attribute, which stores the execution\n\
state for a machine. NIM attempts to keep the value of this attribute\n\
synchronized with the machine's execution state, but NIM does not\n\
guarantee that its accuracy. Perform the \"check\" operation on the\n\
machine for NIM to attempt to determine the machine's execution state.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CSTATE "\
NIM manages this attribute, which indicates the NIM\n\
operation that is being currently being performed on a machine\n\
or that no NIM operations are currently being performed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PREV_STATE "\
This attribute is managed by NIM and is used to indicate the previous\n\
NIM state for a NIM object.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CRESULT "\
This attribute is managed by NIM and is used to indicated the result\n\
of the previous NIM operation which was performed on a machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INT_ETH "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MCA_TOK "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MCA_ETH "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MCA_FDDI "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PIF "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IF "\
This attribute stores network interface information for a\n\
NIM client and requires a sequence number when specified. This\n\
information consists of three required fields and a fourth, optional\n\
field: \n\
   field 1 = Name of the network to which this interface connects\n\
   field 2 = Host name associated with this interface\n\
   field 3 = Network adapter hardware address of this interface\n\
   field 4 = Logical device name of the network adapter used for\n\
             this interface\n\
If the fourth field is not specified, NIM uses a default\n\
based on the type of network interface defined.\n\
This attribute requires a sequence number for NIM to distinguish\n\
between multiple network interfaces. Because machines can be multihomed,\n\
NIM allows more than one \"if\" attribute per machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ROS_EMU "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SERVES "\
NIM manages this attribute, which stores the names of\n\
the resources which a machine serves. NIM adds this attribute to\n\
the definition of a machine object each time a NIM resource is defined\n\
using that machine as the server.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CPUID "\
This attribute stores the CPU ID of a machine.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PUSHABLE "\
NIM manages this attributes, which indicates that a NIM\n\
operation can be pushed onto a client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RSTATE "\
NIM manages this attribute, which indicates whether a\n\
resource is available for clients to use. The following\n\
factors are used to determine availability: \n\
   -> The definition of the resource must be complete. This\n\
      determination is based upon the resource type.\n\
   -> The server of the resource must be accessible. NIM considers a\n\
      a server to be inaccessible if a base system installation is\n\
      being performed on it.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SERVER "\
This attribute stores the NIM name of the server that is\n\
serving the NIM resource.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LOCATION "\
This attribute stores the path name of a NIM resource. It is\n\
required for all NIM resources.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IF_SUPPORTED "\
NIM manages this attribute, which indicates the type of\n\
network interface which is supported by a SPOT. Because SPOTs may\n\
support more than one interface type, a SPOT may have more than one of these\n\
attributes.  Note that this is one piece of information that is used to\n\
determine whether a SPOT can be allocated to a machine: the SPOT must\n\
support the client's primary interface type for the client to\n\
use it.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ALLOC_COUNT "\
NIM manages this attribute, which keeps track of the\n\
number of times a NIM resource is currently allocated to a NIM client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SOURCE "\
This attribute is only used to pass information to NIM operations and\n\
is therefore classified as a \"flag\". For those operations that\n\
accept this flag, its value indicates the NIM resource or device that\n\
will supply the images that are required by the NIM operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SPOT_OPTIONS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SIMAGES_OPTIONS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_EXPORTED "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FIRST_DIR "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NSTATE "\
NIM manages this attribute, which indicates whether the\n\
object definition of the network is complete. NIM requires that\n\
all networks be able to communicate with the NIM master, either by the\n\
master being directly connected to them or by having a NIM route to a\n\
network to which the master connects.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NET_ADDR "\
This attribute stores the network address that is used by\n\
a network.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SNM "\
This attribute stores the subnet mask that is used by a\n\
network.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ROUTING "\
This attribute stores NIM routing information for a network\n\
object. The attribute requires a sequence number when specified.\n\
When a new NIM route is specified, its value consists of three fields: \n\
   field 1 = NIM name of the destination network to which the route\n\
             is being added\n\
   field 2 = Host name of the gateway to use in order to communicate\n\
             with the destination network\n\
   field 3 = Host name of the gateway that the destination network\n\
             uses to get back to the originating network\n\
When NIM adds a \"routing\" attribute to a network's definition, NIM\n\
only includes the first two fields. NIM requires three fields for a new\n\
route because NIM automatically adds a \"routing\" attribute for the\n\
destination network; the third field is required to do this.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RING_SPEED "\
This attribute stores the ring speed that is used for a\n\
Token-Ring network interface. The attribute requires a sequence number\n\
when specified. A sequence number is used to match this information\n\
with a corresponding \"if\" attribute. This attribute is not allowed\n\
without a corresponding \"if\" attribute.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PIF_NAME "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies the logical interface name of the\n\
network interface used as the primary network interface.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MASTER_IP "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MASTER_PORT "\
This attribute stores the port number used\n\
for NIM socket communication between the NIM master and NIM clients.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MASTER_UID "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ADPT_ADDR_OLD "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, it specifies a network adapter hardware address.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ADPT_NAME "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MKNFSEXP_PARAMS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_WARNING "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BUNDLE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_HOSTNAME "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IP "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"ip\" in the\n\
target's /etc/bootptab stanza specifies the IP address that the target\n\
should use when configuring its network interface.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_HA "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"ha\" in the\n\
target's /etc/bootptab stanza specifies the target's network adapter\n\
hardware address.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_HT "\
NIM manages this attribute, which stores information\n\
that is used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"ht\" in the\n\
target's /etc/bootptab stanza specifies the type of network interface\n\
the target will use to boot from.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BF "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"bf\" in the\n\
target's /etc/bootptab stanza specifies the path name of the target's\n\
network boot image.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_GW "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"gw\" in the\n\
target's /etc/bootptab stanza specifies the IP address of the gateway\n\
that the target should use in order to tftp its network boot image.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SM "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"sm\" in the\n\
target's /etc/bootptab stanza specifies the subnet mask that should\n\
be used by the target when configuring its network interface.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SA "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client. Specifically, the string \"sa\" in the\n\
target's /etc/bootptab stanza indicates the server IP address that\n\
should be used when responding to the target's BOOTP request.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_COMMAND "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PERMS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BOOT_INFO "\
NIM manages this attribute, which stores information\n\
used to enable the NIM environment to support a network boot\n\
for a NIM client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_PERMS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_ROOT "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_ACCESS "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_RO "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_RW "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_SECURE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_GRANT "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_REVOKE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DEVICE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CDROM "\
NIM manages this attribute, which indicates when a NIM\n\
resource resides on CD-ROM.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_TAPE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NO_NIM_CLIENT "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence indicates that the bos.sysmgt.nim.client\n\
fileset, which normally would be installed and configured as part of\n\
the operation, should not be installed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PULL "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PULL_REQUEST "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DISP_MNTPNT "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence indicates that the access point on the\n\
server should be displayed for each resource being allocated by the\n\
operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MOUNT_CTRL "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies the directory on the target over\n\
which the NIM resource being allocated should be mounted.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ASYNC "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IGNORE_STATE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INSTALLP_FLAGS "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies the flags that should be used\n\
when NIM invokes the installp command to perform an operation on\n\
optional software products.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INSTALLP_OPTIONS "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies names of optional software\n\
filesets that should be used in the operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_LSLPP_FLAGS "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies the flags that are to be passed\n\
to the lslpp command, which NIM invokes to display information about\n\
optional software products.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FORCE "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence is used to indicate that the normal\n\
state checking procedures that NIM uses to synchronize NIM operations\n\
should be bypassed. In some cases, NIM is able to bypass the\n\
checking and perform the requested operation; in other cases, NIM \n\
does not allow the checking to be bypassed.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_VERBOSE "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, it is used to specify the level of verbose output\n\
NIM operations should display. Verbose output contains more information\n\
than is normally displayed by NIM operations, which can help diagnose\n\
problems encountered during a NIM operation. The value of this attribute\n\
is a positive integer between 1 and 5. The larger the value, the greater\n\
the amount of output.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ALL "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_OPTIONS "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, it specifies the names of optional software products.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DEBUG "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, it indicates that the kernel debugger should be\n\
enabled when building network boot images. Having the kernel debugger\n\
enabled aids the user in diagnosing network boot failures. Refer to\n\
the troubleshooting section of the Network Installation Management\n\
Guide and Reference for more information on diagnosing network boot\n\
failures.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_BOOT_ENV "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DIR_CREATED "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PRESERVE_RES "\
This attribute is only used to pass information to the bos_inst\n\
operation and is, therefore, classified as a \"flag\". The bos_inst\n\
operation performs a base system installation on the specified machine,\n\
which can destroy the target's current file systems.\n\
Therefore, if the target is serving a NIM resources, the bos_inst\n\
operation could actually remove it. NIM assumes this condition by\n\
default and removes the resource objects corresponding\n\
to those resources when the target gets installed. You can override\n\
this behavior by providing this attribute when you perform the bos_inst\n\
operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IGNORE_LOCK "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ENTER_DBG_UP "\
This attribute is used store the address of the \"enter_dbg\" symbol\n\
in the UP version of the kernel. This information is useful when\n\
debugging network boot failures. Refer to the troubleshooting section\n\
of the Network Installation Management Guide and Reference for\n\
further details on how to debug network boot failures.\n\
This attribute becomes part of a SPOT's definition when you specify\n\
the \"debug\" attribute when performing an operation on the SPOT. The\n\
\"debug\" attribute tells NIM to enable the kernel debugger when\n\
rebuilding the network boot images associated with the SPOT. As part\n\
of that processing, NIM determines what the address of the \"enter_dbg\"\n\
symbol is and adds this attribute to the SPOT's definition so that the\n\
user has easy access to it. This attribute is subsequently removed\n\
anytime a SPOT operation is performed without the \"debug\" attribute.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ENTER_DBG_MP "\
This attribute is used store the address of the \"enter_dbg\" symbol\n\
in the MP version of the kernel. This information is useful when\n\
debugging network boot failures. Refer to the troubleshooting section\n\
of the Network Installation Management Guide and Reference for\n\
details on how to debug network boot failures.\n\
This attribute becomes part of a SPOT's definition when you specify\n\
the \"debug\" attribute when performing an operation on the SPOT.  The\n\
\"debug\" attribute tells NIM to enable the kernel debugger when\n\
rebuilding the network boot images associated with the SPOT. As part\n\
of that processing, NIM determines what the address of the \"enter_dbg\"\n\
symbol is and adds this attribute to the SPOT's definition so that the\n\
user has easy access to it. This attribute is subsequently removed\n\
anytime a SPOT operation is performed without the \"debug\" attribute.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MODE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_VFSTYPE "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NUM_PARALLEL_SYNCS "\
This attribute is only used to pass information to NIM operations and\n\
is therefore classified as a \"flag\". For those operations that\n\
accept this flag, its value specifies the number of diskless and\n\
dataless root directories that should be \"sync\"d in parallel.\n\
Diskless and dataless machine configurations use a remote directory (a\n\
SPOT) as their /usr file system. Whenever an installp operation is\n\
performed on the SPOT, the corresponding operation must be performed on\n\
their root directories in order to keep the root and usr parts of their\n\
software products in synchronization. That operation is referred to as\n\
a \"root sync\" and NIM automatically performs it whenever the SPOT\n\
is operated on. By default, NIM performs five root sync operations\n\
at the same time. You can change this behavior by using this attribute\n\
to specify the number of root syncs you want to be performed in parallel.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NO_MKBOOTI "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ST_COMMITTED "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ST_APPLIED "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IPLROM_EMU_OLD "\
This attribute is used to store the name of device that contains the\n\
IPL ROM emulation for machines requiring the IPL ROM emulation in\n\
to perform a network boot.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ROOT_INITIALIZED "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CABLE_TYPE "\
This attribute stores the type of cable used for an Ethernet\n\
network interface. The attribute requires a sequence number\n\
when it is specified. A sequence number is used to match\n\
this information with a corresponding \"if\" attribute: this attribute\n\
is not allowed without a corresponding \"if\" attribute.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DESTROY "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NFS_ANON "\
NIM internal use only\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_CLASS_ATTRS "\
Attributes represent pieces of information that NIM requires.\n\
This information is passed to NIM commands using command line syntax\n\
in the form of:\n\
   -a <attribute name>=<value>\n\
The attribute name that you supply must be a valid NIM attribute, and\n\
it must be one of a list of specific attributes accepted by the NIM\n\
operation you are currently performing.\n\
Use lsnim to display the attributes which any NIM operation accepts: \n\
   # lsnim -q <NIM operation> -t <NIM object type>\n\
NIM classifies attributes into two subclasses:\n\
   -> \"info_attrs\" are those attributes that provide information\n\
      about a specific characteristic of a NIM object and, hence,\n\
      become part of the object's definition.\n\
   -> \"flag_attrs\" are only used to pass information to NIM operations\n\
      and do not become part of an object's definition.\n\
Use the lsnim command to display the list of NIM attributes\n\
that belong to each of these subclasses: \n\
   # lsnim -p -s info_attrs\n\
   # lsnim -p -s flag_attrs\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_SEQNO "\
This attribute classifies the NIM attributes that require a\n\
sequence number. NIM requires that a sequence number be appended to\n\
the attributes that can added to an object's definition more than\n\
once. For example, many \"if\" attributes can be added to a machine\n\
object's definition and, to distinguish between them, NIM\n\
requires you to append a sequence number to each one.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_DKLS_RES "\
This attribute classifies NIM resources that may be allocated\n\
to machines of the diskless configuration type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_DTLS_RES "\
This attribute classifies NIM resources that may be allocated\n\
to machines of the dataless configuration type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_PULL_OPS "\
This attribute classifies the NIM operations that may be\n\
initiated from NIM clients.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_AUTO_EXPAND "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence indicates that file systems\n\
should be automatically expanded if space is needed for a NIM operation.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_STD_RES "\
This attribute classifies NIM resources that may be allocated\n\
to machines of the standalone configuration type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_MSTR_RES "\
This attribute classifies NIM resources that may be allocated\n\
to machines of the master configuration type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_UNCONFIG "\
This NIM operation unconfigures the NIM master fileset. Use this\n\
operation before you deinstall the NIM master fileset\n\
with installp.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PLATFORM_OLD "\
This attribute is used to specify the type of platform a machine uses.\n\
By default, a machine belongs to the rs6k family......\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_PLATFORM "\
This attribute is used to classify NIM machines by their hardware\n\
platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RS6K "\
This attribute represents the RISC System 6000 machine platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RS6KSMP "\
This attribute represents the RISC System 6000 Symmetric Multi-\n\
processor machine platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RSPC_OLD "\
This attribute represents the RISC Personal Computer System machine\n\
platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ENTER_DBG "\
This attribute is used to store the address of the enter_dbg symbol\n\
in the kernel used to create the network boot image associated with\n\
the specified platform type.  This attribute is added to a SPOTs\n\
definition when you use NIM to create network boot images which have\n\
the kernel debugger enabled, which helps you to diagnose network boot\n\
failures.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_RS6K "\
This attribute is used to classify NIM configuration types which are\n\
supported for the rs6k platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_RS6KSMP "\
This attribute is used to classify NIM configuration types which are\n\
supported for the rs6ksmp platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SUBCLASS_RSPC "\
This attribute is used to classify NIM configuration types which are\n\
supported for the rspc platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_DEFAULT "\
This attribute is used to signify that a resource has been designated\n\
as a default resource and can be automatically allocated by NIM when\n\
appropriate.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_OTHER_NET_TYPE "\
Each NIM network object is used to represent one logical network which\n\
exists in the NIM environment.  When the network is defined, the type\n\
of network interface used in the network must be supplied.  Usually,\n\
a network will be composed of only one type.  However, a bridge can be\n\
used to connect network types together to form one logical network.\n\
In that situation, NIM needs to know that the other network interface\n\
types are and this attribute is used to specify that information.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_MK_NETBOOT "\
This attribute is managed by NIM.  The attribute indicates a change \n\
occured to the SPOT.  NIM uses this attribute to know when to rebuild\n\
the network boot images.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RSPC_OLD1 "\
This attribute represents the IBM Personal Computer Power Series\n\
platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_IPLROM_EMU "\
This attribute is used to store the name of the device that contains\n\
the IPL ROM emulation software.  IPL ROM emulation is required for\n\
machines which do not have BOOTP-enabled IPL ROM.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PLATFORM_OLD1 "\
This attribute is used to specify the type of platform a machine uses.\n\
By default, a machine belongs to the rs6k (RISC System 6000) family.\n\
Other supported families include rspc (IBM Personal Computer Power\n\
Series) and rs6ksmp (RISC System 6000 Symmetric Multiprocessor).\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_ADPT_ADDR "\
This attribute is used to pass information to NIM about the hardware\n\
address.  It is classified as a \"flag\".  It is used both internally\n\
and via the command line when the niminit command is unable to\n\
automatically ascertain the hardware address of the primary network\n\
interface.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SYNC_ROOTS "\
This NIM operation synchronizes root directories for all diskless and\
dataless clients which use the specified SPOT.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_RSPC "\
This attribute represents the PowerPC Personal Computer \n\
platform type.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_PLATFORM "\
This attribute is used to specify the type of platform a machine uses.\n\
By default, a machine belongs to the rs6k (RISC System 6000) family.\n\
Other supported families include rspc (PowerPC Personal Computer) and \n\
rs6ksmp (RISC System 6000 Symmetric Multiprocessor).\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FIXES "\
This attribute is used in combination with the cust and fix_query\n\
operations.  It provides a NIM interface to the instfix command\n\
which allows software to be updated and viewed via fix keywords. A fix\n\
can be made up of either a single fileset update or multiple fileset\n\
updates that are related in some way; fixes are identified by unique\n\
keyword. When a fix is identified with an APAR number, it will include\n\
all the fileset updates that are necessary to fix the reported software\n\
problem that is identified by that number.\n\n\
You may update all the filesets you currently have installed, with\n\
applicable software from the source being used for the cust operation,\n\
by specifying a value of \"update_all\" for the fixes attribute:\n\
ie. \"-afixes=update_all\". Note: when doing an \"update_all\" it is\n\
advisable to use options to the installp command which cause updates\n\
to be committed without saving temporarily replaced files. This is the\n\
-cN flag combination or the following settings from the SMIT interface:\n\
\"COMMIT software updates?\"=yes, \"SAVE replaced files?\"=no. In doing\n\
so, you will not be able to revert to the previous level of files being\n\
updated but you will use less space.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FIX_BUNDLE "\
This attribute is a NIM resource type. When a resource object of\n\
this type is defined, it represents the location of a file that\n\
contains fix keywords to be used by the instfix command which is called\n\
by the cust and fix_query operations. Once defined, this resource\n\
can be used by first allocating it to the target, then performing the NIM\n\
operation on the target. (Note that for SPOT targets you use this\n\
resource by specifying fix_bundle as an attribute, ie. you do not\n\
allocate the resource first.) When this is done, NIM uses this\n\
file when it executes the instfix command to: \n\
   1) NFS mount this resource so that the client has local access\n\
      to it\n\
   2) Pass the local access point to instfix by using the\n\
      \"-f\" instfix flag\n\
   3) Unmount this resource when instfix finishes\n\
See the \"fixes\" attribute for more details on fix keywords.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FIXQUERY "\
This NIM operation displays whether specified fix keywords are\n\
installed on a client or SPOT. The \"fixes\" or \"fix_bundle\"\n\
attributes are used to specify fix keywords to the operation. If \n\
neither \"fixes\" nor \"fix_bundle\" are specified, all known fixes\n\
are displayed. This operation provides a NIM interface to a subset\n\
of the query capabilities of the instfix command. See the \"fixes\" \n\
attribute for more details on fix keywords.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FIXQUERY_FLAGS "\
This attribute is used to specify optional flags to the fix_query\n\
operation.  Valid flags are:\n\
   -a Displays symptom text.\n\
   -c Displays output in colon separated format.\n\
   -F Returns failure unless all filesets associated with a fix are\n\
      installed.\n\
   -q Quiet option. If -c is specified, no heading is displayed.\n\
   -v Verbose option. Gives information about each fileset associated\n\
      with a fix (keyword).\n"
$
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_FORCE_PUSH "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence indicates that the client should be modified\n\
to accept commands from the NIM server, even if the client is not currently\n\
configured as a NIM client.\n"
$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_NO_CLIENT_BOOT "\
This attribute is only used to pass information to NIM operations and\n\
is, therefore, classified as a \"flag\". For those operations that\n\
accept this flag, its presence indicates that the client should not be\n\
shutdown and rebooted as part of this operation.  It is reasonable to\n\
set this flag if the server is to perform operations to enable a client\n\
operation which is to be initiated at a later time.\n"

$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_REBOOT "\
This NIM operation will cause the targeted machines to be rebooted.\n"

$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_SET_BOOTLIST "\
This attribute is used to tell the bos_inst operation that an attempt\n\
should be made to order the list of boot devices on a target machine\n\
so that the primary network interface, as defined to NIM, is first.\n\
This attribute is only valid if used in combination with the \n\
no_client_boot attribute and is only meaningful if set to \"yes\" when\n\
no_client_boot is set to \"yes\".  In all other cases, the attribute\n\
is ignored.\n"

$
$ --------------------------------------------------------------------|
$
VERBOSE_ATTR_INST_WARNING "\
This attribute is used in combination with the reboot option to display\n\
a message to all users on the target system indicating that re-installation\n\
will occur after reboot.  This attribute would typically be used with\n\
the reboot operation after a bos_inst operation had previously been\n\
issued with the no_client_boot attribute set to \"yes.\"\n"

$
$ ------------------------------------ end of file
