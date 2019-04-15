#########################################################################
# File Name: pushit.sh
# Author: Lebway
# mail: lebway@foxmail.com
# Created Time: Sat Apr 13 10:59:25 2019
#########################################################################
#!/bin/bash
git add --all
git commit -a -m "$1"
if [ "$2"="" ]
then
	git push
else 
	git push origin $2:$2
fi
