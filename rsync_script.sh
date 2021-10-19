#!/bin/bash

if [[ $1 == '' ]] || [[ $1 != 'push' ]] && [[ $1 != 'pull' ]]
then
	echo "provide argument 'push' or 'pull'"
	exit
fi

if [[ $1 == 'push' ]]
then
	rsync --verbose --archive ./res/ da@nas:~/pool/dev/megastruct/res/ --exclude=backup
	rsync --verbose --archive ./dep/ da@nas:~/pool/dev/megastruct/dep/ --exclude=unused --exclude=SDL2_image* --exclude=imgui* --exclude=SDL2* --exclude=libgame.so
	exit
fi

if [[ $1 == 'pull' ]]
then
	rsync --verbose --archive  da@nas:~/pool/dev/megastruct/res/ ./res/ --exclude=backup --exclude=old
	rsync --verbose --archive  da@nas:~/pool/dev/megastruct/dep/ ./dep/ --exclude=unused --exclude=SDL2_image* --exclude=imgui* --exclude=SDL2* --exclude=libgame.so
fi
