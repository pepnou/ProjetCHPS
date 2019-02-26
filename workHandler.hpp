#ifndef __WH__
#define __WH__

#include <gmp.h>
#include <vector>

void handler(int argc, char** argv);
void worker(int argc, char** argv);

char* create_work(int enough, mpf_t x, mpf_t y, mpf_t w, mpf_t h, std::vector<int> divs);
char* create_work2(mpf_t x, mpf_t y, mpf_t w, mpf_t h);
void getHandlerInfo(bool& needwork, int& img_num);
void sendWork(char* buf);

#endif
