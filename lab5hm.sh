#########################################################################
# File Name: vm.sh
# Author: Lebway
# mail: lebway@foxmail.com
# Created Time: Sun Apr 21 09:46:53 2019
#########################################################################
#!/bin/bash
/OSLAB/gxemul -E testmips -C R3000 -M 64 -d gxemul/fs.img  gxemul/vmlinux
