#pragma once
#include <string>

//同步tcp库，send之后会阻塞等待回应，直到收到数据后返回
struct ThreadParam;
class CTcpClient
{
public:
	CTcpClient();
	~CTcpClient();

	void SetParam(ThreadParam* param);

	bool ConnectServer(const char* ip, short port);

	//此接口接受定制，各测试工具可以参考修改，主要负责组装报文与发送报文
	//内部务必调用SendDate来发送真正的数据
	//包括函数的参数以及类型都接受定制，请配置主界面使用。
	bool SendYourDate(const char* buff, const int len,int cmd);

	bool SendDate(const char* buff, const int len);

	//此接口接受定制，各测试工具可以参考修改，主要负责接受报文与分析报文
	//参数不允许修改，一般用来界面显示接受到的文本内容
	bool OnRecv(std::string & buff, int length);


private:
	SOCKET m_sock;
	ThreadParam* m_param;
};

