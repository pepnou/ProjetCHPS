#ifndef _PPM_
#define _PPM_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	unsigned char r, g, b;
}pixel_t;

class Image
{
	private:
		pixel_t * img;
		int width, height;
	public:
		Image(int width, int height);
		~Image();
		int get_width();
		int get_height();
		void img_save(char* path);
		void img_set_pixel_RGB(int x, int y, unsigned char r, unsigned char g, unsigned char b);
		void img_set_pixel_HSV(int x, int y, unsigned char h, unsigned char s, unsigned char v);
		pixel_t get_le_pixel(int x, int y);
};

class Matrice
{
	private:
		int32_t * mat;
		int width, height;
	public:
		Matrice(int32_t width, int32_t height);
		Matrice(Image image);
		~Matrice();
		int get_width();
		int get_height();
		void blurer(int b);
		void edger_detectorer();
		void edger_detectorer_laplacianer_gaussinerer();
		void edger_detectorer_laplacianer();
		void convolutionner();
		void gaussiblur();
		void gaussilter();
		int * get_co(int x, int y);
		void set_val();
		void filterer_2D();
		Matrice operator=(Matrice matrice);
		Matrice operator=(int32_t i);
};

#define dbg std::cout<<"line : "<<__LINE__<<", function : "<<__FUNCTION__<<"\n";

#endif