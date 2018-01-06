
// TcpTestToolDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include <string>
#include "TcpClient.h"



// CTcpTestToolDlg 对话框
class CTcpTestToolDlg : public CDialogEx
{
// 构造
public:
	CTcpTestToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCPTESTTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnUserConnecting(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserConnectFail(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserSending(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserSendOnce(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserSendFail(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRecving(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRecvFail(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRecved(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRecvData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserRecvOnce(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserTimes(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserTimegap(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserExit(WPARAM wParam, LPARAM lParam);
	static UINT ThreadFun(LPVOID pParam);

	

	void Log(const char* format, ...);
	void SaveExcelFile(CString& fileName);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	static __int64 TimeDiff(SYSTEMTIME& left, SYSTEMTIME& right);
	CIPAddressCtrl m_ipControl;
	short m_port;
	int m_connectNum;
	CString m_send;
	CString m_rec;
	CListBox m_listLog;
	CListCtrl m_listOutput;

	CWinThread** pThread; //一坨线程
	CComboBox m_comboType;
	int m_cycle;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_run;
	CString m_runtime;
	int m_revtotal;
	int m_sendtotal;
	afx_msg void OnBnClickedFile();
	afx_msg void OnBnClickedSave();
	int m_cyclegap;
	volatile bool bTesting;
	int m_runningClient;// 活动的客户端
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedClear();
};

//线程参数
struct ThreadParam
{
	std::string ip;
	SYSTEMTIME stSend;
	SYSTEMTIME stRev;
	short port;
	int index;         //线程所在的序号
	int threadid;      //系统的线程id
	int cycle;         //循环次数
	int sleepTime;     //休眠时长 
	int times;         //已经循环的次数
	CTcpTestToolDlg* pThis; //寄主指针
	CTcpClient Client;
};