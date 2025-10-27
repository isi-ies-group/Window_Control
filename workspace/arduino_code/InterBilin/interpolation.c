#include <interpolation.h>


int find_interval(const float arr[], int n, float q) {
  if (q <= arr[0]) return 0;
  if (q >= arr[n-1]) return n-2;
  int low = 0;
  int high = n - 1;
  while (high - low > 1) {
    int mid = (low + high) / 2;
    if (q < arr[mid]) high = mid;

    else low = mid;
  }
  return low;
}

float interpolate(const float *coords[2], const int n[2],
                  const float values[N][N], const float query_points[2]) {
  int indices[2] = {0};
  float t[2] = {0.0};

  // Find intervals and compute weights
  for (int d = 0; d < 2; d++) {
    indices[d] = find_interval(coords[d], n[d], query_points[d]);
    float x0 = coords[d][indices[d]];
    float x1 = coords[d][indices[d] + 1];
    t[d] = (query_points[d] - x0) / (x1 - x0);     
  }
  // Bilinear interpolation
  int i = indices[0];
  int j = indices[1];

  float z00 = values[i][j];
  float z01 = values[i][j + 1];
  float z10 = values[i + 1][j];
  float z11 = values[i + 1][j + 1];

  float z0 = z00 + t[1] * (z01 - z00);
  float z1 = z10 + t[1] * (z11 - z10);

  return z0 + t[0] * (z1 - z0);
}