![koriki](images/koriki_logo.png)

# Koriki

Koriki is a software compilation for the microSD card slot of [Miyoo Mini](https://lemiyoo.cn/product/143.html) retro console. It runs over stock firmware and brings mainly the SimpleMenu frontend to this device.

In this repository you will find all the software pieces to build the compilation, but if you only want to use one of the builds published in [releases](https://github.com/Rparadise-Team/Koriki/releases), see the documentation on the [wiki](https://github.com/Rparadise-Team/Koriki/wiki) for the installation procedure.

## Components

* SimpleMenu
    * Build: invoker.dge     #### Remove calls to opkrun and delete from Koriki/bin
    * Build: simplemenu             make PLATFORM=MMIYOO MM_NOQUIT=1 NOLOADING=1
* App/Bootscreen Selector
    * Build: bootScreenSelector     make
* App/DinguxCommander:
    * Build: DinguxCommander
* App/Gmu:
    * Build: gmu.bin
* App/System Info
    * Build: systemInfo             make
* Koriki/bin:
    * audioserver.mod by Eggs
    * Build: charging               make
    * Build: keymon                 make
    * Build: show
* Retroarch:
    * Build: retroarch
* Koriki/version.txt
* .deletes
* .update_splash.png

## Telegram channel for updates

Join this Telegram channel to get update notifications: ####
