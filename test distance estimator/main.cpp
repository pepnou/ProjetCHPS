#include <cmath>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	int im_width = 1, im_height = 1, iteration = 100;
	double x_center = 0, y_center = 0, width = 3, height = 3;

	double xsqr, ysqr, xn, yn, xnp1, ynp1, mod;
	double x, y;

	double xpn, ypn, xpnp1, ypnp1;

	FILE* file1 = fopen("test1.txt", "w+");

	double*** d = (double***)malloc(im_width * sizeof(double**));
	for(int i = 0; i < im_width; i++)
		d[i] = (double**)malloc(im_height * sizeof(double*));
	for(int i = 0; i < im_width; i++)
		for(int j = 0; j < im_height; j++)
			d[i][j] = (double*)malloc(iteration * sizeof(double));

	bool in;

	for(int j = 0; j < im_height; j++)
	{
		for(int i = 0; i < im_width; i++)
		{
			x = x_center - width/2 + i*width/im_width;
			y = y_center - height/2 + j*height/im_height;
			xsqr = ysqr = 0;
			xn = x;
			yn = y;
			in = true;

			xpn = 1;
			ypn = 0;

			for(int k = 0; k < iteration; k++)
			{
				/*2zn*zpn + 1 => 2 * (xn + i * yn) * (xpn + i * ypn) + 1
							=> 2 * (xn * xpn - yn * ypn) + 1 + i * 2 * (xn * ypn + yn * xpn)*/
				xpnp1 = 2 * (xn * xpn - yn * ypn) + 1;
				ypnp1 = 2 * (xn * ypn + yn * xpn);

				xpn = xpnp1;
				ypn = ypnp1;



				xsqr = xn * xn;
				ysqr = yn * yn;

				xnp1 = xsqr - ysqr + x;
				ynp1 = 2 * xn * yn + y;

				xn = 0;
				yn = 0;

				mod = sqrt(xsqr + ysqr);

				if(mod >= 2)
					in = false;

				printf("%f, %f\n", mod, sqrt(xpn * xpn + ypn * ypn));
				d[i][j][k] = mod * log(mod) / sqrt(xpn * xpn + ypn * ypn);
			}
			if(in)
				fprintf(file1, ".");
			else
				fprintf(file1, "#");
		}
		fprintf(file1, "\n");
	}

	fclose(file1);

	FILE* file2 = fopen("test2.txt", "w+");

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

	fclose(file2);

	return 0;
}