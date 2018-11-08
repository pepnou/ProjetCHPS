#include "color.hpp"

using namespace cv;
using namespace std;


void coloration(Vec3b &bgr,int n, int iterations, int nbr_div, int nbr_ndiv)
{
	if(n >= iterations)
	{
		bgr = 0;
	}
	else
	{
		if(n < 0)
		{
			bgr[0] = 255;
			bgr[1] = 255;
			bgr[2] = 255;
		}
		else
			bgr = HSBtoRGB( n%360, 1, (float)nbr_div/(nbr_div+nbr_ndiv));
	}
}

void coloration2(Vec3b &bgr,int n, int iterations)
{
	int speed = 10, angle = 240;
	float max_lum = 0.5, h, s, v;
	
	if(n>=iterations)
	{
		bgr = 0;
	}
	else
	{
		n = n % (4 * speed);

		switch(n/speed)
		{
			case 0:
			{
				h = angle;
				s = 1 - (float)(n-0)/speed;
				v = (max_lum - 1)*s + 1;
				break;
			}
			case 1:
			{
				h = (angle + 180)%360;
				s = (float) (n-speed)/speed;
				v = (max_lum - 1)*s + 1;
				break;
			}
			case 2:
			{
				h = (angle + 180)%360;
				s = 1;
				v = 1 - (float) (n-2*speed)/speed;
				break;
			}
			case 3:
			{
				h = angle;
				s = 1;
				v = (float) (n-3*speed)/speed;
				break;
			}
			default:
			{
				cout<<"erreur"<<endl;
				exit(2);
			}
		}
	}
	bgr = HSBtoRGB( h, s, v);
}

void coloration3(Vec3b &bgr,int n, int iterations)
{
	if(n >= iterations)
	{
		bgr = 0;
	}
	else
	{
		double 	x = log(n)/log(2),
				a = 1/log(2),
				b = 1/(3*sqrt(2)*log(2)),
				c = 1/((7-pow( 3, 1/8))*log(2));
		bgr[2] = 255*(1-cos(a*x))/2;
		bgr[1] = 255*(1-cos(b*x))/2;
		bgr[0] = 255*(1-cos(c*x))/2;
	}
}

Vec3b HSBtoRGB( int h, float s, float v)
{
	Vec3b res;
	double x, r2 = 0.0, g2 = 0.0, b2 = 0.0, c, m, tmp;

	c = v*s;
	//x = c*(1-abs((h/60)%2 - 1));

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

	res[0] = (float)(b2+m) * 255;
	res[1] = (float)(g2+m) * 255;
	res[2] = (float)(r2+m) * 255;

	return res;
}
