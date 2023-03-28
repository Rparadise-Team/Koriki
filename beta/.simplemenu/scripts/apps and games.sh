consoles="/mnt/SDCARD/.simplemenu/section_groups/arcades.ini"

cat > ${consoles} <<EOF
[CONSOLES]
consoleList = GAMES,APPS,SEARCH

[GAMES]
execs = #
romDirs = /mnt/SDCARD/.simplemenu/games/
romExts = .sh,.fgl

[APPS]
execs = #
romDirs = /mnt/SDCARD/.simplemenu/apps/
romExts = .sh,.fgl

[SEARCH]
execs = #
romDirs = /mnt/SDCARD/App/Search/data/
romExts = .sh,.fgl,.miyoocmd
EOF
