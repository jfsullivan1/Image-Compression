#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "uarray2.h"
#include "a2plain.h"
#include "pnm.h"
#include "readImage.h"
#include <math.h>
#include "assert.h"
#include "calculation.h"
#include "wordPack.h"


double square(double num){ 
    return num * num;
}

float pnmDiff(Pnm_ppm image1, Pnm_ppm image2){ 

    image1 = trimEven(image1);
    image2 = trimEven(image2);


    if(abs(image1->height - image2->height) > 1 || abs(image1->width - image2->width) > 1){ 
        fprintf(stderr, "The difference is more then 1\n");
        printf("%f", 1.0);
    }

    float denominator1 = image1->denominator;
    float denominator2 = image2->denominator;

    float numerator = 0; 

    for(unsigned int i = 0; i < image1->width; i++){ 
        for(unsigned int j = 0; j < image1->height; j++){ 

            //Technically this is the Pnm_rgb, was rewritten this way to get rid of bugs
            Pnm_rgb pixel1 = UArray2_at(image1->pixels, i ,j);
            Pnm_rgb pixel2 = UArray2_at(image2->pixels, i ,j);

            //printf("Red: %f ", pow(pixel1[0] - pixel2[0], 2));
            //printf("Green: %f ", pow(pixel1[1] - pixel2[1], 2));
            //printf("Blue: %d %d\n", pixel1[2], pixel2[2]);

            numerator += square(pixel1->red/denominator1 - pixel2->red/denominator2); //Red
            numerator += square(pixel1->green/denominator1 - pixel2->green/denominator2); //Green
            numerator += square(pixel1->blue/denominator1 - pixel2->blue/denominator2); //Blue
        }
    }
    float E = sqrt(numerator / (3 * image1->height * image1->width));
    
    return E;
}

int main(int argc, char * argv[]){ 

    assert(argc == 3);
    FILE* fp = fopen(argv[1], "r");
    FILE* fp2 = fopen(argv[2], "r");
    Pnm_ppm image1 = fileRead(fp);
    Pnm_ppm image2 = fileRead(fp2);
    /*
    if(image1->height * image1->width <= image2->height * image2->width){ printf("Use image 1\n");}
    else                                                                { printf("Use image 2\n");}
    */
    //   Will make so the smaller image will always be the first argument or image1 in the pnmDiff function
    if(image1->height * image1->width <= image2->height * image2->width){ printf("%f\n", pnmDiff(image1, image2));}
    else                                                                { printf("%f\n", pnmDiff(image2, image1));}

    return 0;
}