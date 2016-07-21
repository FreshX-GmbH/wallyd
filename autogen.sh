#! /bin/sh
if (autoreconf -i) ; then
        echo "autoreconf ran successfully."
else
        echo "Running autoreconf failed, please make sure you have autoconf installed."
fi

echo Updating version number

BUILD_NUMBER_FILE=version
echo $((`cat ${BUILD_NUMBER_FILE}`+1)) > ${BUILD_NUMBER_FILE}
VERSION=`cat ${BUILD_NUMBER_FILE}`
DATE=`date +'%Y%m%d'`
echo "#define BUILD_NUMBER ${VERSION}"   >src/autoversion.h
echo "#define BUILD_DATE ${DATE}"       >>src/autoversion.h
echo "#define BUILD_NUMBER ${VERSION}"   >lib/autoversion.h
echo "#define BUILD_DATE ${DATE}"       >>lib/autoversion.h
echo "#define BUILD_NUMBER ${VERSION}"   >plugins/autoversion.h
echo "#define BUILD_DATE ${DATE}"       >>plugins/autoversion.h

#./configure
