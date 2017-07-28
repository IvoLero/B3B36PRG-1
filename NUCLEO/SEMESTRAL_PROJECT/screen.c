#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <png.h>

#include "screen.h"
#include "xwin_sdl.h"




void saveScreenPPM(image* img){
	FILE* fd = fopen("tempscreen.ppm", "wb");
	if(!fd) exit(1);

	fprintf(fd, "P6 %d %d %d\n", img->W, img->H, RGB_MAX);
	fwrite(img->data, 3, img->H *img->W, fd);
	fclose(fd);
}

void saveScreenPNG (image *img)
{
    FILE * fd = fopen("tempscreen.png","wb");
    if (!fd) exit(1);


    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    int depth = 8;
    

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) goto png_create_write_struct_failed;

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) goto png_create_info_struct_failed;
    
    if (setjmp (png_jmpbuf (png_ptr)))goto png_failure;
   

    //image attributes
    png_set_IHDR (png_ptr,
                  info_ptr,
                  img->W,
                  img->H,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */

    row_pointers = png_malloc (png_ptr, img->H * sizeof (png_byte *));
    for (y = 0; y < img->H; y++) {
        png_byte *row = png_malloc (png_ptr, img->W * sizeof(pix));
        row_pointers[y] = row;
        for (x = 0; x < img->W; x++) {
            pix pixel = img->data[img->W *y +x];
            *row++ = pixel.r;
            *row++ = pixel.g;
            *row++ = pixel.b;
        }
    }
    
    /* Write the image data to "fp". */

    png_init_io (png_ptr, fd);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */

    
    for (y = 0; y < img->H; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
    png_failure:
    png_create_info_struct_failed:
    	png_destroy_write_struct (&png_ptr, &info_ptr);
    png_create_write_struct_failed:
    	fclose (fd);
}


void repaintScreen(image*d){
	
	xwin_redraw(d->W,d->H, (unsigned char*) d->data);
}
void closeImage(){
	xwin_close();
}
void createImage(int w, int h){
	xwin_init(w,h);	//w x h window
}


image* initImage(int WIDTH, int HEIGHT,  int chunksize){

	//uint16_t** d = (uint16_t**) malloc(HEIGHT * sizeof(uint16_t*));
	pix* d = (pix*) malloc(HEIGHT*WIDTH*sizeof (pix));

	image* img =(image*)malloc(sizeof(image));
	img->chunksinrow = (uint8_t)(WIDTH%chunksize==0)? WIDTH/chunksize : WIDTH/chunksize +1;
	img->nbr_chunks = (uint8_t) ((HEIGHT%chunksize==0)? HEIGHT/chunksize : HEIGHT/chunksize+1)*img->chunksinrow;
	img->H = HEIGHT; img->W = WIDTH; img->chunksize=chunksize;  img->data =d; img->refresh_require =false;
	createImage(img->W, img->H);
	//initchunks(img);
	return img;
}
/*void initchunks (image* img){
	//int nbr_chunks = img->W *img->H/(chunksize*chunksize);
	img->data.chunks= (chunk*) malloc(sizeof (chunk) * img->nbr_chunks);
	for (int i=0;i<nbr_chunks;i++){
		pix* content =(pix*) malloc(sizeof (pix) * img->chunksize * img->chunksize);
		img->data.chunks[i] = {.cid =i, .H=img->chunksize, .W=img->chunksize, .data = content};
	}
}

image* colourImage(image* img, uint8_t r, uint8_t g, uint8_t b){
	for (int i=0;i<img->nbr_chunks;i++){
		for (int j=0; j < img->chunksize*img->chunksize; j++){
			img->data.chunks[i]->data[j]= (pix) {.r=r, .g=g,.b=b};
		}
	}
	repaintScreen(img);
	return img;
}
image* colourPixel(image* img, uint8_t cid, uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b){
	
	if (cid >= img->nbr_chunks){
		fprintf(stderr, "Chunk is out of grid\n");
		return NULL;
	if (x<0 || y<0 || x>=img->chunksize || y>=img->chunksize){
		fprintf(stderr, "Pixel is out of chunk size\n"
		return NULL;
	}

	img->data.chunks[cid]->data[img->chunksize*y +x]=(pix){.r=r,.g=g,.b=b};
	repaintScreen(img);
	return img;
}*/

	


image* setBlackScreen(image* d){
	for (int x = 0; x< d->H * d->W;x++){
		d->data[x] = (pix) {.r=0,.g=0, .b=0};
	}
	repaintScreen(d);
	return d;
}

uint8_t compute(double cR, double cI, double px, double py, uint8_t n){

	double newR,newI, oldR,oldI;
	uint8_t counter =0;	
	newR= px; newI= py;

	while (newR*newR + newI *newI < 4 && counter < n){
			oldR = newR;
		oldI = newI;
		newR = oldR *oldR - oldI* oldI + cR;
		newI = 2* oldR *oldI + cI;
		counter++;
	}
	//printf("Compute pixel: %f %f %f %f %u %u \n", px,py,cR,cI,counter,n);
	return counter;
}

image* setPixelbyIter(image* img, uint8_t cid,int  x, int y, uint8_t iter,uint8_t n){
	double t = (double) iter/(double)n;
	double R = 9 * (1-t) * t*t*t *255;
	double G = 15 * (1-t) * (1-t) * t*t *255;
	double B = 8.5 * (1-t) * (1-t) * (1-t) * t *255;
	return setPixel(img, cid, x, y, (uint8_t)R, (uint8_t) G, (uint8_t) B);
}



image* setPixel(image* img, uint8_t cid,int  x, int y, uint8_t r, uint8_t g, uint8_t b){
	if(!img) exit(1);
	if (cid <0 || cid>= img->nbr_chunks){
		fprintf (stderr, "ERROR: Invalid chunkid -setPixel()\n");
		return NULL;
	}
	
	if(x > img->W|| x<0|| y> img->H|| y<0 ){
		fprintf(stderr, "ERROR: Wrong dim - CPU setPixel()\n");
		return NULL;
	}
	img->data[y* img->W +x].r = r;
	img->data[y* img->W +x].g = g;
	img->data[y* img->W +x].b = b;
	//printf("Pixel num: %d \n",img->W*y+x);
	return img;
}

void freeImage(image* img){
	free(img->data);
	free(img);
}




