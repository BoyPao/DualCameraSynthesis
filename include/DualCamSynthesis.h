//////////////////////////////////////////////////////////////////////////////////////
// Author: Peng Hao
// License: GPL
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// @file: DualCamSynthesis.h
// @brief: head file for class DualCamSynthesis
//////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#define SUCCESS(rc) ((rc) == NO_ERROR)
#define GET_ALIGNED(num, stride) (((num) + (stride) - 1) & (~((stride) - 1)))
size_t getAlignedStride(int32_t num, int32_t stride);

enum DUAL_CAM_SYNTHESIS_IMG_FORMAT {
	DCS_YUV420NV12 = 0,
	DCS_YUV420NV21,
	DCS_NOT_SUPPORT,
};

enum DUAL_CAM_SYNTHESIS_OVERRANGE_STATE {
	NO_OVERRANGE = 0,
	X_OVERRANGE,
	Y_OVERRANGE,
	XY_OVERRANGE,
};

enum DUAL_CAM_SYNTHESIS_MIRRORFLIP_STATE {
	NEEDNOT = 0,
	NEED_X_MIRROR,
	NEED_Y_FLIP,
	NEED_BOTH,
};

enum DUAL_CAM_SYNTHESIS_RESULT {
	NO_ERROR = 0,
	NOT_INITED,
	NO_MEMORY,
	EMPTY_INPUT,
	INVALID_PARAM,
	ORDER_ERROR,
};

struct IMG_INFO {
	uint32_t width;
	uint32_t height;
	uint32_t bufSize;
	uint32_t stride;
	uint32_t scanline;
	uint32_t format;
};

struct BEGIN_POINT {
	uint32_t x;
	uint32_t y;
	bool onOddRow;
	bool onOddCol;
};

struct DUAL_CAM_SYNTHESIS_PARAM {
	IMG_INFO inputFrontInfo;
	IMG_INFO frontScaledInfo;
	IMG_INFO inputBackInfo;
	BEGIN_POINT targetPoint;
	//BEGIN_POINT frontROI;    //neef to add ROI logical
};

class SynthesisEngine {

public:
	SynthesisEngine();
	~SynthesisEngine();

	int32_t Initialize();
	int32_t Deinit();
	int32_t ProcessDownScale(void* src);
	int32_t ProcessDownScale(void* src, void* srcUV);
	int32_t ProcessSynthesis(void* frontData, void* backData);
	int32_t ProcessSynthesis(void* frontDataY, void* frontDataUV, 
							 void* backDataY, void* backDataUV);
	int32_t SetParams(DUAL_CAM_SYNTHESIS_PARAM param);
	int32_t SetInitParams(uint32_t forntW, uint32_t frontH, 
					uint32_t scaledW, uint32_t scaledH,
					uint32_t backW, uint32_t backH,
					uint32_t stride, uint32_t scanline,
					uint32_t targetX, uint32_t targetY,
					uint32_t format);
	int32_t UpdateTargetPoint(BEGIN_POINT point);
	int32_t CheckParams();
	int32_t FixTargetPoint();

	bool mInited;

private:
	bool mParamValid;
	bool mScaled;
	DUAL_CAM_SYNTHESIS_PARAM mParam;
	uint8_t* mScaleBuf;
	int32_t mOverRangeState;
	int32_t mMirrorFlipState;
};
