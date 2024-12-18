consoles="/mnt/SDCARD/.simplemenu/section_groups/apps and games.ini"

cat > ${consoles} <<EOF
[CONSOLES]
consoleList = APPS,GAMES,OVERLAYS,AMIGA,AMSTRAD CPC,ATARI ST,COMMODORE 64,MSX,X68000,DOS,PC98,ZX SPECTRUM,SCUMMVM,OPENBOR,PICO-8,TIC-80,DOOM,QUAKE,WOLF3D,CPS1,CPS2,CPS3,NEO GEO,MAME,FINALBURN ALPHA,FINALBURN NEO,DAPHNE,ATARI LYNX,GAME & WATCH,GAME BOY,GAME BOY COLOR,GAME BOY ADVANCE,NINTENDO DS,GAME GEAR,NEO GEO POCKET,POKEMON MINI,SUPERVISION,WONDERSWAN,ATARI 2600,ATARI 5200,ATARI 7800,INTELLIVISION,ODYSSEY2,SEGA SG-1000,MASTER SYSTEM,NES,FDS,NEO GEO CD,PC ENGINE,PC ENGINE CD,PLAYSTATION,SEGA GENESIS,MSU-MD,SEGA 32X,SEGA CD,SNES,MSU-1,SGB,VIRTUAL BOY,ARDUINO

[AMIGA]
execs = /mnt/SDCARD/.simplemenu/launchers/puae_libretro,/mnt/SDCARD/.simplemenu/launchers/uae4arm_libretro,/mnt/SDCARD/.simplemenu/launchers/puae2021_libretro
romDirs = /mnt/SDCARD/Roms/AMIGA/
romExts = .adf,.adz,.dms,.fdi,.ipf,.hdf,.hdz,.lha,.slave,.info,.cue,.ccd,.nrg,.mds,.iso,.chd,.uae,.m3u,.zip,.7z,.rp9

[AMSTRAD CPC]
execs = /mnt/SDCARD/.simplemenu/launchers/crocods_libretro
romDirs = /mnt/SDCARD/Roms/CPC/
romExts = .dsk,.sna,.tap,.cdt,.voc,.cpr,.m3u,.zip,.7z

[APPS]
execs = #
romDirs = /mnt/SDCARD/.simplemenu/apps/
romExts = .sh,.fgl

[ARDUINO]
execs = /mnt/SDCARD/.simplemenu/launchers/arduous_libretro
romDirs = /mnt/SDCARD/Roms/ARDUBOY/
romExts = .hex

[ATARI 2600]
execs = /mnt/SDCARD/.simplemenu/launchers/stella2014_libretro
romDirs = /mnt/SDCARD/Roms/ATARI/
romExts = .bin,.a26,.zip,.7z

[ATARI 5200]
execs = /mnt/SDCARD/.simplemenu/launchers/a5200_libretro,/mnt/SDCARD/.simplemenu/launchers/atari800_libretro
romDirs = /mnt/SDCARD/Roms/FIFTYTWOHUNDRED/
romExts = .bin,.a52,.zip,.7z

[ATARI 7800]
execs = /mnt/SDCARD/.simplemenu/launchers/prosystem_libretro
romDirs = /mnt/SDCARD/Roms/SEVENTYEIGHTHUNDRED/
romExts = .bin,.a78,.zip,.7z

[ATARI LYNX]
execs = /mnt/SDCARD/.simplemenu/launchers/handy_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_lynx_libretro
romDirs = /mnt/SDCARD/Roms/LYNX/
romExts = .zip,.lnx,.7z
scaling = 1

[ATARI ST]
execs = /mnt/SDCARD/.simplemenu/launchers/hatari_libretro,/mnt/SDCARD/.simplemenu/launchers/hatari_plus_libretro
romDirs = /mnt/SDCARD/Roms/ATARIST/
romExts = .a78,.rom,.zip,.7z

[COMMODORE 64]
execs = /mnt/SDCARD/.simplemenu/launchers/vice_x64_libretro
romDirs = /mnt/SDCARD/Roms/C64/
romExts = .crt,.d64,.t64,.bin,.g64,.7z,.zip

[ODYSSEY2]
execs = /mnt/SDCARD/.simplemenu/launchers/o2em_libretro
romDirs = /mnt/SDCARD/Roms/ODYSSEY2/
romExts = .zip,.bin

[CPS1]
execs = /mnt/SDCARD/.simplemenu/launchers/fbalpha2012_cps1_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/CPS1/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[CPS2]
execs = /mnt/SDCARD/.simplemenu/launchers/fbalpha2012_cps2_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/CPS2/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[CPS3]
execs = /mnt/SDCARD/.simplemenu/launchers/fbalpha2012_cps3_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/CPS3/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[DAPHNE]
execs = /mnt/SDCARD/.simplemenu/launchers/daphne_libretro
romDirs = /mnt/SDCARD/Roms/DAPHNE/
romExts = .zip

[DOS]
execs = /mnt/SDCARD/.simplemenu/launchers/dosbox_pure_libretro,/mnt/SDCARD/.simplemenu/launchers/dosbox_pure_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/dosbox_standalone
romDirs = /mnt/SDCARD/Roms/DOS/
romExts = .zip,.dosz,.exe,.com,.bat,.iso,.cue,.vhd,.m3u,.7z

[DOOM]
execs = /mnt/SDCARD/.simplemenu/launchers/prboom_libretro
romDirs = /mnt/SDCARD/Roms/DOOM/DOOM/,/mnt/SDCARD/Roms/DOOM/DOOM2/
romExts = .wad,.zip,.7z

[FDS]
execs = /mnt/SDCARD/.simplemenu/launchers/fceumm_libretro,/mnt/SDCARD/.simplemenu/launchers/nestopia_libretro,/mnt/SDCARD/.simplemenu/launchers/nestopia_miyoo_libretro
romDirs = /mnt/SDCARD/Roms/FDS/
romExts = .fds,.zip,.7z

[FINALBURN ALPHA]
execs = /mnt/SDCARD/.simplemenu/launchers/fbalpha2012_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fb_32b_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/FBA/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[FINALBURN NEO]
execs = /mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fb_32b_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/FBNEO/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[GAME & WATCH]
execs = /mnt/SDCARD/.simplemenu/launchers/gw_libretro,/mnt/SDCARD/.simplemenu/launchers/gw_miyoo_libretro
romDirs = /mnt/SDCARD/Roms/GW/
romExts = .mgw,.zip,.7z
scaling = 1

[GAME BOY]
execs = /mnt/SDCARD/.simplemenu/launchers/gambatte_libretro,/mnt/SDCARD/.simplemenu/launchers/gearboy_libretro,/mnt/SDCARD/.simplemenu/launchers/sameboy_libretro,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro_server,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro_client,/mnt/SDCARD/.simplemenu/launchers/DoubleCherryGB_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/DoubleCherryGB_libretro
romDirs = /mnt/SDCARD/Roms/GB/
romExts = .gb,.gz,.zip,.7z
scaling = 1

[GAME BOY COLOR]
execs = /mnt/SDCARD/.simplemenu/launchers/gambatte_libretro,/mnt/SDCARD/.simplemenu/launchers/gearboy_libretro,/mnt/SDCARD/.simplemenu/launchers/sameboy_libretro,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro_server,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro_client,/mnt/SDCARD/.simplemenu/launchers/DoubleCherryGB_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/DoubleCherryGB_libretro
romDirs = /mnt/SDCARD/Roms/GBC/
romExts = .gbc,.zip,.7z
scaling = 1

[GAME BOY ADVANCE]
execs = /mnt/SDCARD/.simplemenu/launchers/gpsp_plus_libretro,/mnt/SDCARD/.simplemenu/launchers/mgba_plus_libretro,/mnt/SDCARD/.simplemenu/launchers/gpsp_libretro,/mnt/SDCARD/.simplemenu/launchers/mgba_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_gba_libretro,/mnt/SDCARD/.simplemenu/launchers/meteor_libretro,/mnt/SDCARD/.simplemenu/launchers/vbam_libretro,/mnt/SDCARD/.simplemenu/launchers/vba_next_libretro
romDirs = /mnt/SDCARD/Roms/GBA/
romExts = .gba,.zip,.7z
scaling = 1

[NINTENDO DS]
execs = /mnt/SDCARD/.simplemenu/launchers/drastic_standalone
romDirs = /mnt/SDCARD/Roms/NDS/
romExts = .nds,.7z,.zip,.rar
scaling = 1

[GAME GEAR]
execs = /mnt/SDCARD/.simplemenu/launchers/picodrive_libretro,/mnt/SDCARD/.simplemenu/launchers/genesis_plus_gx_libretro,/mnt/SDCARD/.simplemenu/launchers/gearsystem_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/GG/
romExts = .zip,.gg,.7z
scaling = 1

[GAMES]
execs = #
romDirs = /mnt/SDCARD/.simplemenu/games/
romExts = .sh,.fgl

[INTELLIVISION]
execs = /mnt/SDCARD/.simplemenu/launchers/freeintv_libretro
romDirs = /mnt/SDCARD/Roms/INTELLI/
romExts = .int,.bin

[MAME]
execs = /mnt/SDCARD/.simplemenu/launchers/mame2003_plus_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha2012_libretro,/mnt/SDCARD/.simplemenu/launchers/km_mame2003_xtreme_amped_libretro,/mnt/SDCARD/.simplemenu/launchers/km_mame2003_xtreme_libretro,/mnt/SDCARD/.simplemenu/launchers/mame2000_libretro,/mnt/SDCARD/.simplemenu/launchers/mame2010_libretro,/mnt/SDCARD/.simplemenu/launchers/mame2003_libretro,/mnt/SDCARD/.simplemenu/launchers/mame2003_xtreme_libretro
romDirs = /mnt/SDCARD/Roms/ARCADE/
romExts = .zip
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[MASTER SYSTEM]
execs = /mnt/SDCARD/.simplemenu/launchers/picodrive_libretro,/mnt/SDCARD/.simplemenu/launchers/genesis_plus_gx_libretro,/mnt/SDCARD/.simplemenu/launchers/gearsystem_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/MS/
romExts = .zip,.sms,.7z

[NEO GEO]
execs = /mnt/SDCARD/.simplemenu/launchers/fbalpha2012_neogeo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_libretro,/mnt/SDCARD/.simplemenu/launchers/fb_32b_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha2012_neogeo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbalpha_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/fbneo_standalone
romDirs = /mnt/SDCARD/Roms/NEOGEO/
romExts = .zip,.7z
aliasFile = /mnt/SDCARD/.simplemenu/alias.txt

[NEO GEO CD]
execs = /mnt/SDCARD/.simplemenu/launchers/neocd_libretro
romDirs = /mnt/SDCARD/Roms/NEOCD/
romExts = .zip,.chd

[NEO GEO POCKET]
execs = /mnt/SDCARD/.simplemenu/launchers/race_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_ngp_libretro
romDirs = /mnt/SDCARD/Roms/NGP/
romExts = .ngp,.ngc,.ngpc,.npc,.zip,.7z
scaling = 1

[NES]
execs = /mnt/SDCARD/.simplemenu/launchers/fceumm_libretro,/mnt/SDCARD/.simplemenu/launchers/nestopia_libretro,/mnt/SDCARD/.simplemenu/launchers/nestopia_miyoo_libretro
romDirs = /mnt/SDCARD/Roms/FC/
romExts = .nes,.zip,.7z

[MSU-1]
execs = /mnt/SDCARD/.simplemenu/launchers/msu1_libretro
romDirs = /mnt/SDCARD/Roms/MSU1/
romExts = .smc,.sfc,.zip,.7z
aliasFile = /mnt/SDCARD/.simplemenu/alias_MSU-1.txt

[MSU-MD]
execs = /mnt/SDCARD/.simplemenu/launchers/MSU-MD_libretro
romDirs = /mnt/SDCARD/Roms/MSUMD/
romExts = .md
aliasFile = /mnt/SDCARD/.simplemenu/alias_MSU-MD.txt

[MSX]
execs = /mnt/SDCARD/.simplemenu/launchers/bluemsx_libretro,/mnt/SDCARD/.simplemenu/launchers/fmsx_libretro
romDirs = /mnt/SDCARD/Roms/MSX/
romExts = .rom,.ri,.mx1,.mx2,.col,.dsk,.cas,.sg,.sc,.m3u,.zip,.7z

[OPENBOR]
execs = /mnt/SDCARD/.simplemenu/launchers/openbor_standalone
romDirs = /mnt/SDCARD/Roms/OPENBOR/
romExts = .pak

[OVERLAYS]
execs = #
romDirs = /mnt/SDCARD/.simplemenu/overlays/
romExts = .sh,.fgl

[PC98]
execs = /mnt/SDCARD/.simplemenu/launchers/nekop2_libretro,/mnt/SDCARD/.simplemenu/launchers/np2kai_libretro
romDirs = /mnt/SDCARD/Roms/PC98/
romExts = .hdi,.fdi

[PC ENGINE]
execs = /mnt/SDCARD/.simplemenu/launchers/mednafen_pce_fast_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_pce_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_supergrafx_libretro
romDirs = /mnt/SDCARD/Roms/PCE/
romExts = .pce,.tg16,.cue,.zip,.7z

[PC ENGINE CD]
execs = /mnt/SDCARD/.simplemenu/launchers/mednafen_pce_fast_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_pce_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_supergrafx_libretro
romDirs = /mnt/SDCARD/Roms/PCECD/
romExts = .pce,.tg16,.cue,.chd,.zip,.7z

[PICO-8]
execs = /mnt/SDCARD/.simplemenu/launchers/pico_standalone,/mnt/SDCARD/.simplemenu/launchers/fake08_libretro,/mnt/SDCARD/.simplemenu/launchers/retro8_libretro
romDirs = /mnt/SDCARD/Roms/PICO/
romExts = .png,.p8,.p8.png
aliasFile = /mnt/SDCARD/.simplemenu/alias_PICO-8.txt

[PLAYSTATION]
execs = /mnt/SDCARD/.simplemenu/launchers/pcsx_standalone,/mnt/SDCARD/.simplemenu/launchers/pcsx_rearmed_miyoo_libretro,/mnt/SDCARD/.simplemenu/launchers/pcsx_rearmed_libretro,/mnt/SDCARD/.simplemenu/launchers/pcsx_rearmed_libretro_old,/mnt/SDCARD/.simplemenu/launchers/pcsx_rearmed_libretro_shaun
romDirs = /mnt/SDCARD/Roms/PS/
romExts = .pbp,.chd,.zip,.cue,.img,.iso,.m3u,.mdf

[POKEMON MINI]
execs = /mnt/SDCARD/.simplemenu/launchers/pokemini_libretro
romDirs = /mnt/SDCARD/Roms/POKE/
romExts = .min,.zip,.7z
scaling = 1

[QUAKE]
execs = /mnt/SDCARD/.simplemenu/launchers/tyrquake_libretro
romDirs = /mnt/SDCARD/Roms/QUAKE/id1/,/mnt/SDCARD/Roms/QUAKE/hipnotic/,/mnt/SDCARD/Roms/QUAKE/rogue/,/mnt/SDCARD/Roms/QUAKE/dopa/
romExts = .fbl

[SCUMMVM]
execs = /mnt/SDCARD/.simplemenu/launchers/scummvm_libretro
romDirs = /mnt/SDCARD/Roms/SCUMMVM/
romExts = .scummvm

[SEGA 32X]
execs = /mnt/SDCARD/.simplemenu/launchers/picodrive_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/THIRTYTWOX/
romExts = .zip,.32x,.7z

[SEGA CD]
execs = /mnt/SDCARD/.simplemenu/launchers/picodrive_libretro,/mnt/SDCARD/.simplemenu/launchers/genesis_plus_gx_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/SEGACD/
romExts = .bin,.chd,.cue

[SEGA GENESIS]
execs = /mnt/SDCARD/.simplemenu/launchers/picodrive_libretro,/mnt/SDCARD/.simplemenu/launchers/genesis_plus_gx_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/MD/
romExts = .zip,.bin,.smd,.md,.mdx,.gen,.7z

[SEGA SG-1000]
execs = /mnt/SDCARD/.simplemenu/launchers/gearsystem_libretro,/mnt/SDCARD/.simplemenu/launchers/genesis_plus_gx_libretro,/mnt/SDCARD/.simplemenu/launchers/picodrive_standalone
romDirs = /mnt/SDCARD/Roms/SEGASGONE/
romExts = .zip,.sg,.7z

[SGB]
execs = /mnt/SDCARD/.simplemenu/launchers/msgb_libretro,/mnt/SDCARD/.simplemenu/launchers/tgbdual_libretro
romDirs = /mnt/SDCARD/Roms/SGB/
romExts = .gb,.gbc,.zip,.7z

[SNES]
execs = /mnt/SDCARD/.simplemenu/launchers/snes9x2005_plus_libretro,/mnt/SDCARD/.simplemenu/launchers/snes9x2005_libretro,/mnt/SDCARD/.simplemenu/launchers/snes9x2002_libretro,/mnt/SDCARD/.simplemenu/launchers/snes9x2010_libretro,/mnt/SDCARD/.simplemenu/launchers/snes9x_libretro,/mnt/SDCARD/.simplemenu/launchers/snes9x_next_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_supafaust_plus_libretro,/mnt/SDCARD/.simplemenu/launchers/mednafen_supafaust_libretro,/mnt/SDCARD/.simplemenu/launchers/pocketsnes_standalone
romDirs = /mnt/SDCARD/Roms/SFC/
romExts = .smc,.sfc,.zip,.7z

[SUPERVISION]
execs = /mnt/SDCARD/.simplemenu/launchers/potator_libretro
romDirs = /mnt/SDCARD/Roms/SUPERVISION/
romExts = .bin,.sv,.zip,.7z
scaling = 1

[TIC-80]
execs = /mnt/SDCARD/.simplemenu/launchers/tic80_libretro
romDirs = /mnt/SDCARD/Roms/TIC/
romExts = .tic,.7z

[VIRTUAL BOY]
execs = /mnt/SDCARD/.simplemenu/launchers/mednafen_vb_libretro
romDirs = /mnt/SDCARD/Roms/VB/
romExts = .vb,.vboy,.bin,.zip,.7z

[WOLF3D]
execs = /mnt/SDCARD/.simplemenu/launchers/ecwolf_libretro
romDirs = /mnt/SDCARD/Roms/WOLF3D/
romExts = .ecwolf

[WONDERSWAN]
execs = /mnt/SDCARD/.simplemenu/launchers/mednafen_wswan_libretro
romDirs = /mnt/SDCARD/Roms/WS/
romExts = .ws,.wsc,.pc2,.zip,.7z
scaling = 1

[X68000]
execs = /mnt/SDCARD/.simplemenu/launchers/px68k_libretro,/mnt/SDCARD/.simplemenu/launchers/px68k_miyoo_libretro
romDirs = /mnt/SDCARD/Roms/X68000/
romExts = .dim,.zip,.img,.d88,.88d,.hdm,.dup,.2hd,.xdf,.hdf,.cmd,.m3u,.7z

[ZX SPECTRUM]
execs = /mnt/SDCARD/.simplemenu/launchers/fuse_libretro
romDirs = /mnt/SDCARD/Roms/ZXS/
romExts = .tzx,.tap,.z80,.rzx,.scl,.trd,.dsk,.zip,.7z
EOF