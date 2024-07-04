/*
 * Compile with:
       gcc from5551toRGB.c -o from5551toRGB

       or

       gcc from5551toRGB.c -o from5551toRGB -DLITTLE_ENDIAN
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*
** Convert a 5551 image into rgb format
*/
int main(int argc, char **argv)
{
    FILE *input;
    FILE *output;
    int x, y, xsize, ysize;
    uint16_t* img;
    unsigned char rgb[3];

    if( argc != 5 ) {
        fprintf(stderr,
                "usage: %s inimage.5551 outimage.rgb xsize ysize\n", argv[0]);
        exit(1);
    }
    if((input=fopen(argv[1],"rb")) == NULL ) {
        fprintf(stderr,"%s: can't open input file %s\n", argv[0], argv[1]);
        exit(1);
    }
    if ((output = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr,"%s: can't open output file %s\n", argv[0], argv[2]);
        exit(2);
    }

    xsize = atoi(argv[3]);
    ysize = atoi(argv[4]);
    img = (uint16_t*)malloc(2*xsize*ysize);
    if (fread(img, 2, xsize*ysize, input) != xsize*ysize)
    {
        fprintf(stderr,"%s: couldn't read %d texels from file %s\n",
                argv[0], xsize*ysize, argv[1]);
        exit(1);
    }
    fclose(input);

    #ifdef LITTLE_ENDIAN
    for (y=0;y<ysize;y++)
    {
        for (x=0;x<xsize;x++)
        {
            img[y*xsize+x] = ((img[y*xsize+x] & 0xFF00) >> 8) |
                             ((img[y*xsize+x] & 0x00FF) << 8);
        }
    }
    #endif

    for(y=0; y<ysize; y++)
    {
        for(x=0;x<xsize;x++)
        {
            /* [r:15..11][g:10..6][b:5..1][a:0] */
            int r5 = (img[y*xsize + x]>>11)&31;
            int g5 = (img[y*xsize + x]>>6)&31;
            int b5 = (img[y*xsize + x]>>1)&31;
            rgb[0] = (r5<<3)|(r5>>2);
            rgb[1] = (g5<<3)|(g5>>2);
            rgb[2] = (b5<<3)|(b5>>2);

            fwrite(rgb, 3, 1, output);
        }
    }
    fclose(output);

    free(img);
    return 0;
}
