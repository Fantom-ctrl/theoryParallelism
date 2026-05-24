void initialize(double * A, double * Anew, int m, int n);

double calcNext(double *__restrict A, double *__restrict Anew, int m, int n);

void deallocate(double * A, double * Anew);