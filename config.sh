#!/bin/sh
version=""
clean="no"

config_h="./src/config.h"

# Check options
for opt do
    case "$opt" in
        --clean)
            clean="yes"
        ;;
    esac
done

# Original version
caption2ass_ver="0,3,0,4"
caption_dll_ver="0,4,0,3"

# Output config.h
if test "$clean" = "yes" ; then
cat > "$config_h" << EOF
#undef CAPTION2ASS_PCR_GIT_VERSION
#undef CAPTION_DLL_GIT_VERSION
EOF
else
  if [ -d ".git" ] && [ -n "`git tag`" ]; then
    version="`git describe --tags | cut -d '-' -f 2-3`"
    caption2ass_ver="${caption2ass_ver}-${version}"
    caption_dll_ver="${caption_dll_ver}-${version}"
    echo "$caption2ass_ver"
    echo "$caption_dll_ver"
    echo "#define CAPTION2ASS_PCR_GIT_VERSION     \"$caption2ass_ver\"" > "$config_h"
    echo "#define CAPTION_DLL_GIT_VERSION         \"$caption_dll_ver\"" >> "$config_h"
  else
    echo "#undef CAPTION2ASS_PCR_GIT_VERSION" > "$config_h"
    echo "#undef CAPTION_DLL_GIT_VERSION" >> "$config_h"
  fi
fi
