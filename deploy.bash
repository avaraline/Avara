#!/bin/bash

DEPLOY_HOST=travis@vastan.net
DEPLOY_PATH=travis
DEPLOY_ADDR=$DEPLOY_HOST:$DEPLOY_PATH

SHORT_HASH=`echo $TRAVIS_COMMIT | head -c 7`

cd $TRAVIS_BUILD_DIR
KEY=$TRAVIS_BUILD_DIR/deploy_rsa
chmod 400 $KEY

function upload_file
{
	echo put $1 | sftp -i $KEY -oStrictHostKeyChecking=no -b- $DEPLOY_ADDR
}

BASEFN=Avara-$TRAVIS_BRANCH-$SHORT_HASH

case $1 in
	linux )
		FN=Linux$BASEFN
		mv build $FN
		zip -r $FN.zip $FN
		upload_file $FN.zip
		;;
	mac )
		FN=Mac$BASEFN.zip
		mv build/MacAvara.zip $FN
		upload_file $FN
		;;
	windows )
		FN=Win$BASEFN.zip
		mv build/WinAvara.zip $FN
		upload_file $FN
		;;
esac