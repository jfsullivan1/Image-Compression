#ifndef CAlCULATION_INCLUDED
#define CAlCULATION_INCLUDED
#include <stdbool.h>
#include <stdint.h>
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "uarray2.h"

//Used in compression

/* toComponentVideo this function takes rgb pixels and converts them to video component pixels. 
    * @param source is the source array that it will be converting pixels from. 
*/
UArray2_T   toComponentVideo(UArray2_T source);

/* readAsComponent this will read the RGB values as video component values rather than having to take
    * the extra step of converting them.
    * @param *inFile is the source image that it will be converting pixels from. 
*/
UArray2_T   readAsComponent(FILE *inFile);

/* trimEven this function will ensure that the dimensions of the image are both even (even# x even#) 
    * by trimming off an extra row or column to make the edges even. 
    * @param source is the source array that it will be trimming.
*/
Pnm_ppm     trimEven(Pnm_ppm source);

//Used in decopression
/* componentToRGB this will take video component values and convert them back to RGB pixels. 
    * @param source is the source array that it will be performing calculations on. 
*/
UArray2_T   componentToRGB(UArray2_T source);

/* RGBToPnm this will take a source RGB image and convert it to ppm format for printing. 
    * @param source is the source array that it will be converting and printing. 
*/
Pnm_ppm     RGBToPnm(UArray2_T source);

/* printDecompressed this will take a source ppm image (decompressed) and print it to stdout.
    * @param source is the source array that it will be printing.
*/
void        printDecompressed(Pnm_ppm source);

#endif