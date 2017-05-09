#!/bin/bash

VERSION=0.0.5
ZIPFILE=boards/nocan_pack_$VERSION.zip
JSONFILE=package_omzlo.com_index.json
TEMPLATE=package_omzlo.com_index.template.json
LIBRARY=../omzlo-one-lib

cp -v $LIBRARY/twi_328pb.h $LIBRARY/twi_328pb.cpp src/libraries/NOCAN/
cp -v $LIBRARY/nocan*.h $LIBRARY/nocan*.cpp src/libraries/NOCAN/

rm -rf $ZIPFILE

cp -R src $VERSION

zip -q -r $ZIPFILE $VERSION

CHECKSUM=`shasum -a 256 $ZIPFILE | cut -d ' ' -f 1`
SIZE=`stat -f "%z" $ZIPFILE`

sed -e "s/\${version}/$VERSION/g" -e "s/\${size}/$SIZE/g" -e "s/\${checksum}/$CHECKSUM/g" $TEMPLATE >  $JSONFILE

rm -rf $VERSION/

scp $ZIPFILE $JSONFILE pannetra@paris:/home/pannetra/public_html/

echo "Updated version $VERSION with checksum $CHECKSUM"
