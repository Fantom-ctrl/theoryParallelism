void initialize(double *__restrict grid, double *__restrict grid_new, int cols, int rows);

double calcNext(double *__restrict grid, double *__restrict grid_new, int cols, int rows);

void swap(double *__restrict grid, double *__restrict grid_new, int cols, int rows);

void deallocate(double *__restrict grid, double *__restrict grid_new);

void calcNextNoError(double *__restrict grid, double *__restrict grid_new, int cols, int rows);