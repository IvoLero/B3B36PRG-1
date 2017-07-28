#ifndef SCREEN_H_

#include "xwin_sdl.h"
#include <stdbool.h>
#define SCREEN_H_


#define RGB_MAX 255
//unsigned char* initScreen();


typedef struct {
	uint8_t r;
	uint8_t g; 
	uint8_t b;
} pix;

typedef struct{
	uint8_t H, W,cid;
	pix*data;
} chunk;



typedef struct {
	int H;	//image height
	int W;	//image width
	pix* data;
	uint8_t nbr_chunks;
	uint8_t chunksinrow;
	uint8_t chunksize;
	bool refresh_require;

} image;



void saveScreenPPM (image* image);
void saveScreenPNG (image* image);

image* initImage(int height, int width,  int chunk_size);
//image* colourImage(image* image, uint8_t r, uint8_t g, uint8_t b);
//image* colourPixel(image* image, uint8_t cid, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void repaintScreen(image* img);
void closeImage();
uint8_t compute(double cR, double cI, double px, double py, uint8_t n);
void freeImage(image* img);


image* setBlackScreen(image* dis);
image* setPixel(image* dis, uint8_t cid, int x, int y, uint8_t r, uint8_t g, uint8_t b);
image* setPixelbyIter(image* dis, uint8_t cid, int x, int y, uint8_t iter,uint8_t n);


#endif
