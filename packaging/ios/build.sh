#!/bin/sh
DESTINATION="$PWD"
PRODUCT="CCL Demo"

REPO_ROOT="../../../../../.."
if [ $1 ]; then
    REPO_ROOT=$1
fi
BUILD_PATH="${REPO_ROOT}/build/cmake/ios/Release"

BUNDLE="${BUILD_PATH}/$PRODUCT.app"
${REPO_ROOT}/build/ios/PackageApplication -v "$BUNDLE" -o "$DESTINATION/$PRODUCT.ipa"
