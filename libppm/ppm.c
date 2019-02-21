#include "ppm.h"
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
 
int ppm_image_init( struct ppm_image *im , int w , int h )
{
	im->width = w;
	im->height = h;
	im->px = malloc(w*h*sizeof(struct ppm_pixel));
    return 0;
}
 
int ppm_image_release( struct ppm_image *im )
{
	im->width = 0;
	im->height = 0;
	free(im->px);
	im->px = NULL;
    return 0;
}
 
int ppm_image_dump( struct ppm_image *im, char * path )
{
	FILE* file = fopen(path, "w+");
	if(!file)
	{
		perror(path);
		exit(1);
	}

	fprintf(file, "P6\n%d %d\n255\n", im->width, im->height);
	fwrite(im->px, sizeof(struct ppm_pixel), im->width*im->height, file);

	fclose(file);
    return 0;
}

void ppm_image_setpixel_hsv( struct ppm_image * im, int x_, int y_, unsigned char h , unsigned char s , unsigned char v)
{
    unsigned char r, g, b;

    double x, r2 = 0.0, g2 = 0.0, b2 = 0.0, c, m, tmp;

    c = v*s;

    tmp = (float) h/60;
    while(tmp>2)
        tmp -= 2;
    tmp--;
    x = (float) c *(1-abs(tmp));

    m = v-c;

    switch(h/60)
    {
        case 0:
        {
            r2 = c;
            g2 = x;
            break;
        }
        case 1:
        {
            r2 = x;
            g2 = c;
            break;
        }
        case 2:
        {
            g2 = c;
            b2 = x;
            break;
        }
        case 3:
        {
            g2 = x;
            b2 = c;
            break;
        }
        case 4:
        {
            r2 = x;
            b2 = c;
            break;
        }
        case 5:
        {
            r2 = c;
            b2 = x;
            break;
        }
    }

    b = (float)(b2+m) * 255;
    g = (float)(g2+m) * 255;
    r = (float)(r2+m) * 255;


    ppm_setpixel(&(im->px[y_*im->width + x_]), r, g, b);
}