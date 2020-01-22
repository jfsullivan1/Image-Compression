#include <stdlib.h>

#include <a2plain.h>
#include "uarray2.h"
#include "uarray2b.h"

// define a private version of each function in A2Methods_T that we implement
// Code adapted from a2blocked.c provided by Noah Daniels

typedef A2Methods_Array2 A2; // private abbreviation

static A2 new(int width, int height, int size) {
    return UArray2_new(width, height, size);
}

static A2 new_with_blocksize(int width, int height, int size,
                                        int blocksize) {
    (void) blocksize;
    return UArray2_new(width, height, size);
}

// Frees the pointer to the array
static void a2free (A2 *array2p) {
    UArray2_free((UArray2_T *) array2p);
}


static int width  (A2 array2) { return UArray2_width  (array2); }
static int height (A2 array2) { return UArray2_height (array2); }
static int size   (A2 array2) { return UArray2_size   (array2); }

// Blocksize is 1 since UArray2 does not utilize blocked arrays
static int blocksize(A2 array2) {
    (void) array2;
    return 1;
}

static A2Methods_Object *at(A2 array2, int x, int y){
    return UArray2_at(array2, x, y);
}

typedef void applyfun(int i, int j, UArray2b_T array2b, void *elem, void *cl);
// From a2methods.h
//typedef void A2Methods_applyfun(int i, int j, A2 array2, A2Methods_Object *ptr, void *cl);

static void map_row_major(A2 array2, A2Methods_applyfun apply, void *cl) {
    UArray2_map_row_major(array2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2 array2, A2Methods_applyfun apply, void *cl) {
    UArray2_map_col_major(array2, (UArray2_applyfun*)apply, cl);
}

// VOID:
//static void map_block_major(A2 array2, A2Methods_applyfun apply, void *cl);

/*
// We think row major usually has the best locality2, so we set that as default
static void map_default(A2 array2, A2Methods_applyfun apply, void *cl) {        
    UArray2_map_row_major(array2, (UArray2_applyfun*)apply, cl);
}
*/
// now create the private struct containing pointers to the functions
static struct A2Methods_T array2_methods_plain_struct = {
  new,
  new_with_blocksize,
  a2free,
  width,
  height,
  size,
  blocksize,
  at,
  map_row_major,
  map_col_major,
  NULL, // map_block_major, null because not implemented
  map_row_major
};

// finally the payoff: here is the exported pointer to the struct
A2Methods_T array2_methods_plain = &array2_methods_plain_struct;