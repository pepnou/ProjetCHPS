#include "mandelbrot.hpp"

//using namespace VideoWriter;
using namespace cv;
using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations, int color, Mpmc* mpmc, char* rep) 
	: surEchantillonage(supSample), 
	im_width(im_w), 
	im_height(im_h), 
	iterations(iterations), 
	color(color), 
	mpmc(mpmc) ,
	divMat(new Mat(im_h*supSample, im_w*supSample, CV_32SC1)) ,
	img(new Mat(im_h, im_w, CV_8UC3)) ,
	sEMat(new Mat(im_h, im_w, CV_8UC1))
{
	mpf_init2(this->pos_x, mpf_get_prec(x));
	mpf_init2(this->pos_y, mpf_get_prec(y));

	//ecrire un truc pour que ca compile ici a cause de l'autre connard
	mpf_init2(this->width, mpf_get_prec(w));
	mpf_init2(this->height, mpf_get_prec(h));

	mpf_init2(atomic_w, mpf_get_prec(w) + ceil(log(im_w*supSample) / log(2)));
	mpf_init2(atomic_h, mpf_get_prec(h) + ceil(log(im_h*supSample) / log(2)));

	tasks.store(0);

	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);

	

	//~ this->ThresholdCont = /*125.7071478402/(pow((im_w*im_h),0.2597528761))*/ 5;
	//~ this->ThresholdSave = /*125.7071478402/(pow((im_w*im_h),0.2597528761))*/ 10;
	
	this->ThresholdCont = 181.75*pow(im_w*im_h,-0.309);
	this->ThresholdSave = 93.346*pow(im_w*im_h,-0.217);
	
	//  atomic_w = width / (im_width * surEchantillonage)
	//  atomic_h = height / (im_height * surEchantillonage)
	mpf_div_ui(atomic_w, this->width, this->im_width*this->surEchantillonage);
	mpf_div_ui(atomic_h, this->height, this->im_height*this->surEchantillonage);

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

Mandelbrot::~Mandelbrot()
{
	if(this->divMat != nullptr)
	{
		mpf_clears(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
		
		delete this->divMat;
		delete this->img;
		delete this->sEMat;
		// delete mpmc;
		this->divMat = nullptr;
		this->img = nullptr;
		this->sEMat = nullptr;
	}
}

void Mandelbrot::del_mem()
{
	mpf_clears(this->pos_x, this->pos_y, this->width, this->height, this->atomic_w, this->atomic_h, NULL);
	
	delete this->divMat;
	delete this->img;
	delete this->sEMat;
	// delete mpmc;
	this->divMat = nullptr;
	this->img = nullptr;
	this->sEMat = nullptr;
}


/*void Mandelbrot::escapeSpeedCalcThread2()
>>>>>>> refs/remotes/origin/master
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
}*/

void Mandelbrot::escapeSpeedCalcThread3()
{
	mpf_t tmp1, tmp2;
	mpf_t *x, *y;
	x = new mpf_t[this->im_width*this->surEchantillonage];
	y = new mpf_t[this->im_height*this->surEchantillonage];

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

	// this->partialDraw();
	this->draw();
	Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

	int lowThreshold = 10;
	int ratio = 3;
	int kernel_size = 3;

	cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
	blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
	Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
	filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);
	// matSave(this->sEMat,"sEMat_blur");

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

	delete [] x;
	delete [] y;
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

void Mandelbrot::escapeSpeedCalcThread4()
{
	mpf_t tmp1, tmp2;
	mpf_t *x, *y;
	x = new mpf_t[this->im_width*this->surEchantillonage];
	y = new mpf_t[this->im_height*this->surEchantillonage];

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






	*(this->sEMat) = 1;
	*(this->divMat) = -1;

	threadDraw *args = new threadDraw[this->im_height];
	work wo;
	wo.f = CallThreadCalc;

	int nbTasks = (this->im_height % Mandelbrot::pas == 0)?this->im_height / (Mandelbrot::pas):this->im_height / (Mandelbrot::pas) + 1;
	
	for(int i = 0; i < nbTasks; i++)
	{
		//cout<<this->tasks.fetch_add(1)<<endl;
		this->tasks.fetch_add(1);
		
		args[i].x = x;
		args[i].y = y;
		args[i].deb = i*this->im_height/nbTasks;
		args[i].fin = (i + 1)*this->im_height/nbTasks;
		args[i].M = this;
		//cout<<"+1"<<endl;
		
		wo.arg = (void*)&args[i];
		this->mpmc->push(wo);
	}
	//dbg
	while(this->tasks.load() != 0)
		this_thread::yield();
	//dbg
	
	
	
	
	
	this->draw();
	Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

	int lowThreshold = 10;
	int ratio = 3;
	int kernel_size = 3;

	cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
	blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
	Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
	filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);
	// matSave(this->sEMat,"sEMat_blur");

	*(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;






	for(int i = 0; i < nbTasks; i++)
	{
		this->tasks.fetch_add(1);

		wo.arg = (void*)&args[i];
		this->mpmc->push(wo);
	}
	//dbg
	while(this->tasks.load() != 0)
		this_thread::yield();
	//dbg


	
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_clear(x[i]);
	}
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_clear(y[i]);
	}

	delete [] x;
	delete [] y;
	delete [] args;
}

void Mandelbrot::CallThreadCalc(void* arg)
{
	threadDraw* args = (threadDraw*)arg;
	args->M->threadCalc4(arg);
}

void Mandelbrot::CallThreadCalcVideo(void* arg)
{
	threadDraw* args = (threadDraw*)arg;
	args->M->threadCalcVideo(arg);
}

//#include<fstream>
void Mandelbrot::threadCalc4(void* arg)
{
	//stringstream tmp2(""); tmp2 << "_" << this_thread::get_id();
	//ofstream out(tmp2.str(),"a");
	
	threadDraw* args = (threadDraw*)arg;

	mpf_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

	// cout<<"Calcul ligne : "<<args->ligne<<endl;

	for(int j = args->deb; j < args->fin; j++)
	{
		for (int i = 0; i < this->im_width; i++)
		{
			int sE = this->sEMat->at<char>( j, i);

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
							mpf_add(xnp1, xnp1, args->x[i*this->surEchantillonage+m]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

							//  ynp1 = 2*xn*yn + yc
							mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
							mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
							//mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
							mpf_add(ynp1, ynp1, args->y[j*this->surEchantillonage+n]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

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
	//dbgout<<"-"<<endl;
	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

	this->tasks.fetch_sub(1);
	//cout<<"Calcul ligne : "<<args->ligne<<endl;
	//out<<this->tasks.fetch_sub(1)<<endl;
}

void Mandelbrot::threadCalcVideo(void* arg)
{
	//stringstream tmp2(""); tmp2 << "dbg_" << this_thread::get_id();
	//ofstream dbgout(tmp2.str(),"a");
	
	threadDraw* args = (threadDraw*)arg;


	// cout<<"Calcul ligne : "<<args->ligne<<endl;

	mpf_t ***iter = new mpf_t**[this->im_width*this->surEchantillonage];

	for(int i = 0; i < this->im_width*this->surEchantillonage; i++)
	{
		iter[i] = new mpf_t*[(args->fin - args->deb)*this->surEchantillonage];
		
		for(int j = 0; j < (args->fin - args->deb)*this->surEchantillonage; j++)
		{
			iter[i][j] = new mpf_t[2];

			mpf_init2(iter[i][j][0], mpf_get_prec(this->pos_x));
			mpf_init2(iter[i][j][1], mpf_get_prec(this->pos_y));
		}
	}


	mpf_t xy, mod, xsqr, ysqr;
	mpf_inits( xy, mod, xsqr, ysqr, NULL);

	int iterCurrent = 1, Xindex, Yindex, Yindex2;

	for (int k = 1; k < this->iterations; k++)
	{
		for(int j = args->deb; j < args->fin; j++)
		{
			for (int i = 0; i < this->im_width; i++)
			{
				int sE = this->surEchantillonage;

				for(int m = 0; m < sE; m++)
				{
					for(int n = 0; n < sE; n++)
					{
						Xindex = i*this->surEchantillonage + m;
						Yindex = j*this->surEchantillonage + n;
						Yindex2 = (j - args->deb) * this->surEchantillonage + n;

						if(this->divMat->at<int>( Yindex, Xindex) == iterCurrent)
						{
							mpf_mul(xsqr, iter[Xindex][Yindex2][0], iter[Xindex][Yindex2][0]); //xn²
							mpf_mul(ysqr, iter[Xindex][Yindex2][1], iter[Xindex][Yindex2][1]); //yn²
							mpf_mul(xy, iter[Xindex][Yindex2][0], iter[Xindex][Yindex2][1]); //xn*yn

							mpf_sub(iter[Xindex][Yindex2][0], xsqr, ysqr); //  xnp1 = xsqr - ysqr = xn² - yn²
							mpf_add(iter[Xindex][Yindex2][0], iter[Xindex][Yindex2][0], args->x[Xindex]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

							mpf_mul_ui(iter[Xindex][Yindex2][1], xy, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
							mpf_add(iter[Xindex][Yindex2][1], iter[Xindex][Yindex2][1], args->y[Yindex]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

							//  mod = xnp1² + ynp1²
							mpf_add(mod, xsqr, ysqr); //  mod = xsqr + ysqr = xn² + yn²

							if(mpf_cmp_ui(mod, 4) > 0)
								this->divMat->at<int>(Yindex, Xindex) = iterCurrent;
							else
								this->divMat->at<int>(Yindex, Xindex) = iterCurrent + 1;
						}
					}
				}
			}
		}
		//cout << (*(this->divMat))(Range(args->ligne, args->ligne+1), Range::all()) << endl;
		//cout << (*(this->img))(Range(args->ligne, args->ligne+1), Range::all()) << endl;

		partialDraw( args->deb, args->fin, iterCurrent);
		frameSave( (*(this->img))(Range(args->deb, args->fin), Range::all()), this->rep, k, args->deb);
		
		

		iterCurrent++;
	}

	for(int i = 0; i < this->im_width*this->surEchantillonage; i++)
	{
		for(int j = 0; j < (args->fin - args->deb)*this->surEchantillonage; j++)
		{
			mpf_clear(iter[i][j][0]);
			mpf_clear(iter[i][j][1]);
			
			delete [] iter[i][j];
		}
		delete [] iter[i];
	}
	delete [] iter;


	mpf_clears( xy, mod, xsqr, ysqr, NULL);
	this->tasks.fetch_sub(1);
}

void Mandelbrot::partialDraw(int deb, int fin, int iter)
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < this->im_width; ++i)
	{
		for (int j = deb; j < fin; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < this->surEchantillonage; ++k)
			{
				for(int l = 0; l < this->surEchantillonage; ++l)
				{
					divSpeed = divMat->at<int>( j*this->surEchantillonage + l, i*this->surEchantillonage + k);

					if(divSpeed != -1)
					{
						if(divSpeed == iter)
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
				moy = iter;

			switch(this->color)
			{
				case 1:
				{
					coloration(bgr, moy, iter, nbr_div, nbr_ndiv);
					break;
				}
				case 2:
				{
					coloration2(bgr, moy, iter);
					break;
				}
				case 3:
				{
					coloration3(bgr, moy, iter);
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
	int nume=matSave( this->img, this->rep);

    char nom_inf[128];
    sprintf( nom_inf, "../img/%s/info.txt", this->rep);

    FILE* fichier = fopen(nom_inf, "a");

    fprintf(fichier,"mandel%d",nume);
    fprintf(fichier,"\n\tXposition=");
    mpf_out_str(fichier, 10, 150, pos_x);
    fprintf(fichier,"\n\tYposition=");
    mpf_out_str(fichier, 10, 150, pos_y);
    fprintf(fichier,"\n\twidth=");
    mpf_out_str(fichier, 10, 150, width);
    fprintf(fichier,"\n\theight=");
    mpf_out_str(fichier, 10, 150, height);
    fprintf(fichier,"\n");

    

	/*if(mpf_cmp_ui(this->pos_y, 0) != 0)
	{
		flip( *(this->img), *(this->img), 0);
		nume = matSave( this->img, this->rep);

		fprintf(fichier,"mandel%d",nume);
	    fprintf(fichier,"\n\tx : ");
	    mpf_out_str(fichier, 10, 150, pos_x);
	    fprintf(fichier,"\n\ty : ");
	    mpf_out_str(fichier, 10, 150, pos_y);
	    fprintf(fichier,"\n\tw : ");
	    mpf_out_str(fichier, 10, 150, width);
	    fprintf(fichier,"\n\th : ");
	    mpf_out_str(fichier, 10, 150, height);
	    fprintf(fichier,"\n");
	}*/

	fclose(fichier);
}

/*void Mandelbrot::animation_mais_ca_marche_pô()
{
	static int num = 0;

	if(num == 0)
	{
		sprintf( nom_img, "mkdir -p ../img/%s", rep);
		system(nom_img);
	}

	vid video(this->img,CV_FOURCC('M','J','P','G'),10, Size(this->width, this->height));

	char nom_img[128];
	sprintf( nom_img, "../img/%s/mandel%d.png", rep, num++);
	cout<<"calculating : "<<nom_img<<endl;


	while(condition)
	{
		//boom boom, refaire la matrice avec plus d'iterations
		vid_save(Mat* matarguments?);
	}*/

	//#include "vid_save.hpp"


	/*void vid_save(Mat* matarguments ?)
	{
		static int num = 0;

		if(num == 0)
		{
			sprintf( nom_img, "mkdir -p ../img/%s", rep);
			system(nom_img);
		}

		try
	    {
	    	vid<<this->img;
	        write(const Mat& image);
	    }
	    catch (const Exception& ex)
	    {
	        fprintf(stderr, "Exception adding frame to vid : %s\n", ex.what());
	    }
	}

	cou<<"done"<<endl;
}*/

bool Mandelbrot::IsGood(){
	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);

	int lowThreshold = 30;		//comment changer ça ?
	int ratio = 3;				//inutile de changer ca
	int kernel_size = 3;		//inutile de changer ca

	cvtColor( *(this->img), *(src_gray), CV_BGR2GRAY );
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);

	double res = countNonZero(*detected_edges)*255/(this->im_height*this->im_width);
	// cout << res << endl;

	delete src_gray;
	delete detected_edges;


	if(res >= this->ThresholdCont)
		return true;
	else
		return false;
}


bool Mandelbrot::IsGood_2(bool* filtre)
{

	bool continue_y_or_n;

	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);		//entier non signé 8 bit à 3 dimension
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);	//pareil a 2 dimension
	if(!detected_edges) cout<<"fuck"<<endl;
	if(!src_gray) cout<<"fuck2"<<endl;



	//cout<<im_width<<" x "<<im_height<<endl;
	int lowThreshold = 30;		//comment changer ça ?
	int ratio = 3;				//inutile de changer ca
	int kernel_size = 3;		//inutile de changer ca

	cvtColor( *(this->img), *(src_gray), CV_BGR2GRAY );
	
	//imshow("test",* src_gray);
	//waitKey(0);

	//matSave( this->img, "tout_va_bien");
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	//matSave( detected_edges, "tout_va_bien_oupaslol");
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);

	double res = countNonZero(*detected_edges)*1000/(this->im_height*this->im_width);

	//cout<<res<<endl;

	if(res<this->ThresholdCont)
		continue_y_or_n = false;
	else
		continue_y_or_n = true;


	if(continue_y_or_n)
	{

		//https://fr.wikipedia.org/wiki/Fonction_gaussienne


	//matSave( this->img, "reduction");

		int y0 = this->im_width/2;
		int x0 = this->im_height/2;
		int sigma_x = x0/2;
		int sigma_y = y0/2;

		//cout<<detected_edges->channels()<<endl<<endl;
		//cout<<detected_edges->elemSize1()<<endl<<endl;

		/*cout<<*detected_edges<<endl<<endl;

		for (int i = 0; i < im_height; i++)
		{
			for (int j = 0; j < im_width; j++)
			{
				cout<<(int)detected_edges->at<char>(i, j)<<" ";
			}
			cout<<endl;
		}*/

		res = 0.0;

		for (int i = 0; i < im_height; i++)
		{
			for (int j = 0; j < im_width; j++)
			{

				double X =(pow(i - x0, 2)/(2*pow(sigma_x, 2)));

				double Y =(pow(j - y0, 2)/(2*pow(sigma_y, 2)));

				double flou = exp(-(X + Y));
				//cout<<flou<<" ";

				// detected_edges->at<char>(i, j) *= (1-flou)*255;

				//detected_edges->at<char>(i, j) = ((detected_edges->at<char>(i, j)+256)%256) * flou;
				res += (((detected_edges->at<char>(i, j)+256)%256) * flou);
			}
			//cout<<endl;
		}

		/*cout<<endl;
		cout<<endl;*/


		//matSave( detected_edges, "contours_filtrés");


		//res = countNonZero(*detected_edges)*255/(this->im_height*this->im_width);
		res = res/(this->im_height*this->im_width)*1000/255;

		//cout<<res<<endl<<endl;
		if(res >= this->ThresholdSave)
			*filtre = true;
		else
			*filtre = false;
	}

	delete src_gray;
	delete detected_edges;

	return continue_y_or_n;
}


void Mandelbrot::IterUp(){

	//augmentation du nombre d'iteration en fonction de la profondeur du zoom actuel
	//66.5*racine(2*racine(abs(1 - racine(5*(scale)))))	with scale = this->width/3

	mpf_t temp;
	mpf_inits(temp, NULL);

	//temp = scale = this->width/3
	mpf_div(temp, atomic_w, this->width);

	//temp = 5*temp
	mpf_mul_ui(temp, temp, 5);

	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = 1 - temp
	mpf_ui_sub(temp, 1, temp);

	//temp = abs(temp)
	mpf_abs(temp, temp);
	
	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = 2.temp
	mpf_mul_ui(temp, temp, 2);

	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = temp*66.5
	mpf_mul_ui(temp, temp, 110);

	//convertissation
	double x = mpf_get_d(temp);

	iterations = x;

	mpf_clears(temp, NULL);
	
}


void Mandelbrot::dichotomie(int enough, int prec)
{
	this->escapeSpeedCalcThread4();

	this->draw();

	bool filtre;
	
	if(this->IsGood_2(&filtre)/*this->IsGood*/)
	{
		if(filtre)
			this->save();

		if(--enough)
		{
			int n_prec = prec + ceil(log(/*divs[i]*/2)/log(2));

			//augmente la nombre d'iteration max, ceci est un commentaire Nassim tu le vois celui la ?
			this->IterUp();

			mpf_t nx1, ny1, nx2, ny3, nh, nw, temp;
			mpf_inits(nx1, ny1, nx2, ny3, nh, nw, temp, NULL);

			if(mpf_get_prec(this->pos_x)>=mpf_get_prec(this->pos_y))
				mpf_init2(temp, mpf_get_prec(this->pos_x) + n_prec/64);
			else
				mpf_init2(temp, mpf_get_prec(this->pos_y) + n_prec/64);

			mpf_init2(nx1, mpf_get_prec(this->pos_x) + n_prec/64);
			mpf_init2(nx2, mpf_get_prec(this->pos_x) + n_prec/64);

			mpf_init2(ny1, mpf_get_prec(this->pos_y) + n_prec/64);
			mpf_init2(ny3, mpf_get_prec(this->pos_y) + n_prec/64);

			mpf_init2(nw, mpf_get_prec(this->width));
			mpf_init2(nh, mpf_get_prec(this->height));

			n_prec %= 64;
			
			// newx = x - this->width/2
			mpf_div_ui(temp, this->width, 4);		//calcul nouveaux x pour reiterer
			mpf_sub(nx1, this->pos_x, temp);
			mpf_add(nx2, this->pos_x, temp);
			
			// newy = y + this->height/2
			mpf_div_ui(temp, this->height, 4);		//calcul nouveaux y
			mpf_add(ny1, this->pos_y, temp);
			mpf_sub(ny3, this->pos_y, temp);
			
			// newh = h/2
			mpf_div_ui(nh, this->height, 2);
			
			//neww = w/2
			mpf_div_ui(nw, this->width, 2);

			//int surEchantillonage_bis = this->surEchantillonage, im_width_bis = this->im_width, im_height_bis = this->im_height, iterations_bis = this->iterations;
			this->del_mem();
			//delete l'image ou on est
			
			if(mpf_cmp_ui(this->pos_y, 0) != 0)
			{
				Mandelbrot* M1 = new Mandelbrot(nx1, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->mpmc, this->rep);		//en haut a gauche
				M1->dichotomie(enough, n_prec);
				delete M1;
				
				Mandelbrot* M2 = new Mandelbrot(nx2, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->mpmc, this->rep);		//en haut a droite
				M2->dichotomie(enough, n_prec);
				delete M2;
			}

			Mandelbrot* M3 = new Mandelbrot(nx1, ny3, nw, nh,im_width, im_height, surEchantillonage, iterations, this->color, this->mpmc, this->rep);			//en bas a gauche
			M3->dichotomie(enough, n_prec);
			delete M3;
				
			Mandelbrot* M4 = new Mandelbrot(nx2, ny3, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->mpmc, this->rep);		//en bas a droite
			M4->dichotomie(enough, n_prec);
			delete M4;


			mpf_clears(nx1, ny1, nx2, ny3, nh, nw, temp, NULL);
		}
	}
}


void Mandelbrot::dichotomie2(int enough, int n_div, vector<int>& divs, int prec)
{
	//cout<<"oui"<<endl;
	
	this->escapeSpeedCalcThread4();

	this->draw();

	bool filtre;

	if(/*true*/this->IsGood_2(&filtre))
	{
		if(filtre/*true*/)
			this->save();

		if(--enough)
		{	
			//augmente la nombre d'iteration max, ceci est un commentaire Nassim tu le vois celui la ?
			this->IterUp();

			mpf_t old_pos_x, old_pos_y, old_width, old_height;

			mpf_init2(old_pos_x, mpf_get_prec(this->pos_x));
			mpf_init2(old_pos_y, mpf_get_prec(this->pos_y));
			mpf_init2(old_width, mpf_get_prec(this->width));
			mpf_init2(old_height, mpf_get_prec(this->height));

			mpf_set(old_pos_x, this->pos_x);
			mpf_set(old_pos_y, this->pos_y);
			mpf_set(old_width, this->width);
			mpf_set(old_height, this->height);

			/*recuper dans les old les valeurs de "this->"
			et remplacer les this-> dans la suite par des old_*/

			//delete l'image ou on est
			this->del_mem();

			for(int i = 0; i < n_div; i++)
			{
				cout<<"Welcome back"<<endl;
				int n_prec = prec + ceil(log(divs.at(i))/log(2));
				mpf_t temp, delta_x, delta_y;
				cout<<divs.at(i)<<endl;

				mpf_t* tab_x = new mpf_t[divs.at(i)];
				mpf_t* tab_y = new mpf_t[divs.at(i)];

				//mpf_inits(temp, delta_x, delta_y, NULL);
				if(mpf_get_prec(old_pos_x)>=mpf_get_prec(old_pos_y))
					mpf_init2(temp, mpf_get_prec(old_pos_x) + n_prec/64);
				else
					mpf_init2(temp, mpf_get_prec(old_pos_y) + n_prec/64);

				mpf_init2(delta_x, mpf_get_prec(old_width));
				mpf_init2(delta_y, mpf_get_prec(old_height));

				//initialise chacun des elements du tableau avant de pouvoir s'en servir
				for (int init = 0; init < divs.at(i); init++)
				{
					//mpf_init(tab_x[init]);
					//mpf_init(tab_y[init]);
					mpf_init2(tab_x[init], mpf_get_prec(old_pos_x) + n_prec/64);
					mpf_init2(tab_y[init], mpf_get_prec(old_pos_y) + n_prec/64);
				}

				n_prec %= 64;


					//calcul de delta_x, la distance entre de nouveaux points en x
				mpf_div_ui(delta_x, old_width, divs.at(i));
					//calcul de delta_y, la distance entre de nouveaux points en y
				mpf_div_ui(delta_y, old_height, divs.at(i));


					//tab_x[0] = pos_x - width/2 + width/2*divs.at(i)
				mpf_div_ui(temp, old_width, 2*divs.at(i));
				mpf_add(tab_x[0], old_pos_x, temp);
				mpf_div_ui(temp, old_width, 2);
				mpf_sub(tab_x[0], tab_x[0], temp);

					//tab_y[0] = pos_y - height/2 + height/2*divs.at(i)
				mpf_div_ui(temp, old_height, 2*divs.at(i));
				mpf_add(tab_y[0], old_pos_y, temp);
				mpf_div_ui(temp, old_height, 2);
				mpf_sub(tab_y[0], tab_y[0], temp);

				for (int c = 1; c < divs.at(i); c++)
				{
					//tab_x[c] = tab_x[0] + c*delta_x
					mpf_mul_ui(temp, delta_x, c);
					mpf_add(tab_x[c], tab_x[0], temp);

					//tab_y[c] = tab_y[0] + c*delta_y
					mpf_mul_ui(temp, delta_y, c);
					mpf_add(tab_y[c], tab_y[0], temp);
				}
				//cout<<divs.at(i)<<endl;

				for (int x = 0; x < divs.at(i); x++)
				{
					for (int y = 0; y < divs.at(i); y++)
					{
						if(mpf_cmp_ui(tab_y[y], 0) < 0)
						{
						//cout<<divs.at(i)<<endl;
						//cout<<"i = "<<i<<endl<<"x : "<<x<<endl<<"y : "<<y<<endl;
							
						Mandelbrot* M = new Mandelbrot(tab_x[x], tab_y[y], delta_x, delta_y ,im_width, im_height, surEchantillonage, iterations, color, mpmc, rep);			//en bas a gauche
						
						M->dichotomie2(enough, n_div, divs, n_prec);
						
						delete M;
						
						}
						
					}
				}

				//delete chacun des element du tableau avant pour pouvoir virer le tableau
				for (int del = 0; del < divs.at(i); del++)
				{
					mpf_clear(tab_x[del]);
					mpf_clear(tab_y[del]);
				}

				delete [] tab_x;
				delete [] tab_y;
				mpf_clears(temp, delta_x, delta_y, NULL);
				cout<<"au revoir"<<endl<<endl;
			}
		}
	}
}


bool Mandelbrot::alea(int enough, int prec)
{
	this->escapeSpeedCalcThread4();

	this->draw();

	bool filtre;

	if(this->IsGood_2(&filtre)/*this->IsGood*/)
	{
		if(filtre)
			this->save();
			
		this->IterUp();

		if(--enough)
		{
			gmp_randstate_t state;
			gmp_randinit_default (state);
			gmp_randseed_ui(state, rand());

			int n_prec = prec + ceil(log(/*divs[i]*/2)/log(2));
			
			mpf_t nx, ny, nh, nw, temp, cp_x, cp_y;
			//mpf_inits(nx1, ny1, nx2, ny2, nx3, ny3, nx4, ny4, nh, nw, temp, NULL);

			if(mpf_get_prec(this->pos_x)>=mpf_get_prec(this->pos_y))
			{
				mpf_init2(temp, mpf_get_prec(this->pos_x) + n_prec/64);
			}
			else
			{
				mpf_init2(temp, mpf_get_prec(this->pos_y) + n_prec/64);
			}

			mpf_init2(nx, mpf_get_prec(this->pos_x) + n_prec/64);
			mpf_init2(ny, mpf_get_prec(this->pos_y) + n_prec/64);

			mpf_init2(cp_x, mpf_get_prec(this->pos_x));
			mpf_init2(cp_y, mpf_get_prec(this->pos_y));

			mpf_init2(nw, mpf_get_prec(this->width));
			mpf_init2(nh, mpf_get_prec(this->height));

			n_prec %= 64;
			
			// newh = h/2
			mpf_div_ui(nh, this->height, 2);
			
			//neww = w/2
			mpf_div_ui(nw, this->width, 2);

			bool find = false;

			int bcp_trop = 20;

			while(!find && bcp_trop)
			{
				mpf_urandomb(nx, state, mpf_get_prec(nx));
				mpf_mul(nx, nx, nw);
				mpf_add(nx, nx, cp_x);
				mpf_div_ui(temp, nw, 2);
				mpf_sub(nx, nx, temp);

				mpf_urandomb(ny, state, mpf_get_prec(nx));
				mpf_mul(ny, ny, nh);
				mpf_add(ny, ny, cp_y);
				mpf_div_ui(temp, nh, 2);
				mpf_sub(ny, ny, temp);


				Mandelbrot* M = new Mandelbrot(nx, ny, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->mpmc, this->rep);		//en haut a gauche
				find = M->alea(enough, n_prec);
				delete M;

				bcp_trop--;
			}
			mpf_clears(nx, ny, temp , cp_x, cp_y, NULL);
		}
		del_mem();
		return true;
	}
	return false;
}


void Mandelbrot::video()
{
	//a voir pour la precision

	stringstream videoName("");
	videoName << "mkdir -p ../video/" << this->rep;
	system(videoName.str().c_str());

	videoName.str("");

	videoName << "../video/" << this->rep << "/test.avi";
	cout  << videoName.str().c_str() << endl;



	//Size S = Size(this->im_width*this->surEchantillonage, this->im_height*this->surEchantillonage);
	Size S = Size( this->img->cols, this->img->rows);
	VideoWriter outputVideo;

	int ex = outputVideo.fourcc('H','2','6','4');

	outputVideo.open(videoName.str().c_str(), ex, 20, S, true);
	
	if (!outputVideo.isOpened())
	{
		cout  << "Could not open the output video for write. " << endl;
	}
	else
	{
		mpf_t*** iter = new mpf_t**[this->im_width*this->surEchantillonage];
		for(int i = 0; i < this->im_width*this->surEchantillonage; i++)
		{
			iter[i] = new mpf_t*[this->im_height*this->surEchantillonage];
			for(int j = 0; j < this->im_height*this->surEchantillonage; j++)
			{
				iter[i][j] = new mpf_t[2];

				mpf_init2(iter[i][j][0], mpf_get_prec(this->pos_x));
				mpf_init2(iter[i][j][1], mpf_get_prec(this->pos_x));

				mpf_set_ui(iter[i][j][0], 0);
				mpf_set_ui(iter[i][j][1], 0);
			}
		}


		mpf_t *x, *y, tmp1, tmp2;
		x = new mpf_t[this->im_width*this->surEchantillonage];
		y = new mpf_t[this->im_height*this->surEchantillonage];

		mpf_inits( tmp1, tmp2, NULL);

		mpf_div_ui(tmp1, this->width, 2); //  tmp1 = width/2
		mpf_set_ui( tmp2, 0);
		for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
		{
			mpf_init(x[i]);
			//  xc = pos_x - width/2 + i*atomic_w
			mpf_sub(x[i], this->pos_x, tmp1); //  xc = pos_x - tmp = pos_x - width/2
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
			mpf_add( tmp2, tmp2, atomic_h);
			mpf_add(y[i], y[i], tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
		}

		mpf_clears( tmp1, tmp2, NULL);





		mpf_t xsqr, ysqr, xy, mod;
		mpf_inits( xsqr, ysqr, xy, mod, NULL);

		//Mat* precMat = new Mat(im_height, im_width, CV_8UC3);
		//Vec3b nullVector = { 0, 0, 0};
		*(this->divMat) = 1;
		//*precMat = nullVector;

		int iterCurrent = 1;

		cout << this->iterations << endl;

		//do
		for(int k = 0; k < this->iterations; k++)
		{
			cout<< "iteration : " << k <<endl;

			for(int i = 0; i < this->im_width*this->surEchantillonage; i++)
			{
				for(int j = 0; j < this->im_height*this->surEchantillonage; j++)
				{
					if(this->divMat->at<int>( j, i) == iterCurrent)
					{
						mpf_mul(xsqr, iter[i][j][0], iter[i][j][0]); //xn²
						mpf_mul(ysqr, iter[i][j][1], iter[i][j][1]); //yn²
						mpf_mul(xy, iter[i][j][0], iter[i][j][1]); //xn*yn


						mpf_sub(iter[i][j][0], xsqr, ysqr); //  xnp1 = xsqr - ysqr = xn² - yn²
						mpf_add(iter[i][j][0], iter[i][j][0], x[i]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

						mpf_mul_ui(iter[i][j][1], xy, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
						mpf_add(iter[i][j][1], iter[i][j][1], y[j]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

						//  mod = xnp1² + ynp1²
						mpf_add(mod, xsqr, ysqr); //  mod = xsqr + ysqr = xn² + yn²

						if(mpf_cmp_ui(mod, 4) > 0)
							this->divMat->at<int>(j, i) = iterCurrent;
						else
							this->divMat->at<int>(j, i) = iterCurrent + 1;
					}
				}
			}

			//this->iterations = iterCurrent;

			partialDraw(0, this->im_height, iterCurrent);
			//save();

			outputVideo << *(this->img);

			iterCurrent++;
		}/* while(1);*/

		mpf_clears( xsqr, ysqr, xy, mod, NULL);
		
		for(int i = 0; i < this->im_width*this->surEchantillonage; i++)
		{
			mpf_clear(x[i]);

			for(int j = 0; j < this->im_height*this->surEchantillonage; j++)
			{
				mpf_clear(iter[i][j][0]);
				mpf_clear(iter[i][j][1]);
				
				delete [] iter[i][j];
			}
			delete [] iter[i];
		}
		delete [] iter;

		for(int j = 0; j < this->im_height*this->surEchantillonage; j++)
			mpf_clear(y[j]);

		delete [] x;
		delete [] y;
	}
	
	del_mem();
}

void Mandelbrot::video2()
{
	mpf_t *x, *y, tmp1, tmp2;
	x = new mpf_t[this->im_width*this->surEchantillonage];
	y = new mpf_t[this->im_height*this->surEchantillonage];

	mpf_inits( tmp1, tmp2, NULL);

	mpf_div_ui(tmp1, this->width, 2); //  tmp1 = width/2
	mpf_set_ui( tmp2, 0);
	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_init(x[i]);
		//  xc = pos_x - width/2 + i*atomic_w
		mpf_sub(x[i], this->pos_x, tmp1); //  xc = pos_x - tmp = pos_x - width/2
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
		mpf_add( tmp2, tmp2, atomic_h);
		mpf_add(y[i], y[i], tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
	}

	mpf_clears( tmp1, tmp2, NULL);
	
	*(this->divMat) = 1;

	threadDraw *args = new threadDraw[this->im_height];
	work wo;
	wo.f = CallThreadCalcVideo;

	int nbTasks = (this->im_height % Mandelbrot::pas == 0)?this->im_height / (Mandelbrot::pas):this->im_height / (Mandelbrot::pas) + 1;
	
	for(int i = 0; i < nbTasks; i++)
	{
		this->tasks.fetch_add(1);
		
		args[i].x = x;
		args[i].y = y;
		args[i].deb = i*this->im_height/nbTasks;
		args[i].fin = (i + 1)*this->im_height/nbTasks;
		args[i].M = this;
		
		wo.arg = (void*)&args[i];
		this->mpmc->push(wo);
	}
	while(this->tasks.load() != 0)
		this_thread::yield();
	
	
	/*
		TRUC A COMPLETER :
			>> RECONSTITUTION DES FRAMES
			>> CREATION DE LA VIDEO
	*/

	stringstream videoName("");
	videoName << "mkdir -p ../video/" << this->rep;
	system(videoName.str().c_str());

	videoName.str("");

	videoName << "../video/" << this->rep << "/test.avi";
	cout  << videoName.str().c_str() << endl;

	stringstream framePart("");

	Size S = Size( this->img->cols, this->img->rows);
	VideoWriter outputVideo;

	int ex = outputVideo.fourcc('H','2','6','4');

	outputVideo.open(videoName.str().c_str(), ex, 20, S, true);
	
	if (!outputVideo.isOpened())
	{
		cout  << "Could not open the output video for write. " << endl;
	}
	else
	{
		for(int i = 1; i < this->iterations; i++)
		{
			for(int j = 0; j < nbTasks; j++)
			{
				framePart.str("");
				framePart << "../video/" << rep << "/frames/" << i << "/" << j*this->im_height/nbTasks << ".png";
				
				Mat src = imread( framePart.str().c_str(), IMREAD_COLOR );
				src.copyTo((*(this->img))(cv::Rect(0,j*this->im_height/nbTasks,this->im_width, (j+1)*this->im_height/nbTasks - j*this->im_height/nbTasks)));
			}
			outputVideo << (*(this->img));
		}
	}


	for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
	{
		mpf_clear(x[i]);
	}
	for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
	{
		mpf_clear(y[i]);
	}

	delete [] x;
	delete [] y;
	delete [] args;

}

void testConcatImg()
{
	Mat src1 = imread( "/home/user/Bureau/ProjetCHPS/img/Fri_Nov_30_10-00-56_2018/mandel4.png", IMREAD_COLOR );
	Mat src2 = imread( "/home/user/Bureau/ProjetCHPS/img/Fri_Nov_30_10-00-56_2018/mandel5.png", IMREAD_COLOR );

	imshow( "src1", src1 );
	imshow( "src2", src2 );
	waitKey(0);

	//cout << src1.rows + src2.rows << endl;

	Mat* dst = new Mat(src1.rows + src2.rows, src1.cols, src1.type());
	
	//(*dst)(Range(0, src1.rows - 1), Range::all()) = src1;
	//(*dst)(Range(src1.rows, src1.rows + src2.rows), Range::all()) = src2;
	src1.copyTo((*dst)(cv::Rect(0,0,src1.cols, src1.rows)));
	src2.copyTo((*dst)(cv::Rect(0,src1.rows,src1.cols, src1.rows)));


	imshow( "dst1", (*dst)(Range(0, src1.rows - 1), Range::all()) );
	imshow( "dst2", (*dst)(Range(src1.rows, src1.rows + src2.rows), Range::all()) );
	imshow( "dst", *dst );
	waitKey(0);

	cout << *dst << endl;

	vector<int> compression_params;
    compression_params.push_back( IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
	
	try
    {
        imwrite("../img/test.png", *dst, compression_params);
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    }
}


/*
Any primitive type from the list can be defined by an identifier in the form CV_<bit-depth>{U|S|F}C(<number_of_channels>)
where U is unsigned integer type, S is signed integer type, and F is float type.

*/


bool Mandelbrot::random_img (int enough)
{

  gmp_randstate_t state;
  gmp_randinit_mt(state);
	
	printf("test0\n");
	
	this->escapeSpeedCalcThread3();
	this->draw();
	//this->save();

//	unsigned int na = rand();

	printf("rand_img...\n");

	Mandelbrot* M1;
	//if(this->IsGood())
	bool filtre;

	if(this->IsGood_2(&filtre)/*this->IsGood*/)
	{
		
		//this->iterations = this->IterUp(enough);
		//this->iterations += this->iterations/2;
		//this->save();
		
		if(filtre)
			this->save();

		//this->worthsaving();
		int max_test = 100;
		int enoughcompte=enough;
		mylabel:
		//if(/*--max_test*/--enough ) // this->DeepEnough(enough)
		//{
			this->IterUp();

			printf("good:%d\n",enough);
			mpf_t nx1, ny1, nh, nw, temp;
			
			mpf_inits(nx1, ny1, nh, nw, temp, NULL);
			

			/* newx = x - x/2 & y + y/2
			mpf_div_ui(temp, this->width, 2);		//calcul nouveaux x pour reiterer
			mpf_sub(nx1, this->pos_x, temp);
	
			
			mpf_div_ui(temp, this->height, 2);		//calcul nouveaux y
			mpf_add(ny1, this->pos_y, temp);*/
			

			// newh = h/2, neuh=w/2
			mpf_div_ui(nw, this->width, 2);		//calcul nouveaux W 
			mpf_div_ui(nh, this->height, 2);		//calcul nouveaux H


			
			



			gmp_printf(" --- nw, nh = %.5Ff, %.5Ff \n",nw,nh);

			printf("apres set\n");
			//int max_test = 100;
			Mandelbrot* M2;
			do{
				printf("test -1\n");
				

				if (max_test != 0/*enough==4*/)  mpf_urandomb (nx1, state, 64);
				
			//mpf_set_z(nx1,temp);		//random nw
//				if (enough==4) mpz_urandomm (TEMP, state, NH);
				if (max_test !=0  /*enough == 4*/) mpf_urandomb (ny1, state, 64);

				gmp_printf(" --- nx1, ny1 = %.5Ff, %.5Ff \n",nx1,ny1);
//			mpf_set_z(ny1,TEMP);		//random nh

				//M1 = new Mandelbrot(nx1, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations*2);
				M2 = new Mandelbrot(nx1, ny1, nw, nh, im_width, im_height, surEchantillonage, iterations, this->color, this->rep);	
		
				printf("test1\n");
	
				--max_test;

			 } while( ( !M2->IsGood_2(&filtre) ) && (max_test > 0));
			 enoughcompte--;
			 M2->save();
			 M2->random_img (enough);
			
			mpf_clears(nx1, ny1, nh, nw, temp, NULL);
			delete M2;
			if (enoughcompte>0) goto mylabel;

            
			
		//}

	}
	else{
	    printf("not good:%d\n",enough);
	}   
}