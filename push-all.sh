#!bash

scriptDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
curDir=`pwd`

cd ${scriptDir}

git push bitbucket
git push github
git push stash

cd $curDir
