#!/bin/csh -f
#
# This script launches the command passed as argument and creates
# a file called 'failed_script' if the return status is non-zero.
#
# This is only due to MIDAS, because it is impossible to read the
# return status of a shell call under a MIDAS script...
#
rm -f ./failed_script
$*
if ($status) then
        touch ./failed_script
endif

