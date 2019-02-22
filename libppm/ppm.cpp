#include "ppm.hpp"

//CLASSE DES IMAGES, TOUT CA TOUT CA :)

Image::Image(int w, int h):
	width(w),
	height(h),
	img(new pixel_t[w*h])
{}

Image::~Image()
{
	delete [] this->img;
	this->img = nullptr;
}

int Image::get_width()
{
	return this->width;
}

int Image::get_height()
{
	return this->height;
}

void Image::img_save(char* path)
{
	FILE* file = fopen(path, "w+");
	if(!file)
	{
		perror(path);
		exit(1);
	}

	fprintf(file, "P6\n%d %d\n255\n", this->width, this->height);
	fwrite(img, sizeof(pixel_t), this->width*this->height, file);

	fclose(file);
}

void Image::img_set_pixel_HSV(int x, int y, unsigned char h, unsigned char s, unsigned char v)
{
	pixel_t pixel;
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

    this->img[img_y*this->width + img_x].b = (float)(b2+m) * 255;
    this->img[img_y*this->width + img_x].g = (float)(g2+m) * 255;
    this->img[img_y*this->width + img_x].r = (float)(r2+m) * 255;

}


void Image::img_set_pixel_RGB( int x, int y, unsigned char r , unsigned char g , unsigned char b)
    {
    this->img[img_y*this->width + img_x].b = b;
    this->img[img_y*this->width + img_x].g = g;
    this->img[img_y*this->width + img_x].r = r;
    }

pixel_t* Image::get_le_pixel(int x, int y)
{
	return this->img[y*this->width + x];
}






//CLASSE DES MATRICES, TOUT CA TOUT CA :)

Matrice::Matrice(int w, int h):
	width(w),
	height(h),
	mat(new int32_t[w*h])
{}

Matrice::Matrice(Image image):
	width(image->width),
	height(image->height),
	mat(new int32_t[width*height])
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			pixel_t* pixel = image.get_le_pixel(i, j);
			this->mat[y*this->width + x] = (float)(0.21*pixel->r) + (float)(0.72*pixel->g) + (float)(0.07*pixel->b);
		}
	}
}

Matrice::~Matrice()
{
	delete [] this->mat;
	mat = nullptr;
}

Matrice Matrice::operator=(Matrice droite)
{
	//assert();
	this.height = droite.height;
	this.width = droite.width;
	for (int i = 0; i < dorite.height; i++)
	{
		for (int j = 0; j < droite.width; j++)
		{
			this.mat[y*this->width + x] = droite.mat[y*this->width + x];
		}
	}
}

Matrice Matrice::operator=(int32_t i)
{
	for (int i = 0; i < this->height; i++)
	{
		for (int j = 0; j < this->width; j++)
		{
			this.mat[y*this->width + x] = i;
		}
	}
}

int Matrice::get_width()
{
	return this->width;
}

int Matrice::get_height()
{
	return this->height;
}

void Matrice::blurer(int blur_radius)
{
	int limit = (blur_radius*2)+1;
	int carre = limit*limit;
	int32_t* blured = new int32_t [this->width*this->height];
	int32_t blured_value = NULL;

	for (int i = 0; i < this->width; i++)
	{
		for (int j = 0; j < this->height; j++)
		{
			blured_value = 0;

			for (int i_ = (i - blur_radius); i_ < limit; i_++)
			{
				for (int j_ = (j - blur_radius); j_ < limit; j_++)
				{
					blured_value += get_co(i_, j_);
				}
			}

			blured[j*this->width + i] = blured_value/carre;
		}
	}
	delete [] this->mat; 
	this->mat = blured;
}

void Matrice::convolutionner(double** petite_mat, int width, int height)
{
	int sum = 0;
	for (int i = 0; i < limit_x; ++i)
	{
		for (int j = 0; j < limit_y; ++j)
		{
			sum += petite_mat[i][j];
		}
	}

	int32_t* convolutionned = new int32_t [this->width*this->height];
	int32_t convolutionned_value = NULL;

	for (int i = 0; i < this->width; i++)
	{
		for (int j = 0; j < this->height; j++)
		{
			convolutionned_value = 0;

			for (int i_ = (i - width/2); i_ < width; i_++)
			{
				for (int j_ = (j - height/2); j_ < height; j_++)
				{
					convolutionned_value += get_co(i_, j_)*petite_mat[width][height];
				}
			}

			convolutionned[j*this->width + i] = convolutionned_value/sum;
		}
	}
	delete [] this->mat; 
	this->mat = convolutionned;
}

void Matrice::gaussiblur()
{
	double** convoluer = {{2, 4, 5, 4, 2}, {4, 9, 12, 9, 4} ,{5, 12, 15, 12, 5}, {4, 9, 12, 9, 4}, {2, 4, 5, 4, 2}};
	this->convolutionner(convoluer, 5, 5);
}

void Matrice::gaussilter()
{
	int y0 = this->width/2;
	int x0 = this->height/2;
	int sigma_x = x0/2;
	int sigma_y = y0/2;


	for (int i = 0; i < this->height; i++)
	{
		for (int j = 0; j < this->width; j++)
		{

			double X =(pow(i - x0, 2)/(2*pow(sigma_x, 2)));

			double Y =(pow(j - y0, 2)/(2*pow(sigma_y, 2)));

			double flou = exp(-(X + Y));

			this->mat[j*this->width + i] = (((this->mat[j*this->width + i]+256)%256) * flou);
		}
	}
}

int * Matrice::get_co(int x, int y)
{
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	if(x > this->width)
		x = this->width - 1;
	if(y > this->height)
		y = this->height - 1;

	return this->mat[y*this->width + x];
}

void Matrice::set_val(int x, int y, int32_t val)
{
	this->mat[y*this->width + x] = val;
}

void Matrice::edger_detectorer_laplacianer()
{
	double** convoluer = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
	this->convolutionner(convoluer, 3, 3);
}

void Matrice::edger_detectorer_laplacianer_gaussinerer()
{
	double** convoluer = {{0, 0, -1, 0, 0}, {0, -1, -2, -1, 0}, {-1, -2, 16, -2, -1}, {0, -1, -2, -1, 0}, {0, 0, -1, 0, 0}};
	this->convolutionner(convoluer, 5, 5);
}

void Matrice::edger_detectorer()
{
	double** convoluer = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
	this->convolutionner(convoluer, 3, 3);
}

void Matrice::filterer_2D()
{
	int32_t* convolutionned = new int32_t [this->width*this->height];
	int32_t convolutionned_value = NULL;
	bool trouve = FALSE;

	for (int i = 0; i < this->width; i++)
	{
		for (int j = 0; j < this->height; j++)
		{
			convolutionned_value = 0;

			for (int i_ = i - 3; i_ < 7; i_++)
			{
				for (int j_ = j - 3; j_ < 7; j_++)
				{
					if(get_co(i_, j_) != 0)
					{
						trouve = TRUE;
						break;
					}
				}
				if(trouve)
					break;
			}
			if(trouve)
				this_>mat[j*this->width + i] = 1;
			else
				this_>mat[j*this->width + i] = 0;
		}
	}
	delete [] this->mat; 
	this->mat = convolutionned;
}