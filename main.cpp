#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <math.h>
#include <ctime>
#include <atlstr.h>

//User defined includes
#include "imge_bmp.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	LPCTSTR input, output;
	int Width, Height;
	long Size, new_size;
	std::clock_t start;
	double duration;

	input = L"test3.bmp";
	printf("\033c");

	cout << "\n>> Image processing program for \033[0;32mGREENGROCERS\033[0m ¯\\_(:/)_/¯" << endl;
	cout << ">>\033[0;36m https://github.com/abdllhcay \033[0m\n" << endl;
	cout << "\033[0;32m[+]\033[0m Input image ";
	wcout << input << endl;
	cout << "\033[1;34m[?]\033[0m Rendering ..." << endl;

	start = clock();

	BYTE* buffer = LoadBMP(&Width, &Height, &Size, input);
	BYTE* raw_intensity = ConvertBMPToIntensity(buffer, Width, Height);
	int* hist = CreateHistogram(raw_intensity, Width, Height);
	BYTE* bin = ConvertBinary(raw_intensity, hist, Width*Height);
	BYTE* be = BoundaryExtraction(bin, Width, Height, 3);
	BYTE* er = Erosion(bin, Width, Height, 5);
	BYTE* dil = Dilation(er, Width, Height, 5);
	int* tag = CCA(er, Width, Height);

	int* temp_tag = new int[Width*Height];
	for (int i = 0; i < Width*Height; i++)
		temp_tag[i] = tag[i];
	
	// Sort tag array
	mergeSort(temp_tag, Width*Height);

	 //Count number of unique tags
	int count = 0;
	for (int i = 0; i < Width*Height; i++){
		if (temp_tag[i] != temp_tag[i + 1])
			count++;
	}

	count = count - 1;

	//Extract unique tags
	int* sorted_tag = new int[count];
	int j = 0;
	for (int i = 0; i < Width*Height; i++){
		if (temp_tag[i] != -1 && temp_tag[i] != temp_tag[i + 1]){
			sorted_tag[j] = temp_tag[i];
			j++;
		}
	}

	BYTE* col = ColorTags(tag, sorted_tag, Width*Height, count);
	int* legume = Classification(dil, tag, sorted_tag, Width, Height, count);

	int numChickpea = 0;
	int numLentil = 0;

	for (int i = 0; i < count; i++){
		if (legume[i] == 1)
			numChickpea++;
		else if (legume[i] == 0)
			numLentil++;
	}

	duration = (clock() - start) / (double)CLOCKS_PER_SEC;
	cout << "\033[0;32m[+]\033[0m Elapsed time: " << duration << endl;
	cout << "\033[0;32m[+]\033[0m Number of objects: " << count << endl;
	cout << "\033[0;32m[+]\033[0m Number of chickpeas: " << numChickpea << endl;
	cout << "\033[0;32m[+]\033[0m Number of lentils: " << numLentil << endl << endl;

	delete[] display_imge;
	delete[] temp_tag;
	delete[] buffer;
	delete[] raw_intensity;
	
	return 0;
}
