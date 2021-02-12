#include "stdafx.h"
#include "IppColor.h"
#include "IppImage/IppFeature.h"

const double PI = 3.14159265358979323846;

void IppInverse(IppRgbImage& img)
{
	int size = img.GetSize();
	RGBBYTE* p = img.GetPixels();

	for (int i = 0; i < size; i++)
	{
		p[i].r = 255 - p[i].r;
		p[i].g = 255 - p[i].g;
		p[i].b = 255 - p[i].b;
	}
}

void RGB_TO_HSI(double R, double G, double B, double& H, double& S, double& I)
{
	I = (R + G + B) / 3;
	
	// if it is grayscale
	if ((R == G) && (G == B))
	{
		S = 0;
		H = 0;
	}
	else
	{
		double min_rgb;
		double temp;

		min_rgb = min(min(R, G), B);
		S = 1 - (min_rgb / I);
		temp = ((R - G) + (R - B)) / (2 * sqrt((R - G) * (R - G) + (R - B) * (G - B)));

		H = acos(temp) * 180 / PI;

		if (B > G)
			H = 360 - H;

		H /= 360;
	}
}

void HSI_TO_RGB(double H, double S, double I, double& R, double& G, double& B)
{
	if (I == 0.0) // black
	{
		R = 0.0;
		G = 0.0;
		B = 0.0;
		return;
	}

	if (S == 0.0) // Garyscale
	{
		R = I;
		G = I;
		B = I;
		return;
	}

	H *= 360;

	if (H <= 120)
	{
		B = I * (1 - S);
		R = I * (1 + S * cos(H*PI / 180) / cos((60 - H) * PI / 180));
		R = limit(R, 0., 1.);
	}
	else if (H <= 240)
	{
		H -= 120;

		R = I * (1 - S);
		G = I * (1 + S * cos(H*PI / 80) / cos((60 - H) * PI / 180));
		G = limit(G, 0., 1.);
		B = 3 * I - (R + G);
	}
	else
	{

		H -= 240;

		G = I * (1 - S);
		B = I * (1 + S * cos(H*PI / 180) / cos((60 - H) * PI / 180));
		B = limit(B, 0., 1.);
		R = 3 * I - (G + B);
	}
}

void RGB_TO_YUV(BYTE R, BYTE G, BYTE B, BYTE &Y, BYTE &U, BYTE &V)
{
	Y = (BYTE)limit(0.299*R + 0.587*G + 0.114*B + 0.5);
	U = (BYTE)limit(-0.169*R - 0.331*G + 0.500*B + 128 + 0.5);
	V = (BYTE)limit(0.500*R - 0.419*G - 0.081*B + 128 + 0.5);
}

void YUV_TO_RGB(BYTE Y, BYTE U, BYTE V, BYTE &R, BYTE &G, BYTE& B)
{
	R = (BYTE)limit(Y + 1.4075*(V - 128) + 0.5);
	G = (BYTE)limit(Y - 0.3455*(U - 128) - 0.7169*(V - 128) + 0.5);
	B = (BYTE)limit(Y + 1.7790*(U - 128) + 0.5);
}

void IppColorSplitRGB(IppRgbImage& imgColor, IppByteImage &imgR, IppByteImage& imgG, IppByteImage & imgB)
{
	int w = imgColor.GetWidth();
	int h = imgColor.GetHeight();
	int size = imgColor.GetSize();

	imgR.CreateImage(w, h);
	imgG.CreateImage(w, h);
	imgB.CreateImage(w, h);

	RGBBYTE* pColor = imgColor.GetPixels();
	BYTE* pR = imgR.GetPixels();
	BYTE* pG = imgG.GetPixels();
	BYTE* pB = imgB.GetPixels();

	for (int i = 0; i < size; i++)
	{
		pR[i] = pColor[i].r;
		pG[i] = pColor[i].g;
		pB[i] = pColor[i].b;
	}
}

void IppColorSplitHSI(IppRgbImage& imgColor, IppByteImage& imgH, IppByteImage& imgS, IppByteImage& imgI)
{
	int w = imgColor.GetWidth();
	int h = imgColor.GetHeight();
	int size = imgColor.GetSize();

	imgH.CreateImage(w, h);
	imgS.CreateImage(w, h);
	imgI.CreateImage(w, h);

	RGBBYTE* pColor = imgColor.GetPixels();
	BYTE* pH = imgH.GetPixels();
	BYTE* pS = imgS.GetPixels();
	BYTE* pI = imgI.GetPixels();

	double r, g, b, hh, ss, ii;
	for (int i = 0; i < size; i++)
	{
		r = pColor[i].r / 255;
		g = pColor[i].g / 255;
		b = pColor[i].b / 255;

		RGB_TO_HSI(r, g, b, hh, ss, ii);

		pH[i] = static_cast<BYTE>(limit(hh * 255 + 0.5));
		pS[i] = static_cast<BYTE>(limit(ss * 255 + 0.5));
		pI[i] = static_cast<BYTE>(limit(ii * 255 + 0.5));
	}
}

void IppColorSplitYUV(IppRgbImage& imgColor, IppByteImage& imgY, IppByteImage& imgU, IppByteImage& imgV)
{
	int w = imgColor.GetWidth();
	int h = imgColor.GetHeight();
	int size = imgColor.GetSize();

	imgY.CreateImage(w, h);
	imgU.CreateImage(w, h);
	imgV.CreateImage(w, h);

	RGBBYTE* pColor = imgColor.GetPixels();
	BYTE* pY = imgY.GetPixels();
	BYTE* pU = imgU.GetPixels();
	BYTE* pV = imgV.GetPixels();

	for (int i = 0; i < size; i++)
	{
		RGB_TO_YUV(pColor[i].r, pColor[i].g, pColor[i].b, pY[i], pU[i], pV[i]);
	}
}
bool IppColorCombineRGB(IppByteImage &imgR, IppByteImage &imgG, IppByteImage& imgB, IppRgbImage &imgColor)
{
	int w = imgR.GetWidth();
	int h = imgR.GetHeight();
	int size = imgR.GetSize();

	if (imgG.GetWidth() != w || imgG.GetHeight() != h
		|| imgB.GetWidth() != w || imgB.GetHeight() != h)
		return FALSE;

	imgColor.CreateImage(w, h);

	BYTE* pR = imgR.GetPixels();
	BYTE* pG = imgG.GetPixels();
	BYTE* pB = imgB.GetPixels();
	RGBBYTE* pColor = imgColor.GetPixels();;

	for (int i = 0; i < size; i++)
	{
		pColor[i].r = pR[i];
		pColor[i].g = pG[i];
		pColor[i].b = pB[i];
	}

	return TRUE;
}

bool IppColorCombineHSI(IppByteImage &imgH, IppByteImage &imgS, IppByteImage& imgI, IppRgbImage &imgColor)
{
	int w = imgH.GetWidth();
	int h = imgH.GetHeight();
	int size = imgH.GetSize();

	if (imgS.GetWidth() != w || imgS.GetHeight() != h
		|| imgI.GetWidth() != w || imgI.GetHeight() != h)
		return FALSE;

	imgColor.CreateImage(w, h);

	BYTE* pH = imgH.GetPixels();
	BYTE* pS = imgS.GetPixels();
	BYTE* pI = imgI.GetPixels();
	RGBBYTE* pColor = imgColor.GetPixels();;

	double r, g, b, hh, ss, ii;
	for (int i = 0; i < size; i++)
	{
		hh = pH[i] / 255.;
		ss = pS[i] / 255.;
		ii = pI[i] / 255.;

		HSI_TO_RGB(hh, ss, ii, r, g, b);

		pColor[i].r = static_cast<BYTE>(limit(r * 255 + 0.5));
		pColor[i].g = static_cast<BYTE>(limit(g * 255 + 0.5));
		pColor[i].b = static_cast<BYTE>(limit(b * 255 + 0.5));
	}

	return TRUE;
}

bool IppColorCombineYUV(IppByteImage &imgY, IppByteImage &imgU, IppByteImage& imgV, IppRgbImage &imgColor)
{
	int w = imgY.GetWidth();
	int h = imgY.GetHeight();
	int size = imgY.GetSize();

	if (imgU.GetWidth() != w || imgU.GetHeight() != h
		|| imgV.GetWidth() != w || imgV.GetHeight() != h)
	{
		CString str;
		str.Format(_T("w : %d, h : %d, w : %d, h : %d"), w, h, imgU.GetWidth(), imgU.GetHeight());
		
		AfxMessageBox(str);
		return FALSE;
	}

	imgColor.CreateImage(w, h);

	BYTE* pY = imgY.GetPixels();
	BYTE* pU = imgU.GetPixels();
	BYTE* pV = imgV.GetPixels();
	RGBBYTE* pColor = imgColor.GetPixels();;

	double r, g, b, hh, ss, ii;
	for (int i = 0; i < size; i++)
	{
		YUV_TO_RGB(pY[i], pU[i], pV[i], pColor[i].r, pColor[i].g, pColor[i].b);
	}

	return TRUE;
}

void IppColorEdge(IppRgbImage& imgSrc, IppByteImage& imgEdge)
{
	IppByteImage imgY, imgU, imgV;
	IppColorSplitYUV(imgSrc, imgY, imgU, imgV);

	IppByteImage edgeY, edgeU, edgeV;
	IppEdgePrewitt(imgY, edgeY);
	IppEdgePrewitt(imgU, edgeU);
	IppEdgePrewitt(imgV, edgeV);

	int w = imgSrc.GetWidth();
	int h = imgSrc.GetHeight();
	int size = imgSrc.GetSize();

	imgEdge.CreateImage(w, h);
	BYTE* pEdge = imgEdge.GetPixels();

	BYTE* pY = edgeY.GetPixels();
	BYTE* pU = edgeU.GetPixels();
	BYTE* pV = edgeV.GetPixels();

	double dist;

	for (int i = 0; i < size; i++)
	{
		dist = (pY[i] * pY[i]) +
			(0.5 * pU[i]) * (0.5 * pU[i]) +
			(0.5 * pV[i]) * (0.5 * pV[i]);
		pEdge[i] = static_cast<BYTE>(limit(sqrt(dist)));
	}

}