![KORIKI GITHUB REPO](https://github.com/user-attachments/assets/6d740106-5d96-4131-8cfa-c01191d2fad5)


# Koriki

Koriki is a software compilation for the microSD card slot of [Miyoo Mini](https://lemiyoo.cn/product/143.html) retro console. It runs over stock firmware and brings mainly the [SimpleMenu](https://github.com/fgl82/simplemenu) frontend to this device.

In this repository you will find all the software pieces to build the compilation, but if you only want to use one of the builds published in [releases](https://github.com/Rparadise-Team/Koriki/releases), see the documentation on the [wiki](https://github.com/Rparadise-Team/Koriki/wiki) for the installation procedure.

Although more details are given later in the [wiki](https://github.com/Rparadise-Team/Koriki/wiki), like all free software projects, Koriki rely on the work of many other people. The complete list would be endless, but we can not avoid to mention some names that are the most directly have allowed the existence of Koriki:

* @FGL82: For his practical, agile and customizable SimpleMenu frontend.
* @trngaje: For his SimpleMenu fork for Miyoo Mini that triggered Koriki's idea.
* [Eggs](https://discordapp.com/users/778867980096241715): For its mod of RetroArch for the MiyooMini as well as some system mods such as the sound latency reduction system.
* @shauninman: For its dockerized toolchain and [minimalist distribution](https://github.com/shauninman/MiniUI) concept which has largely inspired Koriki's design.

## Components

The base of the distribution is a series of static files that can be found in the `base` directory of this repository. Over this base we have built/installed a series of projects that we will detail later. The result, i.e. the base plus the binaries, must be copied to a microSD card in FAT32 format with empty label.

Once the binaries are in place, the `generate_release.sh` script allows to generate a zip with the complete distribution (which also does not include the `.gitignore` files in some empty directories) which will be used to update an already installed version of Koriki just by copying the resulting file to the root of the card. The file name has to start with `update_koriki_` to trigger the update procedure during Koriki startup. This system can also be used to apply patches to fix only some files.

Listed below are the binaries that you need to build if you want to construct the distribution yourself, as well as the location of their sources. The path to the location indicated for the binaries starts from the root of the microSD card.

The toolchain of @shauninman has been used to build, that can be found [here](https://github.com/shauninman/union-miyoomini-toolchain).

#### SimpleMenu

* Sources: https://github.com/Rparadise-Team/simplemenu/tree/mmiyoo
* Destination of binary: `.simplemenu/simplemenu`
* Observations: To compile SimpleMenu, we must have previously compiled and installed the following libraries in our toolchain:
    * libini: https://github.com/pcercuei/libini
    * libopk: https://github.com/pcercuei/libopk
* Build command: `make PLATFORM=MMIYOO MM_NOQUIT=1 NOLOADING=1`

#### invoker

* Sources: https://github.com/fgl82/invoker
* Destination of binary: `.simplemenu/invoker.dge`

#### RetroArch

* Sources: https://github.com/libretro/RetroArch/releases/tag/v1.10.3
* Diff patch: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AAALUUX82Tk4USboNHAUBxDNa/RetroArch_Dingux_forMiyooMini_220525.zip?dl=0
* Destination of binary: `RetroArch/retroarch`
* You can download the updated cores at: https://rparadise-team.github.io/Koriki/

#### audioserver (for Miyoo Mini, not for Miyoo Mini+)

* Sources: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AADMQJa8jBJKJw1_RBxLZxFNa/latency_reduction.zip?dl=0
* Destination of binary: `Koriki/bin/audioserver.min`

#### DinguxCommander

* Sources: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AAAaZPs2Dh8gMiYT8c2U71qea/Commander_Italic_rev4.zip?dl=0
* Destination of binary: `App/Commander_Italic/DinguxCommander`

#### GMU

* Sources: https://github.com/TechDevangelist/gmu
* Destination of binary: `App/Gmu/gmu.bin`

#### Bootscreen Selector

* Sources: https://github.com/Rparadise-Team/Koriki/tree/main/src/bootScreenSelector
* Destination of binary: `App/Bootscreen_Selector/bootScreenSelector`

#### System Info

* Sources: https://github.com/Rparadise-Team/Koriki/tree/main/src/systemInfo
* Destination of binary: `App/System_Info/systemInfo`

#### Charging

* Sources: https://github.com/Rparadise-Team/Koriki/tree/main/src/charging
* Destination of binary: `Koriki/bin/charging`

#### Keymon

* Sources: https://github.com/Rparadise-Team/Koriki/tree/main/src/keymon
* Destination of binary: `Koriki/bin/keymon`

#### ShowScreen

* Sources: https://github.com/Rparadise-Team/Koriki/tree/main/src/showScreen
* Destination of binary: `Koriki/bin/show`

## Telegram channel for updates

Join this Telegram channel to get update notifications: [https://t.me/Koriki_MiyooMini](https://t.me/Koriki_MiyooMini)
