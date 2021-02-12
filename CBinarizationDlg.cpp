// CBinarizationDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "ImageTool.h"
#include "CBinarizationDlg.h"
#include "afxdialogex.h"


#include "IppImage/IppImage.h"
#include "IppImage/IppConvert.h"
#include "IppImage/IppGeometry.h"
#include "IppSegment.h"

// CBinarizationDlg 대화 상자

IMPLEMENT_DYNAMIC(CBinarizationDlg, CDialogEx)

CBinarizationDlg::CBinarizationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BINARIZATION, pParent)
	, m_nThreshold(0)
{

}

CBinarizationDlg::~CBinarizationDlg()
{
}

void CBinarizationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_threshold_SLIDER, m_sliderThreshold);
	DDX_Text(pDX, IDC_THRESHOLD_EDIT, m_nThreshold);
	DDV_MinMaxInt(pDX, m_nThreshold, 0, 255);
}


BEGIN_MESSAGE_MAP(CBinarizationDlg, CDialogEx)
	ON_EN_CHANGE(IDC_THRESHOLD_EDIT, &CBinarizationDlg::OnEnChangeThresholdEdit)
	ON_WM_HSCROLL()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CBinarizationDlg 메시지 처리기

BOOL CBinarizationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	// Init Slider
	m_sliderThreshold.SetRange(0, 255);
	m_sliderThreshold.SetTicFreq(32);
	m_sliderThreshold.SetPageSize(32);
	m_sliderThreshold.SetPos(m_nThreshold);

	CRect rct;
	CWnd* pImageWnd = GetDlgItem(IDC_STATIC);
	pImageWnd->GetClientRect(rct);

	IppByteImage imgSrc, imgDst;
	IppDibToImage(m_DibSrc, imgSrc);
	IppResizeNearest(imgSrc, imgDst, rct.Width(), rct.Height());
	IppImageToDib(imgDst, m_DibSrc);

	MakePreviewImage();

	return TRUE;  // return TRUE unless you set the focus to a control
					// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CBinarizationDlg::OnEnChangeThresholdEdit()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.

	UpdateData(TRUE);
	m_sliderThreshold.SetPos(m_nThreshold);

	MakePreviewImage();
	Invalidate(FALSE);
}

void CBinarizationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_sliderThreshold.GetSafeHwnd() == pScrollBar->GetSafeHwnd())
	{
		int nPos = m_sliderThreshold.GetPos();
		m_nThreshold = nPos;
		UpdateData(FALSE);

		MakePreviewImage();
		Invalidate(FALSE);
	}


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CBinarizationDlg::SetImage(IppDib& dib)
{
	m_DibSrc = dib;

	IppByteImage imgSrc;
	IppDibToImage(m_DibSrc, imgSrc);
	m_nThreshold = IppBinarizationIterative(imgSrc);
}

void CBinarizationDlg::MakePreviewImage()
{
	IppByteImage imgSrc, imgDst;
	IppDibToImage(m_DibSrc, imgSrc);
	IppBinarization(imgSrc, imgDst, m_nThreshold); 
	IppImageToDib(imgDst, m_DibRes);
}

void CBinarizationDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.
	CPaintDC dcPreview(GetDlgItem(IDC_STATIC));
	m_DibRes.Draw(dcPreview.m_hDC, 0, 0);
}
