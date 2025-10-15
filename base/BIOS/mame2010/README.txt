*******************************
 PRECOMPILED DATA HEADER FILES
*******************************

The .h header files in this folder have been pre-processed by bin2c, which can be found within /src/tools/bin2c.

Perhaps you have changed the hiscore.dat file or would like to change the mame.ini boilerplate. Compile fresh versions of these headers by passing the parameter BUILD_BIN2C=1 to make as a commandline argument, like this:
    make BUILD_BIN2C=1