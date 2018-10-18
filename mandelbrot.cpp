#include "mandelbrot.hpp"

using namespace cv;
using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations) : surEchantillonage(supSample), im_width(im_w), im_height(im_h), iterations(iterations)
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

	this->divMat = new Mat(im_h*supSample, im_w*supSample, CV_32FC1);
	this->img = new Mat(im_h, im_w, CV_8UC3);
}

Mandelbrot::~Mandelbrot()
{
	mpf_clears(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
	delete this->divMat;
	delete this->img;
}

void Mandelbrot::escapeSpeedCalc()
{
	mpf_t xc, yc, xn, yn, xnp1, ynp1, mod, tmp;
	mpf_inits( xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);
	
	// mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	// mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

    //loading(this->im_width * this->surEchantillonage * this->im_height * this->surEchantillonage);
    //loading(0);

	for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		//  xc = pos_x - width/2 + i*atomic_w
		mpf_div_ui(tmp, this->width, 2); //  tmp = width/2
		mpf_sub(xc, this->pos_x, tmp); //  xc = pos_x - tmp = pos_x - width/2
		mpf_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * 1
		mpf_add(xc, xc, tmp); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

		for (int j = 0; j < this->im_height*this->surEchantillonage; ++j)
		{
			//  yc = pos_y - height/2 + i*atomic_h
			mpf_div_ui(tmp, this->height, 2); //  tmp = height/2
			mpf_sub(yc, this->pos_y, tmp); //  yc = pos_y - tmp = pos_y - height/2
			mpf_mul_ui(tmp, this->atomic_h, j); //  tmp = atomic_h * j
			mpf_add(yc, yc, tmp); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j

			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);

			for (int k = 1; k < this->iterations; ++k)
			{
				//  xnp1 = xn² - yn² + xc
				mpf_pow_ui(tmp, yn, 2); //  tmp = yn²
				mpf_pow_ui(xnp1, xn, 2); //  xnp1 = xn²
				mpf_sub(xnp1, xnp1, tmp); //  xnp1 = xnp1 - tmp = xn² - yn²
				mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc

				//  ynp1 = 2*xn*yn + yc
				mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
				mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
				mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

				//  mod = xnp1² + ynp1²
				mpf_pow_ui(mod, xnp1, 2); //  mod = xnp1²
				mpf_pow_ui(tmp, ynp1, 2); //  tmp = ynp1²
				mpf_add(mod, mod, tmp); //  mod = mod + tmp = xnp1² + ynp1²

				//  xn = xnp1
				//  yn = ynp1
				mpf_set( xn, xnp1); //  xn = xnp1
				mpf_set( yn, ynp1); //  yn = ynp1

				if(mpf_cmp_ui(mod, 4) > 0)
				{
					this->divMat->at<int>(j, i) = k;
					break;
				} else if(k == this->iterations -1)
					{
						this->divMat->at<int>(j, i) = this->iterations;
					}
			}
			//loading(i*(this->im_height*this->surEchantillonage) + j);
		}
	}
	
	mpf_clears( xc, yc, xn, yn, xnp1, ynp1, mod, tmp, NULL);
}

void Mandelbrot::escapeSpeedCalcThread()
{
	int nbr_threads = (float) this->im_height*this->surEchantillonage / ITERATIONS_PER_THREAD * this->im_width*this->surEchantillonage * this->iterations + 1;
	// int nbr_threads = (float) this->im_height*this->surEchantillonage / 10 + 1;
	thread threads[nbr_threads];
		
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i] = thread( &Mandelbrot::threadCalc, this, (i*(this->im_height*this->surEchantillonage)/nbr_threads), ((i+1)*(this->im_height*this->surEchantillonage)/nbr_threads));
	}
	for (int i = 0; i < nbr_threads; ++i)
	{
		threads[i].join();
	}
}

void Mandelbrot::threadCalc(int deb, int fin)
{	
	mpf_t xc, yc, xn, yn, xnp1, ynp1, mod, xsqrt, ysqrt, tmpx, tmpy, tmp1, tmp2;
	mpf_inits( xc, yc, xn, yn, xnp1, ynp1, mod, xsqrt, ysqrt, tmpx, tmpy, tmp1, tmp2, NULL);
	
	mpf_div_ui(tmpx, this->width, 2); //  tmpx = width/2
	mpf_div_ui(tmpy, this->height, 2); //  tmpy = height/2
	
	for(int j = deb; j < fin; ++j)
	{
		//  yc = pos_y - height/2 + i*atomic_h
		mpf_sub(yc, this->pos_y, tmpy); //  yc = pos_y - tmpy = pos_y - height/2
		mpf_mul_ui(tmp2, atomic_h, j); //  tmp = atomic_h * j
		//mpf_add( tmp2, tmp2, atomic_h);
		mpf_add(yc, yc, tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
		
		mpf_set_ui( tmp2, 0);
		
		for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
		{
			//  xc = pos_x - width/2 + i*atomic_w
			mpf_sub(xc, this->pos_x, tmpx); //  xc = pos_x - tmp = pos_x - width/2
			mpf_mul_ui(tmp2, this->atomic_w, i); //  tmp = atomic_w * i
			//mpf_add( tmp2, tmp2, atomic_w);
			mpf_add(xc, xc, tmp2); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i
			
			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);

			for (int k = 1; k < this->iterations; ++k)
			{
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

				//  xnp1 = xn² - yn² + xc
				mpf_sub(xnp1, xsqrt, ysqrt); //  xnp1 = xsqrt - ysqrt = xn² - yn²
				//mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
				mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc

				//  ynp1 = 2*xn*yn + yc
				mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
				mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
				//mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
				mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

				//  xn = xnp1
				//  yn = ynp1
				mpf_set( xn, xnp1); //  xn = xnp1
				mpf_set( yn, ynp1); //  yn = ynp1
			}
		}
	}
	mpf_clears( xc, yc, xn, yn, xnp1, ynp1, mod, xsqrt, ysqrt, tmpx, tmpy, tmp1, tmp2, NULL);
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
	mpf_t xn, yn, xnp1, ynp1, mod, xsqrt, ysqrt, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqrt, ysqrt, NULL);
	

	for(int j = deb; j < fin; ++j)
	{
		for (int i = 0; i < this->im_width*this->surEchantillonage; ++i)
		{
			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);

			for (int k = 1; k < this->iterations; ++k)
			{
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
	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqrt, ysqrt, NULL);
}


void Mandelbrot::draw()
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < this->surEchantillonage; k++)
			{
				for(int l = 0; l < this->surEchantillonage; l++)
				{
					divSpeed = divMat->at<int>( j*this->surEchantillonage + l, i*this->surEchantillonage + k);
					if(divSpeed == this->iterations)
						nbr_ndiv++;
					else
					{
						moy += divSpeed;
						nbr_div++;
					}
				}
			}
			if(nbr_div)
				moy /= nbr_div;

			Vec3b& bgr = this->img->at<Vec3b>( j, i);
			coloration(bgr, moy, this->iterations, nbr_div, nbr_ndiv);
		}
	}
}

void Mandelbrot::draw2()
{
	int moy, nbr_div, divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = 0; j < this->im_height; ++j)
		{
			moy = 0, nbr_div = 0;
			for(int k = 0; k < this->surEchantillonage; k++)
			{
				for(int l = 0; l < this->surEchantillonage; l++)
				{
					divSpeed = divMat->at<int>( j*this->surEchantillonage + l, i*this->surEchantillonage + k);
					if(divSpeed != this->iterations)
					{
						moy += divSpeed;
						nbr_div++;
					}
				}
			}
			Vec3b& bgr = this->img->at<Vec3b>( j, i);

			if(nbr_div)
				moy /= nbr_div;
			else
				moy = this->iterations;
			coloration2(bgr, moy, this->iterations);
		}
	}
}

void Mandelbrot::save()
{
	static int num = 0;
	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
	char nom_img[128];
	sprintf(nom_img,"./img/mandel%d.png",num++);
	
	try
    {
        imwrite(nom_img, *(this->img), compression_params);
    }
    catch (const cv::Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
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

/*void Mandelbrot::IterUp(){
	
	
	
	
}*/

/*void worthsaving(){





}*/

/*void worthcontinue(){





}*/

/*bool Mandelbrot::DeepEnough(auto enough){
	
	//change enough
	
	return false;		//c'est assez on peut s'arreter et cracher l'image
	return true;		//pas assez, on continue
}*/

void Mandelbrot::dichotomie(int enough)
{
	this->escapeSpeedCalcThread2();
	this->draw2();

	if(this->IsGood())
	{
		this->save();
		//this->iterations = this->IterUp(enough);
		//this->iterations += this->iterations/2;

		if(--enough /*this->DeepEnough(enough)*/)
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
			
			Mandelbrot* M1 = new Mandelbrot(nx1, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations);		//en haut a gauche
			M1->dichotomie(enough);
			
			Mandelbrot* M2 = new Mandelbrot(nx2, ny2, nw, nh, im_width, im_height, surEchantillonage, iterations);		//en haut a droite
			M2->dichotomie(enough);
			
			Mandelbrot* M3 = new Mandelbrot(nx3, ny3, nw, nh,im_width, im_height, surEchantillonage, iterations);			//en bas a gauche
			M3->dichotomie(enough);		
				
			Mandelbrot* M4 = new Mandelbrot(nx4, ny4, nw, nh, im_width, im_height, surEchantillonage, iterations);		//en bas a droite
			M4->dichotomie(enough);
			
			
			mpf_clears(nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, nh, nw, temp, NULL);
			delete M1;
			delete M2;
			delete M3;
			delete M4;
		}

	}
	
}
