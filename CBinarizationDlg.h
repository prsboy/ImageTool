#pragma once

#include "afxcmn.h"

#include "IppImage/IppDib.h"

// CBinarizationDlg 대화 상자

class CBinarizationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBinarizationDlg)

public:
	CBinarizationDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CBinarizationDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BINARIZATION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_sliderThreshold;
	int m_nThreshold;
	IppDib m_DibSrc;	// Input Copy of Image
	IppDib m_DibRes;	// using m_nThreshold Image to BinaraztionmDibSrc

	void SetImage(IppDib& dib);
	void MakePreviewImage();

public:
	afx_msg void OnEnChangeThresholdEdit();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
};
