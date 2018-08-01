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
	
	// Check results if required
	/*BYTE* display_imge = 0;
	int choice = 1;
	while (choice != 0){
		cout << "1. Convert to binary" << endl;
		cout << "2. Boundary extraction" << endl;
		cout << "3. Erosion" << endl;
		cout << "4. Dilation" << endl;
		cout << "5. Colored CCA" << endl;
		cout << "0. Exit the program" << endl;
		cout << "> ";
		cin >>  choice;
		cout << endl;

		switch (choice){
		case 1:
			display_imge = ConvertIntensityToBMP(bin, Width, Height, &new_size);
			output = L"1.bmp";
			printf("\033c");
			if (SaveBMP(display_imge, Width, Height, new_size, output))
				cout << "\033[0;32m[+]\033[0m Output Image was successfully saved" << endl << endl;
			else cout << "\033[1;31m[-]\033[0m Error on saving image" << endl;
			break;
		case 2:
			display_imge = ConvertIntensityToBMP(be, Width, Height, &new_size);
			output = L"2.bmp";
			printf("\033c");
			if (SaveBMP(display_imge, Width, Height, new_size, output))
				cout << "\033[0;32m[+]\033[0m Output Image was successfully saved" << endl << endl;
			else cout << "\033[1;31m[-]\033[0m Error on saving image" << endl;
			break;
		case 3:
			display_imge = ConvertIntensityToBMP(er, Width, Height, &new_size);
			output = L"3.bmp";
			printf("\033c");
			if (SaveBMP(display_imge, Width, Height, new_size, output))
				cout << "\033[0;32m[+]\033[0m Output Image was successfully saved" << endl << endl;
			else cout << "\033[1;31m[-]\033[0m Error on saving image" << endl;
			break;
		case 4:
			display_imge = ConvertIntensityToBMP(dil, Width, Height, &new_size);
			output = L"4.bmp";
			printf("\033c");
			if (SaveBMP(display_imge, Width, Height, new_size, output))
				cout << "\033[0;32m[+]\033[0m Output Image was successfully saved" << endl << endl;
			else cout << "\033[1;31m[-]\033[0m Error on saving image" << endl;
			break;
		case 5:
			display_imge = ConvertIntensityToBMP(col, Width, Height, &new_size);
			output = L"5.bmp";
			printf("\033c");
			if (SaveBMP(display_imge, Width, Height, new_size, output))
				cout << "\033[0;32m[+]\033[0m Output Image was successfully saved" << endl << endl;
			else cout << "\033[1;31m[-]\033[0m Error on saving image" << endl;
			break;
		case 0:
			cout << "\033[1;34m[?]\033[0m Exiting program. See ya!" << endl;
		}
	}*/

	delete[] display_imge;
	delete[] temp_tag;
	delete[] buffer;
	delete[] raw_intensity;
	
	return 0;
}
