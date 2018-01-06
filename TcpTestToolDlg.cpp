
// TcpTestToolDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TcpTestTool.h"
#include "TcpTestToolDlg.h"
#include "afxdialogex.h"
#include "oomsFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum ColType
{
	col_index = 0,
	col_threadid = 1,
	col_maxtime = 2,
	col_mintime = 3,
	col_times=4, 
	col_sendonce=5, 
	col_recvonce=6, 
	col_sendall=7,
    col_recvall=8,
	col_state = 9,
	col_total = 10              //这项值标记总共列的数量，请一并维护          
};

HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) {
	// Begin variable-argument list...  
	va_list marker;
	va_start(marker, cArgs);
	if (!pDisp) {
		return 0;
	}
	// Variables used...  
	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	HRESULT hr;
//	char buf[200];
	char szName[200];
	// Convert down to ANSI  
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);
	// Get DISPID for name passed...  
	hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	if (FAILED(hr)) {
		return hr;
	}
	// Allocate memory for arguments...  
	VARIANT *pArgs = new VARIANT[cArgs + 1];
	// Extract arguments...  
	for (int i = 0; i < cArgs; i++) {
		pArgs[i] = va_arg(marker, VARIANT);
	}
	// Build DISPPARAMS  
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;
	// Handle special-case for property-puts!  
	if (autoType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}
	// Make the call!  
	hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
	if (FAILED(hr)) {
		delete[] pArgs;
		return hr;
	}
	// End variable-argument section...  
	va_end(marker);
	delete[] pArgs;
	return hr;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTcpTestToolDlg 对话框



CTcpTestToolDlg::CTcpTestToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TCPTESTTOOL_DIALOG, pParent)
	, m_port(0)
	, m_connectNum(0)
	, m_send(_T(""))
	, m_rec(_T(""))
	, pThread(NULL)
	, m_cycle(0)
	, m_runtime(_T(""))
	, m_revtotal(0)
	, m_sendtotal(0)
	, m_cyclegap(100)
	, m_run(0)
	, bTesting(false)
	, m_runningClient(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTcpTestToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipControl);
	DDX_Text(pDX, IDC_EDIT4, m_port);
	DDX_Text(pDX, IDC_EDIT5, m_connectNum);
	DDX_Text(pDX, IDC_EDIT2, m_send);
	DDX_Text(pDX, IDC_EDIT1, m_rec);
	DDX_Control(pDX, IDC_LIST3, m_listLog);
	DDX_Control(pDX, IDC_LIST1, m_listOutput);
	DDX_Control(pDX, IDC_COMBO1, m_comboType);
	DDX_Text(pDX, IDC_EDIT3, m_cycle);
	DDX_Text(pDX, IDC_EDIT7, m_runtime);
	DDX_Text(pDX, IDC_EDIT8, m_revtotal);
	DDX_Text(pDX, IDC_EDIT9, m_sendtotal);
	DDX_Text(pDX, IDC_EDIT6, m_cyclegap);
	DDX_Text(pDX, IDC_EDIT10, m_runningClient);
}

BEGIN_MESSAGE_MAP(CTcpTestToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, &CTcpTestToolDlg::OnBnClickedStart)
	ON_MESSAGE(WM_USER_CONNECTING, OnUserConnecting)
	ON_MESSAGE(WM_USER_CONNECTFAIL, OnUserConnectFail)
	ON_MESSAGE(WM_USER_SENDING, OnUserSending)
	ON_MESSAGE(WM_USER_SENDFAIL, OnUserSendFail)
	ON_MESSAGE(WM_USER_SENDONCE, OnUserSendOnce)
	ON_MESSAGE(WM_USER_RECVING, OnUserRecving)
	ON_MESSAGE(WM_USER_RECVONCE, OnUserRecvOnce)
	ON_MESSAGE(WM_USER_RECVFAIL, OnUserRecvFail)
	ON_MESSAGE(WM_USER_RECVDATA, OnUserRecvData)
	ON_MESSAGE(WM_USER_RECVED, OnUserRecved)
	ON_MESSAGE(WM_USER_TIMES, OnUserTimes)
	ON_MESSAGE(WM_USER_TIMEGAP, OnUserTimegap)
	ON_MESSAGE(WM_USER_EXIT, OnUserExit)//
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CTcpTestToolDlg::OnBnClickedFile)
	ON_BN_CLICKED(IDC_BUTTON2, &CTcpTestToolDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_BUTTON4, &CTcpTestToolDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_BUTTON5, &CTcpTestToolDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CTcpTestToolDlg 消息处理程序

BOOL CTcpTestToolDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//初始网络
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//初始化com
	CoInitialize(NULL);


	DWORD dwStyle = m_listOutput.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_listOutput.SetExtendedStyle(dwStyle);
	m_listOutput.InsertColumn(col_index, _T("序号"), LVCFMT_LEFT, 40);
	m_listOutput.InsertColumn(col_threadid, _T("线程id"), LVCFMT_LEFT, 60);
	m_listOutput.InsertColumn(col_maxtime, _T("最大响应时间ms"), LVCFMT_LEFT, 110);
	m_listOutput.InsertColumn(col_mintime, _T("最小响应时间ms"), LVCFMT_LEFT, 110);
	m_listOutput.InsertColumn(col_times, _T("测试次数"), LVCFMT_LEFT, 60);
	m_listOutput.InsertColumn(col_sendonce, _T("单次发送"), LVCFMT_LEFT, 80);
	m_listOutput.InsertColumn(col_recvonce, _T("单次接收"), LVCFMT_LEFT, 80);
	m_listOutput.InsertColumn(col_sendall, _T("发送总数"), LVCFMT_LEFT, 80);
	m_listOutput.InsertColumn(col_recvall, _T("接收总数"), LVCFMT_LEFT, 80);
	m_listOutput.InsertColumn(col_state, _T("状态"), LVCFMT_LEFT, 70);

//--------------------OOMS定制代码--------------------------
	//先测试4个协议 1、校时  2、登录  3、获取停车点  4、获取泊位
	m_comboType.InsertString(0, "获取泊位");
	m_comboType.InsertString(0, "获取停车点");
	m_comboType.InsertString(0, "登录");
	m_comboType.InsertString(0, "校时");
	m_comboType.SetCurSel(0);
//--------------------OOMS定制代码end--------------------------

	Log("欢迎使用tcp工具，设置好参数开始测试吧!");
	Log("我是动态日志，我会告诉你最新的状态!");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTcpTestToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTcpTestToolDlg::OnPaint()
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
HCURSOR CTcpTestToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTcpTestToolDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (bTesting)
	{
		MessageBox("上次测试还未结束，请按‘暂停测试‘后再试");
		return;
	}
	bTesting = true;
	//准备tcp连接
	UpdateData(TRUE);
	//尝试获取ip
	DWORD dwIP=0;
	IN_ADDR ia;
	std::string strIP;
	m_ipControl.GetAddress(dwIP);
	ia.S_un.S_addr = htonl(dwIP);
	//strIP = inet_ntoa(ia);
	//inet_ntoa(addr2.sin_addr);
	//将其等价替换为
	char sendBuf[20] = { '\0' };
	inet_ntop(AF_INET, (void*)&ia, sendBuf, 16);
	strIP = sendBuf;
	if (strIP.empty())
	{
		Log("%s", "ip不能为空！");
		return;
	}
	if (m_port<=0)
	{
		Log("端口号不对");
		return;
	}
	if (m_connectNum<=0)
	{
		Log("请设置连接数！");
		return;
	}
	if (m_connectNum>=1000)
	{
		Log("连接数过大！");
		return;
	}
	if (m_send.IsEmpty())
	{
		Log("发送的数据不能为空！");
		return;
	}

	SetTimer(1, 1000, NULL);

	//开启n个线程开始工作
	m_runningClient = m_connectNum;
	m_sendtotal = 0;
	m_revtotal = 0;
	if (pThread!=NULL)
	{
		delete[]pThread;
	}
	pThread = new CWinThread*[m_connectNum];
	//清楚列表
	m_listOutput.DeleteAllItems();
	m_run = 0;
	for (int i=0;i<m_connectNum;i++)
	{
		ThreadParam* param = new ThreadParam;
		param->Client.SetParam(param);
		param->index = i;
		param->pThis = this;
		param->ip = strIP;
		param->port = m_port;
		param->cycle = m_cycle;
		param->sleepTime = m_cyclegap;
		pThread[i] = AfxBeginThread(ThreadFun, param);
		pThread[i]->SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
		param->threadid = pThread[i]->m_nThreadID;

		CString strTemp;
		strTemp.Format("%d", i);
		//插入列表
		m_listOutput.InsertItem(i, strTemp);
		strTemp.Format("0x%x", param->threadid);
		m_listOutput.SetItemText(i, col_threadid, strTemp);
		m_listOutput.SetItemText(i, col_state, "就绪");
	}

	UpdateData(FALSE);
}

UINT CTcpTestToolDlg::ThreadFun(LPVOID pParam)
{
	ThreadParam* param = (ThreadParam*)pParam;
	if (param == NULL)
	{
		goto EXIT;
	}
	::PostMessage(param->pThis->m_hWnd, WM_USER_CONNECTING,(WPARAM)param,0);
	bool bRet = param->Client.ConnectServer(param->ip.c_str(), param->port);
	if (!bRet)
	{
		::PostMessage(param->pThis->m_hWnd, WM_USER_CONNECTFAIL, (WPARAM)param, 0);
		goto EXIT;
	}

	//循环主体，不断发送接收----------------------begin-------------
	for(param->times=1;param->times<=param->cycle;++param->times)
	{
		if (param->pThis->bTesting == false)
		{
			break;
		}

		::PostMessage(param->pThis->m_hWnd, WM_USER_SENDING, (WPARAM)param, 0);

		//-----------------------OOMS定制区-------------------------------------
		int cmd = 0;
		switch (param->pThis->m_comboType.GetCurSel())
		{
		case 0:
			cmd = PT_CHECK_TIME;
			break;
		case 1:
			cmd = PT_LOGIN_IN;
			break;
		case 2:
			cmd = PT_PARKING_LIST;
			break;
		case 3:
			cmd = PT_BERTH_INFO;
			break;
		default:
			break;
		}

		//-----------------------OOMS定制区end-------------------------------------

		bRet = param->Client.SendYourDate(param->pThis->m_send, param->pThis->m_send.GetLength(), cmd);
		if (!bRet)
		{
			::PostMessage(param->pThis->m_hWnd, WM_USER_SENDFAIL, (WPARAM)param, 0);
			goto EXIT;
		}

		//到达这里，成功的完成一次发送与接收
		::PostMessage(param->pThis->m_hWnd, WM_USER_TIMES, (WPARAM)param, param->times);

		//稍微休眠下，保护下cpu。
		if (param->sleepTime<10)
		{
			param->sleepTime = 10;
		}
		Sleep(param->sleepTime);
	}

	//循环主体，不断发送接收----------------------end-------------
EXIT:
	::PostMessage(param->pThis->m_hWnd, WM_USER_EXIT, (WPARAM)param, 0);   //需要销毁此资源param
	return 0;
}

void CTcpTestToolDlg::Log(const char* format, ...)
{
	//日志最多1024字节
	char log[1024] = { 0 };
	//首部添加当前时间到秒
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int n=sprintf_s(log,"%04d-%02d-%02d %02d:%02d:%02d:   ",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond );
	va_list vlist;
	va_start(vlist, format);
	vsprintf_s(log + n,1024,format, vlist);
	va_end(vlist);
	std::string logout = log;
	m_listLog.InsertString(0, logout.c_str());
}


LRESULT CTcpTestToolDlg::OnUserConnecting(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	m_listOutput.SetItemText(param->index, col_state, "开始连接");

	return 0;
}

LRESULT CTcpTestToolDlg::OnUserConnectFail(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	Log("第%d线程连接失败", param->index);
	m_listOutput.SetItemText(param->index, col_state, "连接失败");
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserSending(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	if (lParam == 0)
	{
		m_listOutput.SetItemText(param->index, col_state, "等待发送");
	}
	else
	{
		m_listOutput.SetItemText(param->index, col_state, "发送数据");
		//显示发送的字节数,要加上之前的字节数
		CString strHasSend = m_listOutput.GetItemText(param->index, col_sendall);
		int hassend = atoi(strHasSend);
		hassend += lParam;
		CString strTemp;
		strTemp.Format("%d", hassend);
		m_listOutput.SetItemText(param->index, col_sendall, strTemp);
		
		m_sendtotal += lParam;
		UpdateData(FALSE);
	}
	
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserSendFail(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	Log("第%d线程发送失败", param->index);
	m_listOutput.SetItemText(param->index, col_state, "发送失败");
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserRecving(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	if (lParam==0)
	{
		m_listOutput.SetItemText(param->index, col_state, "等待接收");
	}
	else
	{
		m_listOutput.SetItemText(param->index, col_state, "接收数据");

		//显示发送的字节数,要加上之前的字节数
		CString strHasRecv = m_listOutput.GetItemText(param->index, col_recvall);
		int hasrecv = atoi(strHasRecv);
		hasrecv += lParam;
		CString strTemp;
		strTemp.Format("%d", hasrecv);
		m_listOutput.SetItemText(param->index, col_recvall, strTemp);
	
		m_revtotal += lParam;
		UpdateData(FALSE);
		
	}
	
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserRecvFail(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	Log("第%d线程接收失败", param->index);
	m_listOutput.SetItemText(param->index, col_state, "接收失败");
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserRecved(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	m_listOutput.SetItemText(param->index, col_state, "接收完成");

	return 0;
}

LRESULT CTcpTestToolDlg::OnUserExit(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	m_listOutput.SetItemText(param->index, col_state, "测试结束");
	delete param;
	m_runningClient--;
	if (m_runningClient==0)
	{
		Log("本次测试完成");
		bTesting = false;
		KillTimer(1);
	}
	UpdateData(FALSE);
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserRecvData(WPARAM wParam, LPARAM lParam)
{
	//刷新下recv区
	std::string * pstr = (std::string*)wParam;
	m_rec = pstr->c_str();
	UpdateData(FALSE);
	delete pstr;
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserTimes(WPARAM wParam, LPARAM lParam)
{
	//刷新测试次数
	ThreadParam* param = (ThreadParam*)wParam;
	CString strTemp;
	strTemp.Format("%d", lParam);
	m_listOutput.SetItemText(param->index, col_times, strTemp);
	
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserTimegap(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0)
	{
		return 0;
	}
	//刷新最大时延与最小时延
	ThreadParam* param = (ThreadParam*)wParam;
	//获取之前的最大时延
	int currmax = atoi(m_listOutput.GetItemText(param->index, col_maxtime));
	if (currmax == 0)  //初始化
	{
		currmax = -1;
	}
	if (lParam>currmax)
	{
		//需要更新最大时延
		CString strTemp;
		strTemp.Format("%d", lParam);
		m_listOutput.SetItemText(param->index, col_maxtime, strTemp);
	}
	//获取之前的最小时延
	int currmin = atoi(m_listOutput.GetItemText(param->index, col_mintime));
	if (currmin==0)  //初始化
	{
		currmin = 999999;
	}
	if (lParam < currmin)
	{
		//需要更新最小时延
		CString strTemp;
		strTemp.Format("%d", lParam);
		m_listOutput.SetItemText(param->index, col_mintime, strTemp);
	}
	
	return 0;
}

LRESULT CTcpTestToolDlg::OnUserSendOnce(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	CString strTemp;
	strTemp.Format("%d", lParam);
	m_listOutput.SetItemText(param->index, col_sendonce, strTemp);

	return 0;
}

LRESULT CTcpTestToolDlg::OnUserRecvOnce(WPARAM wParam, LPARAM lParam)
{
	ThreadParam* param = (ThreadParam*)wParam;
	CString strTemp;
	strTemp.Format("%d", lParam);
	m_listOutput.SetItemText(param->index, col_recvonce, strTemp);

	return 0;
}



__int64 CTcpTestToolDlg::TimeDiff(SYSTEMTIME& left, SYSTEMTIME& right)
{
	CTime tmLeft(left.wYear, left.wMonth, left.wDay, 0, 0, 0);
	CTime tmRight(right.wYear, right.wMonth, right.wDay, 0, 0, 0);

	CTimeSpan sp = tmLeft - tmRight;
	long MillisecondsL = (left.wHour * 3600 + left.wMinute * 60 + left.wSecond) * 1000 + left.wMilliseconds;
	long MillisecondsR = (right.wHour * 3600 + right.wMinute * 60 + right.wSecond) * 1000 + right.wMilliseconds;

	return  (__int64)sp.GetDays() * 86400000 + (MillisecondsL - MillisecondsR);//此处返回毫秒  
}



void CTcpTestToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		UpdateData(TRUE);
		m_run++;
		m_runtime.Format("%02d:%02d:%02d", m_run / 3600, m_run % 3600 / 60, m_run % 60);
		UpdateData(FALSE);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CTcpTestToolDlg::OnBnClickedFile()
{
	//读取文件内容放到发送区
	//打开文件对话框选择文件
	UpdateData(TRUE);
	CString strFile;
	CFileDialog openFileDlg(TRUE);
	INT_PTR result = openFileDlg.DoModal();
	if (result != IDOK)
	{
		return;
	}
	strFile = openFileDlg.GetPathName();
	
	//只读的打开文件
	HANDLE hFile = CreateFile(strFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dw = GetLastError();
		Log("打开%s出错", strFile);
		Log("错误码%d", dw);
		return;
	}

	//获取文件大小
	DWORD size = GetFileSize(hFile, NULL);
	void* buff = malloc(size + 1);
	memset(buff, 0, size + 1);
	//开辟空间
	DWORD readed=0;
	if (!ReadFile(hFile, buff, size, &readed, NULL))
	{
		Log("读取%s出错", strFile);
		DWORD dw = GetLastError();
		Log("错误码%d", dw);
		free(buff);
		CloseHandle(hFile);
		return;
	}

	m_send = (char*)buff;
	free(buff);
	CloseHandle(hFile);
	UpdateData(FALSE);
}

void CTcpTestToolDlg::OnBnClickedPause()
{
	if (!bTesting)
	{
		return;
	}
	//暂停测试
	bTesting = false;
	for (int i=0;i<m_connectNum;i++)
	{
		CWinThread* p = pThread[i];
		WaitForSingleObject(*p,INFINITE);
	}
	KillTimer(1);
}

void CTcpTestToolDlg::OnBnClickedSave()
{
	UpdateData(TRUE);
	// 保存到excel文件
// 	if (m_listOutput.GetItemCount() ==0)
// 	{
// 		Log("没有有效的性能报告啊");
// 		return;
// 	}

	CString strFile;
	CFileDialog openFileDlg(FALSE);
	//设置过滤条件为excel类型
	openFileDlg.GetOFN().lpstrFilter = "文件\0*.xls; *.xlsx\0";   //文件过虑的类型
	INT_PTR result = openFileDlg.DoModal();
	if (result != IDOK)
	{
		return;
	}
	strFile = openFileDlg.GetPathName();

	//
	SaveExcelFile(strFile);
}

void CTcpTestToolDlg::SaveExcelFile(CString& fileName)
{
	// 获得EXCEL的CLSID  
	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Excel.Application", &clsid);

	if (FAILED(hr)) {
		Log("CLSIDFromProgID() 函数调用失败！");
		return;
	}

	// 创建实例  
	IDispatch *pXlApp;
	hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **)&pXlApp);
	if (FAILED(hr)) {
		Log("请检查是否已经安装EXCEL！");
		return;
	}

	// 获取Workbooks集合  
	IDispatch *pXlBooks;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlApp, L"Workbooks", 0);
		pXlBooks = result.pdispVal;
	}

	// 调用Workbooks.Add()方法，创建一个新的Workbook  
	IDispatch *pXlBook;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlBooks, L"Add", 0);
		pXlBook = result.pdispVal;
	}

	// 创建一个数组，用于填充表格  
	int line = m_listOutput.GetItemCount();
	line++;                                     //加上行头
	VARIANT arr;
	CStringW strOleTemp;
	arr.vt = VT_ARRAY | VT_VARIANT;
	SAFEARRAYBOUND sab[2];
	sab[0].lLbound = 1; sab[0].cElements = line;
	sab[1].lLbound = 1; sab[1].cElements = col_total;
	arr.parray = SafeArrayCreate(VT_VARIANT, 2, sab);

	//获取行头
	for (int j = 1; j <= col_total; j++) {
		//注意，excel从1开始，但是mfc的listcontrol从0开始
		VARIANT tmp;
		LVCOLUMN columnInfo;
		char str[64] = {0};
		columnInfo.mask = LVCF_TEXT;
		columnInfo.pszText = str;
		columnInfo.cchTextMax = 64;
		m_listOutput.GetColumn(j - 1, &columnInfo);
		tmp.vt = VT_BSTR;
		strOleTemp = CA2W(columnInfo.pszText);
		tmp.bstrVal = SysAllocString(strOleTemp);
		// 添加数据到数组中  
		long indices[] = { 1,j };
		SafeArrayPutElement(arr.parray, indices, (void *)&tmp);
	}

	// 初始化数组内容
	for (int i = 2; i <= line; i++) {
		for (int j = 1; j <= col_total; j++) {
			VARIANT tmp;
			tmp.vt = VT_BSTR;
			//注意，excel从1开始，但是mfc的listcontrol从0开始
			strOleTemp = CA2W(m_listOutput.GetItemText(i - 2, j - 1));
			tmp.bstrVal = SysAllocString(strOleTemp);
			// 添加数据到数组中  
			long indices[] = { i,j };
			SafeArrayPutElement(arr.parray, indices, (void *)&tmp);
		}
	}

	// 从Application.ActiveSheet属性获得Worksheet对象  
	IDispatch *pXlSheet;
	{
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlApp, L"ActiveSheet", 0);
		pXlSheet = result.pdispVal;
	}

	// 选择一个合适大小的Range  
	IDispatch *pXlRange;
	{
		VARIANT parm;
		parm.vt = VT_BSTR;
		strOleTemp.Format(L"A1:%c%d",'A'+col_total-1, line);
 		parm.bstrVal = SysAllocString(strOleTemp);
		
		VARIANT result;
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, pXlSheet, L"Range", 1, parm);
		VariantClear(&parm);

		pXlRange = result.pdispVal;
	}

	// 用我们的数组填充这个Range  
	AutoWrap(DISPATCH_PROPERTYPUT, NULL, pXlRange, L"Value", 1, arr);
	pXlRange->Release();
	
	// 接下来我们该保存文件了，利用Worksheet.SaveAs()方法(我这里忽略了其他所有参数，除了文件名)  
	{
		VARIANT filename;
		filename.vt = VT_BSTR;
		filename.bstrVal = fileName.AllocSysString();
		AutoWrap(DISPATCH_METHOD, NULL, pXlSheet, L"SaveAs", 1, filename);
	}

	// 退出，调用Application.Quit()方法  
	AutoWrap(DISPATCH_METHOD, NULL, pXlApp, L"Quit", 0);

	// 释放所有的接口以及变量  
	pXlRange->Release();
	pXlSheet->Release();
	pXlBook->Release();
	pXlBooks->Release();
	pXlApp->Release();
	VariantClear(&arr);
}








void CTcpTestToolDlg::OnBnClickedClear()
{
	// TODO: 在此添加控件通知处理程序代码
	m_listLog.ResetContent();
}
