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
	uint32_t format = YUV420NV12;
	uint32_t x = 0;
	uint32_t y = 0;
	//uint32_t ROIx = 0;
	//uint32_t ROIy = 0;
	bool onOddRow;
	bool onOddCol;

	if (x % 2 == 1) {
		onOddCol = true;
	}
	else {
		onOddCol = false;
	}

	if (y % 2 == 1) {
		onOddRow = true;
	}
	else {
		onOddRow = false;
	}

	DUAL_CAM_SYNTHESIS_PARAM defaultParam;
	defaultParam.inputFrontInfo.width = w;
	defaultParam.inputFrontInfo.height = h;
	defaultParam.inputFrontInfo.stride = stride;
	defaultParam.inputFrontInfo.scanline = scanline;
	size = getAlignedStride(defaultParam.inputFrontInfo.width, defaultParam.inputFrontInfo.stride) *
		getAlignedStride(defaultParam.inputFrontInfo.height, defaultParam.inputFrontInfo.scanline) * 3 / 2;
	defaultParam.inputFrontInfo.bufSize = size;
	defaultParam.inputFrontInfo.format = format;
	defaultParam.frontScaledInfo.width = w / 10 * 2;
	defaultParam.frontScaledInfo.height = h / 10 * 2;
	defaultParam.frontScaledInfo.stride = stride;
	defaultParam.frontScaledInfo.scanline = scanline;
	defaultParam.frontScaledInfo.bufSize = defaultParam.inputFrontInfo.bufSize;
	defaultParam.frontScaledInfo.format = format;
	defaultParam.inputBackInfo.width = w;
	defaultParam.inputBackInfo.height = h;
	defaultParam.inputBackInfo.stride = stride;
	defaultParam.inputBackInfo.scanline = scanline;
	size = getAlignedStride(defaultParam.inputBackInfo.width, defaultParam.inputBackInfo.stride) *
		getAlignedStride(defaultParam.inputBackInfo.height, defaultParam.inputBackInfo.scanline) * 3 / 2;
	defaultParam.inputBackInfo.bufSize = size;
	defaultParam.inputBackInfo.format = format;
	defaultParam.targetPoint.x = x;
	defaultParam.targetPoint.y = y;
	defaultParam.targetPoint.onOddRow = onOddRow;
	defaultParam.targetPoint.onOddCol = onOddCol;
	//defaultParam.frontROI.x = ;
	//defaultParam.frontROI.y = ;

	// 2. paper input data for process engine
	unsigned char* frontImg = new uint8_t[w * h * 3 / 2];
	unsigned char* backImg = new uint8_t[w * h * 3 / 2];

	ifstream OpenFile("D:\\Synthesis\\frontSrc-W1920-h1080.yuv", ios::in | ios::binary);
	if (OpenFile.fail()) {
		cout << " Open YUV1 failed!" << endl;
	}
	OpenFile.read((char*)frontImg, w * h * 3 / 2);
	OpenFile.close();

	ifstream OpenFile2("D:\\Synthesis\\backSrc-W1920-h1080.yuv", ios::in | ios::binary);
	if (OpenFile2.fail()) {
		cout << " Open YUV2 failed!" << endl;
	}
	OpenFile2.read((char*)backImg, w * h * 3 / 2);
	OpenFile2.close();

	// 3. create and initialize process engine
	SynthesisEngine SE;
	int32_t result;
	result = SE.Initialize(defaultParam);
	cout << " SE.Initialize " << result << endl;
	
	// 4. update target beginning point
	BEGIN_POINT currentPoint;
	currentPoint.x = 777;
	currentPoint.y = 666;
	SE.UpdateTargetPoint(currentPoint);

	// 5. do process
	result = SE.ProcessDownScale(frontImg);
	cout << " SE.ProcessDownScale " << result << endl;

	result = SE.ProcessSynthesis(frontImg, backImg);
	cout << " SE.ProcessSynthesis " << result << endl;

	// 6. save output
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
