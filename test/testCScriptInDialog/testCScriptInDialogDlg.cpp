#include "stdafx.h"
#include "testCScriptInDialog.h"
#include "testCScriptInDialogDlg.h"
#include "afxdialogex.h"
#include "CScriptEng/cscriptBase.h"
#include "test/testCScript/testCallback.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////

class println2 : public runtime::runtimeObjectBase
{
public:
	CtestCScriptInDialogDlg *mDlg;
	
public:
	println2()
		: mDlg(NULL)
	{
	}

	virtual uint32_t GetObjectTypeId() const
	{
		return runtime::DT_UserTypeBegin;
	}

	virtual runtimeObjectBase* Add(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Div(const runtimeObjectBase *obj){ return NULL; }

	// =二元运算
	virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj) { return NULL; }

	// 处理.操作符（一元的）
	virtual runtimeObjectBase* GetMember(const char *memName) { return NULL; }

	// docall（函数调用一元运算）
	virtual runtimeObjectBase* doCall(runtime::doCallContext *context)
	{
		if (context->GetParamCount() < 1)
			return this;

		runtimeObjectBase *o = context->GetParam(0);
		if (!o)
			return this;
		runtime::stringObject *so = o->toString();
		const char *s = so->mVal->c_str();

		if (mDlg && ::IsWindow(mDlg->GetSafeHwnd()))
		{
			mDlg->DispString(CA2W(s).m_psz);
			mDlg->DispString(L"\n");
		}

		so->AddRef();
		so->Release();
		return this;
	}

	// getindex（索引访问一元运算）
	virtual runtimeObjectBase* getIndex(int i) { return NULL; }
	// 对象转化为字符串
	virtual runtime::stringObject* toString() { return NULL; }

	// 比较
	virtual bool isGreaterThan(runtimeObjectBase *obj) { return false; }

	virtual bool isEqual(runtimeObjectBase *obj) { return false; }
};

class printInListBox : public runtime::runtimeObjectBase
{
public:
	CtestCScriptInDialogDlg *mDlg;

public:
	printInListBox()
		: mDlg(NULL)
	{
	}

	virtual uint32_t GetObjectTypeId() const
	{
		return runtime::DT_UserTypeBegin;
	}

	virtual runtimeObjectBase* Add(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Sub(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Mul(const runtimeObjectBase *obj){ return NULL; }
	virtual runtimeObjectBase* Div(const runtimeObjectBase *obj){ return NULL; }

	// =二元运算
	virtual runtimeObjectBase* SetValue(runtimeObjectBase *obj) { return NULL; }

	// 处理.操作符（一元的）
	virtual runtimeObjectBase* GetMember(const char *memName) { return NULL; }

	// docall（函数调用一元运算）
	virtual runtimeObjectBase* doCall(runtime::doCallContext *context)
	{
		if (context->GetParamCount() < 1)
			return this;

		runtimeObjectBase *o = context->GetParam(0);
		if (!o)
			return this;
		runtime::stringObject *so = o->toString();
		const char *s = so->mVal->c_str();

		if (mDlg && ::IsWindow(mDlg->GetSafeHwnd()))
		{
			mDlg->DispString(CA2W(s).m_psz);
		}

		so->AddRef();
		so->Release();
		return this;
	}

	// getindex（索引访问一元运算）
	virtual runtimeObjectBase* getIndex(int i) { return NULL; }
	// 对象转化为字符串
	virtual runtime::stringObject* toString() { return NULL; }

	// 比较
	virtual bool isGreaterThan(runtimeObjectBase *obj) { return false; }

	virtual bool isEqual(runtimeObjectBase *obj) { return false; }
};

///////////////////////////////////////////////////////////////////////////////

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////

CtestCScriptInDialogDlg::CtestCScriptInDialogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtestCScriptInDialogDlg::IDD, pParent)
	, mLoadCodeExists(FALSE)
	, mSourcePath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestCScriptInDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRINTHOST, mPrintHostBox);
	DDX_Check(pDX, IDC_LOADCODE_EXISTS, mLoadCodeExists);
	DDX_Text(pDX, IDC_SOURCE_PATH, mSourcePath);
}

void CtestCScriptInDialogDlg::DispString(const CString &s)
{
	CString left = s, ss;
	int f = left.Find('\n', 0);
	do {
		bool has = mLastString.GetLength() > 0;
		if (f >= 0)
		{
			if (!f)
			{
				mLastString.Truncate(0);
			}
			else
			{
				mLastString += left.Left(f);
				left = left.Mid(f + 1);
				if (has)
					mPrintHostBox.DeleteString(mPrintHostBox.GetCount() - 1);
				mPrintHostBox.AddString(mLastString);
				mPrintHostBox.SetCurSel(mPrintHostBox.GetCount() - 1);
			}
		}
		else
		{
			mLastString += left;
			if (has)
				mPrintHostBox.DeleteString(mPrintHostBox.GetCount() - 1);
			mPrintHostBox.AddString(mLastString);
			mPrintHostBox.SetCurSel(mPrintHostBox.GetCount() - 1);
			break;
		}
		f = left.Find('\n', f + 1);
	} while (1);
}

void CtestCScriptInDialogDlg::ExecuteThreadProc(void *param)
{
	SimpleThread *thread = reinterpret_cast<SimpleThread*>(param);
	reinterpret_cast<CtestCScriptInDialogDlg*>(thread->GetUserData())->ExecuteThreadInner(thread);
}

void CtestCScriptInDialogDlg::ExecuteThreadInner(void *param)
{
	SimpleThread *thread = reinterpret_cast<SimpleThread*>(param);
		
	FILE *file = NULL;
	println2 *println = NULL;
	printInListBox *print = NULL;
	HANDLE compileResult = NULL;
	scriptAPI::ScriptRuntimeContext *rc = NULL;

	do {
		scriptAPI::SimpleCScriptEng::Init();
		
		scriptAPI::ScriptCompiler comp;
		comp.PushName("println2");
		comp.PushName("printInListBox");

		scriptAPI::FileStream fs(CW2A(mSourcePath).m_psz);

		if (!mLoadCodeExists)
		{
			comp.PushName(TESTCALLBACKNAME);
			if ((compileResult = comp.Compile(&fs, true)) == NULL)
			{
				MessageBox(L"编译错误");
				break;
			}
			if (fopen_s(&file, "c:\\work\\codeoutput.txt", "wb"))
				break;
			if (comp.SaveCodeToFile(compileResult, file) < 0)
			{
				MessageBox(L"保存代码失败");
				break;
			}
			if (comp.SaveConstStringTableInResultToFile(compileResult, file) < 0)
			{
				MessageBox(L"保存字符串标失败");
				break;
			}
		}
		else
		{
			if (fopen_s(&file, "c:\\work\\codeoutput.txt", "rb"))
				break;

			compileResult = scriptAPI::ScriptCompiler::CreateCompileResult();
			if (!compileResult)
				break;
			if (comp.LoadCodeFromFile(compileResult, file) < 0)
				break;
			if (comp.LoadConstStringTableToResultFromFile(compileResult, file) < 0)
				break;
		}

		rc = scriptAPI::ScriptRuntimeContext::CreateScriptRuntimeContext();
		if (!rc)
			break;
		println = new runtime::ObjectModule<println2>;
		print = new runtime::ObjectModule<printInListBox>;
		println->mDlg = this;
		print->mDlg = this;
		rc->PushRuntimeObject(println);
		rc->PushRuntimeObject(print);
		rc->ReplaceRuntimeFunc("println", println, &comp);
		rc->ReplaceRuntimeFunc("print", print, &comp);
		rc->PushRuntimeObject(new runtime::ObjectModule<TestCallback>);
		if (rc->Execute(compileResult) < 0)
			break;
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(rc);

		MessageBox(L"执行完毕");

	} while (0);

	if (compileResult)
	{
		scriptAPI::ScriptCompiler::ReleaseCompileResult(compileResult);
	}

	if (rc)
		scriptAPI::ScriptRuntimeContext::DestroyScriptRuntimeContext(rc);

	if (file)
	{
		fclose(file);
		file = NULL;
	}

	scriptAPI::SimpleCScriptEng::Term();
}

BOOL CtestCScriptInDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	mSourcePath = AfxGetApp()->GetProfileString(L"ui", L"lastRun", L"");
	UpdateData(FALSE);

	LOGFONT lf = { 0, };
	lf.lfHeight = -12;
	wcscpy(lf.lfFaceName, L"consolas");
	mFont.CreateFontIndirect(&lf);
	mPrintHostBox.SetFont(&mFont);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BEGIN_MESSAGE_MAP(CtestCScriptInDialogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CtestCScriptInDialogDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SELECT_SOURCE, &CtestCScriptInDialogDlg::OnBnClickedSelectSource)
END_MESSAGE_MAP()

void CtestCScriptInDialogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CtestCScriptInDialogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CtestCScriptInDialogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestCScriptInDialogDlg::OnBnClickedOk()
{
	AfxGetApp()->WriteProfileString(L"ui", L"lastRun", mSourcePath);
	mPrintHostBox.ResetContent();
	mThread.stopThread();
	mThread.startThread(&ExecuteThreadProc, this);
}

void CtestCScriptInDialogDlg::OnBnClickedSelectSource()
{
	CFileDialog fd(TRUE, L".c", L"test", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		L"(*.c c source file)|*.c||");
	if (fd.DoModal() == IDOK)
	{
		mSourcePath = fd.GetPathName();
		UpdateData(FALSE);
	}
}
