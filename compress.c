#include "compress.h"
#include "a2methods.h"
#include "pnm.h"
#include <stdbool.h>
#include <stdint.h>
#include "a2plain.h"
#include "calculation.h"
#include "uarray2.h"
#include "assert.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include "readImage.h"
#include "wordPack.h"

/**     compress 
 * compresses when queued with -c
 */
void compress(FILE *input){
    assert(input);

    //Reads file as a UArray2_T of float values in component form
    UArray2_T temp1 = readAsComponent(input);

    UArray2_T temp2 = blocksToWords(temp1);

    UArray2_T temp3 = cosignTrans(temp2);

    printCompressed(temp3);

    UArray2_free(&temp1);
    UArray2_free(&temp2);
    //temp3 is free'd at printCompressed(temp3);
    return;
}

/**     decompress 
 * decompresses when queued with -d
 */
void decompress(FILE *input){
    //Given code
    assert(input);
    unsigned height, width;
    int read = fscanf(input, "Compressed image format 2\n%u %u", &width, &height);
    assert(read == 2);
    int c = getc(input);
    assert(c =='\n');

    //Create array here
    UArray2_T wordHolder = readCompressed(input, width, height);

    //Component values
    UArray2_T comp = toComponentVideo(wordHolder);

    //Pnm_rgb values 
    UArray2_T pixArray = componentToRGB(comp);

    A2Methods_T methods = array2_methods_plain;
    struct Pnm_ppm pixmap = 
    { 
    .width = width*2, 
    .height = height*2, 
    .denominator = 255, 
    .pixels = pixArray,
    .methods = methods

    };
    Pnm_ppmwrite(stdout, &pixmap);
    UArray2_free(&wordHolder);
    UArray2_free(&comp);
    UArray2_free(&pixArray);
}