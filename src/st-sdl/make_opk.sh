#!/bin/sh

OPK_NAME=st-`date +'%Y%m%d'`.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=ST-SDL
Comment=ST Terminal
Exec=st
Terminal=false
Type=Application
StartupNotify=true
Icon=st
Categories=applications;
EOF

# create opk
FLIST="st"
FLIST="${FLIST} default.gcw0.desktop"
FLIST="${FLIST} st.png"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop
