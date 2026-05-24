#include <cmath>
#include <cstdlib>
#include <cstring>

#define OFFSET(x, y, m) (((x)*(m)) + (y))

void initialize(double *__restrict grid,
                double *__restrict grid_new,
                int cols,
                int rows)
{
    memset(grid, 0, rows * cols * sizeof(double));
    memset(grid_new, 0, rows * cols * sizeof(double));

    for (int i = 0; i < cols; i++) 
    {
        double v = 10.0 + (20.0 - 10.0) * i / (cols - 1);
        grid[i] = v;
        grid_new[i] = v;
    }

    for (int i = 0; i < cols; i++) 
    {
        double v = 20.0 + (30.0 - 20.0) * i / (cols - 1);
        grid[(rows - 1) * cols + i] = v;
        grid_new[(rows - 1) * cols + i] = v;
    }

    for (int j = 0; j < rows; j++) 
    {
        double vL = 10.0 + (20.0 - 10.0) * j / (rows - 1);
        double vR = 20.0 + (30.0 - 20.0) * j / (rows - 1);

        grid[j * cols] = vL;
        grid_new[j * cols] = vL;

        grid[j * cols + (cols - 1)] = vR;
        grid_new[j * cols + (cols - 1)] = vR;
    }

    grid[0] = 10.0;
    grid[cols - 1] = 20.0;
    grid[(rows - 1) * cols] = 20.0;
    grid[rows * cols - 1] = 30.0;

    grid_new[0] = 10.0;
    grid_new[cols - 1] = 20.0;
    grid_new[(rows - 1) * cols] = 20.0;
    grid_new[rows * cols - 1] = 30.0;

    #pragma acc enter data copyin(grid[:rows*cols], grid_new[:rows*cols])
}

double calcNext(double *__restrict grid,
                double *__restrict grid_new,
                int cols,
                int rows)
{
    double error = 0.0;

    #pragma acc parallel loop collapse(2) present(grid, grid_new) reduction(max:error)
    for (int j = 1; j < rows - 1; j++)
    {
        for (int i = 1; i < cols - 1; i++)
        {
            int idx = j * cols + i;

            double val = 0.25 * (
                grid[idx + 1] +
                grid[idx - 1] +
                grid[idx - cols] +
                grid[idx + cols]
            );

            grid_new[idx] = val;

            double diff = val - grid[idx];
            diff = diff > 0 ? diff : -diff;

            if (diff > error)
                error = diff;
        }
    }

    return error;
}

void calcNextNoError(double *__restrict grid,
                     double *__restrict grid_new,
                     int cols,
                     int rows)
{
    #pragma acc parallel loop collapse(2) present(grid, grid_new)
    for (int j = 1; j < rows - 1; j++)
    {
        for (int i = 1; i < cols - 1; i++)
        {
            int idx = j * cols + i;

            grid_new[idx] = 0.25 * (
                grid[idx + 1] +
                grid[idx - 1] +
                grid[idx - cols] +
                grid[idx + cols]
            );
        }
    }
}

void deallocate(double *__restrict grid, double *__restrict grid_new)
{
    #pragma acc exit data delete(grid, grid_new)
    free(grid);
    free(grid_new);
}