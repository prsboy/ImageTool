// CColorCombineDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "ImageTool.h"
#include "CColorCombineDlg.h"
#include "afxdialogex.h"

#include "ImageToolDoc.h"

// CColorCombineDlg 대화 상자

IMPLEMENT_DYNAMIC(CColorCombineDlg, CDialogEx)

CColorCombineDlg::CColorCombineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COLOR_COMBINE, pParent)
	, m_strColorSpace(_T(""))
	, m_pDoc1(NULL)
	, m_pDoc2(NULL)
	, m_pDoc3(NULL)
{

}

CColorCombineDlg::~CColorCombineDlg()
{
}

void CColorCombineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_COLOR_SPACE, m_strColorSpace);
	DDX_Control(pDX, IDC_COMBO_IMAGE1, m_comboImage1);
	DDX_Control(pDX, IDC_COMBO_IMAGE2, m_comboImage2);
	DDX_Control(pDX, IDC_COMBO_IMAGE3, m_comboImage3);
}


BEGIN_MESSAGE_MAP(CColorCombineDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CColorCombineDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CColorCombineDlg 메시지 처리기


BOOL CColorCombineDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	int nIndex = 0;
	CString strTitle;
	CImageToolApp* pApp = (CImageToolApp*)AfxGetApp();
	CImageToolDoc* pDoc = NULL;
	POSITION pos = pApp->m_pImageDocTemplate->GetFirstDocPosition();

	while (pos != NULL)
	{
		pDoc = (CImageToolDoc*)pApp->m_pImageDocTemplate->GetNextDoc(pos);
		if (pDoc->m_Dib.GetBitCount() != 8)
			continue;

		strTitle = pDoc->GetTitle();

		m_comboImage1.InsertString(nIndex, strTitle);
		m_comboImage2.InsertString(nIndex, strTitle);
		m_comboImage3.InsertString(nIndex, strTitle);

		m_comboImage1.SetItemDataPtr(nIndex, (void*)pDoc);
		m_comboImage2.SetItemDataPtr(nIndex, (void*)pDoc);
		m_comboImage3.SetItemDataPtr(nIndex, (void*)pDoc);

		nIndex++;
	}

	if (nIndex == 0)
	{
		AfxMessageBox(_T("There is no grayscale Image"));
		SendMessage(WM_COMMAND, IDCANCEL, 0);
	}

	m_comboImage1.SetCurSel(0);
	m_comboImage2.SetCurSel(0);
	m_comboImage3.SetCurSel(0);
	if (nIndex < 1) m_comboImage2.SetCurSel(1);
	if (nIndex > 2) m_comboImage3.SetCurSel(2);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CColorCombineDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDoc1 = (CImageToolDoc*)m_comboImage1.GetItemDataPtr(m_comboImage1.GetCurSel());
	m_pDoc2 = (CImageToolDoc*)m_comboImage2.GetItemDataPtr(m_comboImage2.GetCurSel());
	m_pDoc3 = (CImageToolDoc*)m_comboImage3.GetItemDataPtr(m_comboImage3.GetCurSel());

	CDialogEx::OnOK();
}
