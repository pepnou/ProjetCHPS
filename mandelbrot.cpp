#include "mandelbrot.hpp"
#include "workHandler.hpp"
#include <string.h>
#include <omp.h>
#include <pthread.h>

using namespace cv;
using namespace std;


Mandelbrot::Mandelbrot(mpfr_t x, mpfr_t y, mpfr_t w, mpfr_t h, int enough, std::vector<int> divs) 
	: enough(enough) ,
        divs(divs) ,
	divMat(new Mat(im_height*surEchantillonage, im_width*surEchantillonage, CV_32SC1)) ,
	img(new Mat(im_height, im_width, CV_8UC3)) ,
	sEMat(new Mat(im_height, im_width, CV_8UC1))
{
	mpfr_init2(pos_x, mpfr_get_prec(x));
	mpfr_init2(pos_y, mpfr_get_prec(y));

	mpfr_init2(width, mpfr_get_prec(w));
	mpfr_init2(height, mpfr_get_prec(h));

	mpfr_init2(atomic_w, mpfr_get_prec(w) + ceil(log(im_width*surEchantillonage) / log(2)));
	mpfr_init2(atomic_h, mpfr_get_prec(h) + ceil(log(im_height*surEchantillonage) / log(2)));

	mpfr_set(pos_x, x, MPFR_RNDN);
	mpfr_set(pos_y, y, MPFR_RNDN);
	mpfr_set(width, w, MPFR_RNDN);
	mpfr_set(height, h, MPFR_RNDN);
	
	ThresholdCont = 181.75*pow(im_width*im_height,-0.309);
	ThresholdSave = 93.346*pow(im_width*im_height,-0.217);


	mpfr_div_ui(atomic_w, width, im_width*surEchantillonage, MPFR_RNDN);
	mpfr_div_ui(atomic_h, height, im_height*surEchantillonage, MPFR_RNDN);
        
        IterUp();
}

Mandelbrot::Mandelbrot(char* buf)
    :divMat(new Mat(im_height*surEchantillonage, im_width*surEchantillonage, CV_32SC1)) ,
    img(new Mat(im_height, im_width, CV_8UC3)) ,
    sEMat(new Mat(im_height, im_width, CV_8UC1))
{
    int prec;
    char* tmp;


    tmp = strtok( buf, ":");
    enough = atoi(tmp);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpfr_init2( pos_x, prec);
    mpfr_set_str( pos_x, tmp, 10, MPFR_RNDN);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpfr_init2( pos_y, prec);
    mpfr_set_str( pos_y, tmp, 10, MPFR_RNDN);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpfr_init2( width, prec);
    mpfr_set_str( width, tmp, 10, MPFR_RNDN);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpfr_init2( height, prec);
    mpfr_set_str( height, tmp, 10, MPFR_RNDN);


    while((tmp = strtok(NULL, ":")))
        divs.push_back(atoi(tmp));


    mpfr_init2(atomic_w, mpfr_get_prec(width) + ceil(log(im_width*surEchantillonage) / log(2)));
    mpfr_init2(atomic_h, mpfr_get_prec(height) + ceil(log(im_height*surEchantillonage) / log(2)));

   	
    ThresholdCont = 181.75*pow(im_width*im_height,-0.309);
    ThresholdSave = 93.346*pow(im_width*im_height,-0.217);
	

    mpfr_div_ui(atomic_w, width, im_width*surEchantillonage, MPFR_RNDN);
    mpfr_div_ui(atomic_h, height, im_height*surEchantillonage, MPFR_RNDN);
        
    IterUp();
}

Mandelbrot::~Mandelbrot()
{
	if(divMat != nullptr)
	{
		mpfr_clears(pos_x, pos_y, width, height, atomic_w, atomic_h, NULL);
		
		delete divMat;
		delete img;
		delete sEMat;

		divMat = nullptr;
	        img = nullptr;
		sEMat = nullptr;
	}
}

void Mandelbrot::del_mem()
{
	mpfr_clears(pos_x, pos_y, width, height, atomic_w, atomic_h, NULL);
	
	delete divMat;
	delete img;
	delete sEMat;

	divMat = nullptr;
	img = nullptr;
	sEMat = nullptr;
}

void Mandelbrot::escapeSpeedCalcSeq()
{

    // initialisation des coordonnées
    mpfr_t tmp1, tmp2;
    mpfr_t *x, *y;
    x = new mpfr_t[this->im_width*this->surEchantillonage];
    y = new mpfr_t[this->im_height*this->surEchantillonage];

    if(!x || !y)
	exit(2);

    mpfr_inits( tmp1, tmp2, NULL);

    mpfr_div_ui(tmp1, this->width, 2, MPFR_RNDN); //  tmp1 = width/2
    mpfr_set_ui( tmp2, 0, MPFR_RNDN);
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
	mpfr_init(x[i]);

	//  xc = pos_x - width/2 + i*atomic_w
	mpfr_sub(x[i], this->pos_x, tmp1, MPFR_RNDN); //  xc = pos_x - tmp = pos_x - width/2
	//mpfr_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * i
	mpfr_add( tmp2, tmp2, atomic_w, MPFR_RNDN);
	mpfr_add(x[i], x[i], tmp2, MPFR_RNDN); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

    }

    mpfr_div_ui(tmp1, this->height, 2, MPFR_RNDN); //  tmp1 = height/2
    mpfr_set_ui( tmp2, 0, MPFR_RNDN);
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
	mpfr_init(y[i]);

	//  yc = pos_y - height/2 + i*atomic_h
	mpfr_sub(y[i], this->pos_y, tmp1, MPFR_RNDN); //  yc = pos_y - tmp = pos_y - height/2
	//mpfr_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
	mpfr_add( tmp2, tmp2, atomic_h, MPFR_RNDN);
	mpfr_add(y[i], y[i], tmp2, MPFR_RNDN); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
    }

    mpfr_clears( tmp1, tmp2, NULL);


    *(this->sEMat) = 1;
    *(this->divMat) = -1;
    

    // calcul de l image
    calcSeq( x, y);	
	
	
	
    // on detecte les zones nécessitant dur sur echantillonage
    this->draw();
    Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

    int lowThreshold = 10;
    int ratio = 3;
    int kernel_size = 3;

    cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
    blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
    Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
    filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);

    *(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;

    // on recalcul l image avec le sur echantillonage
    calcSeq( x, y);

    // on libere tout
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
	    mpfr_clear(x[i]);
    }
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
	    mpfr_clear(y[i]);
    }

    delete [] x;
    delete [] y;
}


void Mandelbrot::calcSeq(mpfr_t* x, mpfr_t* y)
{
    mpfr_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
    mpfr_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

    for(int j = 0; j < im_height; j++)
    {
        for (int i = 0; i < im_width; i++)
        {
            int sE = sEMat->at<char>( j, i);

            for(int m = 0; m < sE; m++)
            {
                for(int n = 0; n < sE; n++)
                {
                    if((sE != 1 && divMat->at<int>(j*surEchantillonage+n ,i*surEchantillonage+m) == -1) || sE == 1)
                    {
                        mpfr_set_ui(xn,0, MPFR_RNDN);
                        mpfr_set_ui(yn,0, MPFR_RNDN);
                        mpfr_set_ui(xsqr,0, MPFR_RNDN);
                        mpfr_set_ui(ysqr,0, MPFR_RNDN);

                        for (int k = 1; k < iterations; k++)
                        {
                            //  xnp1 = xn² - yn² + xc
                            mpfr_sub(xnp1, xsqr, ysqr, MPFR_RNDN); //  xnp1 = xsqr - ysqr = xn² - yn²
                            //mpfr_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
                            mpfr_add(xnp1, xnp1, x[i*surEchantillonage+m], MPFR_RNDN); //  xnp1 = xnp1 + xc = xn² - yn² + xc

                            //  ynp1 = 2*xn*yn + yc
                            mpfr_mul(ynp1, xn, yn, MPFR_RNDN); //  ynp1 = xn * yn
                            mpfr_mul_ui(ynp1, ynp1, 2, MPFR_RNDN); //  ynp1 = ynp1 * 2 = 2 * xn * yn
                            //mpfr_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
                            mpfr_add(ynp1, ynp1, y[j*surEchantillonage+n], MPFR_RNDN); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

                            //  xn = xnp1
                            //  yn = ynp1
                            mpfr_set( xn, xnp1, MPFR_RNDN); //  xn = xnp1
                            mpfr_set( yn, ynp1, MPFR_RNDN); //  yn = ynp1
                                        
                            //xsqr = xn²
                            mpfr_mul(xsqr, xn, xn, MPFR_RNDN);

                            //ysqr = yn²
                            mpfr_mul(ysqr, yn, yn, MPFR_RNDN);

                            //  mod = xnp1² + ynp1²
                            mpfr_add(mod, xsqr, ysqr, MPFR_RNDN); //  mod = xsqr + ysqr = xn² + yn²

                            if(mpfr_cmp_ui(mod, 4) > 0)
                            {
                                divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = k;
                                break;
                            } 
                            else if(k == iterations -1)
                            {
                                divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = iterations;
                            }
                        }
                    }
                }
            }
        }
    }

    mpfr_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

}

void Mandelbrot::escapeSpeedCalcPar()
{

    // initialisation des coordonnées
    mpfr_t tmp1, tmp2;
    mpfr_t *x, *y;
    x = new mpfr_t[this->im_width*this->surEchantillonage];
    y = new mpfr_t[this->im_height*this->surEchantillonage];

    if(!x || !y)
	exit(2);

    //mpfr_inits( tmp1, tmp2, NULL);
    mpfr_init2(tmp1, mpfr_get_prec(pos_x));
    mpfr_init2(tmp2, mpfr_get_prec(pos_x));

    mpfr_div_ui(tmp1, this->width, 2, MPFR_RNDN); //  tmp1 = width/2
    mpfr_set_ui( tmp2, 0, MPFR_RNDN);
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
        mpfr_init2(x[i], mpfr_get_prec(pos_x));

        //  xc = pos_x - width/2 + i*atomic_w
        mpfr_sub(x[i], this->pos_x, tmp1, MPFR_RNDN); //  xc = pos_x - tmp = pos_x - width/2
        //mpfr_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * i
        mpfr_add( tmp2, tmp2, atomic_w, MPFR_RNDN);
        mpfr_add(x[i], x[i], tmp2, MPFR_RNDN); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i
    }

    mpfr_div_ui(tmp1, this->height, 2, MPFR_RNDN); //  tmp1 = height/2
    mpfr_set_ui( tmp2, 0, MPFR_RNDN);
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
        mpfr_init2(y[i], mpfr_get_prec(pos_y));

        //  yc = pos_y - height/2 + i*atomic_h
        mpfr_sub(y[i], this->pos_y, tmp1, MPFR_RNDN); //  yc = pos_y - tmp = pos_y - height/2
        //mpfr_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
        mpfr_add( tmp2, tmp2, atomic_h, MPFR_RNDN);
        mpfr_add(y[i], y[i], tmp2, MPFR_RNDN); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
    }

    mpfr_clears( tmp1, tmp2, NULL);


    *(this->sEMat) = 1;
    *(this->divMat) = -1;
    

    // calcul de l image
    calcPar( x, y);	
	
	
	
    // on detecte les zones nécessitant dur sur echantillonage
    this->draw();
    Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

    int lowThreshold = 10;
    int ratio = 3;
    int kernel_size = 3;

    cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
    blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
    Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
    filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);

    *(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;

    // on recalcul l image avec le sur echantillonage
    calcPar( x, y);

    // on libere tout
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
	    mpfr_clear(x[i]);
    }
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
	    mpfr_clear(y[i]);
    }

    delete [] x;
    delete [] y;
}

void Mandelbrot::calcPar(mpfr_t* x, mpfr_t* y)
{
    mpfr_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
    //mpfr_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);
    mpfr_init2(xn, mpfr_get_prec(x[0]));
    mpfr_init2(yn, mpfr_get_prec(y[0]));
    mpfr_init2(xnp1, mpfr_get_prec(x[0]));
    mpfr_init2(ynp1, mpfr_get_prec(y[0]));
    mpfr_init2(mod, mpfr_get_prec(x[0]));
    mpfr_init2(tmp, mpfr_get_prec(x[0]));
    mpfr_init2(xsqr, mpfr_get_prec(x[0]));
    mpfr_init2(ysqr, mpfr_get_prec(y[0]));

    #pragma omp parrallel for schedule(guided)
    for (int i = 0; i < im_width; i++)
    {
        for(int j = 0; j < im_height; j++)
        {
            int sE = sEMat->at<char>( j, i);

            for(int m = 0; m < sE; m++)
            {
                for(int n = 0; n < sE; n++)
                {
                    if((sE != 1 && divMat->at<int>(j*surEchantillonage+n ,i*surEchantillonage+m) == -1) || sE == 1)
                    {
                        mpfr_set_ui(xn,0, MPFR_RNDN);
                        mpfr_set_ui(yn,0, MPFR_RNDN);
                        mpfr_set_ui(xsqr,0, MPFR_RNDN);
                        mpfr_set_ui(ysqr,0, MPFR_RNDN);

                        for (int k = 1; k < iterations; k++)
                        {
                            //  xnp1 = xn² - yn² + xc
                            mpfr_sub(xnp1, xsqr, ysqr, MPFR_RNDN); //  xnp1 = xsqr - ysqr = xn² - yn²
                            //mpfr_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
                            mpfr_add(xnp1, xnp1, x[i*surEchantillonage+m], MPFR_RNDN); //  xnp1 = xnp1 + xc = xn² - yn² + xc

                            //  ynp1 = 2*xn*yn + yc
                            mpfr_mul(ynp1, xn, yn, MPFR_RNDN); //  ynp1 = xn * yn
                            mpfr_mul_ui(ynp1, ynp1, 2, MPFR_RNDN); //  ynp1 = ynp1 * 2 = 2 * xn * yn
                            //mpfr_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
                            mpfr_add(ynp1, ynp1, y[j*surEchantillonage+n], MPFR_RNDN); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

                            //  xn = xnp1
                            //  yn = ynp1
                            mpfr_set( xn, xnp1, MPFR_RNDN); //  xn = xnp1
                            mpfr_set( yn, ynp1, MPFR_RNDN); //  yn = ynp1
                                        
                            //xsqr = xn²
                            mpfr_mul(xsqr, xn, xn, MPFR_RNDN);

                            //ysqr = yn²
                            mpfr_mul(ysqr, yn, yn, MPFR_RNDN);

                            //  mod = xnp1² + ynp1²
                            mpfr_add(mod, xsqr, ysqr, MPFR_RNDN); //  mod = xsqr + ysqr = xn² + yn²

                            if(mpfr_cmp_ui(mod, 4) > 0)
                            {
                                divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = k;
                                break;
                            } else if(k == iterations -1)
                            {
                                divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = iterations;
                            }
                        }
                    }
                }
            }
        }
    }

    mpfr_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

}


void Mandelbrot::draw()
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < im_width; ++i)
	{
		for (int j = 0; j < im_height; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < surEchantillonage; ++k)
			{
				for(int l = 0; l < surEchantillonage; ++l)
				{
					divSpeed = divMat->at<int>( j*surEchantillonage + l, i*surEchantillonage + k);

					if(divSpeed != -1)
					{
						if(divSpeed == iterations)
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
				moy = iterations;

			switch(color)
			{
				case 1:
				{
					coloration(bgr, moy, iterations, nbr_div, nbr_ndiv);
					break;
				}
				case 2:
				{
					coloration2(bgr, moy, iterations);
					break;
				}
				case 3:
				{
					coloration3(bgr, moy, iterations);
					break;
				}
			}
			img->at<Vec3b>( j, i) = bgr;
		}
	}
}

void Mandelbrot::save(int img_num, int start)
{
    img_partial_save(start, Mandelbrot::im_width, Mandelbrot::im_height, Mandelbrot::img, Mandelbrot::rep, img_num);
}

bool Mandelbrot::IsGood_2(bool* filtre, double* res2)
{
	*filtre = false;
	bool continue_y_or_n;

	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);		//entier non signé 8 bit à 3 dimension
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);	//pareil a 2 dimension
	if(!detected_edges) cout<<"error detected edge\n"<<endl;
	if(!src_gray) cout<<"error src grey\n"<<endl;


	int lowThreshold = 30;	
	int ratio = 3;			
	int kernel_size = 3;	

	cvtColor( *(img), *(src_gray), CV_BGR2GRAY );
	
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);
	double res = countNonZero(*detected_edges)*1000/(im_height*im_width);

	if(res<ThresholdCont)
		continue_y_or_n = false;
	else
		continue_y_or_n = true;


	if(continue_y_or_n)
	{
		int y0 = im_width/2;
		int x0 = im_height/2;
		int sigma_x = x0/2;
		int sigma_y = y0/2;

		res = 0.0;

		for (int i = 0; i < im_height; i++)
		{
			for (int j = 0; j < im_width; j++)
			{

				double X =(pow(i - x0, 2)/(2*pow(sigma_x, 2)));

				double Y =(pow(j - y0, 2)/(2*pow(sigma_y, 2)));

				double flou = exp(-(X + Y));

				res += (((detected_edges->at<char>(i, j)+256)%256) * flou);
			}
		}
		res = res/(im_height*im_width)*1000/255;

		if(res >= ThresholdSave)
			*filtre = true;
		else
			*filtre = false;


        *res2 = (res*0.55)/this->im_width;
	}

	delete src_gray;
	delete detected_edges;

	return continue_y_or_n;
}


void Mandelbrot::IterUp(){

	//augmentation du nombre d'iteration en fonction de la profondeur du zoom actuel
	//66.5*racine(2*racine(abs(1 - racine(5*(scale)))))	with scale = this->width/3

	mpfr_t temp;
	mpfr_inits(temp, NULL);

	//temp = scale = this->width/3
	mpfr_div(temp, atomic_w, width, MPFR_RNDN);

	//temp = 5*temp
	mpfr_mul_ui(temp, temp, 5, MPFR_RNDN);

	//temp = racine(temp)
	mpfr_sqrt(temp, temp, MPFR_RNDN);

	//temp = 1 - temp
	mpfr_ui_sub(temp, 1, temp, MPFR_RNDN);

	//temp = abs(temp)
	mpfr_abs(temp, temp, MPFR_RNDN);
	
	//temp = racine(temp)
	mpfr_sqrt(temp, temp, MPFR_RNDN);

	//temp = 2.temp
	mpfr_mul_ui(temp, temp, 2, MPFR_RNDN);

	//temp = racine(temp)
	mpfr_sqrt(temp, temp, MPFR_RNDN);

	//temp = temp*66.5
	mpfr_mul_ui(temp, temp, 110, MPFR_RNDN);

	//convertissation
	double x = mpfr_get_d(temp, MPFR_RNDN);

	iterations = x;

	mpfr_clears(temp, NULL);
	
}

void Mandelbrot::dichotomie3()
{
    int prec = mpfr_get_prec(pos_x);

    escapeSpeedCalcSeq();
    draw();

    bool filtre, needwork;
    int img_num;
    double res;
    


    /*char* azert = create_work2(pos_x, pos_y, width, height);

    std::cout << enough << std::endl << azert << std::endl;
    std::cout.flush();*/

    if(IsGood_2(&filtre, &res))
    {
        if(mpfr_cmp_ui(pos_y, 0) < 0)
        {
            getHandlerInfo(needwork, img_num, 2);
        }
        else if(mpfr_cmp_ui(pos_y, 0) == 0)
        {
            getHandlerInfo(needwork, img_num, 1);
        }


	/*if(filtre)
	    save(img_num);*/

        {
            char* buf = create_work2(pos_x, pos_y, width, height);
            /*map.insert(std::pair<double, char*>(res, buf));

            if(map.size() > 10)
                map.erase(std::prev(map.end()));*/

            insert_top10(res, buf);
        } 



	if(enough)
	{	
	    mpfr_t old_pos_x, old_pos_y, old_width, old_height;

	    mpfr_init2(old_pos_x, mpfr_get_prec(pos_x));
	    mpfr_init2(old_pos_y, mpfr_get_prec(pos_y));
	    mpfr_init2(old_width, mpfr_get_prec(width));
	    mpfr_init2(old_height, mpfr_get_prec(height));

	    mpfr_set(old_pos_x, pos_x, MPFR_RNDN);
	    mpfr_set(old_pos_y, pos_y, MPFR_RNDN);
	    mpfr_set(old_width, width, MPFR_RNDN);
	    mpfr_set(old_height, height, MPFR_RNDN);

	    // recuper dans les old les valeurs de "this->"
	    // et remplacer les this-> dans la suite par des old_
	    //delete l'image ou on est
	    del_mem();

            mpfr_t equalz;
            mpfr_init(equalz);
    
            if(needwork)
            {
                std::vector<int> divs_cpy = divs;
                bool first = true;
                Mandelbrot *M = nullptr;

                

                for(int i = divs.size() - 1; i >= 0; i--)
                {
                    int n_prec = prec + ceil(log(divs.at(i))/log(2));
                    mpfr_t temp, delta_x, delta_y;

                    mpfr_t* tab_x = new mpfr_t[divs.at(i)];
                    mpfr_t* tab_y = new mpfr_t[divs.at(i)];

                    if(mpfr_get_prec(old_pos_x)>=mpfr_get_prec(old_pos_y))
                        mpfr_init2(temp, mpfr_get_prec(old_pos_x) + n_prec/64);
                    else
                        mpfr_init2(temp, mpfr_get_prec(old_pos_y) + n_prec/64);

                    mpfr_init2(delta_x, mpfr_get_prec(old_width));
                    mpfr_init2(delta_y, mpfr_get_prec(old_height));

                    //initialise chacun des elements du tableau avant de pouvoir s'en servir
                    for (int init = 0; init < divs.at(i); init++)
                    {
                            mpfr_init2(tab_x[init], mpfr_get_prec(old_pos_x) + n_prec/64);
                            mpfr_init2(tab_y[init], mpfr_get_prec(old_pos_y) + n_prec/64);
                    }

                    n_prec %= 64;


                    //calcul de delta_x, la distance entre de nouveaux points en x
                    mpfr_div_ui(delta_x, old_width, divs.at(i), MPFR_RNDN);
                    //calcul de delta_y, la distance entre de nouveaux points en y
                    mpfr_div_ui(delta_y, old_height, divs.at(i), MPFR_RNDN);


                    //tab_x[0] = pos_x - width/2 + width/2*divs.at(i)
                    mpfr_div_ui(temp, old_width, 2*divs.at(i), MPFR_RNDN);
                    mpfr_add(tab_x[0], old_pos_x, temp, MPFR_RNDN);
                    mpfr_div_ui(temp, old_width, 2, MPFR_RNDN);
                    mpfr_sub(tab_x[0], tab_x[0], temp, MPFR_RNDN);

                    //tab_y[0] = pos_y - height/2 + height/2*divs.at(i)
                    mpfr_div_ui(temp, old_height, 2*divs.at(i), MPFR_RNDN);
                    mpfr_add(tab_y[0], old_pos_y, temp, MPFR_RNDN);
                    mpfr_div_ui(temp, old_height, 2, MPFR_RNDN);
                    mpfr_sub(tab_y[0], tab_y[0], temp, MPFR_RNDN);

                    for (int c = 1; c < divs.at(i); c++)
                    {
                        //tab_x[c] = tab_x[0] + c*delta_x
                        mpfr_mul_ui(temp, delta_x, c, MPFR_RNDN);
                        mpfr_add(tab_x[c], tab_x[0], temp, MPFR_RNDN);
                        
                        
                        mpfr_set_prec(equalz, mpfr_get_prec(tab_y[c]));
                        //tab_y[c] = tab_y[0] + c*delta_y
                        mpfr_mul_ui(temp, delta_y, c, MPFR_RNDN);
                        mpfr_add(tab_y[c], tab_y[0], temp, MPFR_RNDN);

                        mpfr_abs(equalz, tab_y[c], MPFR_RNDN);
                        mpfr_div(equalz, equalz, old_height, MPFR_RNDN);
                        if(mpfr_cmp_d(equalz, 0.00001) < 0)
                            mpfr_set_ui(tab_y[c], 0, MPFR_RNDN);
                    }

                    for (int x = 0; x < divs.at(i); x++)
                    {
                        for (int y = 0; y < divs.at(i); y++)
                        {
                            if(mpfr_cmp_ui(tab_y[y], 0) <= 0)
                            {
                                if(first)
                                {
                                    M = new Mandelbrot(tab_x[x], tab_y[y], delta_x, delta_y , enough - 1, divs_cpy);
                                    first = false;
                                }
                                else
                                {
                                    char* buf = create_work(enough - 1, tab_x[x], tab_y[y], delta_x, delta_y, divs_cpy);
                                    sendWork(buf);
                                    free(buf);
                                }
                            }
                        }
                    }

                    //delete chacun des element du tableau avant pour pouvoir virer le tableau
                    for (int del = 0; del < divs.at(i); del++)
                    {
                        mpfr_clear(tab_x[del]);
                        mpfr_clear(tab_y[del]);
                    }

                    delete [] tab_x;
                    delete [] tab_y;
                    mpfr_clears(temp, delta_x, delta_y, NULL);

                    divs_cpy.pop_back();
                }
                
                if(M != nullptr)
                {
                    M->dichotomie3();
                    delete M;
                }
            }
            else
            {
                for(int i = divs.size() - 1; i >=0; i--)
                {
                    int n_prec = prec + ceil(log(divs.at(i))/log(2));
                    mpfr_t temp, delta_x, delta_y;

                    mpfr_t* tab_x = new mpfr_t[divs.at(i)];
                    mpfr_t* tab_y = new mpfr_t[divs.at(i)];

                    if(mpfr_get_prec(old_pos_x)>=mpfr_get_prec(old_pos_y))
                        mpfr_init2(temp, mpfr_get_prec(old_pos_x) + n_prec/64);
                    else
                        mpfr_init2(temp, mpfr_get_prec(old_pos_y) + n_prec/64);

                    mpfr_init2(delta_x, mpfr_get_prec(old_width));
                    mpfr_init2(delta_y, mpfr_get_prec(old_height));

                    //initialise chacun des elements du tableau avant de pouvoir s'en servir
                    for (int init = 0; init < divs.at(i); init++)
                    {
                            mpfr_init2(tab_x[init], mpfr_get_prec(old_pos_x) + n_prec/64);
                            mpfr_init2(tab_y[init], mpfr_get_prec(old_pos_y) + n_prec/64);
                    }

                    n_prec %= 64;


                    //calcul de delta_x, la distance entre de nouveaux points en x
                    mpfr_div_ui(delta_x, old_width, divs.at(i), MPFR_RNDN);
                    //calcul de delta_y, la distance entre de nouveaux points en y
                    mpfr_div_ui(delta_y, old_height, divs.at(i), MPFR_RNDN);


                    //tab_x[0] = pos_x - width/2 + width/2*divs.at(i)
                    mpfr_div_ui(temp, old_width, 2*divs.at(i), MPFR_RNDN);
                    mpfr_add(tab_x[0], old_pos_x, temp, MPFR_RNDN);
                    mpfr_div_ui(temp, old_width, 2, MPFR_RNDN);
                    mpfr_sub(tab_x[0], tab_x[0], temp, MPFR_RNDN);

                    //tab_y[0] = pos_y - height/2 + height/2*divs.at(i)
                    mpfr_div_ui(temp, old_height, 2*divs.at(i), MPFR_RNDN);
                    mpfr_add(tab_y[0], old_pos_y, temp, MPFR_RNDN);
                    mpfr_div_ui(temp, old_height, 2, MPFR_RNDN);
                    mpfr_sub(tab_y[0], tab_y[0], temp, MPFR_RNDN);

                    for (int c = 1; c < divs.at(i); c++)
                    {
                        //tab_x[c] = tab_x[0] + c*delta_x
                        mpfr_mul_ui(temp, delta_x, c, MPFR_RNDN);
                        mpfr_add(tab_x[c], tab_x[0], temp, MPFR_RNDN);

                        mpfr_set_prec(equalz, mpfr_get_prec(tab_y[c]));
                        //tab_y[c] = tab_y[0] + c*delta_y
                        mpfr_mul_ui(temp, delta_y, c, MPFR_RNDN);
                        mpfr_add(tab_y[c], tab_y[0], temp, MPFR_RNDN);
                        
                        mpfr_abs(equalz, tab_y[c], MPFR_RNDN);
                        mpfr_div(equalz, equalz, old_height, MPFR_RNDN);
                        if(mpfr_cmp_d(equalz, 0.00001) < 0)
                            mpfr_set_ui(tab_y[c], 0, MPFR_RNDN);
                    }

                    for (int x = 0; x < divs.at(i); x++)
                    {
                        for (int y = 0; y < divs.at(i); y++)
                        {
                            
                            if(mpfr_cmp_ui(tab_y[y], 0) <= 0)
                            {
                                /*if(enough == 2)
                                {
                                    char* azerty = create_work(enough - 1, tab_x[x], tab_y[y], delta_x, delta_y, divs);
                                    std::cout << azerty << std::endl;
                                    free(azerty);
                                }*/
                                Mandelbrot* M = new Mandelbrot(tab_x[x], tab_y[y], delta_x, delta_y , enough - 1, divs);
                                //en bas a gauche
                                                    
                                M->dichotomie3();
                                                    
                                delete M;
                                                    
                            }
                        }
                    }

                    //delete chacun des element du tableau avant pour pouvoir virer le tableau
                    for (int del = 0; del < divs.at(i); del++)
                    {
                        mpfr_clear(tab_x[del]);
                        mpfr_clear(tab_y[del]);
                    }

                    delete [] tab_x;
                    delete [] tab_y;
                    mpfr_clears(temp, delta_x, delta_y, NULL);
                
                    divs.pop_back();
                }
            }
        }
    }
    else
    {
        unsigned long images_faites = 0;
        if(divs.size() == 2)
        {
            for(int k = 0; k <= enough; k++)
            {
                for(int j = 0; j <= k; j++)
                {
                    images_faites += pow(4, j) * pow(9, k-j);
                }
            }
        }
        else if(divs.size() == 1)
        {
            for(int k = 0; k <= enough; k++)
                images_faites += pow(4,k);
        }


        if(mpfr_cmp_ui(pos_y, 0) < 0)
        {
            getHandlerInfo(needwork, img_num, 2*images_faites);
        }
        else if(mpfr_cmp_ui(pos_y, 0) == 0)
        {
            getHandlerInfo(needwork, img_num, images_faites);
        }
    }
}