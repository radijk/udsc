#!/bin/bash

# use git commit hash or tag if available as a version number

HASH=$(git log --pretty=format:'%h' -n 1 2> /dev/null)
TAG=$(git describe --exact-match $HASH 2> /dev/null)
if [[ $? -eq 0 ]]; then
	VERSION=$TAG
else
	VERSION=$HASH
fi
echo $VERSION
