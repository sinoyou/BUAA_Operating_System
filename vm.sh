#########################################################################
# File Name: vm.sh
# Author: Lebway
# mail: lebway@foxmail.com
# Created Time: Sun Apr 21 09:46:53 2019
#########################################################################
#!/bin/bash
if ["$1"=""]
then
	/OSLAB/gxemul -E testmips -C R3000 -M 64 gxemul/vmlinux
else	
	/OSLAB/gxemul -E testmips -C R3000 -M 64 -v gxemul/vmlinux
fi
