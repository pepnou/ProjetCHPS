#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#include "../../aise/Cours_7/libppm/ppm.h"

//plot for [i=1:25] 'test2.txt' using 0:i with line title 'distance '.i

int main()
{
	int im_width = 1920, im_height = 1080, iteration = 1000;
	double x_center = -0.5, y_center = 0.0, width = 3.0, height = 3.0;
	double diag = sqrt(pow(width, 2) + pow(height, 2));

	double xn, yn, xnp1, ynp1, mod;
	double x, y;

	double xpn, ypn, xpnp1, ypnp1, modp;

	struct ppm_image im;
	ppm_image_init( &im, im_width, im_height);

	/*double*** d = (double***)malloc(im_width * sizeof(double**));
	for(int i = 0; i < im_width; i++)
		d[i] = (double**)malloc(im_height * sizeof(double*));
	for(int i = 0; i < im_width; i++)
		for(int j = 0; j < im_height; j++)
			d[i][j] = (double*)malloc(iteration * sizeof(double));*/

	double** d = (double**)malloc(im_width * sizeof(double*));
	for(int i = 0; i < im_width; i++)
		d[i] = (double*)malloc(im_height * sizeof(double));

	bool in;

	#pragma omp parallel for private(in)
	for(int j = 0; j < im_height; j++)
	{
		for(int i = 0; i < im_width; i++)
		{
			x = x_center - width/2 + i*width/im_width;
			y = y_center - height/2 + j*height/im_height;

			xn = 0;
			yn = 0;

			xpn = 0;
			ypn = 0;

			in = true;

			for(int k = 0; k < iteration; k++)
			{
				/*2*zn*zpn + 1 => 2 * (xn + i * yn) * (xpn + i * ypn) + 1
							=> 2 * (xn * xpn - yn * ypn) + 1 + i * 2 * (xn * ypn + yn * xpn)*/
				xpnp1 = 2 * (xn * xpn - yn * ypn) + 1;
				ypnp1 = 2 * (xn * ypn + yn * xpn);

				xpn = xpnp1;
				ypn = ypnp1;

				modp = sqrt(xpn*xpn + ypn*ypn);




				xnp1 = xn*xn - yn*yn + x;
				ynp1 = 2 * xn * yn + y;

				xn = xnp1;
				yn = ynp1;

				mod = sqrt(xn*xn + yn*yn);




				/*if(in)
					d[i][j][k] = mod * log(mod) / modp;
				else
					d[i][j][k] = d[i][j][k-1];*/



				if(mod >= 2)
				{
					in = false;
					ppm_image_setpixel_hsv( &im, i, j, k%360, 1, 1);
					break;
				}
			}

			d[i][j] =  mod * log(mod) / modp;

			if(in)
			{
				//fprintf(file1, " ");
				ppm_image_setpixel( &im, i, j, 0, 0, 0);
			}
			else
			{
				if(d[i][j] < 0)
					 d[i][j] = 0;

				int tmp = (int)(d[i][j] / diag * 10000) % 360;
				//ppm_image_setpixel_hsv( &im, i, j, tmp, 1, 1);

				//fprintf(file1, "%c", tmp);
			}
		}
		//fprintf(file1, "\n");
	}


	/*FILE* file2 = fopen("test2.txt", "w+");

	for(int k = 0; k < iteration; k++)
	{
		fprintf(file2, "%5d ", k);
		for(int j = 0; j < im_height; j++)
		{
			for(int i = 0; i < im_width; i++)
			{
				fprintf(file2, "%1.8f ", d[i][j][k]);
			}
		}
		fprintf(file2, "\n");
	}

	fclose(file2);*/

	ppm_image_dump( &im,"mandel.ppm");
	ppm_image_release( &im);

	return 0;
}