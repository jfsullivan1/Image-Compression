#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include "uarray2.h"
#include "a2methods.h"
#include "a2plain.h"
#include "pnm.h"

/**     fileRead
 * Reads a FILE* and reuturns a Pnm_ppm
 */
Pnm_ppm fileRead(FILE *inFile){ 

    A2Methods_T methods = array2_methods_plain;
    Pnm_ppm source = Pnm_ppmread(inFile, methods);
    
    return source;
}

