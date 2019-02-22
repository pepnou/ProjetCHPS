#include "mandelbrot.hpp"
#include <cmath>

using namespace std;

Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int im_w, int im_h, int supSample, int iterations, int color, Mpmc* mpmc, char* rep) 
	: surEchantillonage(supSample), 
	im_width(im_w), 
	im_height(im_h), 
	iterations(iterations), 
	color(color), 
	mpmc(mpmc)
	/*divMat(new Mat(im_h*supSample, im_w*supSample, CV_32SC1)) ,
	img(new Mat(im_h, im_w, CV_8UC3)) ,
	sEMat(new Mat(im_h, im_w, CV_8UC1))
	*/

{
	divMat = new Matrice(this->im_width*supSample, this->im_height*supSample);
	img = new Image(this->im_width, this->im_height);
	sEMat = new Matrice(this->im_width, this->im_height);

	mpf_init2(this->pos_x, mpf_get_prec(x));
	mpf_init2(this->pos_y, mpf_get_prec(y));

	mpf_init2(this->width, mpf_get_prec(w));
	mpf_init2(this->height, mpf_get_prec(h));

	mpf_init2(atomic_w, mpf_get_prec(w) + ceil(log(im_w*supSample) / log(2)));
	mpf_init2(atomic_h, mpf_get_prec(h) + ceil(log(im_h*supSample) / log(2)));

	tasks.store(0);

	mpf_set(this->pos_x, x);
	mpf_set(this->pos_y, y);
	mpf_set(this->width, w);
	mpf_set(this->height, h);
	
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

	this->divMat = nullptr;
	this->img = nullptr;
	this->sEMat = nullptr;
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
	
	
	this->draw();
	Matrice kernel = Matrice( 7, 7);
	kernel = 1;

	//cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY ); //constructeur.
	{	//set en noir et blanc
	Matrice N_et_B(*this->img);
	*(this->sEMat) = N_et_B;
	}

	//blur( *(this->sEMat), *(this->sEMat), Size(3,3) );	//flou, 3
	this->sEMat->blurer(3);
	//Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);	//edger dectorer blablabla
	this->sEMat->edger_detectorer();
	//filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT); //ceci est le grossisseur de lignes convo, 7
	this->sEMat->filterer_2D();

	*(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;	


	for(int i = 0; i < nbTasks; i++)
	{
		this->tasks.fetch_add(1);

		wo.arg = (void*)&args[i];
		this->mpmc->push(wo);
	}

	while(this->tasks.load() != 0)
		this_thread::yield();


	
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

void Mandelbrot::threadCalc4(void* arg)
{
	threadDraw* args = (threadDraw*)arg;

	mpf_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
	mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

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

	mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

	this->tasks.fetch_sub(1);
}




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

bool Mandelbrot::IsGood(){
	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);

	int lowThreshold = 30;		
	int ratio = 3;				
	int kernel_size = 3;		

	cvtColor( *(this->img), *(src_gray), CV_BGR2GRAY );
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);

	double res = countNonZero(*detected_edges)*255/(this->im_height*this->im_width);

	delete src_gray;
	delete detected_edges;


	if(res >= this->ThresholdCont)
		return true;
	else
		return false;
}


bool Mandelbrot::IsGood_2(bool* filtre)
{
	*filtre = false;
	bool continue_y_or_n;

	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);		//entier non signé 8 bit à 3 dimension
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);	//pareil a 2 dimension
	if(!detected_edges) cout<<"fuck"<<endl;
	if(!src_gray) cout<<"fuck2"<<endl;


	int lowThreshold = 30;	
	int ratio = 3;			
	int kernel_size = 3;	

	cvtColor( *(this->img), *(src_gray), CV_BGR2GRAY );
	
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);
	double res = countNonZero(*detected_edges)*1000/(this->im_height*this->im_width);

	if(res<this->ThresholdCont)
		continue_y_or_n = false;
	else
		continue_y_or_n = true;


	if(continue_y_or_n)
	{
		int y0 = this->im_width/2;
		int x0 = this->im_height/2;
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
		res = res/(this->im_height*this->im_width)*1000/255;

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
	
	if(this->IsGood_2(&filtre))
	{
		if(filtre)
			this->save();

		if(--enough)
		{
			int n_prec = prec + ceil(log(2)/log(2));

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
	this->escapeSpeedCalcThread4();

	this->draw();

	bool filtre;

	if(this->IsGood_2(&filtre))
	{
		if(filtre)
			this->save();

		if(--enough)
		{	
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
				int n_prec = prec + ceil(log(divs.at(i))/log(2));
				mpf_t temp, delta_x, delta_y;

				mpf_t* tab_x = new mpf_t[divs.at(i)];
				mpf_t* tab_y = new mpf_t[divs.at(i)];

				if(mpf_get_prec(old_pos_x)>=mpf_get_prec(old_pos_y))
					mpf_init2(temp, mpf_get_prec(old_pos_x) + n_prec/64);
				else
					mpf_init2(temp, mpf_get_prec(old_pos_y) + n_prec/64);

				mpf_init2(delta_x, mpf_get_prec(old_width));
				mpf_init2(delta_y, mpf_get_prec(old_height));

				//initialise chacun des elements du tableau avant de pouvoir s'en servir
				for (int init = 0; init < divs.at(i); init++)
				{
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

				for (int x = 0; x < divs.at(i); x++)
				{
					for (int y = 0; y < divs.at(i); y++)
					{
						if(mpf_cmp_ui(tab_y[y], 0) < 0)
						{
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
			}
		}
	}
}

bool Mandelbrot::random_img (int enough, double zoom, gmp_randstate_t& state)
{    
    this->escapeSpeedCalcThread4();
    this->draw();
    
    bool filtre;
    this->IsGood_2(&filtre);

    if(filtre)
    {
        this->save();

    
        int max_test = 10;
        bool good = false;

        if(--enough)
        {
			this->IterUp();
			this->iterations *= 2;

			mpf_t nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2;

			mpf_inits(nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2, NULL);

			mpf_set_d(zoom_mpf, zoom);
						
			mpf_div(nw, this->width, zoom_mpf);        //calcul nouveaux W 
			mpf_div(nh, this->height, zoom_mpf);        //calcul nouveaux H
			
			mpf_set(cx,this->pos_x);
			mpf_set(cy,this->pos_y);
			
			
			mpf_sub(Wdiff, this->width, nw);
			
			mpf_neg(Wdiff2, Wdiff);
			mpf_div_ui(Wdiff2, Wdiff2, 2);
			mpf_add(Wdiff2, Wdiff2, cx);
			
			
			mpf_sub(Hdiff, this->height, nh);
			
			mpf_neg(Hdiff2, Hdiff);
			mpf_div_ui(Hdiff2, Hdiff2, 2);
			mpf_add(Hdiff2, Hdiff2, cy);
			
			
			del_mem();
            
            do{
				mpf_urandomb (nx, state, mpf_get_prec(nx));
				mpf_mul(nx, nx, Wdiff);
				mpf_add(nx, nx, Wdiff2);

				mpf_urandomb (ny, state, mpf_get_prec(ny));
				mpf_mul(ny, ny, Hdiff);
				mpf_add(ny, ny, Hdiff2);

				Mandelbrot* M = new Mandelbrot(nx, ny, nw, nh ,im_width, im_height, surEchantillonage, iterations, color, mpmc, rep);

				good = M->random_img(enough, zoom, state);
				delete M;    
            } while( (good == false) && (--max_test > 0) );
            
            mpf_clears(nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2, NULL);
            if (!max_test) return false;
        }

        return true;
    }
    else{
        return false;
    }   
}
