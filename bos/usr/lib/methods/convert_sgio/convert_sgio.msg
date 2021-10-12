$ @(#)54	1.1  src/bos/usr/lib/methods/convert_sgio/convert_sgio.msg, inputdd, bos41J, 9518A_all 5/1/95 12:51:22
$
$ COMPONENT_NAME: inputdd
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$
$ (C) COPYRIGHT International Business Machines Corp. 1995
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ messages for the convert_sgio.sh shell script
$ Note to translators: Do not add a newline character (
$ \ or
$ is not there already.
$quote "         define quote character.

$len
$set 1        Define initial set#
$ Do not translate %d, %s, %1$d, %1$s, or any flags eg. -c, -F etc.


1                    "Could not remove %s. Please close all programs which\naccess this device (including X-server) before retrying.\n"
$ "\n" between words is for format control. Do not change.

2                    "Unable to complete configuration for serial %s device. TTY\nconfiguration on serial port with parent %s and port %s failed.\n"
$ "\n" between words is for format control. Do not change.

3                    "Unable to complete configuration for serial %s device. Device\nconfiguration with device instance %s and TTY %s failed.\n"
$ "\n" between words is for format control. Do not change.

4                    "Please refer to the \"Setting Up X11 Graphic Input Devices\" section\nof your documentation for information on how to configure graphic\ninput devices.\n"
$ "\n" between words is for format control. Do not change.
