#include <queue>

typedef struct
{
    char *x, *y, *width, *height;
} Work;

class WorkHandler
{
    private:
        int size;
        std::queue<int> waiting;

    
    public:
        WorkHandler();
        ~WorkHandler();
};
