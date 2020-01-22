--------------------------------------------------------------
|   John Sullivan & Andrew Lefebvre                          |
|                     IMAGE COMPRESSION                      |
--------------------------------------------------------------

We recieved aid from CSC411 TA's, one notably was Najib Ishaq. 
We also asked for help on several occasions from Prof. Daniels. We did not discuss
the assignment with any other students, and simply collaborated together on the program.

==================
| IMPLEMENTATION |
==================

All functionality of our compression and decompression works as intended to our belief.
Every single function in our program was extensively tested with hand-written unit tests. 
All features have been correctly implemented. 

==================
|  ARCHITECTURE  |
==================

The structure of our program is broken up into many different functions. 
Looking at the code from the top down, we see that it's not complicated to understand.
We stuck with descriptive function names and throughouly commented all of our code to 
describe what's going on. Our program starts with the image.c file, that will call compress.c
which is the interface we wrote containing our compress and decompress functions. The decision to 
compress or decompress the input (whether it be stdin, or a file), will be decided upon command line
inputs. From the compress and decompress functions, we call upon our extensive wordPack.c file that
handles bitpacking using the bitpack.c interface, cosine transformations and reading and writing codewords 
from and to disk, respectively. Then our interface calculation.c handles all component and and RGB caluculations.
When an image is compressed in our program, we use the bitpack inteface to store the image in "codewords" in 
binary, and when decompression happens, we use bitpack again to revert the "codewords" back to the non-compressed image.
We separated the program into a bunch of functions for reusability, so if we had to make a change for some specification
(i.e, the challenge) we wouldn't have to change much as our program is so nicely separated. It did turn out to be more lines
of code than expected, but we believe our implementation exhibits simplicity and straightfowardness. For the arith challenge,
all variables that we believe could be changed are located within sharedGloabls.h so hypethetically that will be the only
file changed.

=================
|  TIME SPENT   |
=================

For analyzing and understanding the problems posed in the assignment: both partners spent collectively
about 10 hours to fully wrap our heads around what was expected, and what direction to head in.

For solving the problems after our analysis of the assignment: both partners spent collectively about
25 hours working through the code, unit testing, and debugging. 

===========================
| DESCRIPTION OF PROBLEMS |
===========================

Image-Compression:

//////////
(image.c):
//////////
This file serves as the "main" function for the whole entire program. It will read from standard input
a -d or -c specifying whether the user wants their image to be compressed or decompressed, and if none is given,
it will automatically compress by default. The user can either input a file, or image.c will read from stdin what to compress.
An example of input would be:
./image -c MARBLES.ppm
./image -d MARBLES.ppm
or...
./image -c MARBLES.ppm > example.txt
./image -d example.txt > MARBLESDecompressed.ppm

////////////
readImage.c:
////////////
This file only exists to read the ppm image from the specified file
and store it in a Pnm_ppm type variable. It then returns the file to be used by 
the rest of the program.

////////////
wordPack.c:
////////////
This module of the program handles cosine transformations, writing bits to disk,
and reading bits from disk. We seemed to have no trouble with this and it worked on our
first pass through of test cases. 

This is code from our function to read a codeword from file and assign our video component values:

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

////////////
compress.c:
////////////
This module will carry out the compression or decompression of the image based on user input.
It calls funtions from wordPack.c in order to help with this for increased modularity.
Having this file separated from the rest of the code deemed useful in solving problems
in the rest of our program, as we knew exactly what function was being called from here
that was causing our compiler errors or segmentation faults. 

/////////////
calculation.c:
/////////////
This part of code handles calculations for RGB to video component values, and vice versa. 
We ran into problems here, as our whole image would come up in grayscale when we decompressed.
We found out that this was a simple order of operations error because we missed some parenthesis.
We also ran into a problem where the R value was being stored as a number in the millions, causing
each decompressed image to come up as some shade of red. We found out this was because of how we were
storing the pointer returned from at as a Pnm_rgb rather than declaring it as a struct Pnm_rgb.
Here is the code that gave us trouble:
UArray2_T componentToRGB(UArray2_T source){
    int R;
    int B;
    int G;
    struct Pnm_rgb newRGB; <---- this is the line we declared as Pnm_rgb newRGB; that did not work.

Here is where we ran into problems with order of operations:
R = DENOMINATOR * ( (1.0 * floats->Y) + (0 * floats->Pb)       +  (1.402 * floats->Pr) );
            R = fmax(fmin(R, DENOMINATOR), 0);
            G = DENOMINATOR * ( (1.0 * floats->Y) - (0.344136 * floats->Pb)-  (0.714136 * floats->Pr) );
            G = fmax(fmin(G, DENOMINATOR), 0);
            B = DENOMINATOR * ( (1.0 * floats->Y) + (1.772 * floats->Pb)   +  (0 * floats->Pr) );  
            B = fmax(fmin(B, DENOMINATOR), 0);  

When an image is compressed, we store the image in Pnm_ppm format and then we use nested 
for loops to loop through each 2x2 block of the image, and convert four pixels at each iteration 
from RGB to video component format. When those four pixels are calculated, we index where to put these 
pixels using the index of chroma function provided to us, and then we use discrete cosine transformations 
which give us a 2x2 pixel block. The compression is lossy because of the averages we take. This 2x2 
block is then packed into a codeword by using bitpack.c and wordPack.c.
When we decompress an image, a Pnm_ppm struct is used just like in compression to store the image, 
and then we do the same process we described in compression backwards. It reads the 32-bit codeword 
that was previously packed and unpacks the codeword into the 2x2 block of video component pixels by 
doing reverse calculations from compression, and it does this by using bitpack.c and wordPack.c When the 
video component pixels are calculated from finding the chroma of the index as well as doing the inverse 
discrete cosine transformation, we convert them to RGB values and place them in a new UArray2. This is all
done in nested for loops. Then, the decompressed image is printed to the standard output in the terminal using Pnm_ppmwrite().

////////////
Bit-packing:
////////////
This part of the code is where we did all binary operations to the code that would be used in our other interfaces.
Problems arose when creating the "new" functions as there was confusion between the two different types of right shifts.
We tackled this problem by logically coming up with step by step solutions to the sub problems within it.  Everything now 
works as expected and was sucessful in compressing and decompressing the files.  The header file was the one given and 
the implementation is found in bitpack.c.


===========================
|         ADVICE          |
===========================

For a problem like this with many moving parts the best way that we found to go about it is to focus 
on dividing and conquering.  You do this with modulation and encapsulation. At first this seems tedious,
but later in the project's life it becomes easier to test and troubleshoot to narrowdown a bug.  It also 
helps to conceptulize a bigger problem better by breaking it down into smaller ones.  With bitpacking the 
best advice to give would be to write alot of test cases with proven math that you understand before coding.
This helps you have a better sense of how to go about the problem in addition to any special cases that many 
arrise.
