# source this file to set shell variables needed for cross compile "configure"
EXE_DIR="${HOME}/devel/toolchains/raspberry/bin"
LIB_DIR="${HOME}/devel/toolchains/raspberry/lib"
BUILD_TOOL_NAME_PREFIX="arm-linux-gnueabihf-"
TARGETCC="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}gcc"
TARGETLD="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ld"
TARGETAR="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ar"
TARGETAR_RC="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ar rc"
TARGETSTRIP="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}strip"
TARGETCXX="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}g++"
TARGETRANLIB="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ranlib"
TARGETSTRINGS="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}strings"
TARGETOBJCOPY="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}objcopy"
TARGETCPP="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}cpp"

export CPP=$TARGETCPP
export CXX=$TARGETCXX
export LD=$TARGETLD
export RANLIB=$TARGETRANLIB

# these settings fix bug in "configure" not finding malloc and realloc
# for cross compile.
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes
# required for glib
export ac_cv_func_posix_getpwuid_r=yes

