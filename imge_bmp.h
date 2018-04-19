#include <windows.h>

int* slice(int *arr, int start, int end);
void merge(int *result, int *left, int *right, int leftLen, int rightLen);
void mergeSort(int *arr, int len);
BYTE* LoadBMP(int* width, int* height, long* size, LPCTSTR bmpfile);
bool  SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile);
BYTE* ConvertBMPToIntensity(BYTE* Buffer, int width, int height);
BYTE* ConvertIntensityToBMP(BYTE* Buffer, int width, int height, long* newsize);
void DrawSquare(BYTE* buffer, int x, int y, int a, int width, int height);
//BYTE* ZoomImage(BYTE* buffer, int x1, int y1, int x2, int y2, int width, int height);
int* CreateHistogram(BYTE* buffer, int width, int height);
BYTE* HistogramEqu(BYTE* buffer, int Width, int Height);
BYTE* ConvertBinary(BYTE* img, int* hist, int imgSize);
BYTE* Dilation(BYTE* img, int width, int height, int N);
BYTE* Erosion(BYTE* img, int width, int height, int N);
BYTE* BoundaryExtraction(BYTE* img, int width, int height, int N);
int* CCA(BYTE* img, int width, int height);
void PrintTags(int* tag, int size);
BYTE* ColorTags(int* tag, int* sorted_tag, int size, int count);
int* Classification(BYTE* img, int* tag, int* sorted_tag, int width, int height, int count);
