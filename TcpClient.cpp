#include "stdafx.h"
#include "TcpClient.h"
#include "TcpTestToolDlg.h"
#include "oomsFunc.h"
#define RECV_SZIE 4096

CTcpClient::CTcpClient():m_param(NULL)
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
}


CTcpClient::~CTcpClient()
{
	closesocket(m_sock);
}

bool CTcpClient::ConnectServer(const char* ip, short port)
{
	SOCKADDR_IN ServerAddr = { 0 };

	//客户端目前只连接一次,不能断线重连
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(ip);
	ServerAddr.sin_port = htons(port);

	int nRet = connect(m_sock, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (nRet == SOCKET_ERROR)
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool CTcpClient::SendDate(const char* buff, const int len)
{
	//先向服务端发送
	int nLeft, nWritten;
	const char* pBuf = buff;
	nLeft = len;

	while (nLeft > 0)
	{
		nWritten = send(m_sock,pBuf, nLeft,0);
		if (nWritten == SOCKET_ERROR)
			return false;
		::PostMessage(m_param->pThis->m_hWnd, WM_USER_SENDING, (WPARAM)m_param, nWritten);
		nLeft -= nWritten;
		pBuf += nWritten;
	}
	::GetLocalTime(&m_param->stSend);
	::PostMessage(m_param->pThis->m_hWnd, WM_USER_SENDONCE, (WPARAM)m_param, len);

	//准备接受报文
	::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVING, (WPARAM)m_param, 0);

	char szRecValue[RECV_SZIE] = { 0 };
	int nRead = 0;
	std::string recvdata;
	//接收服务器发送回来的内容(该方法会阻塞, 在此等待有内容接收到才继续向下执行)
	while (1)
	{
		nRead = recv(m_sock,szRecValue,RECV_SZIE,0);
		if (nRead== SOCKET_ERROR)
		{
			::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVFAIL, (WPARAM)m_param, 0);
			return false;
		}
		::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVING, (WPARAM)m_param, nRead);
		std::string temp;
		temp.resize(recvdata.size() + nRead);
		memcpy((void*)temp.data(), recvdata.data(), recvdata.size());
		memcpy((void*)(temp.data() + recvdata.size()), szRecValue, nRead);
		recvdata.swap(temp);

		if (nRead < RECV_SZIE)
		{
			break;
		}
	}
	::GetLocalTime(&m_param->stRev);
	long timegap = CTcpTestToolDlg::TimeDiff(m_param->stRev, m_param->stSend);
	::PostMessage(m_param->pThis->m_hWnd, WM_USER_TIMEGAP, (WPARAM)m_param, timegap);
	::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVONCE, (WPARAM)m_param, recvdata.size());

	OnRecv(recvdata, recvdata.size());

	::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVED, (WPARAM)m_param, 0);
	return true;
}

bool CTcpClient::OnRecv(std::string & buff, int  length)
{
	//-----------------------------OOMS服务器定制代码-------------------
	//这里根据自己协议的需要修改这块逻辑
	//解析下报文
	bool bRet = CheckPacket(buff.data(), length);
	if (!bRet)
	{
		::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVFAIL, (WPARAM)m_param, 0);
		return false;
	}
	//抽取数据报文
	std::string resjson = ExtractPacket(buff.data(), length);
	//解密
	resjson = AES_EnDeCrypt::Decode(resjson.data(), resjson.size());
	//utf-8--->ansi
	resjson = Convert(resjson.c_str(), CP_UTF8, CP_ACP);
	//---------------------------OOMS服务器定制代码end-------------------
	if (m_param->index == 0)   //只显示某一个的返回内容，别的只显示收到的报文长度
	{
		std::string* data = new std::string;
		*data = resjson;
		::PostMessage(m_param->pThis->m_hWnd, WM_USER_RECVDATA, (WPARAM)data, 0);  //对方删除数据
	}
	return true;
}

void CTcpClient::SetParam(ThreadParam* param)
{
	m_param = param;
}

bool CTcpClient::SendYourDate(const char* buff, const int len, int cmd)
{
	//-----------------------------OOMS服务器定制代码-------------------
	//这里根据自己协议的需要修改这块逻辑
	//组装下报文
	//res 转utf-8
	std::string temp = buff;     //ooms的buff是一个json字符串
	std::string req = Convert(temp.c_str(), CP_ACP, CP_UTF8);
	//res加密
	req = AES_EnDeCrypt::Encode(req.data(), req.size());

	//准备发送
	std::string respacket = BuildPacket(req.c_str(), req.length(), cmd);
	return SendDate(respacket.data(), respacket.length());
	//---------------------------OOMS服务器定制代码end-------------------
	
}

