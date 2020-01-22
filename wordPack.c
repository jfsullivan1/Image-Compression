#include "a2methods.h"
#include "pnm.h"
#include <stdbool.h>
#include <stdint.h>
#include "a2plain.h"
#include "uarray2.h"
#include "assert.h"
#include "readImage.h"
#include "bitpack.h"
#include <math.h>
#include "arith411.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include "sharedGlobals.h"

/*____________________________________________________________________________*
 *                              Local Structures                              *
 *____________________________________________________________________________*/

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

/**     vidComponent 
 * Stores video component format
 */
struct vidComponent {
    float Y;
    float Pb;
    float Pr;
}; typedef struct vidComponent vid;

/*____________________________________________________________________________*
 *                              Local Helper Function                         *
 *____________________________________________________________________________*/

/**     writeWord
 * Writes a single word to disk
 * For use in printCompressed 
 */
void writeWord(wordPartsS blocks){ 
    //Initialize the word to write
    uint64_t wordToDisk = 0;

    //Write the information into the word
    wordToDisk = Bitpack_newu(wordToDisk, BITSOFPR, LSBOFPR, (uint64_t)blocks.Pr);
    wordToDisk = Bitpack_newu(wordToDisk, BITSOFPB, LSBOFPB, (uint64_t)blocks.Pb);
    wordToDisk = Bitpack_news(wordToDisk, BITSOFD, LSBOFD, (int64_t)blocks.d);
    wordToDisk = Bitpack_news(wordToDisk, BITSOFC, LSBOFC, (int64_t)blocks.c);
    wordToDisk = Bitpack_news(wordToDisk, BITSOFB, LSBOFB, (int64_t)blocks.b);
    wordToDisk = Bitpack_newu(wordToDisk, BITSOFA, LSBOFA, (uint64_t)blocks.a);

    //Write one byte to disk at a time
    putchar(Bitpack_getu(wordToDisk, 8, 0));
    putchar(Bitpack_getu(wordToDisk, 8, 8));
    putchar(Bitpack_getu(wordToDisk, 8, 16));
    putchar(Bitpack_getu(wordToDisk, 8, 24));

    return;
}

/**     readWord
 * Reads a single block to disk
 * For use in readCompressed 
 */
wordPartsS readWord(FILE * inFile){ 
    assert(inFile);
    wordPartsS temp;
    uint64_t word = 0;

    word = Bitpack_newu(word, 8, 0, (uint64_t)getc(inFile));
    word = Bitpack_newu(word, 8, 8, (uint64_t)getc(inFile));
    word = Bitpack_newu(word, 8, 16, (uint64_t)getc(inFile));
    word = Bitpack_newu(word, 8, 24, (uint64_t)getc(inFile));
    temp.Pr = Bitpack_getu(word, BITSOFPR, LSBOFPR);
    temp.Pb = Bitpack_getu(word, BITSOFPB, LSBOFPB);
    temp.d = Bitpack_gets(word, BITSOFD, LSBOFD);
    temp.c = Bitpack_gets(word, BITSOFC, LSBOFC);
    temp.b = Bitpack_gets(word, BITSOFB, LSBOFB);
    temp.a = Bitpack_getu(word, BITSOFA, LSBOFA);
    return temp;
}

/**     idxCosine 
 *  For use in cosine transformation 
 */
int idxCosine(float i){
    if(i >= MAXN)
        return MAXX;
    if(i <= -MAXN)
        return -MAXX;
    return(int)(COFB * i);
}

/**     idxCosineInv 
 * For use in cosine detransformation 
 * assert(idxCosineInv(idxCosine(.25)) == .25);
 */
float idxCosineInv(int i){
    float i_ = (float)i;
    float den = 50;
    if(i > MAXX)
        return MAXN;
    if(i < -MAXX)
        return -MAXN;
    return(float)(i_ / den);
}

/*____________________________________________________________________________*
 *                         Interface Implmentations (Compress)                *
 *___________________________________________________________________________*/

/**     blocksToWords 
 * Turns an array of w and h to w/2 and h/2 with average values 
 */
UArray2_T   blocksToWords(UArray2_T source){

    //Create the 2d array which will be outputted
    UArray2_T tempArray = UArray2_new(UArray2_width(source) / 2, UArray2_height(source) / 2, sizeof(struct wordParts));
    assert(UArray2_width(source)/2 == UArray2_width(tempArray));
    assert(UArray2_height(source)/2 == UArray2_height(tempArray));

    //
    for(int i = 0; i < UArray2_height(source); i+=2){
        for(int j = 0; j < UArray2_width(source); j+=2){ 

            //Get an component block
            vid *at1 = UArray2_at(source, j, i);
            vid *at2 = UArray2_at(source, j+1, i);
            vid *at3 = UArray2_at(source, j, i+1);
            vid *at4 = UArray2_at(source, j+1, i+1);
            assert(at1 && at2 && at3 && at4);

            //Get average Pb and Pr values
            float averagePb = (at1->Pb + at2->Pb + at3->Pb + at4->Pb) / 4.0;
            float averagePr = (at1->Pr + at2->Pr + at3->Pr + at4->Pr) / 4.0;

            //Compute a,b,c and d
            float a = (at1->Y + at2->Y + at3->Y + at4->Y) / 4.0;
            float b = (at1->Y + at2->Y - at3->Y - at4->Y) / 4.0;
            float c = (at1->Y - at2->Y + at3->Y - at4->Y) / 4.0;
            float d = (at1->Y - at2->Y - at3->Y + at4->Y) / 4.0;

            //Make word
            wordParts word; 
            word.a = a;
            word.b = b; 
            word.c = c; 
            word.d = d;
            word.Pb = averagePb;
            word.Pr = averagePr;

            //Insert into the outputted array
            wordParts *wordIn = UArray2_at(tempArray, j/2, i/2);
            assert(wordIn);

            *wordIn = word;
        }
    }
    return tempArray;
}

/**     cosignTrans 
 * Uses cosign tranformation on an array of blocks
 */
UArray2_T   cosignTrans(UArray2_T source){
     //Create the 2d array which will be outputted
    UArray2_T tempArray = UArray2_new(UArray2_width(source), UArray2_height(source), sizeof(struct wordPartsSigned));
    assert(UArray2_width(source) == UArray2_width(tempArray));
    assert(UArray2_height(source) == UArray2_height(tempArray));

    for(int i = 0; i < UArray2_height(source); i++){
        for(int j = 0; j < UArray2_width(source); j++){ 
            
            //Get information from source
            wordParts *at = UArray2_at(source, j, i);
            wordPartsS *atNew = UArray2_at(tempArray, j, i);
            assert(at);
            assert(atNew);

            //Math of cosign transformation
            wordPartsS temp;
            temp.a = (int)floor(at->a * 511);
            temp.b = (int)idxCosine(at->b);
            temp.c = (int)idxCosine(at->c);
            temp.d = (int)idxCosine(at->d);
            temp.Pb = Arith_index_of_chroma(at->Pb);
            temp.Pr = Arith_index_of_chroma(at->Pr);

            *atNew = temp;

        }
    }
    return tempArray;
}
/**     printCompressed
 * Prints all words to disk
 */
void        printCompressed(UArray2_T source){ 

    printf("Compressed image format 2\n%u %u", UArray2_width(source), UArray2_height(source));
    printf("\n");
    for(int i = 0; i < UArray2_height(source); i++){
        for(int j = 0; j < UArray2_width(source); j++){ 

            //Take and write the word
            wordPartsS *temp = UArray2_at(source, j, i);
            writeWord(*temp);

        }
    }
    UArray2_free(&source);
    return;
}

/*____________________________________________________________________________*
 *                         Interface Implmentations (Decompress)              *
 *____________________________________________________________________________*/

/**     readCompressed 
 * Read the compressed word and return the information using readWord()
 */
UArray2_T   readCompressed(FILE *inFile, int w, int h){ 

    UArray2_T words = UArray2_new(w, h, sizeof(wordPartsS));

    for(int i = 0; i < h; i++){ 
        for(int j = 0; j < w; j++){

            //Read word and store
            wordPartsS * temp = UArray2_at(words, j, i);
            *temp = readWord(inFile);

        }
    }
    return words;
}

/**      toComponentVideo 
 * Changes the word information to component format
 */
UArray2_T toComponentVideo(UArray2_T source){ 

    UArray2_T component = UArray2_new(UArray2_width(source)*2, UArray2_height(source)*2, sizeof(struct vidComponent));

    for(int i = 0; i < UArray2_height(source)*2; i+=2){ 
        for(int j = 0; j < UArray2_width(source)*2; j+=2){ 

            //Get a word struct
            wordPartsS * temp = UArray2_at(source, j/2, i/2);
            struct vidComponent * TL = UArray2_at(component, j, i); //Top left 
            struct vidComponent * TR = UArray2_at(component, j+1, i); //Top right
            struct vidComponent * BL = UArray2_at(component, j, i+1); //Bottom Left
            struct vidComponent * BR = UArray2_at(component, j+1, i+1); //Bottom Right

            //Break into letters
            float a = (float) ((float)temp->a / (float) 511);
            float b = idxCosineInv(temp->b);
            float c = idxCosineInv(temp->c);    
            float d = idxCosineInv(temp->d);    

            //Solve the y components 
            float Y1 = a - b - c + d;
            float Y2 = a - b + c - d;    
            float Y3 = a + b - c - d;
            float Y4 = a + b + c + d;

            float Pb = Arith_chroma_of_index(temp->Pb);
            float Pr = Arith_chroma_of_index(temp->Pr);

            //Set values
            TL->Y = Y1; //Top left
            TL->Pb = Pb;
            TL->Pr = Pr;

            TR->Y = Y2; //Top right
            TR->Pb = Pb;
            TR->Pr = Pr;

            BL->Y = Y3; //Bottom left
            BL->Pb = Pb;
            BL->Pr = Pr;

            BR->Y = Y4; //Bottom Right
            BR->Pb = Pb;
            BR->Pr = Pr;
            
        }
    }
    return component;
}

