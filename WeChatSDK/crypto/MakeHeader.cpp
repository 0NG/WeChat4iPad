#include "stdafx.h"
#include "MakeHeader.h"
#include "fun.h"
#include "WxUser.h"
#include "TT.h"





std::string BaseHeader::MakeHeader(void*handle, int cgiType, int nLenProtobuf, int nLenCompressed)
{
	string strHeader;
	WxUser*Auth = (WxUser*)handle;

	int nCur = 0;

	//����ƶ��豸�����־
	strHeader.push_back(0xbf);
	nCur++;
	
	//�Ƿ�ʹ��ѹ���㷨(���2bits)(1��ʾʹ��zlibѹ��)(ѹ���󳤶ȿ��ܱ䳤,��һ��ʹ��ѹ���㷨)
	unsigned char SecondByte = (nLenProtobuf == nLenCompressed) ? 0x2 : 0x1;

	//��ͷ�������д��
	strHeader.push_back(SecondByte);
	nCur++;

	//�����㷨(ǰ4bits),Ĭ��ʹ��aes����(5),��Ҫrsa���ܵ�CGI���ش��麯��
	unsigned char ThirdByte = 0x5 << 4;

	//cookie����(��4bits)����ǰЭ��Ĭ��15λ
	ThirdByte += 0xf;

	strHeader.push_back(ThirdByte);
	nCur++;

	//д��汾��(���4�ֽ�����)
	DWORD dwVer = htonl(0x16070124);
	strHeader = strHeader + string((const char *)&dwVer,4);
	nCur += 4;

	//д��uin(���4�ֽ�����)
	DWORD dwUin = htonl(Auth->m_uin);
	strHeader = strHeader + string((const char *)&dwUin, 4);
	nCur += 4;

	//д��cookie
	strHeader = strHeader + Auth->m_cookie.c_str();
	nCur += 0xf;

	//cgi type(�䳤����)
	string strCgi = Dword2String(cgiType);
	strHeader = strHeader + strCgi;
	nCur += strCgi.size();

	//protobuf����(�䳤����)
	string strProtobuf = Dword2String(nLenProtobuf);
	strHeader = strHeader + strProtobuf;
	nCur += strProtobuf.size();

	//protobufѹ���󳤶�(�䳤����)
	string strCompressed = Dword2String(nLenCompressed);
	strHeader = strHeader + strCompressed;
	nCur += strCompressed.size();

	//ecdhУ��ֵ��mmtlsЭ����õ��Ĳ���(15 byte)(��0 ��λ)
	//string yw = Dword2String(10000);
	//strHeader = strHeader + yw;
	//nCur += yw.size();

	//strHeader = strHeader + string((const char *)2, 1);
	//nCur += 1;


	//string check = Dword2String(Auth->checkx);
	//strHeader = strHeader + check;
	//nCur += check.size();

	//string unknown = Dword2String(0x01004567);
	//strHeader = strHeader + unknown;
	//nCur += unknown.size();

	char szBuf[15] = { 0 };
	strHeader = strHeader + string(szBuf, 15);
	nCur += 15;

	//����ͷ����д��ڶ��ֽ�ǰ6bits(��ͷ���Ȳ��ᳬ��6bits)
	SecondByte += (nCur << 2);


	//����ȷ�ĵڶ��ֽ�д���ͷ
	strHeader[1] = SecondByte;

	return strHeader;
}

BaseHeader::HeaderInfo BaseHeader::UnPackHeader(void* Wx, string pack)
{
	string body;
	HeaderInfo info;

	WxUser*Auth = (WxUser*)Wx;
	int nCur = 0;

	if (pack.size() < 0x20)	return info;

	//������׿��־bf(�����ͷ�ܳ���)
	if (0xbf == (unsigned char)pack[nCur])
	{
		nCur++;
	}

	//������ͷ����(ǰ6bits)
	info.nHeadLen = (unsigned char)pack[nCur] >> 2;

	//�Ƿ�ʹ��ѹ��(��2bits)
	info.bCompressed = (1 == ((unsigned char)pack[nCur] & 0x3)) ? TRUE : FALSE;

	nCur++;

	//�����㷨(ǰ4 bits)(05:aes / 07:rsa)(�����ֽ׶εķ���ʹ��rsa��Կ����,����û��˽Կ�հ�һ��aes����)
	info.nDecryptType = (unsigned char)pack[nCur] >> 4;

	//cookie����(��4 bits)
	int nCookieLen = (unsigned char)pack[nCur] & 0xF;
	printf("CookieLen:%d", nCookieLen);
	nCur++;

	//�������汾,����(4�ֽ�)
	nCur += 4;

	//uin �ǵ�¼��,����(4�ֽ�)
	nCur += 4;

	//ˢ��cookie(����15�ֽ�˵��Э��ͷ�Ѹ���)
	if (nCookieLen && nCookieLen <= 0xf)
	{
	
		Auth->m_cookie = pack.substr(nCur, nCookieLen);
		printf("CookieLen:%s", Auth->m_cookie.c_str());
		nCur += nCookieLen;
	}
	else if (nCookieLen > 0xf)
	{
		return info;
	}

	//cgi type,�䳤����,����
	DWORD dwLen = 0;
	DWORD dwCgiType = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//��ѹ��protobuf���ȣ��䳤����
	info.nLenRespProtobuf = String2Dword(pack.substr(nCur, 5), dwLen);
	//protoLen = (int)m_nLenRespProtobuf;
	nCur += dwLen;

	//ѹ����(����ǰ)��protobuf���ȣ��䳤����
	info.nLenRespCompressed = String2Dword(pack.substr(nCur, 5), dwLen);
	//compressed = (int)m_nLenRespCompressed;
	nCur += dwLen;

	//������������

	//������,ȡ����
	if (info.nHeadLen < pack.size())
	{
		info.body = pack.substr(info.nHeadLen);
	}

	return info;
}

std::string BaseHeader::LoginHead(int nLenProtobuf, int nLenCompressed)
{
	string strHeader;

	int nCur = 0;

	//��¼������Ҫ����ƶ��豸�����־
	//strHeader.push_back(0xbf);
	//nCur++;

	//��¼�����������������,����ֱ��ʹ��ѹ���㷨(���2bits��Ϊ2)
	unsigned char SecondByte = 0x2;

	//��ͷ�������д��
	strHeader.push_back(SecondByte);
	nCur++;

	//�����㷨(ǰ4bits),RSA����(7)
	unsigned char ThirdByte = 0x7 << 4;

	//cookie����(��4bits)����ǰЭ��Ĭ��15λ
	ThirdByte += 0xf;

	strHeader.push_back(ThirdByte);
	nCur++;

	DWORD dwVer = htonl(0x16070124);
	strHeader = strHeader + string((const char *)&dwVer, 4);
	nCur += 4;

	//��¼������Ҫuin ȫ0ռλ����
	DWORD dwUin = 0;
	strHeader = strHeader + string((const char *)&dwUin, 4);
	nCur += 4;

	//��¼������Ҫcookie ȫ0ռλ����
	char szCookie[15] = { 0 };
	strHeader = strHeader + string((const char *)szCookie, 15);
	nCur += 15;

	string strCgi = Dword2String(701);
	strHeader = strHeader + strCgi;
	nCur += strCgi.size();

	string strLenProtobuf = Dword2String(nLenProtobuf);
	strHeader = strHeader + strLenProtobuf;
	nCur += strLenProtobuf.size();

	string strLenCompressed = Dword2String(nLenCompressed);
	strHeader = strHeader + strLenCompressed;
	nCur += strLenCompressed.size();

	byte rsaVer = LOGIN_RSA_VER;
	strHeader = strHeader + string((const char *)&rsaVer, 1);
	nCur++;

	byte unkwnow[2] = { 0x01,0x02 };
	strHeader = strHeader + string((const char *)unkwnow, 2);
	nCur += 2;

	//����ͷ����д��ڶ��ֽ�ǰ6bits(��ͷ���Ȳ��ᳬ��6bits)
	SecondByte += (nCur << 2);

	//����ȷ�ĵڶ��ֽ�д���ͷ
	strHeader[0] = SecondByte;

	return strHeader;
}


std::string  BaseHeader::RsaHeader(int cgiType, int nLenProtobuf, int nLenCompressed, int nLenRsa)
{

	string strHeader;

	int nCur = 0;

	//�ƶ��豸�����־
	strHeader.push_back(0xbf);
	nCur++;

	//protobufѹ����־λ
	unsigned char SecondByte = 0x1;

	//��ͷ�������д��
	strHeader.push_back(SecondByte);
	nCur++;

	//�����㷨(ǰ4bits)
	unsigned char ThirdByte = 0x1 << 4;

	//cookie����,д��Ҳ��д,������cookie
	ThirdByte += 0x0;

	strHeader.push_back(ThirdByte);
	nCur++;

	//�汾��
	DWORD dwVer = htonl(0x16070124);
	strHeader = strHeader + string((const char *)&dwVer, 4);
	nCur += 4;

	//δ��¼�ɹ�ʱ����Ҫuin ȫ0ռλ����
	DWORD dwUin = 0;
	strHeader = strHeader + string((const char *)&dwUin, 4);
	nCur += 4;

	string strCgi = Dword2String(cgiType);
	strHeader = strHeader + strCgi;
	nCur += strCgi.size();

	string strLenProtobuf = Dword2String(nLenProtobuf);
	strHeader = strHeader + strLenProtobuf;
	nCur += strLenProtobuf.size();

	string strLenCompressed = Dword2String(nLenCompressed);
	strHeader = strHeader + strLenCompressed;
	nCur += strLenCompressed.size();

	byte rsaVer = LOGIN_RSA_VER;
	strHeader = strHeader + string((const char *)&rsaVer, 1);
	nCur++;

	byte unkwnow[2] = { 0x01,0x02 };
	strHeader = strHeader + string((const char *)unkwnow, 2);
	nCur += 2;

	//���ܺ����ݳ���
	string subHeader;
	short uLenProtobuf = nLenRsa;
	strHeader = strHeader + string((const char *)&uLenProtobuf, 2);
	nCur += 2;

	//���������5���ֽ�(������д)
	char szUnknown[5] = { 0 };
	strHeader = strHeader + string((const char *)szUnknown, 5);
	nCur += 5;

	//����ͷ����д��ڶ��ֽ�ǰ6bits(��ͷ���Ȳ��ᳬ��6bits)
	SecondByte += (nCur << 2);

	//����ȷ�ĵڶ��ֽ�д���ͷ
	strHeader[1] = SecondByte;

	return strHeader;

}