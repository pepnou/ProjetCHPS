#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>



uint64_t tree(int i)
{
    if(i == 0)
        return 1;
    else
    {
        return 4*tree(i-1) + 9*tree(i-1);
    }
}

int main()
{
    FILE* f = fopen("perf2.txt", "w+");
    
    uint64_t sum1, sum2 = 0;

    for(int i = 0; i < 100; i++)
    {
        sum1 = 0;
        for(int k = 0; k <= i; k++)
        {
            for(int j = 0; j <= k; j++)
            {
                sum1 += pow(4, j) * pow(9, k-j);
            }
        }
    
        //sum2 += tree(i);

        fprintf(f, "%d %ld\n", i, sum1);
        //printf("%ld\n%ld\n\n", sum1, sum2);
    }

    fclose(f);
}
