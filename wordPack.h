#ifndef WORDPACK_INCLUDED
#define WORDPACK_INCLUDED
#include <stdbool.h>
#include <stdint.h>
#include "a2methods.h"
#include "a2plain.h"
#include "pnm.h"
#include "uarray2.h"

//Used in compression

/** Receives an UArray2_T holding "struct vidComponents" and turns into "struct wordParts"
 * @param  source defined in uarray.h
 */
UArray2_T   blocksToWords(UArray2_T source);    

/** Recieves an UArray2_T holding "struct wordParts" and performs cosTrans on them producing an array 1/2 the size
 * @param source defined in uarray.h
 */       
UArray2_T   cosignTrans(UArray2_T source);   

/** Recieves an UArray2_T holding "struct wordParts" and writes them to disk
 * @param source defined in uarray.h
 */         
void        printCompressed(UArray2_T source);        

//Used in decompression

/** Read words from disk and creates words
 * @param inFile defined in <stdint.h> and @param w, @param h used for dimensions
 */
UArray2_T   readCompressed(FILE *inFile, int w, int h);

/** Recevies an UArray2_T holding "struct wordParts" and turns them into component parts
 * @param source defined in uarray2.c
 */
UArray2_T   toComponentVideo(UArray2_T source);  
#endif