#include "color.hpp"

using namespace cv;

void coloration(Vec3b &bgr,int n, int iterations)
{
	if(n >= iterations -1)
	{
		bgr[0] = 0;
		bgr[1] = 0;
		bgr[2] = 0;
	}
	else
	{
		bgr = HSBtoRGB( n%360, 1, 1);
	}
}

Vec3b HSBtoRGB( int h, int s, int v)
{
	Vec3b res;
	int c, x, m, r2 = 0, g2 = 0, b2 = 0;

	c = v*s;
	x = c*(1-abs((h/60)%2 - 1));
	m = v-c;

	switch(h%60)
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

	res[0] = (g2+m) * 255;
	res[1] = (b2+m) * 255;
	res[2] = (r2+m) * 255;

	return res;
}