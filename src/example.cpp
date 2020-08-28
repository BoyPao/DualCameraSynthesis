//////////////////////////////////////////////////////////////////////////////////////
// Author: Peng Hao
// License: GPL
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// @file: example.cpp
// @brief: A simple example which shows how to use DualCamSynthesis algorithm.
//////////////////////////////////////////////////////////////////////////////////////

#include "DualCamSynthesis.h"

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream> 

using namespace std;

int main() {
	// 1. set param values for process engine
	int32_t w = 1920;
	int32_t h = 1080;
	int32_t size;
	uint32_t stride = 0;
	uint32_t scanline = stride;
	uint32_t format = DCS_YUV420NV12;
	uint32_t x = 0;
	uint32_t y = 0;

	// 2. paper input data for process engine
	unsigned char* frontImg = new uint8_t[w * h * 3 / 2];
	unsigned char* frontImgY;
	unsigned char* frontImgUV;
	unsigned char* backImg = new uint8_t[w * h * 3 / 2];
	unsigned char* backImgY;
	unsigned char* backImgUV;

	ifstream OpenFile("D:\\Synthesis\\frontSrc-W1920-h1080.yuv", ios::in | ios::binary);
	if (OpenFile.fail()) {
		cout << " Open YUV1 failed!" << endl;
	}
	OpenFile.read((char*)frontImg, w * h * 3 / 2);
	frontImgY = frontImg;
	frontImgUV = frontImgY + w * h;
	OpenFile.close();

	ifstream OpenFile2("D:\\Synthesis\\backSrc-W1920-h1080.yuv", ios::in | ios::binary);
	if (OpenFile2.fail()) {
		cout << " Open YUV2 failed!" << endl;
	}
	OpenFile2.read((char*)backImg, w * h * 3 / 2);
	backImgY = backImg;
	backImgUV = backImgY + w * h;
	OpenFile2.close();

	// 3. create Engine and set initial Params
	SynthesisEngine SE;
	int32_t result;
	result = SE.SetInitParams(w, h,
							  w / 10 * 2, h / 10 * 2, 
							  w, h,
							  stride, scanline,
							  x, y,
							  format);
	cout << " SE.SetInitParm " << result << endl;

	// 4.initialize process engine
	result = SE.Initialize();
	cout << " SE.Initialize " << result << endl;
	
	// 5. update target beginning point
	BEGIN_POINT currentPoint;
	currentPoint.x = 333;
	currentPoint.y = 444;
	SE.UpdateTargetPoint(currentPoint);
	cout << " SE.UpdateTargetPoint " << result << endl;

	// 6. do process
	result = SE.ProcessDownScale(frontImgY, frontImgUV);
	cout << " SE.ProcessDownScale " << result << endl;

	result = SE.ProcessSynthesis(frontImgY, frontImgUV, backImgY, backImgUV);
	cout << " SE.ProcessSynthesis " << result << endl;

	// 7. save output
	ofstream OpenFile3("D:\\Synthesis\\output.yuv");
	if (OpenFile3.fail()) {
		cout << " Open YUV3 failed!" << endl;
	}
	OpenFile3.write((char*)backImg, w * h * 3 / 2);
	OpenFile3.close();
	cout << " YUV3 write finished" << endl;

	cin >> result;
	return 0;
}
