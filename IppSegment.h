#pragma once

#include "IppImage/IppImage.h"

#include <vector>
#include "IppImage/IppImage.h"
#include "IppImage/IppFeature.h"

class IppLabelnfo
{
public:
	vector<IppPoint> pixels;
	int cx, cy;
	int minx, miny, maxx, maxy;

public:
	IppLabelnfo() : cx(0), cy(0), minx(9999), miny(9999), maxx(0), maxy(0)
	{
		pixels.clear();
	}
};


void IppBinarization(IppByteImage& imgSrc, IppByteImage& imgDst, int threshold);
int IppBinarizationIterative(IppByteImage& imgSrc);
int IppLabeling(IppByteImage& imgSrc, IppIntImage& imgDst, vector<IppLabelnfo>& labels);
void IppContourTracing(IppByteImage& imgSrc, int sx, int sy, vector<IppPoint>& cp);
void IppMorphologyErosion(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorpholgyDilation(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorphologyOpening(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorphologyClosing(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorphologyGrayErosion(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorpholgyGaryDilation(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorphologyGrayOpening(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppMorpholgyGaryClosing(IppByteImage& imgSrc, IppByteImage& imgDst);
void IppFourierDescriptor(IppByteImage& img, int sx, int sy, int percent, std::vector<IppPoint>& cp);
IppPoint IppTemplateMatching(IppByteImage& imgSrc, IppByteImage& imgTmp1, IppIntImage& imgMap);