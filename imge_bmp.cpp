#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <iostream>

#define PI 3.14159265358979323846
#define CLUSTER_NUM 2

using namespace std;

int* slice(int *arr, int start, int end){
	int *result = (int *)malloc((end - start) * sizeof(int));

	for (int i = start; i < end; i++)
		result[i - start] = arr[i];

	return result;
}

void merge(int *result, int *left, int *right, int leftLen, int rightLen){
	int i = 0, j = 0;
	while (i < leftLen && j < rightLen)
	{
		if (left[i] < right[j]){
			result[i + j] = left[i];
			i++;
		}
		else{
			result[i + j] = right[j];
			j++;
		}
	}

	for (; i < leftLen; i++)
		result[i + j] = left[i];

	for (; j < rightLen; j++)
		result[i + j] = right[j];

	free(left);
	free(right);
}

void mergeSort(int *arr, int len){
	if (len <= 1)
		return;

	int *left = slice(arr, 0, len / 2 + 1);
	int *right = slice(arr, len / 2, len);

	mergeSort(left, len / 2);
	mergeSort(right, len - (len / 2));

	merge(arr, left, right, len / 2, len - (len / 2));
}

BYTE* LoadBMP(int* width, int* height, long* size, LPCTSTR bmpfile)
{
	// declare bitmap structures
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	// value to be used in ReadFile funcs
	DWORD bytesread;
	// open file to read from
	HANDLE file = CreateFile(bmpfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (NULL == file)
		return NULL; // coudn't open file

	// read file header
	if (ReadFile(file, &bmpheader, sizeof (BITMAPFILEHEADER), &bytesread, NULL) == false)  {
		       CloseHandle(file);
		       return NULL;
	      }
	//read bitmap info
	if (ReadFile(file, &bmpinfo, sizeof (BITMAPINFOHEADER), &bytesread, NULL) == false) {
		        CloseHandle(file);
		        return NULL;
	      }
	// check if file is actually a bmp
	if (bmpheader.bfType != 'MB')  	{
		       CloseHandle(file);
		       return NULL;
	      }
	// get image measurements
	*width = bmpinfo.biWidth;
	*height = abs(bmpinfo.biHeight);

	// check if bmp is uncompressed
	if (bmpinfo.biCompression != BI_RGB)  {
		      CloseHandle(file);
		      return NULL;
	      }
	// check if we have 24 bit bmp
	if (bmpinfo.biBitCount != 24) {
		      CloseHandle(file);
		      return NULL;
	     }

	// create buffer to hold the data
	*size = bmpheader.bfSize - bmpheader.bfOffBits;
	BYTE* Buffer = new BYTE[*size];
	// move file pointer to start of bitmap data
	SetFilePointer(file, bmpheader.bfOffBits, NULL, FILE_BEGIN);
	// read bmp data
	if (ReadFile(file, Buffer, *size, &bytesread, NULL) == false)  {
		     delete[] Buffer;
		     CloseHandle(file);
		     return NULL;
	      }
	// everything successful here: close file and return buffer
	CloseHandle(file);

	return Buffer;
}//LOADPMB

bool SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile)
{
	// declare bmp structures 
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;

	// andinitialize them to zero
	memset(&bmfh, 0, sizeof (BITMAPFILEHEADER));
	memset(&info, 0, sizeof (BITMAPINFOHEADER));

	// fill the fileheader with data
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+paddedsize;
	bmfh.bfOffBits = 0x36;		// number of bytes to start of bitmap bits

	// fill the infoheader

	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;			// we only have one bitplane
	info.biBitCount = 24;		// RGB mode is 24 bits
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;		// can be 0 for 24 bit images
	info.biXPelsPerMeter = 0x0ec4;     // paint and PSP use this values
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;			// we are in RGB mode and have no palette
	info.biClrImportant = 0;    // all colors are important

	// now we open the file to write to
	HANDLE file = CreateFile(bmpfile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL)  	{
		    CloseHandle(file);
		    return false;
	   }
	// write file header
	unsigned long bwritten;
	if (WriteFile(file, &bmfh, sizeof (BITMAPFILEHEADER), &bwritten, NULL) == false)  {
		       CloseHandle(file);
		       return false;
	      }
	// write infoheader
	if (WriteFile(file, &info, sizeof (BITMAPINFOHEADER), &bwritten, NULL) == false)  {
		     CloseHandle(file);
		     return false;
	      }
	// write image data
	if (WriteFile(file, Buffer, paddedsize, &bwritten, NULL) == false)  {
		      CloseHandle(file);
		      return false;
	     }

	// and close file
	CloseHandle(file);

	return true;
} // SaveBMP

BYTE* ConvertBMPToIntensity(BYTE* Buffer, int width, int height)
{
	// first make sure the parameters are valid
	if ((NULL == Buffer) || (width == 0) || (height == 0))
		return NULL;

	// find the number of padding bytes

int padding = 0;
int scanlinebytes = width * 3;
while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
padding++;
// get the padded scanline width
int psw = scanlinebytes + padding;

// create new buffer
BYTE* newbuf = new BYTE[width*height];

// now we loop trough all bytes of the original buffer, 
// swap the R and B bytes and the scanlines
long bufpos = 0;
long newpos = 0;
for (int row = 0; row < height; row++)
	for (int column = 0; column < width; column++)  {
		newpos = row * width + column;
		bufpos = (height - row - 1) * psw + column * 3;
		newbuf[newpos] = BYTE(0.11*Buffer[bufpos + 2] + 0.59*Buffer[bufpos + 1] + 0.3*Buffer[bufpos]);
	}

return newbuf;
}//ConvetBMPToIntensity

BYTE* ConvertIntensityToBMP(BYTE* Buffer, int width, int height, long* newsize)
{
	// first make sure the parameters are valid
	if ((NULL == Buffer) || (width == 0) || (height == 0))
		return NULL;

	// now we have to find with how many bytes
	// we have to pad for the next DWORD boundary	

	int padding = 0;
	int scanlinebytes = width * 3;
	while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
		padding++;
	// get the padded scanline width
	int psw = scanlinebytes + padding;
	// we can already store the size of the new padded buffer
	*newsize = height * psw;

	// and create new buffer
	BYTE* newbuf = new BYTE[*newsize];

	// fill the buffer with zero bytes then we dont have to add
	// extra padding zero bytes later on
	memset(newbuf, 0, *newsize);

	// now we loop trough all bytes of the original buffer, 
	// swap the R and B bytes and the scanlines
	long bufpos = 0;
	long newpos = 0;
	for (int row = 0; row < height; row++)
		for (int column = 0; column < width; column++)  	{
			bufpos = row * width + column;     // position in original buffer
			newpos = (height - row - 1) * psw + column * 3;           // position in padded buffer
			newbuf[newpos] = Buffer[bufpos];       //  blue
			newbuf[newpos + 1] = Buffer[bufpos];   //  green
			newbuf[newpos + 2] = Buffer[bufpos];   //  red
		}

	return newbuf;
} //ConvertIntensityToBMP

void DrawSquare(BYTE* buffer, int x, int y, int a, int width, int height){
	int pos, i, j;
	pos = ((y - 1) * width - 1) + x;

	//Üst ve alt çizgi
	for (i = 0; i < a; i++){
		buffer[pos + i] = 0;
		buffer[pos + (a - 1)*width + i] = 0;
	}

	//Yan çizgiler
	for (i = 0; i < a; i++){
		buffer[pos + i*width] = 0;
		buffer[pos + i*width + (a - 1)] = 0;
	}
}

BYTE MaskOperation(BYTE* maskArray, BYTE* imageArray){
	BYTE sum = 0;

	for (int i = 0; i < 9; i++){
		sum = sum + maskArray[i] * imageArray[i];
	}

	return sum;
}

//BYTE* ZoomImage(BYTE* buffer, int x1, int y1, int x2, int y2, int width, int height){
//	int pos, i, j, maskArraySize;
//
//	int selectedAreaWidth = x2 - x1;
//	int selectedAreaHeight = y2 - y1;
//	pos = (y1 - 1) * width + x1 - 1;
//	BYTE* selectedAreaArray = new BYTE[selectedAreaWidth*selectedAreaHeight];
//
//	//Copy bytes
//	for (i = 0; i < selectedAreaHeight; i++)
//		for (j = 0; j < selectedAreaWidth; j++)
//			selectedAreaArray[i*selectedAreaWidth + j] = buffer[pos + i*width + j];
//
//	int expandedWidth = (selectedAreaWidth + 1) * 2 - 1;
//	int expandedHeight = (selectedAreaHeight + 1) * 2 - 1;
//	BYTE* expandedArray = new BYTE[expandedWidth*expandedHeight];
//
//	//Expand bytes
//	for (i = 0; i < expandedHeight; i++){
//		for (j = 0; j < expandedWidth; j++){
//			if ((i + 1) % 2 == 0 && (j + 1) % 2 == 0)
//				expandedArray[i*expandedWidth + j] = selectedAreaArray[(i / 2)*selectedAreaWidth + (j / 2)];
//			else
//				expandedArray[i*expandedWidth + j] = 0;
//		}
//	}
//
//	delete[] selectedAreaArray;
//
//	BYTE maskArray[9] = { 4, 2, 4, 2, 1, 2, 4, 2, 4 };
//	BYTE toMask[9] = {0};
//	BYTE* zoomed = new BYTE[(width - 2)*(height - 2)];
//
//	//Masking
//	for (i = 0; i<height-2; i++) {
//		for (j = 0; j<width-2; j++) {
//			pos = i*expandedWidth + j;
//			for (int k = 0; k < 3; k++) {
//				for (int l = 0; l < 3; l++) {
//					toMask[i*3 + j] = expandedArray[pos + i*expandedWidth + j];
//				}
//			}
//			zoomed[i*(width-2) + j] = maskOperation(maskArray, toMask);  //zoomed is our boy!
//		}
//	}
//
//	delete[] expandedArray;
//	delete[] maskArray;
//
//	return zoomed;
//}

int* CreateHistogram(BYTE* buffer, int width, int height){
	int* histogram = new int[256];

	for (int i = 0; i < 256; i++) {
		histogram[i] = 0;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			histogram[buffer[i*width + j]]++;
		}
	}

	return histogram;
}

BYTE* HistogramEqu(BYTE* buffer, int Width, int Height){
	int i, j;
	float sum = 0;
	float hSum = 0;
	BYTE* histogram = new BYTE[256];

	for (i = 0; i < 256; i++)
		histogram[i] = 0;

	for (i = 0; i < Width*Height; i++)
		histogram[buffer[i]]++;

	for (i = 0; i < 256; i++)
		sum = sum + histogram[i];

	for (i = 0; i < 256; i++){
		hSum = hSum + histogram[i];
		histogram[i] = hSum / sum * 255;
	}
		
	for (i = 0; i < Width*Height; i++)
		buffer[i] = histogram[buffer[i]];

	delete[] histogram;

	return buffer;

}

BYTE* ConvertBinary(BYTE* img, int* hist, int imgSize){
	// Converts image to binary representation with K-Means algorithm.

	int t0 = 0;
	int t1 = 255;
	int newt0 = t0;
	int newt1 = t1;
	int c0;
	int c1;
	int sum0;
	int sum1;
	BYTE* flags = new BYTE[256];

	do{
		t0 = newt0;
		t1 = newt1;

		for (int i = 0; i < 256; i++) {
			if (abs(t0 - i) < abs(t1 - i)) {
				flags[i] = 0;
			}
			else 
				flags[i] = 1;
		}

		c0 = 0;
		c1 = 0;
		sum0 = 0;
		sum1 = 0;

		for (int i = 0; i < 256; i++) {
			if (flags[i] == 0) {
				c0 += hist[i];
				sum0 += i*hist[i];
			}
			else {
				c1 += hist[i];
				sum1 += i*hist[i];
			}
		}

		newt0 = sum0 / c0;
		newt1 = sum1 / c1;

	} while (t0 != newt0 && t1 != newt1);

	for (int i = 0; i < imgSize; i++) 
		img[i] = flags[img[i]] * 255;

	delete[] flags;
	return img;
}

BYTE* Dilation(BYTE* img, int width, int height, int N){
	int imgSize = width*height;
	BYTE* dilated = new BYTE[imgSize];
	int* mask = new int[N*N];
	int pos, posEnd, i, j, k;
	int unit = N / 2;

	for (i = 0; i < imgSize; i++)
		dilated[i] = 0;

	for (i = 0; i < N*N; i++)
		mask[i] = 1;
	
	for (i = 0; i < imgSize; i++){
		if (img[i] == 255){
			pos = i - unit*width - unit;  //pos --> Left upper corner of mask. 
			posEnd = pos + (N - 1)*width + N - 1;  //posEnd --> Right bottom corner of mask.
			if (pos > 0 && posEnd < width*height){  //Check for any overflow
				for (j = 0; j < N; j++){
					for (k = 0; k < N; k++)
						dilated[pos + j*width + k] = (img[pos + j*width + k] || mask[j*width + k])*255;
				}
			}
		}
	}

	delete[] mask;
	return dilated;
}

BYTE* Erosion(BYTE* img, int width, int height, int N){
	int imgSize = width*height;
	int* mask = new int[N*N];
	BYTE* temp = new BYTE[imgSize];
	int pos, posEnd, i, j, k;
	int unit = N / 2;
	int championBit; //Result of masking

	for (i = 0; i < imgSize; i++)
		temp[i] = 0;

	for (i = 0; i < N*N; i++)
		mask[i] = 1;

	for (i = 0; i < imgSize; i++){
		championBit = 1;
		if (img[i] == 255){
			pos = i - unit*width - unit;  //pos --> Left upper corner of mask. 
			posEnd = pos + (N - 1)*width + N - 1;  //posEnd --> Right bottom corner of mask.
			if (pos >= 0 && posEnd < imgSize){  //Check for any overflow
				for (j = 0; j < N; j++){
					for (k = 0; k < N; k++){
						championBit = championBit && img[pos + j*width + k] && mask[j*N + k];
						temp[i] = championBit*255;
					}	
				}
			}
		}
	}

	delete[] mask;
	return temp;
}

BYTE* BoundaryExtraction(BYTE* img, int width, int height, int N){
	int imgSize = width*height;
	BYTE* dilated = new BYTE[imgSize];
	BYTE* buffer = new BYTE[imgSize];

	for (int i = 0; i < imgSize; i++)
		buffer[i] = img[i];

	dilated = Erosion(buffer, width, height, N);

	for (int i = 0; i < imgSize; i++){
		buffer[i] = buffer[i] ^ dilated[i];
	}

	delete[] dilated;
	return buffer;
}

int* CCA(BYTE* img, int width, int height){
	//Connected Component Analysis

	int imgSize = width*height;
	int* tag = new int[imgSize];
	int t = 2;

	for (int i = 0; i < imgSize; i++)
		tag[i] = -1;

	for (int i = 0; i < imgSize; i++){  //First iteration
		if (img[i] == 255){
			if (tag[i - width] * tag[i - 1] == 1){  //No tag in two neighbors
				tag[i] = t;
				t++;
			}
			else if (tag[i - width] * tag[i - 1] < 0){ //One has a tag and the other one has not
				if (tag[i - 1] == -1)
					tag[i] = tag[i - width];
				else if (tag[i - width] == -1)
					tag[i] = tag[i - 1];
			}
			else if (tag[i - width] * tag[i - 1] > 0 && tag[i - width] == tag[i - 1]){  //All of two neighbors have a tag
				tag[i] = tag[i - width];
			}
			else if (tag[i - width] * tag[i - 1] > 0 && tag[i - width] != tag[i - 1]){
				tag[i] = 0; //Collision
			}
			else if (tag[i - width] * tag[i - 1] == 0){  //One collision and the other one is a tag
				if (tag[i - width] == 0 && tag[i - 1] == 0)
					tag[i] = 0;
				else if (tag[i - width] > 0)
					tag[i] = tag[i - width];
				else if (tag[i - 1] > 0)
					tag[i] = tag[i - 1];
			}
		}
	}

	int smallerOne;

	for (int i = 0; i < imgSize; i++){  //Second iteration
		if (tag[i] == 0){
			if (tag[i - width] < tag[i - 1]){
				tag[i] = tag[i - width];

				smallerOne = tag[i - 1]; //Prevent overwriting tag[i-1]

				for (int j = 0; j < imgSize; j++){
					if (tag[j] == smallerOne)
						tag[j] = tag[i - width];
				}
			}

			else{
				tag[i] = tag[i - 1];
				smallerOne = tag[i - width];

				for (int j = 0; j < imgSize; j++){
					if (tag[j] == smallerOne)
						tag[j] = tag[i - 1];
				}
			}
		}
	}

	return tag;
}

void PrintTags(int* tag, int size){
	printf("\nTags = [ ");
	for (int i = 0; i < size; i++){
		printf("%d ", tag[i]);
	}
	printf("]");
}

BYTE* ColorTags(int* tag, int* unique_tag, int size, int count){
	BYTE* colorful = new BYTE[size];
	int color = 0;
	int p;

	for (int i = 0; i < size; i++){
		colorful[i] = 0;
	}

	for (int j = 0; j < count; j++){
		color = color + 256 / count;
		for (int i = 0; i < size; i++){
			if (tag[i] == unique_tag[j])
				colorful[i] = color;
		}
	}
	
	return colorful;
}

int* Classification(BYTE* img, int* tag, int* sorted_tag, int width, int height, int count){
	int imgSize = width*height;
	int area;
	int edge;
	float thinness;
	BYTE* buffer = new BYTE[imgSize];
	BYTE* objBoundary;

	struct featureSpace{
		float thinness;
		int area;
	};

	struct weight{
		float x;
		int y;
	};

	struct featureSpace* object = new struct featureSpace[count];
	struct weight t[CLUSTER_NUM];
	struct weight new_t[CLUSTER_NUM];  // Default is 2

	// Feature space extraction ----------------------------------->

	int k = 0;
	while (k < count){
		area = 0;
		edge = 0;

		for (int i = 0; i < imgSize; i++)
			buffer[i] = 0;

		for (int i = 0; i < imgSize; i++){
			if (tag[i] == sorted_tag[k]){
				buffer[i] = 255;
				area++;
			}
		}

		objBoundary = BoundaryExtraction(buffer, width, height, 3);

		for (int i = 0; i < imgSize; i++)
			if (objBoundary[i] == 255)
				edge++;

		thinness = 4 * PI * area / pow(edge, 2);

		object[k].thinness = thinness;
		object[k].area = area;

		k++;
	}

	float minT = object[0].thinness; 
	int minA = object[0].area;

	for (int i = 1; i < count; i++){
		if (object[i].thinness < minT)
			minT = object[i].thinness;
		if (object[i].area < minA)
			minA = object[i].area;
	}

	cout << "\033[0;32m[+]\033[0m Feature space __" << endl;
	for (int i = 0; i < count; i++){
		cout << "\t\t    |__> Object " << i << " (" << object[i].thinness << ", " << object[i].area << ")" << endl;
	}

	cout << endl;

	for (int i = 0; i < CLUSTER_NUM; i++){
		t[i].x = minT + 0.01*i;  // Thinness
		t[i].y = minA + 10*i;  // Area
		new_t[i].x = t[i].x;
		new_t[i].y = t[i].y;
	}

	cout << "\033[0;32m[+]\033[0m Weights _______" << endl;
	for (int i = 0; i < CLUSTER_NUM; i++){
		cout << "\t\t    |__> T" << i << " (" << t[i].x << ", " << t[i].y << ")" << endl;
	}

	cout << endl;

	// Classification ----------------------------------------->

	int* cluster = new int[count];
	float distance[CLUSTER_NUM];
	int min;
	bool flag = TRUE;

	do{
		for (int i = 0; i < CLUSTER_NUM; i++)
			t[i] = new_t[i];

		for (int i = 0; i < count; i++) {
			for (int j = 0; j < CLUSTER_NUM; j++){
				distance[j] = sqrt((pow((t[j].x - object[i].thinness), 2) + pow((t[j].y - object[i].area), 2)));  // Distance
			}

			min = 0;
			for (int k = 0; k < CLUSTER_NUM; k++){
				if (distance[k] < distance[min])
					min = k;	// T with minimum distance
			}

			cluster[i] = min;  // Which T has closer to tag "i". İndex is tag, value is T.
		}

		int sampleCount = 0;
		float thinnessSum = 0;
		int areaSum = 0;

		for (int i = 0; i < CLUSTER_NUM; i++){
			for (int j = 0; j < count; j++){
				if (cluster[j] == i){
					sampleCount++;
					thinnessSum = thinnessSum + object[j].thinness;
					areaSum = areaSum + object[j].area;
				}
			}

			new_t[i].x = thinnessSum / sampleCount;
			new_t[i].y = areaSum / sampleCount;
		}

		for (int i = 0; i < CLUSTER_NUM; i++){
			if (t[i].x == new_t[i].x && t[i].y == new_t[i].y)
				flag = FALSE;
		}

	} while (flag);

	cout << "\033[0;32m[+]\033[0m New weights ___" << endl;
	for (int i = 0; i < CLUSTER_NUM; i++){
		cout << "\t\t    |__> T" << i << " (" << new_t[i].x << ", " << new_t[i].y << ")" << endl;
	}

	cout << endl;

	cout << "\033[0;32m[+]\033[0m Classification _" << endl;
	for (int i = 0; i < count; i++){
		cout << "\t\t    |__> Tag " << i << " is class of " << "T" << cluster[i] << endl;
	}

	cout << endl;

	return cluster;

	/*delete[] buffer;
	delete[] objBoundary;*/

}
