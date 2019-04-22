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

case $1 in
	linux )
		FN=LinuxAvara-$SHORT_HASH
		mv build $FN
		zip -r $FN.zip $FN
		upload_file $FN.zip
		;;
	mac )
		FN=MacAvara-$SHORT_HASH.zip
		mv build/MacAvara.zip $FN
		upload_file $FN
		;;
	windows )
		FN=WinAvara-$SHORT_HASH.zip
		mv build/WinAvara.zip $FN
		upload_file $FN
		;;
esac