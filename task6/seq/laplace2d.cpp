#include <cmath>
#include <cstdlib>
#include <cstring>

void initialize(double *grid, double *gridNext, int width, int height)
{
    memset(grid, 0, height * width * sizeof(double));
    memset(gridNext, 0, height * width * sizeof(double));

    for (int i = 0; i < width; i++) 
    {
        double valueTop = 10.0 + (20.0 - 10.0) * i / (width - 1);
        grid[i] = valueTop;
        gridNext[i] = valueTop;
    }

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
}

double calcNext(double *__restrict grid, double *__restrict gridNext, int width, int height)
{
    double maxError = 0.0;

    for(int j = 1; j < height - 1; j++)
    {
        int row = j * width;
        int rowUp = (j - 1) * width;
        int rowDown = (j + 1) * width;

        for (int i = 1; i < width - 1; i++)
        {
            int idx = row + i;

            double newValue =
                0.25 * (
                    grid[row + i + 1] +
                    grid[row + i - 1] +
                    grid[rowUp + i] +
                    grid[rowDown + i]
                );

            gridNext[idx] = newValue;

            maxError = fmax(maxError, fabs(newValue - grid[idx]));
        }
    }

    return maxError;
}

void deallocate(double *grid, double *gridNext)
{
    free(grid);
    free(gridNext);
}