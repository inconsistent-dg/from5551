/*
 * Compile with:
       gcc from5551toPNG.c -o from5551toPNG -lpng -lz -lm

       or

       gcc from5551toPNG.c -o from5551toPNG -DLITTLE_ENDIAN -lpng -lz -lm
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <png.h>

/*
** Convert a 5551 image into png format
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
                "usage: %s inimage.5551 outimage.png xsize ysize\n", argv[0]);
        exit(1);
    }
    if((input=fopen(argv[1],"rb")) == NULL ) {
        fprintf(stderr,"%s: can't open input file %s\n", argv[0], argv[1]);
        exit(1);
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

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "Could not allocate write struct\n");
        exit(1);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "Could not allocate info struct\n");
        png_destroy_write_struct(&png_ptr, NULL);
        exit(1);
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        exit(1);
    }

    if((output = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr,"%s: can't open output file %s\n", argv[0], argv[2]);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        exit(2);
    }

    png_init_io(png_ptr, output);

    png_set_IHDR(png_ptr, info_ptr, xsize, ysize,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_bytep row_pointers[ysize];
    for (y = 0; y < ysize; y++)
    {
        row_pointers[y] = (png_bytep)malloc(3 * xsize * sizeof(png_byte));
        for (x = 0; x < xsize; x++)
        {
            /* [r:15..11][g:10..6][b:5..1][a:0] */
            int r5 = (img[y*xsize + x]>>11)&31;
            int g5 = (img[y*xsize + x]>>6)&31;
            int b5 = (img[y*xsize + x]>>1)&31;
            row_pointers[y][3*x+0] = (r5<<3)|(r5>>2);
            row_pointers[y][3*x+1] = (g5<<3)|(g5>>2);
            row_pointers[y][3*x+2] = (b5<<3)|(b5>>2);
        }
    }

    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    for (y = 0; y < ysize; y++)
    {
        free(row_pointers[y]);
    }

    fclose(output);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    free(img);

    return 0;
}
