#include "stdafx.h"
#include "IppSegment.h"
#include "IppImage/IppEnhance.h"
#include "IppImage/IppFourier.h"

void IppBinarization(IppByteImage& imgSrc, IppByteImage& imgDst, int threshold)
{
	imgDst.CreateImage(imgSrc.GetWidth(), imgSrc.GetHeight());

	int size = imgSrc.GetSize();
	BYTE* pSrc = imgSrc.GetPixels();
	BYTE* pDst = imgDst.GetPixels();

	for (int i = 0; i < size; i++)
	{
		pDst[i] = (pSrc[i] <= threshold) ? 0 : 255;
	}
}

int IppBinarizationIterative(IppByteImage& imgSrc)
{
	float hist[256] = { 0, };
	IppHistogram(imgSrc, hist);

	int i, T, Told;

	float sum = 0.f;
	for (i = 0; i < 256; i++)
	{
		sum += (i * hist[i]);
	}

	T = static_cast<int>(sum + .5f);

	float a1, b1, u1, a2, b2, u2;

	do {
		Told = T;

		a1 = b1 = u1 = 0.f;
		for (i = 0; i <= Told; i++)
		{
			a1 += (i * hist[i]);
			b1 += hist[i];
		}

		if (b1 != 0.f)
			u1 = a1 / b1;

		a2 = b2 = u2 = 0.f;

		for (i = Told + 1; i < 256; i++)
		{
			a2 += (i*hist[i]);
			b2 += hist[i];
		}

		if (b2 != 0.f)
			u2 = a2 / b2;

		T = static_cast<int>((u1 + u2) / 2 + 0.5f);
	} while (T != Told);

	return T;
}

int IppLabeling(IppByteImage& imgSrc, IppIntImage& imgDst, vector<IppLabelnfo>& labels)
{
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();

	BYTE** pSrc = imgSrc.GetPixels2D();

	// dynamic memory, created table

	IppIntImage imgMap(w, h);
	int ** pMap = imgMap.GetPixels2D();

	const int MAX_LABEL = 100000;
	int eq_tbl[MAX_LABEL][2] = { {0, } };

	// first scan

	register int i, j;
	int label = 0, max1, min1, min_eq, max_eq;

	for (j = 1; j < h; j++)
	{
		for (i = 1; i < w; i++)
		{
			if (pSrc[j][i] == 255)
			{
				if ((pMap[j - 1][i] != 0) && (pMap[j][i - 1] != 0))
				{
					if (pMap[j - 1][i] == pMap[j][i - 1])
						pMap[j][i] = pMap[j - 1][i];
					else
					{
						max1 = __max(pMap[j - 1][i], pMap[j][i - 1]);
						min1 = __min(pMap[j - 1][i], pMap[j][i - 1]);

						pMap[j][i] = min1;

						min_eq = __min(eq_tbl[max1][1], eq_tbl[min1][1]);
						max_eq = __max(eq_tbl[max1][1], eq_tbl[min1][1]);

						eq_tbl[eq_tbl[max_eq][1]][1] = min_eq;
					}
				}
				else if (pMap[j - 1][i] != 0)
					pMap[j][i] = pMap[j - 1][i];
				else if (pMap[j][i - 1] != 0)
					pMap[j][i] = pMap[j][i - 1];
				else
				{
					label++;
					pMap[j][i] = label;
					eq_tbl[label][0] = label;
					eq_tbl[label][1] = label;
				}
			}
		}
	}

	// table
	int temp;
	for (i = 1; i <= label; i++)
	{
		temp = eq_tbl[i][1];
		if (temp != eq_tbl[i][0])
			eq_tbl[i][1] = eq_tbl[temp][1];
	}

	// gain from 1

	int* hash = new int[label + 1];
	memset(hash, 0, sizeof(int)*(label + 1));

	for (i = 1; i <= label; i++)
	{
		hash[eq_tbl[i][1]] = eq_tbl[i][1];
	}

	int label_cnt = 1;
	for (i = 1; i <= label; i++)
	{
		if (hash[i] != 0)
			hash[i] = label_cnt++;
	}

	for (i = 1; i <= label; i++)
		eq_tbl[i][1] = hash[eq_tbl[i][1]];

	delete[] hash;

	// second

	imgDst.CreateImage(w, h);
	int** pDst = imgDst.GetPixels2D();

	int idx;
	for (j = 1; j < h; j++)
	{
		for (i = 1; i < w; i++)
		{
			if (pMap[j][i] != 0)
			{
				idx = pMap[j][i];
				pDst[j][i] = eq_tbl[idx][1];
			}
		}
	}

	// write IppLabelInfo Information

	labels.resize(label_cnt - 1);

	IppLabelnfo* pLabel;
	for (j = 1; j < h; j++)
	{
		for (i = 1; i < w; i++)
		{
			if (pDst[j][i] != 0)
			{
				pLabel = &labels.at(pDst[j][i] - 1);
				pLabel->pixels.push_back(IppPoint(i, j));
				pLabel->cx += i;
				pLabel->cy += j;

				if (i < pLabel->minx) pLabel->minx = i;
				if (i > pLabel->maxx) pLabel->maxx = i;
				if (j < pLabel->miny) pLabel->miny = j;
				if (j > pLabel->maxy) pLabel->maxy = j;
			}
		}
	}

	for (IppLabelnfo& label : labels)

	{
		label.cx /= label.pixels.size();
		label.cy /= label.pixels.size();
	}

	return label_cnt - 1;
}

void IppContourTracing(IppByteImage& imgSrc, int sx, int sy, vector<IppPoint>& cp)
{
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();

	BYTE** pSrc = imgSrc.GetPixels2D();

	cp.clear();

	if (pSrc[sy][sx] != 255)
		return;

	int x, y, nx, ny;
	
	int d, cnt;
	int dir[8][2] = {
		{1,0}
		,{1, 1}
		,{0, 1}
		,{-1, 1}
		,{-1, 0}
		,{-1, -1}
		,{0, -1}
		,{1, -1}
	};

	x = sx;
	y = sy;
	d = cnt = 0;

	while (true)
	{
		nx = x + dir[d][0];
		ny = y + dir[d][1];

		if (nx < 0 || nx >= w || ny < 0 || ny > h || pSrc[ny][nx] == 0)
		{
			if (++d > 7) d = 0;
			cnt++;

			if (cnt >= 8)
			{
				cp.push_back(IppPoint(x, y));
				break;
			}
		}
		else
		{
			cp.push_back(IppPoint(x, y));

			x = nx;
			y = ny;

			cnt = 0;
			d = (d + 6) % 8;
		}

		if (x == sx && y == sy && d == 0)
			break;
	}
}

void IppMorphologyErosion(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	int i, j;
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();

	imgDst = imgSrc;

	BYTE** pDst = imgDst.GetPixels2D();
	BYTE** pSrc = imgSrc.GetPixels2D();

	for (j = 1; j < h - 1; j++)
	for (i = 1; i < w - 1; i++)
	{
		if (pSrc[j][i] != 0)
		{
			if (pSrc[j - 1][i] == 0 || pSrc[j - 1][i + 1] == 0 ||
				pSrc[j][i - 1] == 0 || pSrc[j][i + 1] == 0 ||
				pSrc[j + 1][i - 1] == 0 || pSrc[j + 1][i] == 0 ||
				pSrc[j + 1][i + 1] == 0 || pSrc[j - 1][i - 1] == 0)
			{
				pDst[j][i] = 0;
			}
		}
			
	}
}


void IppMorpholgyDilation(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	int i, j;
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();
	imgDst = imgSrc;

	BYTE** pDst = imgDst.GetPixels2D();
	BYTE** pSrc = imgSrc.GetPixels2D();

	for (j = 1; j < h - 1; j++)
	{
		for (i = 1; i < w - 1; i++)
		{
			if (pSrc[j][i] == 0)
			{
				if (pSrc[j - 1][i] != 0 || pSrc[j - 1][i + 1] != 0 ||
					pSrc[j][i - 1] != 0 || pSrc[j][i + 1] != 0 ||
					pSrc[j + 1][i - 1] != 0 || pSrc[j + 1][i] != 0 ||
					pSrc[j + 1][i + 1] != 0 || pSrc[j - 1][i - 1] != 0)
				{
					pDst[j][i] = 255;
				}
			}
			
		}
	}
}

void IppMorphologyOpening(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	IppByteImage imgTmp;
	IppMorphologyErosion(imgSrc, imgTmp);
	IppMorpholgyDilation(imgTmp, imgDst);
}
void IppMorphologyClosing(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	IppByteImage imgTmp;
	IppMorpholgyDilation(imgSrc, imgTmp);
	IppMorphologyErosion(imgTmp, imgDst);
}

void IppMorphologyGrayErosion(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	int i, j, m, n, x, y, pmin;
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();

	imgDst = imgSrc;

	BYTE** pDst = imgDst.GetPixels2D();
	BYTE** pSrc = imgSrc.GetPixels2D();

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			pmin = 255;

			for (n = -1; n <= 1; n++)
			{
				for (m = -1; m <= 1; m++)
				{
					x = i + m;
					y = j + n;

					if (x >= 0 && x < w && y >= 0 && y < h)
					{
						if (pSrc[y][x] < pmin)
							pmin = pSrc[y][x];
					}
				}
			}

			pDst[j][i] = pmin;
		}
	}
}

void IppMorpholgyGaryDilation(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	int i, j, m, n, x, y, pmax;
	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();

	imgDst = imgSrc;

	BYTE** pDst = imgDst.GetPixels2D();
	BYTE** pSrc = imgSrc.GetPixels2D();

	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			pmax = 0;

			for (n = -1; n <= 1; n++)
			{
				for (m = -1; m <= 1; m++)
				{
					x = i + m;
					y = j + n;

					if (x >= 0 && x < w && y >= 0 && y < h)
					{
						if (pSrc[y][x] > pmax)
							pmax = pSrc[y][x];
					}
				}
			}

			pDst[j][i] = pmax;
		}
	}
}

void IppMorphologyGrayOpening(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	IppByteImage imgTmp;
	IppMorphologyGrayErosion(imgSrc, imgTmp);
	IppMorpholgyGaryDilation(imgTmp, imgDst);
}

void IppMorpholgyGaryClosing(IppByteImage& imgSrc, IppByteImage& imgDst)
{
	IppByteImage imgTmp;
	IppMorpholgyGaryDilation(imgSrc, imgTmp);
	IppMorphologyGrayErosion(imgTmp, imgDst);
}

void IppFourierDescriptor(IppByteImage& img, int sx, int sy, int percent, std::vector<IppPoint>& cp)
{
	// get ContourTracing
	IppContourTracing(img, sx, sy, cp);

	// get technicer in Fouri
	int num = cp.size();
	double* x = new double[num];
	double* y = new double[num];

	for (int i = 0; i < num; i++)
	{
		x[i] = static_cast<double>(cp[i].x);
		y[i] = static_cast<double>(cp[i].y);
	}

	DFT1d(x, y, num, 1);

	int p = num * percent / 100;

	for (int i = p; i < num; i++)
	{
		x[i] = 0.;
		y[i] = 0.;
	}

	DFT1d(x, y, num, -1);

	int w = img.GetWidth();
	int h = img.GetHeight();

	cp.clear();
	IppPoint pt;

	for (int i = 0; i < num; i++)
	{
		pt.x = limit(static_cast<int>(x[i] + 0.5), 0, w - 1);
		pt.y = limit(static_cast<int>(y[i] + 0.5), 0, h - 1);
		cp.push_back(pt);
	}

	delete[] x;
	delete[] y;
}

IppPoint IppTemplateMatching(IppByteImage& imgSrc, IppByteImage& imgTmp1, IppIntImage& imgMap)
{
	int sw = imgSrc.GetWidth();
	int sh = imgSrc.GetHeight();
	int tw = imgTmp1.GetWidth();
	int th = imgTmp1.GetHeight();
	int tw2 = tw / 2;
	int th2 = th / 2;

	imgMap.CreateImage(sw, sh);

	BYTE** pSrc = imgSrc.GetPixels2D();
	BYTE** pTmpl = imgTmp1.GetPixels2D();
	int** pMap = imgMap.GetPixels2D();

	IppPoint dp;

	int i, j, x, y;
	int min_value = 99999;
	int diff, sum_of_diff;
	for (j = th2; j <= sh - th2; j++)
	{
		for (i = tw2; i <= sw - tw2; i++)
		{
			sum_of_diff = 0;
			for (y = 0; y < th; y++)
			{
				for (x = 0; x < tw; x++)
				{
					diff = pSrc[j - th2 + y][i - tw2 + x] - pTmpl[y][x];
					sum_of_diff += (diff * diff);
				}
			}

			pMap[j][i] = sum_of_diff / (tw * th);

			if (pMap[j][i] < min_value)
			{
				min_value = pMap[j][i];
				dp.x = i;
				dp.y = j;
			}
		}
	}

	return dp;
}