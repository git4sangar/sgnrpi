# source this file to set shell variables needed for cross compile "configure"
EXE_DIR="${HOME}/toolchains/arm7mtk/bin"
LIB_DIR="${HOME}/toolchains/arm7mtk/lib"
BUILD_TOOL_NAME_PREFIX="armv7a-mediatek451_001_vfp-linux-gnueabi-"
TARGETCC="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}gcc"
TARGETLD="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ld"
TARGETAR="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ar"
TARGETAR_RC="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ar rc"
TARGETSTRIP="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}strip"
TARGETCXX="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}g++"
TARGETRANLIB="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}ranlib"
TARGETSTRINGS="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}strings"
TARGETM4="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}m4"
TARGETBISON="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}bison"
TARGETYACC="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}yacc"
TARGETOBJCOPY="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}objcopy"
TARGETCPP="${EXE_DIR}/${BUILD_TOOL_NAME_PREFIX}cpp"

export CC="$TARGETCC -I${HOME}/devel/lgarm7mtk/include"
#export CFLAGS="-DFORMAT_ELF -D_INTERNAL_PCI_"
export LDFLAGS="-L${LIB_DIR} -L${HOME}/devel/lgarm7mtk/lib"
#export LIBS
export CPP=$TARGETCPP
export CXX=$TARGETCXX
export LD=$TARGETLD

# these settings fix bug in "configure" not finding malloc and realloc
# for cross compile.
export ac_cv_func_malloc_0_nonnull=yes
export ac_cv_func_realloc_0_nonnull=yes
# required for glib
export ac_cv_func_posix_getpwuid_r=yes

