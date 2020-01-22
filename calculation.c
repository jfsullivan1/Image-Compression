#include "a2methods.h"
#include "pnm.h"
#include <stdbool.h>
#include <stdint.h>
#include "a2plain.h"
#include "calculation.h"
#include "uarray2.h"
#include "assert.h"
#include "readImage.h"
#include "bitpack.h"
#include <math.h>
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include "sharedGlobals.h"

/*____________________________________________________________________________*
 *                              Local Structures                              *
 *____________________________________________________________________________*/

/**     RGB
 * Stores RGB files
 * For use in PNM -> RGB 
 */
struct RGB { 
    float red;
    float green;
    float blue;
};

/**     vidComponent 
 * Stores video component format
 */
struct vidComponent {
    float Y;
    float Pb;
    float Pr;
}; typedef struct vidComponent vid;

/**     wordParts 
 * Stores parts of unwritten words 
 * Uses floats
 */
struct wordParts{
    float a;
    float b;
    float c;
    float d;
    float Pb;
    float Pr;
}; typedef struct wordParts wordParts;

/**     wordPartsSigned 
 * Stores parts of unwritten words
 * Uses signed ints 
 */
struct wordPartsSigned{
    int a;
    int b;
    int c;
    int d;
    unsigned Pb;
    unsigned Pr;
}; typedef struct wordPartsSigned wordPartsS;

/*____________________________________________________________________________*
 *                              Local Helper Function                         *
 *____________________________________________________________________________*/

/**     toStructRGB
 * Computes floats from RGB and denominator
 * For use in toFloat
*/
struct RGB toStructRGB(Pnm_rgb source, int denominator){ 
    assert(source && denominator);
    struct RGB temp; 
    float denominator_ = (float)denominator;
    float R = (float)source->red;
    float G = (float)source->green;
    float B = (float)source->blue;

    temp.red = R / denominator_;
    temp.green = G / denominator_;
    temp.blue = B / denominator_;


    return temp;
}

/*____________________________________________________________________________*
 *                         Interface Implmentations (Compress)                *
 *___________________________________________________________________________*/

/**     trimEven
 * Makes edges even if they are odd. Returns if the edges are already even.
 * Used in readAsComponent
 */ 
Pnm_ppm trimEven(Pnm_ppm source){
    assert(source);
    if(source->width % 2 != 0){
        source->width -= 1;
    }
    if(source->height %2 != 0){                  
        source->height -= 1;
    }
    return source;
}

/**     toFloat
 * Changes a Pnm file to float values in a UArray2_T
*/
UArray2_T toFloat(Pnm_ppm source){
    //Set methods to plain
    A2Methods_T methods = array2_methods_plain;

    //Create the 2d array which will be outputted
    UArray2_T tempArray = UArray2_new(methods->width(source), methods->height(source), sizeof(struct RGB));
    assert(sizeof(UArray2_at(tempArray, 0, 0)) == sizeof(methods->at(source, 0, 0)));

    for(int i = 0; i < methods->height(source); i++){
        for(int j = 0; j < methods->width(source);j++){ 
            //Get an rgb value from the image
            struct Pnm_rgb *tempVal = methods->at(source->pixels, j, i);
            assert(tempVal);

            //Insert into the array
            struct RGB *tempRGB = UArray2_at(tempArray, j, i);
            assert(tempRGB);

            //Store with computed values (see toStructRGB)
            *tempRGB = toStructRGB(tempVal, source->denominator);

        }
    }
    
    return tempArray;
}

/**     rgbToComponent
 * Changes RGB array to an component array
*/
UArray2_T rgbToComponent(UArray2_T source){
    //Set methods to plain
    A2Methods_T methods = array2_methods_plain;

    //Create the 2d array which will be outputted
    UArray2_T tempArray = UArray2_new(UArray2_width(source), UArray2_height(source), sizeof(struct vidComponent));
    assert(UArray2_width(source) == UArray2_width(tempArray));
    assert(UArray2_height(source) == UArray2_height(tempArray));

    for(int i = 0; i < methods->height(source); i++){
        for(int j = 0; j < methods->width(source);j++){ 
            //Get an rgb value from the image
            struct RGB *floats = UArray2_at(source, j, i);
            assert(floats);

            //Compute component values
            vid components;
            components.Y = (0.299 * floats->red) + (0.587 * floats->green) + (0.114 * floats->blue);
            components.Pb = (-0.168736 * floats->red) - (0.331264 * floats->green) + (0.5 * floats->blue);
            components.Pr = (0.5 * floats->red) - (0.418688 * floats->green) - (0.081312 * floats->blue);

            //Insert into the array
            struct vidComponent *componentVal = UArray2_at(tempArray, j, i);
            assert(componentVal);

            *componentVal = components;
        }
    }

    return tempArray;
}

/**     readAsComponent 
 * Combines above implemented functions into one 
 */
UArray2_T readAsComponent(FILE *inFile){
    //Read the file
    Pnm_ppm source = fileRead(inFile);
    
    //Resolve to even # height and width
    source = trimEven(source);
    
    //Save as float values
    UArray2_T floatValues = toFloat(source);

    //Save as comnponent values
    UArray2_T componentValues = rgbToComponent(floatValues);

    Pnm_ppmfree(&source);
    UArray2_free(&floatValues);
    return componentValues;
 }

/*____________________________________________________________________________*
 *                         Interface Implmentations (Decompress)              *
 *____________________________________________________________________________*/

/**     componentToRGB 
 * Change from component to RGB format
 */
UArray2_T componentToRGB(UArray2_T source){
    int R;
    int B;
    int G;
    struct Pnm_rgb newRGB;
    //Set methods to plain

    //Create the 2d array which will be outputted
    UArray2_T tempArray = UArray2_new(UArray2_width(source), UArray2_height(source), sizeof(struct Pnm_rgb));
    assert(UArray2_width(source) == UArray2_width(tempArray));
    assert(UArray2_height(source) == UArray2_height(tempArray));

    for(int i = 0; i < UArray2_height(source); i++){
        for(int j = 0; j < UArray2_width(source);j++){ 
            //Get a Y value from the image
            vid *floats = UArray2_at(source, j, i);
            assert(floats);

            //Compute rgb values
            R = DENOMINATOR * ( (1.0 * floats->Y) + (0 * floats->Pb)       +  (1.402 * floats->Pr) );
            R = fmax(fmin(R, DENOMINATOR), 0);
            G = DENOMINATOR * ( (1.0 * floats->Y) - (0.344136 * floats->Pb)-  (0.714136 * floats->Pr) );
            G = fmax(fmin(G, DENOMINATOR), 0);
            B = DENOMINATOR * ( (1.0 * floats->Y) + (1.772 * floats->Pb)   +  (0 * floats->Pr) );  
            B = fmax(fmin(B, DENOMINATOR), 0);   

            //Insert RGB values 
            struct Pnm_rgb *colors = UArray2_at(tempArray, j, i);
            
            newRGB.red = R;
            newRGB.green = G; 
            newRGB.blue = B;

            *colors = newRGB;      
            
        }
    }
    return tempArray;
}