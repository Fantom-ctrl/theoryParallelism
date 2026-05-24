void initialize(double *__restrict grid, double *__restrict gridNext, int width, int height);

double calcNext(double *__restrict grid, double *__restrict gridNext, int width, int height);

void deallocate(double *__restrict grid, double *__restrict gridNext);