#!/bin/sh
ERR_EXIT () {
    echo $1; exit 1
}

cwd="$(cd $(dirname $0); pwd)"

version=''
clean='no'

config_h="$cwd/src/config.h"

# Parse options
for opt do
    optarg="${opt#*=}"
    case "$opt" in
        --clean)
            clean='yes'
            ;;
        *)
            ERR_EXIT "Unknown option ... $opt"
            ;;
    esac
done

# Original version
caption2ass_ver="0,3,0,4"
caption_dll_ver="0,4,0,3"

# Output config.h
if [ "$clean" = 'yes' -o ! -d "$cwd/.git" ]; then
    cat > "$config_h" << EOF
#undef CAPTION2ASS_PCR_GIT_VERSION
#undef CAPTION_DLL_GIT_VERSION
EOF
else
    cd "$cwd"
    if [ -n "`git tag`" ]; then
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
