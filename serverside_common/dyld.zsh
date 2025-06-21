source ~/.zprofile

echo "building dylib"

pushd "../apple_platforms/Make The Thing"

 echo "
 #ifndef version_hpp 
 #define version_hpp 
 #define VERSION_NUMBER $1
 #define VERSION(f_name) f_name ## _ ## $1
 #endif /* version_hpp */" > version.hpp

xcodebuild -quiet -scheme dynamic_dev -configuration Debug OTHER_CGLAGS="-Wno-unused-function -Wno-unused-variable" OTHER_CPLUSPLUSFLAGS="-fno-rtti -Wno-unused-function -Wno-unused-variable" ONLY_ACTIVE_ARCH=YES GCC_OPTIMIZATION_LEVEL=0 CODE_SIGNING_ALLOWED="NO" -destination 'platform=macOS,arch=arm64' EXECUTABLE_NAME="libdynamic_dev"$1".dylib" build 
#xcodebuild -scheme dynamic_dev -configuration Debug OTHER_CFLAGS="-ftime-trace" OTHER_CPLUSPLUSFLAGS="-ftime-trace" COTHER_CPLUSPLUSFLAGS="-fno-rtti" ONLY_ACTIVE_ARCH=YES GCC_OPTIMIZATION_LEVEL=0 CODE_SIGNING_ALLOWED="NO" -destination 'platform=macOS,arch=arm64' EXECUTABLE_NAME="libdynamic_dev"$1".dylib" build

#codesign -f -s - "./build/Debug/libdynamic_dev"$1".dylib"
xattr -w com.apple.xcode.CreatedByBuildSystem true './build'

echo "dyld done"

popd
