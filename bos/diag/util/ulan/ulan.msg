$ @(#)54	1.5.1.1  src/bos/diag/util/ulan/ulan.msg, dsalan, bos41J, 9523B_all 6/6/95 16:17:06
$
$   COMPONENT_NAME: DSALAN
$
$   FUNCTIONS: 
$		
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1992,1995
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 1
$quote "

$
$ Note: Do not translate %1$s, %2$s
$
INPUT_INTERNET_ADDRESSES "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s     802176\n\n\
Type or select values in entry fields.\n\
Use cursor key to move between fields.\n\
Press Enter AFTER making all desired changes."

$
$ Note: Do not translate %1$s, %2$s
$
RING_SELECTION "\
TOKEN-RING SERVICE AID %1$s IN LOCATION %2$s\n"
$
$ Note: Do not translate %1$s, %2$s
$
LAN_SELECTION "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s"


$
LAN_SELECTION_NO_DEVICE "\
LOCAL AREA NETWORK (LAN) SERVICE AID\n"

$
WARNING "\
* * * WARNING! * * *\n\n\
ENSURE that the correct ring speed is chosen.\n\
If the wrong ring speed is chosen, the network will be\n\
slowed down and unexpected results will occur.\n\n\
Move cursor to selection, then press Enter."

$
FOUR_MEG_RING "4  megabit data rate ring"
$
SIXTEEN_MEG_RING "16 megabit data rate ring"
$
UNKNOWN "Data rate unknown"

$
$ Note: Do not translate %1$s, %2$s
$
STANDBY "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
This test may require up to ten minutes.\n\n\
Please stand by." 

$
$ Note: Do not translate %1$s, %2$s
$
RING_DIAGNOSTICS "\
MONITOR RING FROM %1$s IN LOCATION %2$s\n\n\
This test may require up to ten minutes.\n\n\
Please stand by." 

$
SOURCE_IP "\
Source      Internet Address [XXX.XXX.XXX.XXX] "

$
SOURCE_IP_ERROR "\
Source Internet Address must be in the format [XXX.XXX.XXX.XXX] \n\
Example:  129.35.129.1 "

$
DESTINATION_IP_ERROR "\
Destination Internet Address must have the format [XXX.XXX.XXX.XXX] \n\
Example:  129.35.129.211 "

$
GATEWAY_IP_ERROR "\
Gateway Internet Address must have the format [XXX.XXX.XXX.XXX] \n\
Example:  129.35.129.211 \n\
or If there is no Gateway just leave in blank "

$
DESTINATION_IP "\
Destination Internet Address [XXX.XXX.XXX.XXX] "

$
GATEWAY_IP "\
Gateway     Internet Address [XXX.XXX.XXX.XXX]\n\
(NOTE: Gateway is optional. If the destination\n\
       station is reached via gateway, the gateway\n\
       internet address must be provided)"

$
SA_ENTER "To continue, press Enter."

$
$ Note: Do not translate %1$s, %2$s
$
GOOD_TRANSMISSION "\
Data has been successfully transmitted between\n\n\
        - Source      Internet Address %1$s\n\
                           and\n\
        - Destination Internet Address %2$s"


$
$ Note: Do not translate %1$s, %2$s
$
BAD_TRANSMISSION "\
Data has been UNSUCCESSFULLY transmitted between\n\n\
        - Source      Internet Address %1$s\n\
                           and\n\
        - Destination Internet Address %2$s"

$
NO_ADAPS "\
- The ODM indicates that there are no LAN adapters\n\
  in this system unit."


$
ACTION "\
LOCAL AREA NETWORK (LAN) SERVICE AID\n\n\
Select the adapter to be exercised.\n"

$
$ Note: Do not translate %1$s, %2$s
$

TOKENRING_ADAP "Token-Ring High-Performance adapter     %1$s   IN LOCATION %2$s"

$
$ Note: Do not translate %1$s, %2$s
$

ETHERNET_ADAP  "Ethernet  High-Performance adapter      %1$s   IN LOCATION %2$s"

$
$ Note: Do not translate %1$s, %2$s
$

FDDI_ADAP      "Fiber Distributed Data Interface (FDDI) %1$s  IN LOCATION %2$s"

$
LAN_HEADER " LOCAL AREA NETWORK (LAN) SERVICE AID\n"


$
SELECT "Move cursor to selection, then press Enter."
$
$ This instruction is displayed on menus that require the user to select
$ an option.

ADAPTER_BUSY "\
ADDITIONAL RESOURCE IS REQUIRED FOR TESTING\n\n\
The Service Aid was not executed because the system\n\
indicated that the resource was in use."

$
FREE_RESOURCE "\n\
To run LOCAL AREA NETWORK (LAN) SERVICE AID, you must\n\
do one of the following:\n\n\
   - Free this resource and retry the Service Aid.\n\
   - Shutdown the system and run in maintenance mode.\n\
   - Run Service Aid from the Diagnostics Diskettes package."


$
NO_ADAPTER_TO_TEST "\
LOCAL AREA NETWORK (LAN) SERVICE AID\n\n\
The system configuration indicates that there is no\n\
LAN adapter to be exercised with this Service Aid.\n\n\
None of these following adapters were found in the system:\n\n\
    - Token-Ring High Performance Adapter.\n\
    - Ethernet  High Performance Adapter.\n\
    - FDDI (Fiber Distributed Data Interface) Adapter."    

$
$ Note: Do not translate %1$s, %2$s
$

NO_NETWORK_ADDRESS_W_GW "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
The network indicates that the network address of the \n\
the destination station cannot be found.\n\n\
ENSURE:\n\n\
   - The network cable is properly connected.\n\
   - The Internet Address of the destination\n\
     station is correct.\n\
   - TCP/IP is up and running on the destination station.\n\
   - The Internet Address of the gateway\n\
     station is correct.\n\
   - TCP/IP is up and running on the gateway station.\n\
   - Advanced Diagnostics has been run successfully for\n\
     this resource."

$
$ Note: Do not translate %1$s, %2$s
$
NO_NETWORK_ADDRESS  "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
The network indicates that the network address of the \n\
the destination station cannot be found.\n\n\
ENSURE:\n\n\
   - The network cable is properly connected.\n\
   - The Internet Address of the destination\n\
     station is correct.\n\
   - TCP/IP is up and running on the destination station.\n\
   - Advanced Diagnostics has been run successfully for\n\
     this resource."

$
$ Note: Do not translate %1$s, %2$s
$
DUPLICATE_NET_ID  "\
DUPLICATE NETWORK ID  %1$s  at location %2$s\n\n\
The system indicates that the network id is duplicated.\n\
Recommended Actions:\n\n\
   - Check with Local Area Network administrator.\n\
   - Replace the resource."
$
$ Note: Do not translate %1$s, %2$s
$

UNIDENTIFIED_ERROR "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
The Service Aid detected an error.\n\
ENSURE:\n\n\
   - Advanced Diagnostics has been run successfully for this resource.\n\
   - The network cable is operational." 
$
$ Note: Do not translate %1$s, %2$s
$
RING_DIAG_ERROR "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
The Service Aid detected an error."

$
$ Note: Do not translate %1$s, %2$s
$

HARDWARE_ERROR "\
HARWARE ERROR  %1$s  at location %2$s\n\n\
The system indicates that there is a hardware error.\n\
Recommended Action:\n\n\
        - Running Advanced Diagnostics for the resource."

$
$ Note: Do not translate %1$s, %2$s, %3$02d:%4$02d
$
RING_NORMAL_AND_TESTING "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
Ring State : NORMAL\n\
Time remaining: %3$02d:%4$02d\n\
The Service Aid is in progress.\n\n\
Please stand by." 

$
$ Note: Do not translate %1$s, %2$s
$
RING_ABNORMAL "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
Ring State : ABNORMAL\n\
The Service Aid is in progress.\n\n\
Please stand by." 


$
$ Note: Do not translate %1$s, %2$s, %3$012X, %4$s
$
RING_BEACON_WITH_NAUN "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
Ring State : BEACONING\n\
Nearest Ative Upstream Neighbour (NAUN): %3$012X\n\
Internet Address of NAUN:                %4$s"

$
$ Note: Do not translate %1$s, %2$s, %3$02X%4$02X%5$02X%6$02X%7$02X%8$02X
$       and  %9$02X%10$02X%11$02X%12$02X%13$02X%14$02X"
$
RING_BEACON_NO_NAUN "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s\n\n\
Ring State : BEACONING\n\
Nearest Ative Upstream Neighbour (NAUN) Network Address: \
%3$02X%4$02X%5$02X%6$02X%7$02X%8$02X\n\
Beaconing Adapter's Network Address: \
%9$02X%10$02X%11$02X%12$02X%13$02X%14$02X"

$
PRETEST_INFO "\
ENSURE the following items are available before running\n\
this Service Aid:\n\n\
   - The Internet Addresses of the source and\n\
     destination stations. The network system administrator\n\
     should be able to provide these addresses.\n\
   - TCP/IP is up and running on the destination station.\n\
   - The network cable is properly connected.\n\
   - Advanced Diagnostics has been run successfully for the\n\
     resource to be exercised."



$
OPTION_PRETEST "\
This service aid is used to exercise the LAN communication adapters -\n\
Token-Ring, Ethernet and Fiber Distributed Data Interface (FDDI).\n\
The following services are available:\n\n\
   - Connectivity testing between two network stations. Data is\n\
     transferred between these two stations. This requires the\n\
     user to input the Internet Addresses of both stations.\n\n\
   - Monitoring ring (Token-Ring only). The ring is monitored for\n\
     a period of time. Soft and hard errors are analyzed."


$
CONNECTIVITY "\
   - Connectivity testing between two stations."

$
MONITORING "\
   - Monitoring ring."


$
$ Note: Do not translate %1$s, %2$s
$

SELECTION_WITH_OPTION "\
LOCAL AREA NETWORK (LAN)SERVICE AID %1$s IN LOCATION %2$s\n\n\
Select one of the following options."


$
OPEN_ERROR "\
The Token-Ring adapter returns the following open error:"

$
FUNCTION_FAILURE "FUNTION FAILURE."

$
ENSURE_1 "\
ENSURE:\n\n\
   - Advanced Diagnostics has been run successfully for the \n\
     resource to be tested.\n\
   - The Multistation Access Unit (MAU / MSAU) is operational.\n\
   - The cable from the Token-Ring adapter to the MAU\n\
     is operational."

$
SIGNAL_LOSS "SIGNAL LOSS."

$
ACTION_1 "\n\
RECOMMENDED ACTION:\n\n\
   - UNPLUG the Token-Ring cable from the ring. Use another\n\
     station on the ring to determine if the ring is operational."


$
WIRE_FAULT "WIRE FAULT."


$
FREQUENCY_ERROR "FREQUENCY ERROR."

$
TIME_OUT_ERROR "TIME OUT."


$
YES_OPTION "YES"

$
NO_OPTION  "NO"

$
$ Note: Do not translate %1$s, %2$s
$
WARNING_1 "* * * WARNING * * *\n\n\
The chosen ring speed is %1$d megabit data rate ring.\n\
The ring speed in the database is set to %2$d megabit data ring.\n\n\
Do you want to continue testing?"



$
RING_FAILURE "RING FAILURE."

$
RING_BEACONING "RING BEACONING."


$
ENSURE_2 "\
ENSURE:\n\n\
   - Advanced Diagnostics has been run successfully for the \n\
     resource to be tested.\n\
   - The Multistation Access Unit (MAU / MSAU) is operational.\n\
   - The ring data rate matches the adapter data rate.\n\
   - The cable from the Token-Ring adapter to the MAU\n\
     is operational."

$
DUPLICATE_ADDRESS "DUPLICATE ADDRESS."


$
ACTION_2 "\n\
RECOMMENDED ACTION:\n\n\
   - Contact the local network administrator."


$
REQUEST_PARAMETERS "REQUEST PARAMETERS."


$
REMOVE_RECEIVED "REMOVE RECEIVED."


$
IMPL_FORCE_RECEIVED "IMPL FORCE RECEIVED."


$
DUPLICATE_MODIFIER_ADDRESS "DUPLICATE MODIFIER ADDRESS."


$
NETWORK_DOWN "\
The Service Aid has detected that the network is down.\n\n\
ENSURE:\n\n\
   - The network cables are operational.\n\
   - The correct data rate ring is chosen."


$
ENSURE_3 "\n\
ENSURE:\n\n\
   - The network cable is properly connected.\n\
   - The network is operational.\n\
   - The Internet Address of the destination\n\
     station is correct.\n\
   - TCP/IP is up and running on the destination station.\n\
   - Advanced Diagnostics has been run successfully for\n\
     this resource."

$
$ Note: Do not translate %1$d, %2$d,  %3$d, %4$d,  %5$d, %6$d,  %7$d,    
$ 	%8$d, %9$d,  %10$d
$
STATISTICS "\
Errors Report:\n\n\
   - Lost Frame Error   %1$d frame(s).\n\
   - Receive congestion %2$d frame(s).\n\
   - Frame copied error %3$d frame(s).\n\
   - Frequency Error    %4$d frame(s).\n\
   - Token Error        %5$d frame(s).\n\
   - Line Errors        %6$d frame(s).\n\
   - Internal Errors    %7$d frame(s).\n\
   - Burst Errors       %8$d frame(s).\n\
   - A/C errors         %9$d frame(s).\n\
   - Abort Del Trans    %10$d frame(s)."
$

DONE_RING_MONITOR "\
LOCAL AREA NETWORK (LAN)SERVICE AID %1$s IN LOCATION %2$s\n\n\
   - Monitoring ring  has completed."

LPP_DEVICE_CANNOT_UNCONFIGURED "\
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be unconfigured."

LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE "\
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set to DIAGNOSE\n\
state."

LPP_DEVICE_CANNOT_SET_TO_DEFINE "\
THE LICENSED PRODUCT PROGRAM (LPP) DEVICE CANNOT BE SET TO DIAGNOSE STATE\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be set from DIAGNOSE\n\
state to DEFINE state."

DIAG_DEVICE_CANNOT_CONFIGURED "\
THE DIAGNOSTICS DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be configured."

DEVICE_DOES_NOT_EXIST "\
THE SYSTEM INDICATES THAT THE DEVICE DOES NOT EXIST. \n\
No trouble was found.  However, the because the system indicated that \n\
the resource does not exist."

DIAG_DEVICE_CANNOT_UNCONFIGURED "\
THE DIAGNOSTICS DEVICE CANNOT BE UNCONFIGURED\n\
No trouble was found.  However, the diagnostics device may not be operational\n\
because the system indicated that the resource could not be unconfigured."

LPP_DEVICE_CANNOT_CONFIGURED "\
THE LPP(LICENSED PRODUCT PROGRAM) DEVICE CANNOT BE CONFIGURED\n\
No trouble was found.  However, the LPP device may not be operational\n\
because the system indicated that the resource could not be configured."

NEW_FREE_RESOURCE "\n\
To run LOCAL AREA NETWORK (LAN) SERVICE AID, you must\n\
do one of the following:\n\n\
   - Free this resource and retry the Service Aid.\n\
   - Shutdown the system and run in maintenance mode.\n\
   - Run Service Aid from the Standalone Diagnostics package."

RESOURCE_BUSY "\n\
The LOCAL AREA NETWORK (LAN) SERVICE\n\
AID was not executed because the system\n\
indicated that the resource was in use.\n\
To run this service aid, you must\n\
do one of the following:\n\n\
   - Free the resource and retry the Service Aid.\n\
   - Shutdown the system and run in maintenance\n\
     mode.\n\
   - Run Service Aid from the Standalone\n\
     Diagnostics package."

INPUT_INTERNET_TITLE "\
LOCAL AREA NETWORK (LAN) SERVICE AID %1$s IN LOCATION %2$s"
$
$ Note: Do not translate %1$s, %2$s
$

INPUT_INTERNET_PROMPT "\
Type values in entry fields.\n\
Use cursor key to move between fields.\n\
Press COMMIT after making all desired changes.\n\n"

$
$
