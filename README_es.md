![koriki](images/koriki_logo.png)

# Koriki

Koriki es una recopilación de software para la tarjeta microSD de la retro consola [Miyoo Mini](https://lemiyoo.cn/product/143.html). Se ejecuta sobre el firmware stock y fundamentalmente porta el frontend [SimpleMenu](https://github.com/fgl82/simplemenu) a este dispositivo.

En este repositorio se encuentran todas las piezas software necesarias para construir la distribución, pero en caso de estar unicamente interesado en utilizar el resultado final, acudir al apartado [releases](https://github.com/Rparadise-Team/Koriki/releases) para obtener una compilación funcional de la misma. Consultar la documentación en el [wiki](https://github.com/Rparadise-Team/Koriki/wiki) para conocer el procedimiento de instalación de la distribución ya compilada.

Aunque más adelante y en el [wiki](https://github.com/Rparadise-Team/Koriki/wiki) se aportan más detalles, como todos los proyectos de software libre, Koriki se apoya en el trabajo de muchas otras personas. La lista completa sería interminable, pero no podemos dejar de mencionar algunos nombres que son los que de forma más directa han permitido la existencia de Koriki:

* @FGL82: Por su práctico, ágil, y personalizable frontend SimpleMenu.
* @trngaje: Por su fork de SimpleMenu para Miyoo Mini que prendió la idea de Koriki.
* [Eggs](https://discordapp.com/users/778867980096241715): Por su adaptación de RetroArch para la MiyooMini así como algunos mods del sistema como el sistema de reducción de latencia de sonido.
* @shauninman: Por su toolchain dockerizada y por su concepto de [distribución minimalista](https://github.com/shauninman/MiniUI) que ha inspirado en gran parte el diseño de Koriki.

## Componentes

La base de la distribución son una serie de ficheros estáticos que pueden encontrarse en el directorio `base` de este repositorio. Sobre esta base se han compilado/instalado una serie de proyectos de otras personas que más adelante detallaremos. El resultado, es decir la base más los binarios, se deben copiar sobre una tarjeta microSD en formato FAT32 sin label.

Una vez colocados los binarios, el script `generate_release.sh` permite generar un zip con la distribución completa (que además no inclye los ficheros `.gitignore` que hay en algunos directorios vacíos) que servirá para actualizar una versión ya instalada de Koriki únicamente copiando el fichero resultante a la raíz de la tarjeta. El nombre del fichero tiene que empezar por `update_koriki_` para que se dispare el procedimiento de actualización durante el arranque de Koriki. Mediante este sistema también se pueden aplicar parches para corregir únicamente algunos ficheros aislados.

A continuación se enumeran los binarios que es necesario compilar si se quiere construir la distribución por uno mismo, así como la localización de sus fuentes. La ruta de la ubicación que se indica para los binarios parte de la raíz de la tarjeta microSD.

Para compilar se ha utilizado el toolchain de @shauninman que puede encontrarse [aquí](https://github.com/shauninman/union-miyoomini-toolchain).

#### SimpleMenu

* Fuentes: https://github.com/Rparadise-Team/simplemenu/tree/mmiyoo
* Ubicación del binario final: `.simplemenu/simplemenu`
* Observaciones: Para compilar SimpleMenu, previamente deberemos haber compilado e instalado en nuestro toolchain las siguientes librerías:
    * libini: https://github.com/pcercuei/libini
    * libopk: https://github.com/pcercuei/libopk
* Comando compilación: `make PLATFORM=MMIYOO MM_NOQUIT=1 NOLOADING=1`

#### invoker

* Fuentes: https://github.com/fgl82/invoker
* Ubicación del binario final: `.simplemenu/invoker.dge`

#### RetroArch

* Fuentes: https://github.com/libretro/RetroArch/releases/tag/v1.10.3
* Diff patch: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AAALUUX82Tk4USboNHAUBxDNa/RetroArch_Dingux_forMiyooMini_220525.zip?dl=0
* Ubicación del binario final: `RetroArch/retroarch`
* puedes descargar los cores actualizados en: https://rparadise-team.github.io/Koriki/

#### audioserver (para Miyoo Mini, no para Miyoo Mini+)

* Fuentes: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AADMQJa8jBJKJw1_RBxLZxFNa/latency_reduction.zip?dl=0
* Ubicación del binario final: `Koriki/bin/audioserver.min`

#### DinguxCommander

* Fuentes: https://www.dropbox.com/sh/hqcsr1h1d7f8nr3/AAAaZPs2Dh8gMiYT8c2U71qea/Commander_Italic_rev4.zip?dl=0
* Ubicación del binario final: `App/Commander_Italic/DinguxCommander`

#### GMU

* Fuentes: https://github.com/TechDevangelist/gmu
* Ubicación del binario final: `App/Gmu/gmu.bin`

#### Bootscreen Selector

* Fuentes: https://github.com/Rparadise-Team/Koriki/tree/main/src/bootScreenSelector
* Ubicación del binario final: `App/Bootscreen_Selector/bootScreenSelector`

#### System Info

* Fuentes: https://github.com/Rparadise-Team/Koriki/tree/main/src/systemInfo
* Ubicación del binario final: `App/System_Info/systemInfo`

#### Charging

* Fuentes: https://github.com/Rparadise-Team/Koriki/tree/main/src/charging
* Ubicación del binario final: `Koriki/bin/charging`

#### Keymon

* Fuentes: https://github.com/Rparadise-Team/Koriki/tree/main/src/keymon
* Ubicación del binario final: `Koriki/bin/keymon`

#### ShowScreen

* Fuentes: https://github.com/Rparadise-Team/Koriki/tree/main/src/showScreen
* Ubicación del binario final: `Koriki/bin/show`

## Telegram channel for updates

Join this Telegram channel to get update notifications: [https://t.me/Koriki_MiyooMini](https://t.me/Koriki_MiyooMini)
