#pragma once
#include <atlstr.h>
#include <string>



using namespace std;

//protobuf��stringת��
string Hex2bin(CStringA strHex);
string Bin2Hex(string &strBin);

//Utf8�������
std::string string_To_UTF8(const std::string & str);
std::string UTF8_To_string(const std::string & str);

//�䳤�����������
string Dword2String(DWORD dw);
DWORD String2Dword(string str,DWORD &dwOutLen);

//�쳣;ǿ��



void LOG(const char *szLog, ...);






