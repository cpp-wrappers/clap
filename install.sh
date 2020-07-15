[ -z "$1" ] && echo "arg is root for include dir" \
|| ( \
    echo "uninstalling..." \
    & \
    rm -rv $1/include/clap \
    & \
    echo "installing..." \
    & \
    cp -rv include $1 \
)