#ifndef input
#define input
int pollEvent();
int getEventType();
int getPressedKey();
int getKeyDown();
int getKeyUp();
void enableKeyRepeat();
void initializeKeys();
void pumpEvents();
#endif
