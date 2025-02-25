// RandomDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Random.h"
#include "RandomDlg.h"
#include ".\randomdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRandomDlg 对话框



CRandomDlg::CRandomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRandomDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRandomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RESULT, m_edtResult);
}

BEGIN_MESSAGE_MAP(CRandomDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GEN_HEX, OnBnClickedGenHex)
	ON_BN_CLICKED(IDC_GEN_REG_CODE, OnBnClickedGenRegCode)
	ON_BN_CLICKED(IDC_GEN_STR, OnBnClickedGenStr)
END_MESSAGE_MAP()


// CRandomDlg 消息处理程序

BOOL CRandomDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CRandomDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRandomDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CRandomDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRandomDlg::OnBnClickedGenHex()
{
	srand(time(0));

	CString sRes;
	CString s;

	for(ULONG i = 0 ;i<16;i++)
	{
		UCHAR v = rand()%0x100;
		s.Format("0x%X",v);
		sRes += s;
		if(i!=16-1)
			sRes += ",";
	}

	m_edtResult.SetWindowText(sRes);
}
void CRandomDlg::OnBnClickedGenRegCode()
{
	m_edtResult.SetWindowText(_T(""));

	enum{NKey = 100};

	static CHAR charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789";//没有0，以免和O混淆
	static ULONG nChar = sizeof(charSet)-1;

	CString sResult;
	::srand(::time(NULL));
	for(ULONG iKey = 0;iKey<NKey;iKey++)
	{
		CString s;
		for(ULONG i = 0;i<5;i++)
		{						
			for(ULONG j = 0;j<5;j++)
			{
				s += charSet[::rand()%nChar];
			}
			if(i!=5-1)
				s += '-';
		}
		sResult += s;
		sResult += _T("\r\n");
	}
	m_edtResult.SetWindowText(sResult);
}
void CRandomDlg::OnBnClickedGenStr()
{
	static CHAR charSet[] = "abcdefghijklmnopqrstuvwxyz";
	static ULONG nChar = sizeof(charSet)-1;

	srand(time(0));

	CString sRes;
	CString s;

	ULONG len = rand()%4 + 2;

	for(ULONG i = 0 ;i<len;i++)
	{
		s += charSet[::rand()%nChar];
	}

	m_edtResult.SetWindowText(s);
}