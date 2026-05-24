#include <cmath>
#include <cstdlib>
#include <cstring>

#define OFFSET(x, y, m) (((x)*(m)) + (y))

void initialize(double *__restrict grid, double *__restrict grid_new, int cols, int rows)
{
    memset(grid, 0, rows * cols * sizeof(double));
    memset(grid_new, 0, rows * cols * sizeof(double));

    for (int col = 0; col < cols; col++) 
    {
        double v = 10.0 + (20.0 - 10.0) * col / (cols - 1);
        grid[col] = v;
        grid_new[col] = v;
    }

    for (int col = 0; col < cols; col++) 
    {
        double v = 20.0 + (30.0 - 20.0) * col / (cols - 1);
        grid[(rows - 1) * cols + col] = v;
        grid_new[(rows - 1) * cols + col] = v;
    }

    for (int row = 0; row < rows; row++) 
    {
        double vL = 10.0 + (20.0 - 10.0) * row / (rows - 1);
        double vR = 20.0 + (30.0 - 20.0) * row / (rows - 1);

        grid[row * cols] = vL;
        grid_new[row * cols] = vL;

        grid[row * cols + (cols - 1)] = vR;
        grid_new[row * cols + (cols - 1)] = vR;
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

double calcNext(double *grid, double *grid_new, int cols, int rows)
{
    double max_err = 0.0;

    #pragma acc parallel loop reduction(max:max_err) present(grid,grid_new)
    for (int j = 1; j < rows - 1; j++)
    {
        double *cur_row  = &grid[j * cols];
        double *prev_row = &grid[(j - 1) * cols];
        double *next_row = &grid[(j + 1) * cols];

        #pragma acc loop
        for (int i = 1; i < cols - 1; i++)
        {
            double new_val =
                0.25 * (
                    cur_row[i + 1] +
                    cur_row[i - 1] +
                    prev_row[i] +
                    next_row[i]
                );

            grid_new[j * cols + i] = new_val;

            double delta = new_val - cur_row[i];
            if (delta < 0) delta = -delta;
            if (delta > max_err) max_err = delta;
        }
    }

    return max_err;
}

void deallocate(double *__restrict grid, double *__restrict grid_new)
{
    #pragma acc exit data delete(grid,grid_new)
    free(grid);
    free(grid_new);
}