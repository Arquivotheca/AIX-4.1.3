#@(#)52	1.3  src/bos/usr/lib/nim/methods/master.example.S, cmdnim, bos411, 9428A410j  5/23/94  08:40:12 
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS:
#		
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.



                             Setting Up a NIM Environment:
                                  A Detailed Example
                            -------------------------------


Overview
--------
This file contains a detailed example of how to setup a NIM environment.  Before
examining this example, you should read the Network Installation Management 
Guide and have a basic understanding of NIM concepts.  Note that only the NIM
command line interface will be documented here: the SMIT interface for NIM
will not be shown.

In this example, "$" will represent the command line prompt.

Prerequisites
-------------
For the purpose of this example, we will assume that the physical network in
which NIM will be setup is configured correctly; this includes having routing 
and name resolution configured properly.  We will also assume that at least
one machine has already been installed with AIX version 4.1 or later and that
it is currently running.

Step #1: draw a picture
-----------------------
We start by drawing a picture which represents the physical network topology.
For this example, we have an environment which contains 3 LANs:
	-> net1 = a 16 Mbit token-ring which connects 6 machines together
	-> net2 = an ethernet which connects 5 machines together
	-> net3 = a FDDI network which connects 3 machine together
Two the machines in this environment are configured as gateways:
	-> the machine between net1 and net2
	-> the machine between net2 and net3
We will assume that gateways have already been installed with AIX version 4.1 
or later, are currently running, and that both their network interfaces are 
configured correctly.

The following represents how this network topology might be drawn:

                                                                                
        +-------+--- < net1 > --+---------------+-------+-------+         
        |       |               |               |       |       |         
        |       |               |               |       |       |         
      |-+-|   |-+-|             |             |-+-|   |-+-|   |-+-|       
      |   |   |   |             |             |   |   |   |   |   |       
      |   |   |   |             |             |   |   |   |   |   |       
      |   |   |   |      |------+-------|     |   |   |   |   |   |       
      |   |   |   |      |              |     |   |   |   |   |   |        
      |---|   |---|      |              |     |---|   |---|   |---|        
                         |              |                                
                         |              |                                
                         |              |                                
                         |              |                                
                         |              |                                  
                         |              |                                  
                         |------+-------|                                 
                                |                                         
                                |                    |----------|         
                             < net2 >                |          +------+  
                                |                    |----------|      |  
                                |                                      |  
                                |      |----------|                    |  
                                +------+          |                 < net3 >
                                |      |----------|                    |  
              |----------|      |                                      |  
              |          +------+                                      |   
              |----------|      |                                      |  
                                |           |--------------|           |  
                                +-----------+              +-----------+      
                                |           |--------------|           |
                                |                                      |  
              |----------|      |                                      |  
              |          +------+                                      |  
              |----------|      |                    |----------|      |  
                                |                    |          +------+        
                                |                    |----------|      |        


Step #2: choose a NIM master
----------------------------
Since only one machine in the NIM environment can be a NIM "master", the
machine which you want to be the master should be chosen carefully.  Some
things to consider when choosing a master are:
	-> it must already be installed with version 4.1 or later
	-> all machines which will participate in this NIM environment must be able
		to communicate with the master
	-> the master should reside at a conveniant, physical location
	-> if the master is going to also serve NIM resources, then there should be
		enough free disk space to store those resources

For this example, we choose a machine which is already configured as a gateway
between the net1 and net2 networks.

Step #3: install and configure the bos.sysmgt.nim.master fileset
----------------------------------------------------------------
(Note: all commands executed for step #3 will be executed on the gateway
between net1 and net2)
We install the NIM master fileset using the installp command with the source 
device being a local tape drive (this example assumes that an available tape 
drive is connected and contains the appropriate install media):

   $ installp -ag -d/dev/rmt0.1 bos.sysmgt.nim.master

Once the bos.sysmgt.nim.master package is installed, we will use the nimconfig
command to configure the master package.  In order to determine what parameters
the nimconfig will need, we execute it with the "-q" option and it displays
output similar to this:

   $ nimconfig -q

     usage: nimconfig [-q] -a <attr=value> [ -a <attr=value>]... 
 
     the following attributes are required:
             -a pif_name=<value>
             -a master_port=<value>
             -a netname=<value>

     the following attributes are optional:
             -a ring_speed=<value>
             -a cable_type=<value>
             -a force=<value>
             -a verbose=<value>

The "pif_name" attribute represents the primary network interface which will
be associated with the NIM master.  Since this machine has two interfaces,
either one may be chosen.  To determine what the choices are, we execute the
lsdev command:

   $ lsdev -C -c if -S available

		en0 Available  Standard Ethernet Network Interface
		tr0 Available  Token Ring Network Interface

"tr0" represents the connection to net1; "en0" represents the connection to
net2.  For this example, we will use "tr0", however, "en0" could have been used
also.

The "master_port" attribute represents the port number which will be used for
socket communications between the NIM master and its clients.  Since NIM needs
a dedicated port, we examine the /etc/services file and determine that port
number 1058 is unused.

The "netname" attribute represents the name of the NIM network object which will
be created by the nimconfig command to represent the network which the master's
primary interface connects to.  For this example, we will name it "net1".

Since the tr0 interface on this machine is a token ring interface, we must also
supply the "ring_speed" attribute to nimconfig.  We now execute the nimconfig
command:

   $ nimconfig -a pif_name=tr0 -a master_port=1058 -a netname=net1 \
        -a ring_speed=16

Some of the proceesing which nimconfig performs to configure the NIM master 
fileset includes:
   1) creating NIM objects to represent the master, its primary network, the
      network boot resource, and the NIM script resource
   2) starting the nimesis daemon, which is used for communications between the
      NIM master and its clients
   3) initializing the /etc/niminfo file
When this command returns successfully, the master fileset has been configured
and, from the perspective of the NIM master, the network topology will look
like this:
                                                                                
        +-------+--- < net1 > --+---------------+-------+-------+         
        .       .               |               .       .       .         
        .       .               |               .       .       .         
      ..+..   ..+..             |             ..+..   ..+..   ..+..       
      .   .   .   .             |             .   .   .   .   .   .       
      .   .   .   .             |             .   .   .   .   .   .       
      .   .   .   .      |------+-------|     .   .   .   .   .   .       
      .   .   .   .      |              |     .   .   .   .   .   .        
      .....   .....      |   master     |     .....   .....   .....        
                         |              |                                
                         | [boot]       |                                
                         | [nim_script] |                                
                         |              |                                
                         |              |                                  
                         |              |                                  
                         |------+-------|                                 
                                .                                         
                                .                    ............         
                             . net2 .                .          +......+  
                                .                    ............      .  
                                .                                      .  
                                .      ............                    .  
                                +......+          .                 . net3 .
                                .      ............                    .  
              ............      .                                      .  
              .          +......+                                      .   
              ............      .                                      .  
                                .           ................           .  
                                +...........+              +...........+      
                                .           ................           .
                                .                                      .  
              ............      .                                      .  
              .          +......+                                      .  
              ............      .                    ............      .  
                                .                    .          +......+        
                                .                    ............      .        


The objects outlined in "." indicate that they are physically present in the
environment, but are unknown to NIM at this time.  If an object is unknown
to NIM, it cannot actively participate in NIM operations.  Note that objects
may actually participate "passively" in the NIM environment, as when a machine 
has been configured as a gateway.  However, since NIM does not perform network
management, this kind of passive participation is not managed by NIM and 
therefore, does not require a NIM object to represent it.

Once the nimconfig command has been executed successfully, the lsnim command
can be used to display information about the NIM environment.  For example,
we can display the objects which NIM currently knows about:

   $ lsnim

   master         machines     master
   net1           networks     tok
   boot           resources    boot
   nim_script     resources    nim_script

We can use lsnim to display detailed information about any or all objects.  If
we display detailed information about the master, we will see something similar
to this:

   $ lsnim -l master

   master:
      class       =  machines
      type        =  master
      Cstate      =  ready for a NIM operation
      Mstate      =  currently running
      if1         =  net1 Mtokenring 10005AC9430C
      ring_speed1 =  16
      serves      =  boot
      serves      =  nim_script
      reserved    =  yes
      comments    =  machine which controls the NIM environment

Note the "if1" attribute: this defines the machine's primary network interface.
In this case, it tells us that the master's tr0 interface is connected to the
"net1" network object, that the hostname associated with this interface is
"Mtokenring", and that the hardware address of the network adapter is 
"10005AC9430C".  NIM uses this information for a variety of purposes: most 
importantly for NIM connectivity.

At this point, the order of steps to complete the setup of a NIM environment is
fairly open.  The general approach is to perform the "define" operation for
each machine, resource, or network which is part of the NIM environment: this
will create a NIM object to represent that entity.  After the objects have been
"define"d, NIM can be used to perform more complex operations on them.

Since NIM requires that the user identify the machine's primary interface for
the machine "define" operation, a network object must exist before this
operation is performed.  Therefore, we will start by defining the other
networks which participate in this NIM environment.

Step #4: "define" the other networks in the NIM environment
-----------------------------------------------------------
The nim command is used to "define" the net2 network object:

   $ nim -o define -t ent -a net_addr=192.192.192.0 -a snm=255.255.255.0 net2

The "net_addr" attributes tells NIM what the network address is and the "snm"
attribute specifies the subnet mask used on that network.  After performing
the "define" operation, we can use lsnim to display information about "net2":

   $ lsnim -l net2

   net2:
      class    =  networks
      type     =  ent
      net_addr =  192.192.192.0
      snm      =  255.255.255.0
      missing  =  route to the NIM master
      Nstate   =  information is missing from this object's definition

As you can see, the definition of net2 is not complete.  NIM requires that
all networks you define be able to communicate with the NIM master by either:
   1) having an interface on the master which connects to that network
   -or-
   2) having a NIM route to a network which the master connects to
In this case, neither condition currently exists.  To complete the definition
of net2, the master's other network interface (ie, the ethernet interface) is 
now identified to NIM using the "change" operation:

   $ nim -o change -a if1='net2 Methernet 08005A190023' \
     -a cable_type1=bnc master

With this syntax, you are creating another "if" attribute which tells NIM that
the master has an ethernet interface which uses a hostname of "Methernet", that
the ethernet adapter's hardware address is "08005A190023" and that it connects 
to the "net2" network object.  Also, since net2 is an ethernet, you must also
specify the "cable_type" attribute.  Listing information about net2 now shows:

   $ lsnim -l net2

   net2:
      class    =  networks
      type     =  ent
      net_addr =  192.192.192.0
      snm      =  255.255.255.0
      Nstate   =  ready for use

The nim command is now used to define the 3rd network which will participate
in this environment:

   $ nim -o define -t fddi -a net_addr=130.35.130.0 -a snm=255.255.255.0 net3

Displaying information about net3 reveals:

   $ lsnim -l net3

   net3:
      class    =  networks
      type     =  fddi
      net_addr =  130.35.130.0
      snm      =  255.255.255.0
      missing  =  route to the NIM master
      Nstate   =  information is missing from this object's definition

Again, because NIM sees no connection between the net3 network object and the
NIM master, the state of net3 is essentially "incomplete".  In this case, the
master does not have an interface which connects directly to net3, so a NIM
route must be established between a network which the master does connect to
and net3.  The change operation is used to do this and either network object
could be operated on.  For this example, the net3 object will be "change"d:

   $ nim -o change -a routing='net2 fddigw ethergw' net3

The value of the "routing" attribute is specified in the following stanza
format:

   field 1 = the name of the destination network object
   field 2 = the hostname of the gateway to use to get to the destination
   field 3 = the hostname of the gateway which the destination network uses to
                get back to the origination network

In this example, net3 is the originating network, the destination network is
net2, net3 uses the hostname "fddigw" to get to net2, and net2 uses the hostname
"ethergw" to get back to net3.

The network topology from NIM's perspective now looks like this:

                                                                                
        +-------+--- < net1 > --+---------------+-------+-------+         
        .       .               |               .       .       .         
        .       .               |               .       .       .         
      ..+..   ..+..             |             ..+..   ..+..   ..+..       
      .   .   .   .             |             .   .   .   .   .   .       
      .   .   .   .             |             .   .   .   .   .   .       
      .   .   .   .      |------+-------|     .   .   .   .   .   .       
      .   .   .   .      |              |     .   .   .   .   .   .        
      .....   .....      |   master     |     .....   .....   .....        
                         |              |                                
                         | [boot]       |                                
                         | [nim_script] |                                
                         |              |                                
                         |              |                                  
                         |              |                                  
                         |------+-------|                                 
                                |                                         
                                |                    ............         
                             < net2 >                .          +......+  
                                |                    ............      |  
                                |                                      |  
                                |      ............                    |  
                                +......+          .                 < net3 >
                                |      ............                    |  
              ............      |                                      |  
              .          +......+                                      |   
              ............      |                                      |  
                                |           ................           |  
                                +...........+              +...........+      
                                |           ................           |
                                |                                      |  
              ............      |                                      |  
              .          +......+                                      |  
              ............      |                    ............      |  
                                |                    .          +......+        
                                |                    ............      |        


                                                                                
Note that while there is a machine which is being used as a gateway between 
net2 and net3, NIM doesn't know about it because no machine object has been
"define"d to represent it.  This conidition is acceptable: a machine object
is only required when it's going to actively participate in a NIM operation.
For this example, the gateway will remain unknown to NIM.

step #5: "define" the machines which will participate in the NIM environment
----------------------------------------------------------------------------
So far, only one machine has been "define"d in the NIM environment: the NIM
master.  Other machines will now be defined using the nim command:

   $ nim -o define -t standalone -a if1='net1 bogus1 10005aa88500' \
        -a ring_speed1=16 tok1
   $ nim -o define -t dataless   -a if1='net1 bogus2 10005aa88500' \
        -a ring_speed1=16 tok2
   $ nim -o define -t standalone -a if1='net2 net2_1 10005aa88500' \
        -a cable_type1=bnc eclient1
   $ nim -o define -t standalone -a if1='net2 net2_2 10005aa88500' \
        -a cable_type1=bnc eclient2
   $ nim -o define -t diskless   -a if1='net3 syzygy 10005aa88500' syzygy

Note that in the above operations, the "if" attribute and any associated
attributes have been specified with a sequence number.  NIM uses unique 
sequence numbers when the same attribute can be added to an object's definition
multiple times.  The "if" attribute is one of these kinds of attributes.

Another important concept to note in these operations is that the hostname
of the machine's primary interface is independent of the name specified as the
object's name.  For example, the "eclient1" object was defined such that the
hostname of its primary interface is "net2_1".  This concept is an important
benefit within the NIM environment, as all objects are operated on using their
global, hostname independent, NIM name.

After defining these machines, the network topology from NIM's perspective
looks like this:

        +-------+--- < net1 > --+---------------+-------+-------+         
        .       .               |               .       .       .         
        .       .               |               .       .       .         
      ..+..   |-+-|             |             ..+..   |-+-|   ..+..       
      .   .   | t |             |             .   .   | t |   .   .       
      .   .   | o |             |             .   .   | o |   .   .       
      .   .   | k |      |------+-------|     .   .   | k |   .   .       
      .   .   | 1 |      |              |     .   .   | 2 |   .   .        
      .....   |---|      |   master     |     .....   |---|   .....        
                         |              |                                
                         | [boot]       |                                
                         | [nim_script] |                                
                         |              |                                
                         |              |                                  
                         |              |                                  
                         |------+-------|                                 
                                |                                         
                                |                    |----------|         
                             < net2 >                | syzygy   +------+  
                                |                    |----------|      |  
                                |                                      |  
                                |      ............                    |  
                                +......+          .                 < net3 >
                                |      ............                    |  
              |----------|      |                                      |  
              | eclient1 +------+                                      |   
              |----------|      |                                      |  
                                |           ................           |  
                                +...........+              +...........+      
                                |           ................           |
                                |                                      |  
              |----------|      |                                      |  
              | eclient2 +------+                                      |  
              |----------|      |                    ............      |  
                                |                    .          +......+        
                                |                    ............      |        


step #6: "define" NIM resources
-------------------------------
The next step in configuring a NIM environment is to define the resources
which will be required to perform other NIM operations.  For example, the
"bos_inst" (BOS install) operation requires the following resources: 
   spot
   lpp_source
To perform the "dkls_init" (diskless initialization) operation, the following
resources are required:
   spot
   root
   dump 
   paging

Because the "spot" type resource is required for all network boot operations,
it is an important resource that will be used often.  If the "define" operation
is performed for a SPOT at this time, an error will be encountered:

   $ nim -o define -t spot -a location=/usr -a server=master SPOT1

0042-001 nim: processing error encountered on "master":
   0042-021 m_mkspot: the "source" attribute is required for this operation

As you can see, the "source" attribute is required when defining a SPOT.  This
attribute is used to specify where the images used to construct a SPOT come
from and may be one of the following:
   1) a local device which contains install media
   2) a previously "define"d spot resource
   3) a previously "define"d lpp_source resource which has the "simages"
         attribute

Since an lpp_source is going to be required for the bos_inst operation anyway,
a good place to start when defining resources is with the lpp_source.  

The lpp_source resource represents a directory which contains install images.
NIM requires that the images reside in a directory because NIM is responsible
for exporting all resources for client use and NIM is unable to do this for
devices: remote device access (eg, remote access to a tape drive) is not 
currently supported.  When defining a resource of this type, the user can 
specify an existing directory which contains images or can specify that the 
images are to be copied from some other location.  For example, if the user has
already put images into the /export/images directory on the master, then an
lpp_source could be defined as follows:

   $ nim -o define -t lpp_source -a location=/export/images \
        -a server=master images

Or, the user can request NIM to copy the images from a device into a local
directory.  For example, assuming that the master has a tape drive which 
contains an installable product tape, then an lpp_source can be created via
NIM using the "source" attribute:

   $ nim -o define -t lpp_source -a location=/export/images -a server=master \
        -a source=/dev/rmt0 images

As part of the "define" operation, regardless of whether the "source" attribute
is provided or not, NIM will create a ".toc" file.  This file is used by
installp to determine what images are available in a directory.  NIM will also
check for the set of images which NIM requires in order to support NIM install
operations.  This set of images is called "support images" or "simages".  If
an lpp_source has the complete set of "simages", NIM will add the "simages"
attribute to the definition of the lpp_source.

Once an lpp_source has been defined, the "check" operation can be performed
on the lpp_source, which will cause NIM to rebuild the ".toc" file and check for
"simages".  This operation should be performed anytime an install image is
added to or removed from the lpp_source.

For this example, the "images" resource defined above contains all the support
images NIM requires, so the "simages" attribute is part of the object's
definition.  Using lsnim to list information about this object yields output
similar to this:

   $ lsnim -l images

  images:
     class       = resources
     type        = lpp_source
     server      = master
     location    = /export/images
     alloc_count = 0
     Rstate      = ready to use
     simages     = yes

Note that other information associated with resource objects includes the
"alloc_count", which indicates how many clients this resource is currently
allocated to, and "Rstate", which indicates whether the resource can be used
currently or not.

As stated above, one of the ways to make a SPOT is to use an lpp_source.  Since
one has been defined, a SPOT can now be defined:

   $ nim -o define -t spot -a location=/usr -a server=master \
        -a source=images SPOT1

For this example, a "location" of /usr is used.  This indicates to NIM that
the /usr filesystem of the NIM master is to be converted into a SPOT.  Doing
this represents the fastest way to build a SPOT and is faster than specifying
some other location (eg, /export/exec) because many of the filesets which NIM
requires a SPOT to have are already installed into a /usr filesystem when it
is initialized during base system installation.

When this operation completes successfully, the SPOT is ready to be used.  If,
however, an error is encountered, the SPOT will remain "define"d, but it's
Rstate will indicate an error, which will prevent it from being used in other
NIM operations.  Errors can occur for many reasons and whenever one is
encountered, the user should follow the procedures outlined in the trouble
shooting section of the Network Install Management Guide.

One common error which can occur is that one or more of the "simages" filesets
cannot be installed into the SPOT due to lack of free space.  When this occurs,
the user must expand the filesystem in which the SPOT resides, then perform
the "cust" operation on the SPOT.  The "cust" operation is used to install
software into the SPOT after it has been defined.

Another common SPOT error occurs when there is insufficient free space in the
/tftpboot directory.  This directory is used to store the network boot images
which NIM creates automatically when an SPOT is performed on a SPOT.  When a
SPOT operation fails for this reason, the user should expand the filesystem
that contains the /tftpboot directory, then perform the "check" operation on
the SPOT: this operation will verify that all the "simages" filesets have been
installed and create the types of network boot images which the SPOT supports.

For this example, the SPOT "define" operation specified above had completed
successfully so that, using lsnim, the SPOT's definition looks like this:

   $ lsnim -l SPOT1

   SPOT1:
     class        = resources
     type         = spot
     server       = master
     location     = /usr
     alloc_count  = 0
     Rstate       = ready to use
     version      = 04
     release      = 01
     if_supported = tok
     if_supported = ent
     if_supported = fddi

Note that there are several attributes which apply to SPOTs only:
   - "version" and "release" specify the version/release level which the SPOT
        has been created with
   - "if_supported" indicates the type of network interface which the SPOT
        supports
The "if_supported" attribute is particularly important because it is used when
the SPOT is "allocated" to a client to verify that the SPOT supports the 
client's primary network interface type.

The network topology from NIM's perspective now looks like this:


        +-------+--- < net1 > --+---------------+-------+-------+         
        .       .               |               .       .       .         
        .       .               |               .       .       .         
      ..+..   |-+-|             |             ..+..   |-+-|   ..+..       
      .   .   | t |             |             .   .   | t |   .   .       
      .   .   | o |             |             .   .   | o |   .   .       
      .   .   | k |      |------+-------|     .   .   | k |   .   .       
      .   .   | 1 |      |              |     .   .   | 2 |   .   .        
      .....   |---|      |   master     |     .....   |---|   .....        
                         |              |                                
                         | [boot]       |                                
                         | [nim_script] |                                
                         | [images]     |                                
                         | [SPOT1]      |                                  
                         |              |                                  
                         |------+-------|                                 
                                |                                         
                                |                    |----------|         
                             < net2 >                | syzygy   +------+  
                                |                    |----------|      |  
                                |                                      |  
                                |      ............                    |  
                                +......+          .                 < net3 >
                                |      ............                    |  
              |----------|      |                                      |  
              | eclient1 +------+                                      |   
              |----------|      |                                      |  
                                |           ................           |  
                                +...........+              +...........+      
                                |           ................           |
                                |                                      |  
              |----------|      |                                      |  
              | eclient2 +------+                                      |  
              |----------|      |                    ............      |  
                                |                    .          +......+        
                                |                    ............      |        


step #7: perform a base system installation (the bos_inst operation)
--------------------------------------------------------------------
At this point, there are enough NIM resources to perform a base system
installation on a standalone machine.  Assume we now want to install the machine
"eclient2".  To do that, we must first allocate the NIM resources which are
required to support the install process.  To find out what resources will
be required, we execute the lsnim command:

   $ lsnim -q bos_inst eclient2

   the following resources are required:
           spot
           lpp_source

   the following resources are optional:
           mksysb
           installp_bundle
           script
           log

   the following attributes are optional:
           -a source=<value>
           -a no_nim_client=<value>
           -a installp_flags=<value>
           -a filesets=<value>
           -a preserve_res=<value>
           -a debug=<value>
           -a verbose=<value>

As you can see, a "spot" and "lpp_source" resource will be required.  Since
we've already defined these resources in previous operations, we can now
"allocate" them for eclient2 to use:

   $ nim -o allocate -a spot=SPOT1 -a lpp_source=images eclient2

The "allocate" operation is used to give a specific machine permissions to
access a NIM resource, so it is a prepatory step for performing more complex
NIM operations, like bos_inst (base system installation).  The allocate
operation also does verification to ensure:
   1) that, from NIM's perspective, it is able to communicate with the server
         of the resource; this is called "NIM connectivity"
   2) that the client's configuration type is allowed to use the type of
         resource being allocated
   3) that the resource is "ready to use"

NIM connectivity is another beneficial aspect of NIM : since NIM coordinates
the resource access between NIM clients and servers, users don't have to
worry about potential problems associated with mutli-homed machines.  To
determine connectivity between a NIM client and server, NIM uses the following
algorithm:

                 -------------------------------------
                 | get the client's primary interface|
                 | (ie, the "if1" attribute)         |
                 ------------------+------------------
                                   |
                                   |
                 ------------------+------------------
                 | find the server of the resource   |
                 | (ie, get the "server" attribute)  |
                 ------------------+------------------
                                   |
                                   |
                 ------------------+------------------
                 | get server's interface information|
                 | (ie, get all server's "if" attrs) |
                 ------------------+------------------
                                   |
                                   |<--------------------------------+
                                   |                                 |
                 <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>               |
  <_____yes______<  is server connected to same      >               |
  |              <  network as client?               >               |
  |              <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>               |
  |                                |                                 |
  |                             < no >                               |
  |                                |                                 |
  |              <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>               |
  |<____yes______<  does client's network have a NIM >               |
  |              <  route to server's network?       >               |
  |              <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>               |
  |                                |                                 |
  |                             < no >                               |
  |                                |                                 |
  |              <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>               |
  |              <  are there any more server        >______yes_____>|
  |              <  interfaces to try?               >
  |              <<<<<<<<<<<<<<<<<<+>>>>>>>>>>>>>>>>>>
  |                                |
  |                             < no >
  |                                |
  |              ------------------+------------------
  |              | there is no NIM route between     |
  |              | client and server: resource       |
  |              | allocation is denied              |
  |              -------------------------------------
  |
  |
  |              ------------------+------------------
  |              | there is a NIM route between the  |
  |------------->| client and server: resource       |
                 | allocation is granted             |
                 -------------------------------------


After the required resources have been allocated, the "bos_inst" operation
can be performed:  

   $ nim -o bos_inst eclient2

This operation finishes the configuration of the NIM environment to support a
base system installation by:
   1) allocatings a network boot resource; this enables the target to perform a
         network boot
   2) allocatings a nim_script resource; this resource represents a script 
         which NIM constructs to perform customization on the target

In order to perform a base system installation, the target must have access to
some form of BOS runtime image.  In the NIM environment, there are always at
least two sources which could be used:
   1) the SPOT which has been allocated to support the network boot processing
   2) the BOS runtime image (the file named "bos") that is part of the 
         lpp_source which has been allocated to the target
A third potential source for the BOS runtime files is a mksysb resource which
has been allocated to the target.

In order to avoid ambiguity, the "source" attribute can be specified with
the "bos_inst" operation.  When supplied, NIM will use the specified source
when performing the base system installation: valid values are "rte" (which 
stands for "runtime"), "spot", "mksysb".  If this attribute is not supplied
(as in the above example), NIM uses a value of "rte" by default.

When the "bos_inst" operation returns successfully, the NIM environment has
been enabled to perform a base system installation on the target.  Note that
we say "enabled" because, in order to actually begin the install process, the
target must issue a BOOTP request.  The "bos_inst" operation attempts to
force the target to issue a BOOTP request, but there are many reasons why it
will not be able to.  In these cases, NIM will display a warning message
similar to this:

   $ nim -o bos_inst eclient2

     warning nim: unable to initiate network boot on "eclient2"

As with all NIM commands, "warning"s are non-fatal error conditions which
may require user intervention to remedy the situation.  In this particular
case, a user must go to the "eclient2" machine and follow the "How to
Initiate a BOOTP Request" procedure (as documented in the Network Installation 
Management Guide).

Once the target issues a BOOTP request, recieves a replay, and retrieves its
network boot image, the install process will begin.  During the install,
you can monitor what is currently happening from the NIM master by displaying
information about the target:

   $ lsnim -l eclient2

   eclient2:
      class         =  machines
      type          =  standalone
      Mstate        =  in the process of booting
      Cstate        =  Base Operating System installation is being performed
      if1           =  "net2 net2_2 10005aa88500"
      cable_type1   =  bnc
      spot          =  SPOT1
      control       =  master
      lpp_source    =  images
      boot          =  boot
      info          =  Initializing disk environment

The "info" attribute is updated periodically by the BOS install program and
gives a rough indication of the processing that is currently occuring on the
target.  The "Cstate" attribute reflects what phase the install process is
in.  Currently, there are four install phases:

   1) "BOS installation has been enabled"
         indicates that the NIM environment is "enabled" to perform a base 
         system install; when the target issues a BOOTP request, the install 
         process will begin

   2) "Base Operating System installation is being performed"
         indicates that base system installation has begun (ie, that the BOS
         install program has begun executing)

   3) "customization is being performed"
         indicates that NIM customization is being performed; this kind of
         customization includes configuring the target's network interface,
         configuring the NIM client fileset, installing any other filesets the
         user has indicated should be installed, and executing any user-defined
         shell scripts which have been allocated to the target

   4) "post install processing is being performed"
         indicates that misc. post install processing is being performed (eg,
         creation of a local boot image is done in this phase)

When the actual install processing on the target finishes, several things will 
happen automatically:

   1) all resources which have been allocated to the target will be deallocated

   2) the "Cstate" will be reset so that another NIM operation can no be
         performed

   3) an attribute named "Cstate_result" will be added to the target's
         definition to indicate the result of the install operation

As stated above, if the "bos_inst" operation returns successfully, then the
NIM environment is "enabled" to perform a base system installation.  If any
error is encountered after this point, users should follow the NIM 
troubleshooting procedures (as documented in the Network Installation
Management Guide) to determine why the has occurred and how to fix it.  Many
times, it will be a problem with the physical, network environment.  For
example, if there is a gateway between a target and its server, and the
gateway is not functioning, the target will not be able to communicate with
the server, which will prevent the target from being installed.  These kinds
of problems will require manual intervention on the part of the user.

Once the "bos_inst" operation has completed, you may execute the command
again to enable to the install for another standalone machine.  NIM allows you
to enable the NIM environment to install multiple machines simultaneously.  Note
that there is some pratical limit to the number of machines which can be
installed simultaneously and it will vary from environment to environment.
Some of the limiting factors are:
   1) the throughput of your network
   2) disk access throughput on the install servers
   3) the platform type of your servers
Obviously, if you have a very fast server and very little network traffic, more
machines can be installed simultaneously than if you're experiencing heavy
network traffic and/or your server is bogged down.  NIM does not have the
ability to determine the practical limit to the number of simultaneous installs,
so it will up to the user to decide this.

For this example, we'll perform two installs simultaneously.  Let's assume
that we've just enabled the install for "eclient2".  When the "bos_inst"
operation returns for eclient2, we immediately allocate the required
resource and enable the install of "tok1":

   $ nim -o allocate -a spot=SPOT1 -a lpp_source=images tok1
   $ nim -o bos_inst -a source=spot tok1

     warning nim: unable to initiate network boot on "tok1"

The NIM environment is now enabled to install tok1 and eclient2, as soon as
those machines issue a BOOTP request.  Since we recieved a "warning" message
from the bos_inst operation, we now go to each machine and follow the "How
to Initate a BOOTP Request" procedure to get the machines to issue a BOOTP
request, which they do.  This begins the process and, sometime later, the
install is finished, the machines reboot automatically, and they're ready to
use.  Note that since these machines were installed by NIM, they are
automatically installed with the NIM client fileset, which enables them to
participate in the NIM environment after install.

step #8: enable a diskless machine to boot
------------------------------------------
We will now setup the NIM environment to support a machine which has a
configuration type of "diskless".  The operation we need to perform is the
"dkls_init" (diskless initialization) and, to find out what this operation
requires, we execute the lsnim command:

   $ lsnim -q -t diskless

   the following resources are required:
           root
           spot
           paging
           dump

   the following resources are optional:
           home
           shared_home
           tmp
           log

As you can see, we'll need some other types of resources which haven't been
defined yet: root, paging, and dump.  Since we've already defined a SPOT,
we don't need to build another one.

For this example, we decide to distribute the diskless resources among two
machines.  In the NIM environment, any machine with a standalone configuration
may serve a NIM resource.  Since we just installed tok1 and eclient2, we'll
use these machines as servers of some diskless resources.

On the NIM master, we execute the following:

   $ nim -o define -t root -a server=tok1 -a location=/export/roots root_dirs
   $ nim -o define -t dump -a server=tok1 -a location=/export/dumps dump_dirs
   $ nim -o define -t paging -a server=eclient2 \
        -a location=/export/paging pfiles

Note that we do this work from the NIM master: since tok1 and eclient2 are
running NIM clients, the NIM master has permissions to execute commands on
these machines.  This is another important aspect of NIM: all "push" operations
are performed from the NIM master, so the NIM master provides a central
point for administrative tasks.

After "define"ing these new resources, the environment from NIM's perspective
looks like this:

        +-------+--- < net1 > --+---------------+-------+-------+         
        .       .               |               .       .       .         
        .       .               |               .       .       .         
      ..+..   |-+-|             |             ..+..   |-+-|   ..+..       
      .   .   | t |             |             .   .   | t |   .   .       
      .   .   | o |             |             .   .   | o |   .   .       
      .   .   | k |      |------+-------|     .   .   | k |   .   .       
      .   .   | 1 |      |              |     .   .   | 2 |   .   .        
      .....   |   |      |   master     |     .....   |---|   .....        
              |   |      |              |                                
           [root_dirs]   | [boot]       |                                
           [dump_dirs]   | [nim_script] |                                
              |   |      |              |                                
              |---|      |              |                                  
                         |              |                                  
                         |------+-------|                                 
                                |                                         
                                |                    |----------|         
                             < net2 >                | syzygy   +------+  
                                |                    |----------|      |  
                                |                                      |  
                                |      ............                    |  
                                +......+          .                 < net3 >
                                |      ............                    |  
              |----------|      |                                      |  
              | eclient1 +------+                                      |   
              |----------|      |                                      |  
                                |           ................           |  
                                +...........+              +...........+      
                                |           ................           |
                                |                                      |  
              |----------|      |                                      |  
              | eclient2 +------+                                      |  
              |          |      |                    ............      |  
              | [pfiles] |      |                    .          +......+        
              |          |      |                    ............      |        
              |----------|


Now, we allocate the diskless resources to the machine which we want to boot,
the machine known as "syzygy":

   $ nim -o allocate -a spot=SPOT1 -a paging=pfiles -a root=root_dirs \
      -a dump=dump_dirs syzygy

Executing the above yields an error similar to this:

   0042-048 nim: "syzygy" is unable access the "root_dirs" resource due to
        network routing

This error is received because the NIM connectivity algorithm (which is run
during the allocate operation) has determined that there is no "NIM route"
between the target (syzygy), which is on net3, and the server of the root_dirs
resource (tok1), which is connected to net1.  No error was encountered for
the SPOT1 or pfiles resources because we had already established a NIM route
between net3 and net2 earlier in this example.  Note that, while network routing
may be setup correctly to allow net1 to communicate with net3, NIM doesn't
know about it because no NIM route has been established between these network
objects.  To fix this problem, we establish a NIM route between net1 and net3:

   $ nim -o change -a routing='net3 Mtokenring fddigw' net1

Again, this syntax is used to indicate:
   1) that net1 communicates with net3 using the gateway hostname "Mtokenring"
   2) that net3 communicates with net1 using the gateway hostname "fddigw"
This NIM route involves two gateways, one of which happens to be represented
as a NIM machine (the NIM master).  This illustrates another important feature:
NIM does not perform "network management" and therefore does not manage 
gateways.  NIM "uses" gateways in order to enable clients to communicate
with servers and the only information NIM needs to do that is the hostname
which should be used to communicate with the gateway.  Therefore, machines
which function as gateways do not have to represented as machine objects in
the NIM environment, but they can be if the user so desires.

We can now finish allocating the rest of the diskless resources to syzygy:

   $ nim -o allocate -a spot=SPOT1 -a paging=pfiles -a root=root_dirs \
        -a dump=dump_dirs syzygy

Note that we can re-execute the same line we tryed before because it is not an
error to request allocation of a resource which is already allocated to the
client.

We can now finish the configuration of the NIM environment for syzygy:

   $ nim -o dkls_init -a size=32 syzygy

When the "size" attribute is used with this operation, it tells NIM how big
to make the client's remote paging file, so, in this case, NIM will create
a 32 Megabyte paging file for syzygy.

When the dkls_init operation finishes, the environment has been enabled to
support syzygy in a diskless configuration.  Again, NIM just "enables" the
environment: the user must go to syzygy and follow the "How to Initiate a
BOOTP Request" procedures to initiate the network boot process.  Once syzygy
is booted once, however, its boot device list will be initialized such that
the user shouldn't need to interact with the IPL ROM menus again.

Summary
-------
NIM performs network installs and supports diskless/dataless configurations
by managing the resources which are required for those operations.  In order
to do this, NIM represents part of the physical environment as "objects".  These
objects are classified into 3 classes (networks, machines, resources) and 
within each class, many types (eg, tok, ent, fddi, standalone, etc).  The
"define" operation is used to create a new object and, once created
succesfully, it may be used in other NIM operations.  NIM uses unique, user
defined names to identify these objects and, for machines, it is important to
realize that the NIM name is hostname independent.  NIM requires that all
network objects be able to communicate with the NIM master and, when
distributing NIM resources over multiple networks, that these network objects
be interconnected via "NIM routes".  

When errors are encountered, the user should refer to the NIM troubleshooting
procedures in the "Network Installation Management Guide".

