#include <cmath>
#include <cstdlib>
#include <cstring>

#define OFFSET(x, y, width) (((x) * (width)) + (y))

void initialize(double *__restrict grid, double *__restrict gridNext, int width, int height)
{
    memset(grid, 0, height * width * sizeof(double));
    memset(gridNext, 0, height * width * sizeof(double));

    for (int i = 0; i < width; i++) 
    {
        double valueBottom = 20.0 + (30.0 - 20.0) * i / (width - 1);
        grid[(height - 1) * width + i] = valueBottom;
        gridNext[(height - 1) * width + i] = valueBottom;
    }

    for (int j = 0; j < height; j++) 
    {
        double valueLeft = 10.0 + (20.0 - 10.0) * j / (height - 1);
        double valueRight = 20.0 + (30.0 - 20.0) * j / (height - 1);

        grid[j * width] = valueLeft;
        gridNext[j * width] = valueLeft;

        grid[j * width + (width - 1)] = valueRight;
        gridNext[j * width + (width - 1)] = valueRight;
    }

    grid[0] = 10.0;
    grid[width - 1] = 20.0;
    grid[(height - 1) * width] = 20.0;
    grid[height * width - 1] = 30.0;

    gridNext[0] = 10.0;
    gridNext[width - 1] = 20.0;
    gridNext[(height - 1) * width] = 20.0;
    gridNext[height * width - 1] = 30.0;

    #pragma acc enter data copyin(grid[:width * height], gridNext[:width * height])
}

double calcNext(double *__restrict grid, double *__restrict gridNext, int width, int height)
{
    double maxError = 0.0;

    #pragma acc parallel loop reduction(max:maxError) present(grid, gridNext)
    for (int j = 1; j < height - 1; j++)
    {
        #pragma acc loop
        for (int i = 1; i < width - 1; i++)
        {
            gridNext[OFFSET(j, i, width)] =
                0.25 * (
                    grid[OFFSET(j, i + 1, width)] +
                    grid[OFFSET(j, i - 1, width)] +
                    grid[OFFSET(j - 1, i, width)] +
                    grid[OFFSET(j + 1, i, width)]
                );

            maxError = fmax(
                maxError,
                fabs(gridNext[OFFSET(j, i, width)] - grid[OFFSET(j, i, width)])
            );
        }
    }

    return maxError;
}

void swap(double *__restrict grid, double *__restrict gridNext, int width, int height)
{
    #pragma acc parallel loop present(grid, gridNext)
    for (int j = 1; j < height - 1; j++)
    {
        #pragma acc loop
        for (int i = 1; i < width - 1; i++)
        {
            grid[OFFSET(j, i, width)] = gridNext[OFFSET(j, i, width)];
        }
    }
}

void deallocate(double *__restrict grid, double *__restrict gridNext)
{
    #pragma acc exit data delete(grid, gridNext)
    free(grid);
    free(gridNext);
}