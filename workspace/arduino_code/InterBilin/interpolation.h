#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#ifdef __cplusplus
extern "C" {
#endif

#define N 86

int find_interval(const float arr[], int n, float q);
float interpolate(const float *coords[2], const int n[2],
                  const float values[N][N], const float query_points[2]);

#ifdef __cplusplus
}
#endif

#endif
