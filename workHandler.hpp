#ifndef __WH__
#define __WH__

#include <mpfr.h>
#include <mpf2mpfr.h>
#include <gmp.h>
#include <vector>

void resize(int sig);

void handler(int argc, char** argv);
void handler2(int argc, char** argv);
void worker(int argc, char** argv);
void worker2(int argc, char** argv);

char* create_work(int enough, mpfr_t x, mpfr_t y, mpfr_t w, mpfr_t h, std::vector<int> divs);
char* create_work2(mpfr_t x, mpfr_t y, mpfr_t w, mpfr_t h);
void getHandlerInfo(bool& needwork, int& img_num, long images_faites);
void sendWork(char* buf);


void init_top10();
void insert_top10(double key, char* val);

#endif
