#include "mandelbrot.hpp"

using namespace cv;
using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations, int color, char* rep) : surEchantillonage(supSample), im_width(im_w), im_height(im_h), iterations(iterations), color(color)
{
	mpf_inits(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
	
	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);
	
	
	//  atomic_w = width / (im_width * surEchantillonage)
	//  atomic_h = height / (im_height * surEchantillonage)
	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

	this->divMat = new Mat(im_h*supSample, im_w*supSample, CV_32SC1);
	this->img = new Mat(im_h, im_w, CV_8UC3);
	this->sEMat = new Mat(im_h, im_w, CV_8UC1);

	if(rep == nullptr)
	{
		time_t now = time(0);
		this->rep = ctime(&now);
		int i = 0;
		while(this->rep[i] != '\0')
		{
			if(this->rep[i] == ' ')
			{
				this->rep[i] = '_';
			}
			else if(this->rep[i] == ':')
			{
				this->rep[i] = '-';
			}
			else if(this->rep[i] == '\n')
			{
				this->rep[i] = '\0';
			}
			i++;
		}
	}
	else
	{
		this->rep = rep;
	}
		
}

/*Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations, int color, char* rep) : surEchantillonage(supSample), im_width(im_w), im_height(im_h), iterations(iterations), color(color), rep(rep)
{
	mpf_inits(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
	
	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);
	
	
	//  atomic_w = width / (im_width * surEchantillonage)
	//  atomic_h = height / (im_height * surEchantillonage)
	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

	this->divMat = new Mat(im_h*supSample, im_w*supSample, CV_32SC1);
	this->img = new Mat(im_h, im_w, CV_8UC3);
	this->sEMat = new Mat(im_h, im_w, CV_8UC1);
}*/

Mandelbrot::~Mandelbrot()
{
	mpf_clears(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
	delete this->divMat;
	delete this->img;
	delete this->sEMat;
}

void Mandelbrot::escapeSpeedCalcThread2()
{
	mpf_t tmp1, tmp2;
	mpf_t *x, *y;
	x = (mpf_t*)malloc(sizeof(mpf_t)*this->im_width*this->surEchantillonage);
	y = (mpf_t*)malloc(sizeof(mpf_t)*this->im_height*this->surEchantillonage);

	if(!x || !y)
		exit(2);

	mpf_inits( tmp1, tmp2, NULL);

	mpf_div_ui(tmp1, this->width, 2); //  tmp1 = width/2
	mpf_set_ui( tmp2, 0);
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_init(x[i]);

		//  xc = pos_x - width/2 + i*atomic_w
		mpf_sub(x[i], this->pos_x, tmp1); //  xc = pos_x - tmp = pos_x - width/2
		//mpf_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * i
		mpf_add( tmp2, tmp2, atomic_w);
		mpf_add(x[i], x[i], tmp2); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

	}

	mpf_div_ui(tmp1, this->height, 2); //  tmp1 = height/2
	mpf_set_ui( tmp2, 0);
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_init(y[i]);

		//  yc = pos_y - height/2 + i*atomic_h
		mpf_sub(y[i], this->pos_y, tmp1); //  yc = pos_y - tmp = pos_y - height/2
		//mpf_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
		mpf_add( tmp2, tmp2, atomic_h);
		mpf_add(y[i], y[i], tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
	}

	mpf_clears( tmp1, tmp2, NULL);

	//int nbr_threads = (float) this->im_height*this->surEchantillonage / ITERATIONS_PER_THREAD * this->im_width*this->surEchantillonage * this->iterations + 1;
	int nbr_threads = (float) this->im_height*this->surEchantillonage / 10 + 1;
	thread threads[nbr_threads];
	
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i] = thread( &Mandelbrot::threadCalc2, this, (i*(this->im_height*this->surEchantillonage)/nbr_threads), ((i+1)*(this->im_height*this->surEchantillonage)/nbr_threads), x, y);
		// threads[i] = thread( &Mandelbrot::threadCalc2_2, this, (i*(this->im_height*this->surEchantillonage)/nbr_threads), ((i+1)*(this->im_height*this->surEchantillonage)/nbr_threads), x, y);
	}
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i].join();
	}
	
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_clear(x[i]);
	}
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_clear(y[i]);
	}

	free(x);
	free(y);
}

void Mandelbrot::threadCalc2(int deb, int fin, mpf_t* x, mpf_t* y)
{	
	mpf_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

	//mpf_set_prec( mpf_t, int)
	//mpf_set_prec_raw( mpf_t, int)

	for(int j = deb; j < fin; ++j)
	{
		for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
		{
			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);
			mpf_set_ui(xsqr,0);
			mpf_set_ui(ysqr,0);

			for (int k = 1; k < this->iterations; ++k)
			{
				//  xnp1 = xn² - yn² + xc
				mpf_sub(xnp1, xsqr, ysqr); //  xnp1 = xsqr - ysqr = xn² - yn²
				//mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
				mpf_add(xnp1, xnp1, x[i]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

				//  ynp1 = 2*xn*yn + yc
				mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
				mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
				//mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
				mpf_add(ynp1, ynp1, y[j]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

				//  xn = xnp1
				//  yn = ynp1
				mpf_set( xn, xnp1); //  xn = xnp1
				mpf_set( yn, ynp1); //  yn = ynp1
				
				//xsqr = xn²
				mpf_mul(xsqr, xn, xn);

				//ysqr = yn²
				mpf_mul(ysqr, yn, yn);

				//  mod = xnp1² + ynp1²
				mpf_add(mod, xsqr, ysqr); //  mod = xsqr + ysqr = xn² + yn²

				if(mpf_cmp_ui(mod, 4) > 0)
				{
					this->divMat->at<int>(j, i) = k;
					break;
				} else if(k == this->iterations -1)
					{
						this->divMat->at<int>(j, i) = this->iterations;
					}
			}
		}
	}
	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);
}

void Mandelbrot::escapeSpeedCalcThread3()
{
	mpf_t tmp1, tmp2;
	mpf_t *x, *y;
	x = (mpf_t*)malloc(sizeof(mpf_t)*this->im_width*this->surEchantillonage);
	y = (mpf_t*)malloc(sizeof(mpf_t)*this->im_height*this->surEchantillonage);

	if(!x || !y)
		exit(2);

	mpf_inits( tmp1, tmp2, NULL);

	mpf_div_ui(tmp1, this->width, 2); //  tmp1 = width/2
	mpf_set_ui( tmp2, 0);
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_init(x[i]);

		//  xc = pos_x - width/2 + i*atomic_w
		mpf_sub(x[i], this->pos_x, tmp1); //  xc = pos_x - tmp = pos_x - width/2
		//mpf_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * i
		mpf_add( tmp2, tmp2, atomic_w);
		mpf_add(x[i], x[i], tmp2); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

	}

	mpf_div_ui(tmp1, this->height, 2); //  tmp1 = height/2
	mpf_set_ui( tmp2, 0);
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_init(y[i]);

		//  yc = pos_y - height/2 + i*atomic_h
		mpf_sub(y[i], this->pos_y, tmp1); //  yc = pos_y - tmp = pos_y - height/2
		//mpf_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
		mpf_add( tmp2, tmp2, atomic_h);
		mpf_add(y[i], y[i], tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
	}

	mpf_clears( tmp1, tmp2, NULL);




	//int nbr_threads = (float) this->im_height*this->surEchantillonage / ITERATIONS_PER_THREAD * this->im_width*this->surEchantillonage * this->iterations + 1;
	int nbr_threads = (float) this->im_height / 10 + 1;
	thread threads[nbr_threads];


	*(this->sEMat) = 1;
	*(this->divMat) = -1;

	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i] = thread( &Mandelbrot::threadCalc3, this, (i*(this->im_height)/nbr_threads), ((i+1)*(this->im_height)/nbr_threads), x, y);
	}
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i].join();
	}

	this->partialDraw();
	Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

	int lowThreshold = 10;
	int ratio = 3;
	int kernel_size = 3;

	cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
	blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
	Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
	filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);
	matSave(this->sEMat,"sEMat_blur");

	*(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;


	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i] = thread( &Mandelbrot::threadCalc3, this, (i*(this->im_height)/nbr_threads), ((i+1)*(this->im_height)/nbr_threads), x, y);
	}
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i].join();
	}

	
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_clear(x[i]);
	}
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_clear(y[i]);
	}

	free(x);
	free(y);
}

void Mandelbrot::threadCalc3(int deb, int fin, mpf_t* x, mpf_t* y)
{
	mpf_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

	for(int j = deb; j < fin; j++)
	{
		for (int i = 0; i < this->im_width; i++)
		{
			int sE = this->sEMat->at<char>(j, i);

			for(int m = 0; m < sE; m++)
			{
				for(int n = 0; n < sE; n++)
				{
					if((sE != 1 && this->divMat->at<int>(j*this->surEchantillonage+n ,i*this->surEchantillonage+m) == -1) || sE == 1)
					{
						mpf_set_ui(xn,0);
						mpf_set_ui(yn,0);
						mpf_set_ui(xsqr,0);
						mpf_set_ui(ysqr,0);

						for (int k = 1; k < this->iterations; k++)
						{
							//  xnp1 = xn² - yn² + xc
							mpf_sub(xnp1, xsqr, ysqr); //  xnp1 = xsqr - ysqr = xn² - yn²
							//mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
							mpf_add(xnp1, xnp1, x[i*this->surEchantillonage+m]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

							//  ynp1 = 2*xn*yn + yc
							mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
							mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
							//mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
							mpf_add(ynp1, ynp1, y[j*this->surEchantillonage+n]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

							//  xn = xnp1
							//  yn = ynp1
							mpf_set( xn, xnp1); //  xn = xnp1
							mpf_set( yn, ynp1); //  yn = ynp1
							
							//xsqr = xn²
							mpf_mul(xsqr, xn, xn);

							//ysqr = yn²
							mpf_mul(ysqr, yn, yn);

							//  mod = xnp1² + ynp1²
							mpf_add(mod, xsqr, ysqr); //  mod = xsqr + ysqr = xn² + yn²

							if(mpf_cmp_ui(mod, 4) > 0)
							{
								this->divMat->at<int>(j*this->surEchantillonage+n, i*this->surEchantillonage+m) = k;
								break;
							} else if(k == this->iterations -1)
								{
									this->divMat->at<int>(j*this->surEchantillonage+n, i*this->surEchantillonage+m) = this->iterations;
								}
						}
					}
				}
			}
		}
	}
	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);
}

void Mandelbrot::partialDraw()
{
	int divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			divSpeed = divMat->at<int>( j*this->surEchantillonage, i*this->surEchantillonage);
			Vec3b bgr;
			switch(this->color)
			{
				case 1:
				{
					if(divSpeed == this->iterations)
						coloration(bgr, divSpeed, this->iterations, 0, 1);
					else
						coloration(bgr, divSpeed, this->iterations, 1, 0);
					break;
				}
				case 2:
				{
					coloration2(bgr, divSpeed, this->iterations);
					break;
				}
				case 3:
				{
					coloration3(bgr, divSpeed, this->iterations);
					break;
				}
			}
			this->img->at<Vec3b>( j, i) = bgr;
		}
	}
}

/*void Mandelbrot::threadCalc2_2(int deb, int fin, mpf_t* x, mpf_t* y)
{
	double threshold = 0.001; //  e=0.001²
	
	mpf_t xn, yn, xnp1, ynp1, mod, xsqrt, ysqrt, dxn, dyn, dxnp1, dynp1, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqrt, ysqrt, dxn, dyn, dxnp1, dynp1, NULL);

	//mpf_set_prec( mpf_t, int)
	//mpf_set_prec_raw( mpf_t, int)

	for(int j = deb; j < fin; ++j)
	{
		for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
		{
			mpf_set(xn,x[i]);
			mpf_set(yn,y[j]);
			
			mpf_set_ui(dxn,1);
			mpf_set_ui(dyn,0);

			for (int k = 1; k < this->iterations; ++k)
			{
				//xsqrt = dxn²
				mpf_mul(xsqrt, dxn, dxn);

				//ysqrt = dyn²
				mpf_mul(ysqrt, dyn, dyn);

				//  mod = dxn² + dyn²
				mpf_add(mod, xsqrt, ysqrt); //  mod = xsqrt + ysqrt = dxn² + dyn²

				if(mpf_cmp_d(mod, threshold) < 0)
				{
					this->divMat->at<int>(j, i) = this->iterations;
					// this->divMat->at<int>(j, i) = k;
					// this->divMat->at<int>(j, i) = -1;
					break;
				}
				
				//xsqrt = xn²
				mpf_mul(xsqrt, xn, xn);

				//ysqrt = yn²
				mpf_mul(ysqrt, yn, yn);

				//  mod = xnp1² + ynp1²
				mpf_add(mod, xsqrt, ysqrt); //  mod = xsqrt + ysqrt = xn² + yn²

				if(mpf_cmp_ui(mod, 4) > 0)
				{
					this->divMat->at<int>(j, i) = k;
					break;
				} else if(k == this->iterations -1)
					{
						this->divMat->at<int>(j, i) = this->iterations;
					}
				
				//  dxnp1 = (dxn*xn - dyn*yn)*2
				mpf_mul( dxnp1, dxn, xn);
				mpf_mul( tmp, dyn, yn);
				mpf_sub( dxnp1, dxnp1, tmp);
				mpf_mul_ui( dxnp1, dxnp1, 2);
				
				//  dynp1 = (dxn*yn + dyn*xn)*2
				mpf_mul( dynp1, dxn, yn);
				mpf_mul( tmp, dyn, xn);
				mpf_add( dynp1, dynp1, tmp);
				mpf_mul_ui( dynp1, dynp1, 2);
				
				//  dxn = dxnp1
				//  dyn = dynp1
				mpf_set( dxn, dxnp1); //  xn = xnp1
				mpf_set( dyn, dynp1); //  yn = ynp1
				
				
				//  xnp1 = xn² - yn² + xc
				mpf_sub(xnp1, xsqrt, ysqrt); //  xnp1 = xsqrt - ysqrt = xn² - yn²
				//mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
				mpf_add(xnp1, xnp1, x[i]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

				//  ynp1 = 2*xn*yn + yc
				mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
				mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
				//mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
				mpf_add(ynp1, ynp1, y[j]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

				//  xn = xnp1
				//  yn = ynp1
				mpf_set( xn, xnp1); //  xn = xnp1
				mpf_set( yn, ynp1); //  yn = ynp1
			}
		}
	}
	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqrt, ysqrt, dxn, dyn, dxnp1, dynp1, NULL);
}*/

void Mandelbrot::draw()
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < this->surEchantillonage; ++k)
			{
				for(int l = 0; l < this->surEchantillonage; ++l)
				{
					divSpeed = divMat->at<int>( j*this->surEchantillonage + l, i*this->surEchantillonage + k);

					if(divSpeed != -1)
					{
						if(divSpeed == this->iterations)
							nbr_ndiv++;
						else
						{
							moy += divSpeed;
							nbr_div++;
						}
					}
				}
			}

			Vec3b bgr;

			if(nbr_div)
				moy /= nbr_div;
			else 
				moy = this->iterations;

			switch(this->color)
			{
				case 1:
				{
					coloration(bgr, moy, this->iterations, nbr_div, nbr_ndiv);
					break;
				}
				case 2:
				{
					coloration2(bgr, moy, this->iterations);
					break;
				}
				case 3:
				{
					coloration3(bgr, moy, this->iterations);
					break;
				}
			}
			this->img->at<Vec3b>( j, i) = bgr;
		}
	}
}

void Mandelbrot::save()
{
	matSave( this->img, this->rep);
}

bool Mandelbrot::IsGood(){
	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC3);

	int lowThreshold = 30;		//comment changer ça ?
	int ratio = 3;				//inutile de changer ca
	int kernel_size = 3;		//inutile de changer ca

	cvtColor( *(this->img), *(src_gray), CV_BGR2GRAY );
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);

	while(detected_edges->cols > 1 || detected_edges->rows > 1){
		
    	pyrDown( *(detected_edges), *(detected_edges), Size( detected_edges->cols/2, detected_edges->rows/2) );
	}
	int res = detected_edges->at<char>( 0);

	//cout<<res<<endl;

	if(res<THRESHOLD)
		return false;
	else
		return true;
}

void Mandelbrot::IterUp(){
	
	//equation a changer en fonction de this->pos_x ou de enough

	iterations += iterations/4;
	
}

void worthcontinue(){

	//condition de saving

	//this->save();

}

void worthsaving(){
	



}

/*bool Mandelbrot::DeepEnough(auto enough){
	
	//change enough
	
	return false;		//c'est assez on peut s'arreter et cracher l'image
	return true;		//pas assez, on continue
}*/

void Mandelbrot::dichotomie(int enough)
{
	this->escapeSpeedCalcThread3();
	this->draw();
	this->save();

	if(this->IsGood())
	{
		//this->worthsaving();
		this->IterUp();

		if(--enough /*this->DeepEnough(enough) || worthcontinue()*/)
		{
			mpf_t nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, nh, nw, temp;
			
			mpf_inits(nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, nh, nw, temp, NULL);
			
			// newx = x - x/2 & y + y/2
			mpf_div_ui(temp, this->width, 4);		//calcul nouveaux x pour reiterer
			mpf_sub(nx1, this->pos_x, temp);
			mpf_add(nx2, this->pos_x, temp);
			mpf_sub(nx3, this->pos_x, temp);
			mpf_add(nx4, this->pos_x, temp);
			
			mpf_div_ui(temp, this->height, 4);		//calcul nouveaux y
			mpf_add(ny1, this->pos_y, temp);
			mpf_add(ny2, this->pos_y, temp);
			mpf_sub(ny3, this->pos_y, temp);
			mpf_sub(ny4, this->pos_y, temp);
			
			// newh = h/2
			mpf_div_ui(nh, this->height, 2);
			
			//neww = w/2
			mpf_div_ui(nw, this->width, 2);

			//new iterations
			this->IterUp();
			
			Mandelbrot* M1 = new Mandelbrot(nx1, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->rep);		//en haut a gauche
			M1->dichotomie(enough);
			
			Mandelbrot* M2 = new Mandelbrot(nx2, ny2, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->rep);		//en haut a droite
			M2->dichotomie(enough);
			
			Mandelbrot* M3 = new Mandelbrot(nx3, ny3, nw, nh,im_width, im_height, surEchantillonage, iterations, this->color, this->rep);			//en bas a gauche
			M3->dichotomie(enough);		
				
			Mandelbrot* M4 = new Mandelbrot(nx4, ny4, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->rep);		//en bas a droite
			M4->dichotomie(enough);
			
			
			mpf_clears(nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, nh, nw, temp, NULL);
			delete M1;
			delete M2;
			delete M3;
			delete M4;
		}

	}
	
}



/*

Any primitive type from the list can be defined by an identifier in the form CV_<bit-depth>{U|S|F}C(<number_of_channels>)
where U is unsigned integer type, S is signed integer type, and F is float type.

*/