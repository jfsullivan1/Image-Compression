#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "except.h"
#include "assert.h"

uint64_t shiftLeft(uint64_t word, unsigned amount){ 
    assert(amount <= 64);
    if(amount <= 63)            word = (word << amount);
    else                        word = 0;
    return word;
}
//For use in get signed function, as "right2" is not functionally correct in all situations
int64_t right1(int64_t word, unsigned amount){
    assert(amount <= 64);
    if      (amount <= 63)      word = (word >> amount);
    else if (word >= 0)         word = 0;
    else                        word = ~0;
    return word;
}

uint64_t right2(uint64_t word, unsigned amount){ 
    assert(amount <= 64);
    if      (amount <= 63)      word = (word >> amount);
    else                        word = 0;
    return word;
}

Except_T Bitpack_Overflow = { "Overflow packing bits" };

//Check to see if n will fit into w bits
bool Bitpack_fitsu(uint64_t n, unsigned width){ 

    assert(width <= 64); 
    uint64_t maxN = ~0; 
    maxN = shiftLeft(maxN, width); 
    maxN = ~maxN;

    if(n <= maxN) { 
        return true;
    }else{ 
        return false;
    }
}
//Check to see if n will fit into w bits
bool Bitpack_fitss(int64_t n, unsigned width){ 

    assert(width <= 64);

    int64_t maxN; 
    int64_t minN = ~0;
    minN = (int64_t)shiftLeft(minN, width-1); 
    maxN = ~minN;

    if(minN <= n && maxN >= n){
        return true;
    }else{ 
        return false;
    }
}

//Extracts a field from a word given the width of the field and the 
//location of the field’s least significant bit
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb){ 

    assert(width <= 64);
    assert(width + lsb <= 64);

    uint64_t buffer = ~0;
    buffer = shiftLeft(buffer, (64 - width)); 
    buffer = right2(buffer, (64 - width - lsb));
    word = (word & buffer);
    word = right2(word, lsb);

    return word;
}

//Extracts a field from a word given the width of the field and the 
//location of the field’s least significant bit
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb){ 

    assert(width <= 64);
    assert(width + lsb <= 64);

    uint64_t buffer = ~0;
    buffer = shiftLeft(buffer, (64 - width)); 
    buffer = right2(buffer, (64 - width - lsb));
    word = (word & buffer);
    word = shiftLeft(word, (64 - width - lsb));
    word = right1(word, (64 - width));

    return (int64_t)word;
}


uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value){ 

    assert(width <= 64);
    assert(width + lsb <= 64);
    if(Bitpack_fitsu(value, width)){ 
        uint64_t buffer = ~0;
        buffer = shiftLeft(buffer, (64 - width));
        buffer = right2(buffer, (64 - width - lsb));
        value = shiftLeft(value, lsb);
        value = (value & buffer);
        word = (word & ~buffer);
        word = (word | value);
    }else{ 
        RAISE(Bitpack_Overflow);
    }

    return word;
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  int64_t value){ 

    assert(width <= 64);
    assert(width + lsb <= 64);
    if(Bitpack_fitss(value, width)){ 
        uint64_t buffer = ~0;
        buffer = shiftLeft(buffer, (64 - width));
        buffer = right2(buffer, (64 - width - lsb));
        value = shiftLeft(value, lsb);
        value = (value & buffer);
        word = (word & ~buffer);
        word = (word | value);
    }else{ 
        //Raise bitpack overflow
        RAISE(Bitpack_Overflow);
    }

    return word;
}