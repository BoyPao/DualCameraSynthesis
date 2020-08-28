//////////////////////////////////////////////////////////////////////////////////////
// Author: Peng Hao
// License: GPL
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
// @file: DualCamSynthesis.cpp
// @brief: An algorithm to synthesis two images into one.
//      it will downscale front image, let the size of it be smaller than back image.
// @param: the input Img format should be YUV420, and 8bit for per pixel
//////////////////////////////////////////////////////////////////////////////////////

#include "DualCamSynthesis.h"
#include "libyuv.h"


size_t getAlignedStride(int32_t num, int32_t stride)
{
	return (stride > 1) ? GET_ALIGNED(num, stride) : num;
}


SynthesisEngine::SynthesisEngine()
	:mInited(false)
	,mParamValid(false)
	,mScaled(false)
{
}

SynthesisEngine::~SynthesisEngine() 
{ 
	Deinit(); 
};

int32_t SynthesisEngine::Initialize() 
{
	int32_t result = NOT_INITED;
	mInited = false;
	mOverRangeState = NO_OVERRANGE;
	mMirrorFlipState = NEEDNOT; //less time cosumption

	if (mParamValid) {
		mScaleBuf = new uint8_t[mParam.frontScaledInfo.bufSize];
		memset(mScaleBuf, 0, mParam.frontScaledInfo.bufSize);

		if (mScaleBuf == nullptr) {
			result = NO_MEMORY;
		}
		else {
			mInited = true;
			result = NO_ERROR;
		}
	}
	else {
		result = ORDER_ERROR;
	}

	return result;
}

int32_t SynthesisEngine::Deinit() {
	int32_t result = NO_ERROR;

	delete[] mScaleBuf;
	mScaleBuf = nullptr;
	mInited = false;
	mScaled = false;
	mParamValid = false;

	return result;
}

int32_t SynthesisEngine::ProcessDownScale(void* src) {
	int32_t result = NO_ERROR;
	mOverRangeState = NO_OVERRANGE;

	uint8_t *srcY = nullptr, *srcUVnv = nullptr, *srcUI420 = nullptr, *srcVI420 = nullptr;
	uint8_t *dstY = nullptr, *dstUVnv = nullptr, *dstUI420 = nullptr, *dstVI420 = nullptr;

	int32_t alignedSrcW = getAlignedStride(mParam.inputFrontInfo.width,
		mParam.inputFrontInfo.stride);
	int32_t alignedSrcH = getAlignedStride(mParam.inputFrontInfo.height,
		mParam.inputFrontInfo.scanline);
	int32_t alignedDstW = getAlignedStride(mParam.frontScaledInfo.width,
		mParam.frontScaledInfo.stride);
	int32_t alignedDstH = getAlignedStride(mParam.frontScaledInfo.height,
		mParam.frontScaledInfo.scanline);

	if (src == nullptr) {
		result = EMPTY_INPUT;
	}
	if (src == nullptr) {
		result = EMPTY_INPUT;
	}
	if (SUCCESS(result)) {
		result = CheckParams();
	}

	if (SUCCESS(result)) {
		if (mParam.inputFrontInfo.width == mParam.frontScaledInfo.width &&
			mParam.inputFrontInfo.height == mParam.frontScaledInfo.height) {
			mScaled = true;
			return NO_ERROR;
		}
	}

	if (SUCCESS(result)) {
		memcpy(mScaleBuf, src, mParam.inputFrontInfo.bufSize);
	}

	if (SUCCESS(result)) {
		srcY = mScaleBuf;
		srcUVnv = srcY + alignedSrcW * alignedSrcH;
		srcUI420 = srcY + alignedSrcW * mParam.inputFrontInfo.height;
		srcVI420 = srcUI420 + alignedSrcW * mParam.inputFrontInfo.height / 4;
		dstY = static_cast<uint8_t*>(src);
		dstUVnv = dstY + alignedSrcW * alignedSrcH;
		dstUI420 = dstY + alignedSrcW * mParam.inputFrontInfo.height;
		dstVI420 = dstUI420 + alignedSrcW * mParam.inputFrontInfo.height / 4;
	}

	if (SUCCESS(result)) {
		result = libyuv::NV12ToI420(srcY, alignedSrcW, srcUVnv, alignedSrcW,
			dstY, alignedSrcW, dstUI420, alignedSrcW / 2, dstVI420, alignedSrcW / 2,
			mParam.inputFrontInfo.width, mParam.inputFrontInfo.height);
	}

	if (SUCCESS(result)) {
		srcY = mScaleBuf;
		srcUI420 = srcY + alignedDstW * mParam.frontScaledInfo.height;
		srcVI420 = srcUI420 + alignedDstW * mParam.frontScaledInfo.height / 4;
	}

	if (SUCCESS(result)) {
		result = libyuv::I420Scale(
			dstY, alignedSrcW, dstUI420, alignedSrcW / 2, dstVI420, alignedSrcW / 2,
			mParam.inputFrontInfo.width, mParam.inputFrontInfo.height,
			srcY, alignedDstW, srcUI420, alignedDstW / 2, srcVI420, alignedDstW / 2,
			mParam.frontScaledInfo.width, mParam.frontScaledInfo.height,
			libyuv::kFilterBilinear);
	}

	if (SUCCESS(result)) {
		dstY = static_cast<uint8_t*>(src);
		dstUVnv = dstY + alignedDstW * alignedDstH;
	}

	if (SUCCESS(result)) {
		result = libyuv::I420ToNV12(srcY, alignedDstW,
			srcUI420, alignedDstW / 2, srcVI420, alignedDstW / 2,
			dstY, alignedDstW, dstUVnv, alignedDstW,
			mParam.frontScaledInfo.width, mParam.frontScaledInfo.height);
	}

	if (SUCCESS(result)) {
		mScaled = true;
	}

	return result;
}

int32_t SynthesisEngine::ProcessDownScale(void* dataY, void* dataUV) {
	int32_t result = NO_ERROR;
	mOverRangeState = NO_OVERRANGE;

	uint8_t *srcY = nullptr, *srcUVnv = nullptr, *srcUI420 = nullptr, *srcVI420 = nullptr;
	uint8_t *dstY = nullptr, *dstUVnv = nullptr, *dstUI420 = nullptr, *dstVI420 = nullptr;

	int32_t alignedSrcW = getAlignedStride(mParam.inputFrontInfo.width,
		mParam.inputFrontInfo.stride);
	int32_t alignedSrcH = getAlignedStride(mParam.inputFrontInfo.height,
		mParam.inputFrontInfo.scanline);
	int32_t alignedDstW = getAlignedStride(mParam.frontScaledInfo.width,
		mParam.frontScaledInfo.stride);
	int32_t alignedDstH = getAlignedStride(mParam.frontScaledInfo.height,
		mParam.frontScaledInfo.scanline);

	if (dataY == nullptr || dataUV == nullptr) {
		result = EMPTY_INPUT;
	}

	if (SUCCESS(result)) {
		result = CheckParams();
	}

	if (SUCCESS(result)) {
		if (mParam.inputFrontInfo.width == mParam.frontScaledInfo.width &&
			mParam.inputFrontInfo.height == mParam.frontScaledInfo.height) {
			mScaled = true;
			return NO_ERROR;
		}
	}

	if (SUCCESS(result)) {
		memcpy(mScaleBuf, dataY, alignedSrcW * alignedSrcH);
		memcpy(mScaleBuf + alignedSrcW * alignedSrcH, dataUV, alignedSrcW * alignedSrcH / 2);
	}

	if (SUCCESS(result)) {
		srcY = mScaleBuf;
		srcUVnv = srcY + alignedSrcW * alignedSrcH;
		srcUI420 = srcY + alignedSrcW * mParam.inputFrontInfo.height;
		srcVI420 = srcUI420 + alignedSrcW * mParam.inputFrontInfo.height / 4;
		dstY = static_cast<uint8_t*>(dataY);
		dstUVnv = static_cast<uint8_t*>(dataUV);
		dstUI420 = dstUVnv;
		dstVI420 = dstUI420 + alignedSrcW * mParam.inputFrontInfo.height / 4;
	}

	if (SUCCESS(result)) {
		result = libyuv::NV12ToI420(srcY, alignedSrcW, srcUVnv, alignedSrcW,
			dstY, alignedSrcW, dstUI420, alignedSrcW / 2, dstVI420, alignedSrcW / 2,
			mParam.inputFrontInfo.width, mParam.inputFrontInfo.height);
	}

	if (SUCCESS(result)) {
		srcY = mScaleBuf;
		srcUI420 = srcY + alignedDstW * mParam.frontScaledInfo.height;
		srcVI420 = srcUI420 + alignedDstW * mParam.frontScaledInfo.height / 4;
	}

	if (SUCCESS(result)) {
		result = libyuv::I420Scale(
			dstY, alignedSrcW, dstUI420, alignedSrcW / 2, dstVI420, alignedSrcW / 2,
			mParam.inputFrontInfo.width, mParam.inputFrontInfo.height,
			srcY, alignedDstW, srcUI420, alignedDstW / 2, srcVI420, alignedDstW / 2,
			mParam.frontScaledInfo.width, mParam.frontScaledInfo.height, 
			libyuv::kFilterBilinear);
	}

	if (SUCCESS(result)) {
		dstY = static_cast<uint8_t*>(dataY);
		dstUVnv = static_cast<uint8_t*>(dataUV);
	}

	if (SUCCESS(result)) {
		result = libyuv::I420ToNV12(srcY, alignedDstW, 
			srcUI420, alignedDstW / 2, srcVI420, alignedDstW / 2,
			dstY, alignedDstW, dstUVnv, alignedDstW,
			mParam.frontScaledInfo.width, mParam.frontScaledInfo.height);
	}

	if (SUCCESS(result)) {
		mScaled = true;
	}

	return result;
}

int32_t SynthesisEngine::ProcessSynthesis(void* frontData, void* backData)
{
	int32_t result = NO_ERROR;
	mOverRangeState = NO_OVERRANGE;
	
	uint8_t *frontY, *frontUV, *backY, *backUV;
	
	if (frontData == nullptr || backData == nullptr) {
		result = EMPTY_INPUT;
	}
	else if (!mScaled){
		result = ORDER_ERROR;
	}

	if (SUCCESS(result)) {
		result = CheckParams();
		if (SUCCESS(result) && mOverRangeState!= NO_OVERRANGE) {
			result = FixTargetPoint();
		}
	}

	if (SUCCESS(result)) {
		int32_t alignedFW = getAlignedStride(mParam.frontScaledInfo.width,
			mParam.inputFrontInfo.stride);
		int32_t alignedFH = getAlignedStride(mParam.frontScaledInfo.height,
			mParam.inputFrontInfo.scanline);
		int32_t alignedBW = getAlignedStride(mParam.inputBackInfo.width,
			mParam.inputBackInfo.stride);
		int32_t alignedBH = getAlignedStride(mParam.inputBackInfo.height,
			mParam.inputBackInfo.scanline);
		frontY = static_cast<uint8_t*>(frontData);
		frontUV = frontY + alignedFW * alignedFH;
		backY = static_cast<uint8_t*>(backData);
		backUV = backY + alignedBW * alignedBH;
		int32_t x = mParam.targetPoint.x;
		int32_t y = mParam.targetPoint.y;

		// need to think more about this segment.  this will lead to distortion for 1 pixel.
		int32_t Yoffset = y * mParam.inputBackInfo.width + x;
		int32_t UVoffset;
		if (mParam.targetPoint.onOddRow && mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x - 1;
		}
		else if (mParam.targetPoint.onOddRow && !mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x;
		}
		else if (!mParam.targetPoint.onOddRow && mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x - 1;
		}
		else {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x;
		}

		for (int32_t row = 0; row < mParam.frontScaledInfo.height; row++) {
			switch (mMirrorFlipState) {
			case NEED_X_MIRROR:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width; col++) {
					backY[(row + y) * alignedBW + x + col] =
						frontY[(row + 1) * alignedFW - 1 - col];
					if (row % 2 == 0 && col % 2 ==0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[(row + y) / 2 * alignedBW + x + col - 1] =
								frontUV[(row / 2 + 1) * alignedFW - 2 - col];
							backUV[(row + y) / 2 * alignedBW + x + col] =
								frontUV[(row / 2 + 1) * alignedFW - 1 - col];
						}
						else {
							backUV[(row + y) / 2 * alignedBW + x + col] =
								frontUV[(row / 2 + 1) * alignedFW - 2 - col];
							backUV[(row + y) / 2 * alignedBW + x + col + 1] =
								frontUV[(row / 2 + 1) * alignedFW - 1 - col];
						}
					}
				}
				break;
			case NEED_Y_FLIP:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width; col++) {
					backY[(y + mParam.frontScaledInfo.height - row - 1) * alignedBW + x + col] =
						frontY[row * alignedFW + col];
					if (row % 2 == 0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[((y + mParam.frontScaledInfo.height - row) / 2 - 1) * alignedBW + x + col - 1] =
								frontUV[row / 2 * alignedFW + col];
						}
						else {
							backUV[((y + mParam.frontScaledInfo.height - row) / 2 - 1) * alignedBW + x + col] =
								frontUV[row / 2 * alignedFW + col];
						}
					}
				}
				break;
			case NEED_BOTH:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width ; col++) {
					backY[(y + row) * alignedBW + x + col] =
						frontY[(mParam.frontScaledInfo.height - row) * alignedFW - col - 1];
					if (row % 2 == 0 && col % 2 == 0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[(y + row) / 2 * alignedBW + x + col - 1] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 2];
							backUV[(y + row) / 2 * alignedBW + x + col] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 1];
						}
						else {
							backUV[(y + row) / 2 * alignedBW + x + col] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 2];
							backUV[(y + row) / 2 * alignedBW + x + col + 1] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 1];
						}		
					}
				}
				break;
			case NEEDNOT:
			default:
				memcpy((void*)(backY + Yoffset),
					(void*)(frontY + row * alignedFW),
					mParam.frontScaledInfo.width);

				if (row % 2 == 0) {
					memcpy((void*)(backUV + UVoffset),
						(void*)(frontUV + row / 2 * alignedFW),
						mParam.frontScaledInfo.width);

					UVoffset += mParam.inputBackInfo.width;
				}
				Yoffset += mParam.inputBackInfo.width;
			}
		}
	}

	return result;
}

int32_t SynthesisEngine::ProcessSynthesis(
	void* frontDataY, void* frontDataUV, 
	void* backDataY, void* backDataUV)
{
	int32_t result = NO_ERROR;
	mOverRangeState = NO_OVERRANGE;

	uint8_t *frontY, *frontUV, *backY, *backUV;

	if (frontDataY == nullptr || frontDataUV == nullptr ||
		backDataY == nullptr || backDataUV == nullptr) {
		result = EMPTY_INPUT;
	}
	else if (!mScaled) {
		result = ORDER_ERROR;
	}

	if (SUCCESS(result)) {
		result = CheckParams();
		if (SUCCESS(result) && mOverRangeState != NO_OVERRANGE) {
			result = FixTargetPoint();
		}
	}

	if (SUCCESS(result)) {
		int32_t alignedFW = getAlignedStride(mParam.frontScaledInfo.width,
			mParam.inputFrontInfo.stride);
		int32_t alignedFH = getAlignedStride(mParam.frontScaledInfo.height,
			mParam.inputFrontInfo.scanline);
		int32_t alignedBW = getAlignedStride(mParam.inputBackInfo.width,
			mParam.inputBackInfo.stride);
		int32_t alignedBH = getAlignedStride(mParam.inputBackInfo.height,
			mParam.inputBackInfo.scanline);
		frontY = static_cast<uint8_t*>(frontDataY);
		frontUV = static_cast<uint8_t*>(frontDataUV);
		backY = static_cast<uint8_t*>(backDataY);
		backUV = static_cast<uint8_t*>(backDataUV);
		int32_t x = mParam.targetPoint.x;
		int32_t y = mParam.targetPoint.y;

		// need to think more about this segment.  this will lead to distortion for 1 pixel.
		int32_t Yoffset = y * mParam.inputBackInfo.width + x;
		int32_t UVoffset;
		if (mParam.targetPoint.onOddRow && mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x - 1;
		}
		else if (mParam.targetPoint.onOddRow && !mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x;
		}
		else if (!mParam.targetPoint.onOddRow && mParam.targetPoint.onOddCol) {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x - 1;
		}
		else {
			UVoffset = y / 2 * mParam.inputBackInfo.width + x;
		}

		for (int32_t row = 0; row < mParam.frontScaledInfo.height; row++) {
			switch (mMirrorFlipState) {
			case NEED_X_MIRROR:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width; col++) {
					backY[(row + y) * alignedBW + x + col] =
						frontY[(row + 1) * alignedFW - 1 - col];
					if (row % 2 == 0 && col % 2 == 0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[(row + y) / 2 * alignedBW + x + col - 1] =
								frontUV[(row / 2 + 1) * alignedFW - 2 - col];
							backUV[(row + y) / 2 * alignedBW + x + col] =
								frontUV[(row / 2 + 1) * alignedFW - 1 - col];
						}
						else {
							backUV[(row + y) / 2 * alignedBW + x + col] =
								frontUV[(row / 2 + 1) * alignedFW - 2 - col];
							backUV[(row + y) / 2 * alignedBW + x + col + 1] =
								frontUV[(row / 2 + 1) * alignedFW - 1 - col];
						}
					}
				}
				break;
			case NEED_Y_FLIP:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width; col++) {
					backY[(y + mParam.frontScaledInfo.height - row - 1) * alignedBW + x + col] =
						frontY[row * alignedFW + col];
					if (row % 2 == 0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[((y + mParam.frontScaledInfo.height - row) / 2 - 1) * alignedBW + x + col - 1] =
								frontUV[row / 2 * alignedFW + col];
						}
						else {
							backUV[((y + mParam.frontScaledInfo.height - row) / 2 - 1) * alignedBW + x + col] =
								frontUV[row / 2 * alignedFW + col];
						}
					}
				}
				break;
			case NEED_BOTH:
				for (int32_t col = 0; col < mParam.frontScaledInfo.width; col++) {
					backY[(y + row) * alignedBW + x + col] =
						frontY[(mParam.frontScaledInfo.height - row) * alignedFW - col - 1];
					if (row % 2 == 0 && col % 2 == 0) {
						if (mParam.targetPoint.onOddCol) {
							backUV[(y + row) / 2 * alignedBW + x + col - 1] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 2];
							backUV[(y + row) / 2 * alignedBW + x + col] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 1];
						}
						else {
							backUV[(y + row) / 2 * alignedBW + x + col] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 2];
							backUV[(y + row) / 2 * alignedBW + x + col + 1] =
								frontUV[(mParam.frontScaledInfo.height - row - 1) / 2 * alignedFW + mParam.frontScaledInfo.width - col - 1];
						}
					}
				}
				break;
			case NEEDNOT:
			default:
				memcpy((void*)(backY + Yoffset),
					(void*)(frontY + row * alignedFW),
					mParam.frontScaledInfo.width);

				if (row % 2 == 0) {
					memcpy((void*)(backUV + UVoffset),
						(void*)(frontUV + row / 2 * alignedFW),
						mParam.frontScaledInfo.width);

					UVoffset += mParam.inputBackInfo.width;
				}
				Yoffset += mParam.inputBackInfo.width;
			}
		}
	}

	return result;
}

int32_t SynthesisEngine::CheckParams() {
	if (!mInited) {
		return NOT_INITED;
	}

	int32_t result = NO_ERROR;

	//we only support down scale
	if (mParam.frontScaledInfo.width > mParam.inputFrontInfo.width ||
		mParam.frontScaledInfo.height > mParam.inputFrontInfo.height) {
		return INVALID_PARAM;
	}

	if (mParam.frontScaledInfo.width > mParam.inputBackInfo.width ||
		mParam.frontScaledInfo.height > mParam.inputBackInfo.height) {
		return INVALID_PARAM;
	}

	if (mParam.inputFrontInfo.format >= DCS_NOT_SUPPORT ||
		mParam.frontScaledInfo.format >= DCS_NOT_SUPPORT ||
		mParam.inputBackInfo.format >= DCS_NOT_SUPPORT) {
		return INVALID_PARAM;
	}

	//if tragetOpint is outside image, it can be dealed as OVER_RANGE
	/*if (mParam.targetPoint.x > mParam.inputBackInfo.width ||
		mParam.targetPoint.y > mParam.inputBackInfo.height) {
		return INVALID_PARAM;
	}*/

	if (mParam.targetPoint.x % 2 == 1) {
		mParam.targetPoint.onOddCol = true;
	}
	else {
		mParam.targetPoint.onOddCol = false;
	}

	if (mParam.targetPoint.y % 2 == 1) {
		mParam.targetPoint.onOddRow = true;
	}
	else {
		mParam.targetPoint.onOddRow = false;
	}

	int32_t targetW = (mParam.targetPoint.onOddCol) ?
		mParam.targetPoint.x + mParam.frontScaledInfo.width + 1:
		mParam.targetPoint.x + mParam.frontScaledInfo.width;
	int32_t targetH = (mParam.targetPoint.onOddRow) ?
		mParam.targetPoint.y + mParam.frontScaledInfo.height + 1:
		mParam.targetPoint.y + mParam.frontScaledInfo.height;

	if (targetW > mParam.inputBackInfo.width &&
		targetH > mParam.inputBackInfo.height) {
		mOverRangeState = XY_OVERRANGE;
	}
	else if (targetW > mParam.inputBackInfo.width) {
		mOverRangeState = X_OVERRANGE;
	}
	else if (targetH > mParam.inputBackInfo.height) {
		mOverRangeState = Y_OVERRANGE;
	}

	return result;
}

int32_t SynthesisEngine::FixTargetPoint() 
{
	int32_t result = NO_ERROR;

	BEGIN_POINT fixedPoint;
	//Color convertion problem will be dealed in ProcessSynthesis().
	if (mOverRangeState == XY_OVERRANGE) {
		fixedPoint.x = mParam.inputBackInfo.width - mParam.frontScaledInfo.width - 1;
		fixedPoint.y = mParam.inputBackInfo.height - mParam.frontScaledInfo.height - 1;
	}
	else if (mOverRangeState == X_OVERRANGE) {
		fixedPoint.x = mParam.inputBackInfo.width - mParam.frontScaledInfo.width - 1;
		fixedPoint.y = mParam.targetPoint.y;
	}
	else if (mOverRangeState == Y_OVERRANGE) {
		fixedPoint.x = mParam.targetPoint.x;
		fixedPoint.y = mParam.inputBackInfo.height - mParam.frontScaledInfo.height - 1;
	}
	result = UpdateTargetPoint(fixedPoint);

	return result;
};

int32_t SynthesisEngine::SetParams(DUAL_CAM_SYNTHESIS_PARAM param)
{
	int32_t result = NO_ERROR;

	memcpy(&mParam, &param, sizeof(DUAL_CAM_SYNTHESIS_PARAM));
	mParamValid = true;

	return result;
}

int32_t SynthesisEngine::SetInitParams(uint32_t forntW, uint32_t frontH,
	uint32_t scaledW, uint32_t scaledH,
	uint32_t backW, uint32_t backH,
	uint32_t stride, uint32_t scanline,
	uint32_t targetX, uint32_t targetY,
	uint32_t format)
{
	int32_t result = NO_ERROR;

	int32_t size;
	bool onOddRow = (targetX % 2 == 1) ? true : false;
	bool onOddCol = (targetY % 2 == 1) ? true : false;

	DUAL_CAM_SYNTHESIS_PARAM defaultParam;
	defaultParam.inputFrontInfo.width = forntW;
	defaultParam.inputFrontInfo.height = frontH;
	defaultParam.inputFrontInfo.stride = stride;
	defaultParam.inputFrontInfo.scanline = scanline;
	size = getAlignedStride(defaultParam.inputFrontInfo.width, defaultParam.inputFrontInfo.stride) *
		getAlignedStride(defaultParam.inputFrontInfo.height, defaultParam.inputFrontInfo.scanline) * 3 / 2;
	defaultParam.inputFrontInfo.bufSize = size;
	defaultParam.inputFrontInfo.format = format;

	defaultParam.frontScaledInfo.width = scaledW;
	defaultParam.frontScaledInfo.height = scaledH;
	defaultParam.frontScaledInfo.stride = stride;
	defaultParam.frontScaledInfo.scanline = scanline;
	defaultParam.frontScaledInfo.bufSize = defaultParam.inputFrontInfo.bufSize;
	defaultParam.frontScaledInfo.format = format;

	defaultParam.inputBackInfo.width = backW;
	defaultParam.inputBackInfo.height = backH;
	defaultParam.inputBackInfo.stride = stride;
	defaultParam.inputBackInfo.scanline = scanline;
	size = getAlignedStride(defaultParam.inputBackInfo.width, defaultParam.inputBackInfo.stride) *
		getAlignedStride(defaultParam.inputBackInfo.height, defaultParam.inputBackInfo.scanline) * 3 / 2;
	defaultParam.inputBackInfo.bufSize = size;
	defaultParam.inputBackInfo.format = format;

	defaultParam.targetPoint.x = targetX;
	defaultParam.targetPoint.y = targetY;
	defaultParam.targetPoint.onOddRow = onOddRow;
	defaultParam.targetPoint.onOddCol = onOddCol;

	result = SetParams(defaultParam);

	return result;
}

int32_t SynthesisEngine::UpdateTargetPoint(BEGIN_POINT point) {
	mParam.targetPoint.x = point.x;
	mParam.targetPoint.y = point.y;
	if (mParam.targetPoint.x % 2 == 1) {
		mParam.targetPoint.onOddCol = true;
	}
	else {
		mParam.targetPoint.onOddCol = false;
	}

	if (mParam.targetPoint.y % 2 == 1) {
		mParam.targetPoint.onOddRow = true;
	}
	else {
		mParam.targetPoint.onOddRow = false;
	}
	return NO_ERROR;
}

