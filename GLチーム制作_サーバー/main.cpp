//=============================================================================
//	mainファイル [ main.cpp ]
//	Auther : KOTARO NAGASAKI
//=============================================================================

//*****************************************************************************
//	定数定義
//*****************************************************************************
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//*****************************************************************************
//	インクルード
//*****************************************************************************
#include <ws2tcpip.h>
#include <time.h>
#include<process.h>
#include "main.h"
#include "AI.h"

//*****************************************************************************
//	ライブラリのリンク
//*****************************************************************************
#pragma comment ( lib , "ws2_32.lib" )
#pragma comment (lib, "winmm.lib")

//*****************************************************************************
//	プロトタイプ宣言
//*****************************************************************************
unsigned __stdcall aiUpdate(void *p);

//*****************************************************************************
//	定数定義
//*****************************************************************************
const int charcterMax = 6;
const int hostNameMax = 256;
const int sendPort = 20000;
USER_INFO userInfo[charcterMax];
SOCKET sendSock;
sockaddr_in sendAdd;

const VECTOR3 ROCK_POSITION_LIST[] = {
	VECTOR3(-214.0f,100.0f,421.0f),
	VECTOR3(359.0f,100.0f,188.0f),
	VECTOR3(94.0f,100.0f,-458.0f),
	VECTOR3(-198.0f,100.0f,222.0f),
	VECTOR3(419.0f,100.0f,293.0f),
	VECTOR3(-335.0f,100.0f,164.0f),
	VECTOR3(-471.0f,100.0f,-115.0f),
	VECTOR3(368.0f,100.0f,-363.0f),
	VECTOR3(-476.0f,100.0f,231.0f),
	VECTOR3(-249.0f,100.0f,-319.0f),
	VECTOR3(-243.0f,100.0f,481.0f),
	VECTOR3(345.0f,100.0f,-253.0f),
	VECTOR3(444.0f,100.0f,-118.0f),
	VECTOR3(186.0f,100.0f,27.0f),
	VECTOR3(387.0f,100.0f,-438.0f),
	VECTOR3(-12.0f,100.0f,-439.0f),
	VECTOR3(496.0f,100.0f,-332.0f),
	VECTOR3(-247.0f,100.0f,143.0f),
	VECTOR3(-302.0f,100.0f,-296.0f),
	VECTOR3(-171.0f,100.0f,-274.0f),

};
//=============================================================================
//	自作バインド関数
//=============================================================================
bool myBind(SOCKET* _socket, SOCKADDR_IN* _sockAdd)
{
	int ret;

	for (;;)
	{
		int portAdd = 1;

		ret = bind(*_socket, (sockaddr*)_sockAdd, sizeof(*_sockAdd));
		ret = WSAGetLastError();

		if (ret == WSAEADDRINUSE)
		{
			_sockAdd->sin_port = htons(20000 + portAdd);
			portAdd++;
			continue;
		}
		else
			break;
	}

	return true;
}

//=============================================================================
//	ユーザー情報リセット処理
//=============================================================================
void initUserInfo()
{
	for (int count = 0; count < charcterMax; count++)
	{
		memset(&userInfo[count].fromaddr, 0, sizeof(userInfo[count].fromaddr));
		userInfo[count].entryFlag = false;
		userInfo[count].death = 0;
		userInfo[count].kill = 0;
		userInfo[count].pos = VECTOR3(0.0f, 0.0f, 0.0f);
		userInfo[count].rot = VECTOR3(0.0f, 0.0f, 0.0f);
		userInfo[count].cannonRot = VECTOR3(0.0f, 0.0f, 0.0f);
		userInfo[count].cannon = false;
	}
}

//=============================================================================
//	ai用位置セット＆送信処理
//=============================================================================
void aiSetPos(int _charNum, VECTOR3 _pos)
{
	NET_DATA data;

	//	位置情報のセット
	userInfo[_charNum].pos.x = _pos.x;
	userInfo[_charNum].pos.y = _pos.y;
	userInfo[_charNum].pos.z = _pos.z;

	data.charNum = _charNum;
	data.type = DATA_TYPE_POS;
	data.servID = SERV_ID;
	data.data_pos.posX = _pos.x;
	data.data_pos.posY = _pos.y;
	data.data_pos.posZ = _pos.z;

	//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
	sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));
}

//=============================================================================
//	ai用位置セット＆送信処理
//=============================================================================
void aiSetRot(int _charNum, VECTOR3 _rot)
{
	NET_DATA data;

	//	位置情報のセット
	userInfo[_charNum].rot.x = _rot.x;
	userInfo[_charNum].rot.y = _rot.y;
	userInfo[_charNum].rot.z = _rot.z;

	data.charNum = _charNum;
	data.type = DATA_TYPE_ROT;
	data.servID = SERV_ID;
	data.data_rot.rotX = _rot.x;
	data.data_rot.rotY = _rot.y;
	data.data_rot.rotZ = _rot.z;

	//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
	sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));
}

//=============================================================================
//	ai用位置セット＆送信処理
//=============================================================================
void aiSetCannonRot(int _charNum, VECTOR3 _cannonRot)
{
	NET_DATA data;

	//	位置情報のセット
	userInfo[_charNum].cannonRot.x = _cannonRot.x;
	userInfo[_charNum].cannonRot.y = _cannonRot.y;
	userInfo[_charNum].cannonRot.z = _cannonRot.z;

	data.charNum = _charNum;
	data.type = DATA_TYPE_CANNONROT;
	data.servID = SERV_ID;
	data.data_cannonRot.rotX = _cannonRot.x;
	data.data_cannonRot.rotY = _cannonRot.y;
	data.data_cannonRot.rotZ = _cannonRot.z;

	//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
	sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));
}

//=============================================================================
//	ai用位置セット＆送信処理
//=============================================================================
void aiSetCannon(int _charNum, bool _flag)
{
	NET_DATA data;

	//	位置情報のセット
	userInfo[_charNum].cannon = _flag;

	data.charNum = _charNum;
	data.type = DATA_TYPE_CANNON;
	data.servID = SERV_ID;
	data.data_cannon.flag = _flag;

	//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
	sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));
}

//=============================================================================
//	メイン処理関数
//=============================================================================
int main(void)
{
	WSADATA wsaData;

	// ホスト名・IPアドレス
	char hostName[hostNameMax];
	char ip[16];
	PHOSTENT phostent;
	IN_ADDR in;

	//	現在のキャラクター数
	int charNum = -1;

	//	Winsockの初期化
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NULL)
	{
		printf("Winsockの初期化失敗\nエラーコード : %d\n", WSAGetLastError());

		return false;
	}

	//	自分のホスト名取得
	gethostname(hostName, sizeof(hostName));
	phostent = gethostbyname(hostName);

	//	自分のIPアドレスを表示（ユーザーは表示されているIPアドレスを"address.txt"にセットする）
	memcpy(&in, phostent->h_addr, 4);
	sprintf_s(ip, inet_ntoa(in));
	printf("%s:%s\n", hostName, ip);

	//	送信時用変数群生成
	//-------------------------------------------------
	//	ソケットの生成
	sendSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//	送信先アドレス
	sendAdd.sin_port = htons(2000);
	sendAdd.sin_family = AF_INET;
	sendAdd.sin_addr.s_addr = inet_addr("239.0.0.23");//マルチキャストアドレス

	int param = 1;
	int ret = setsockopt(sendSock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&param, sizeof(param));
	ret = WSAGetLastError();
	//-------------------------------------------------

	//	受信時用変数群生成
	//-------------------------------------------------
	//	ソケットの生成
	SOCKET recvSock;
	recvSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//	受信先のアドレス
	sockaddr_in recvAdd;
	recvAdd.sin_port = htons(3000);
	recvAdd.sin_family = AF_INET;
	recvAdd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int recvAddLength = sizeof(recvAdd);
	
	setsockopt(recvSock, SOL_SOCKET, SO_BROADCAST, (char *)&param, sizeof(param));

	myBind(&recvSock, &recvAdd);
	//-------------------------------------------------

	AI::Initialize();
	
	UINT threadID = 0;

	HANDLE ai = 0;
#ifdef _DEBUG
	ai = (HANDLE)_beginthreadex(NULL,0,&aiUpdate,NULL,0,&threadID);
#endif
	NET_DATA data;

	for (;;)
	{
		//	受信
		int ret = recvfrom(recvSock, (char*)&data, sizeof(data), 0, (sockaddr*)&recvAdd, &recvAddLength);
		//ret = WSAGetLastError();
		//printf("%d", ret);]

		if (ret == SOCKET_ERROR)
		{
		}
		else
		{
			if (data.servID == SERV_ID)
			{
				//	データタイプによって分岐
				switch (data.type)
				{
				case DATA_TYPE_POS:

					//	位置情報のセット
					userInfo[data.charNum].pos.x = data.data_pos.posX;
					userInfo[data.charNum].pos.y = data.data_pos.posY;
					userInfo[data.charNum].pos.z = data.data_pos.posZ;

					//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
					sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));

					break;

				case DATA_TYPE_ROT:

					//	回転情報のセット
					userInfo[data.charNum].rot.x = data.data_rot.rotX;
					userInfo[data.charNum].rot.y = data.data_rot.rotY;
					userInfo[data.charNum].rot.z = data.data_rot.rotZ;

					//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
					sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));

					break;

				case DATA_TYPE_CANNONROT:

					//	回転情報のセット
					userInfo[data.charNum].cannonRot.x = data.data_cannonRot.rotX;
					userInfo[data.charNum].cannonRot.y = data.data_cannonRot.rotY;
					userInfo[data.charNum].cannonRot.z = data.data_cannonRot.rotZ;

					//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
					sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));

					break;

				case DATA_TYPE_CANNON:

					//	発射フラグのセット
					userInfo[data.charNum].cannon = data.data_cannon.flag;

					//	マルチキャストで送信（送信先で自分のデータだったら勝手にはじけ）
					sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));

					break;

				case DATA_TYPE_KILL:

					//	殺した数インクリメント
					userInfo[data.charNum].kill++;

					break;

				case DATA_TYPE_DEATH:

					//	殺された数インクリメント
					userInfo[data.charNum].death++;

					break;

				case DATA_TYPE_PAUSE:

					//	ポーズ時にデータを渡す
					//	指定のキャラクタのところにのみ送信
					for (int count = 0; count < charcterMax; count++)
					{
						//	自分以外の場合
						if (count != data.charNum)
						{
							data.charNum = count;

							//	送信データ格納
							data.servID = SERV_ID;
							data.type = DATA_TYPE_PAUSE;
							data.data_pause.kill = userInfo[count].kill;
							data.data_pause.death = userInfo[count].death;

							//	データ送信
							sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&userInfo[data.charNum].fromaddr, sizeof(userInfo[data.charNum].fromaddr));
						}
					}

					break;

				case DATA_TYPE_GAME_START:

					//	最上位クライアントから、ゲーム開始を受け取ったら
					if (data.charNum == 0)
					{
						// AI処理用スレッド開始
						ai = (HANDLE)_beginthreadex(NULL, 0, &aiUpdate, NULL, NULL, NULL);
					}

					break;

				case DATA_TYPE_ENTRY:

					//	エントリー処理
					//	現在のプレイヤー数を返し、自分のプレイヤー番号をセットさせる。
				{
					//	ループ処理判定フラグ
					bool errorFlag = true;

					//	キャラクター番号をインクリメント（キャラクター番号は0〜5）
					charNum++;

					//	キャラクター数が一定値を超えていたら処理をはじく
					if (charNum < charcterMax)
					{
						//	エラー処理（エントリー状態ならエラーとする）
						if (userInfo[charNum].entryFlag == true)
						{
							//	キャラクタ番号をマイナスし、処理を弾く
							charNum--;
							errorFlag = false;

							break;
						}

						//	エラー処理を抜けたら
						if (errorFlag == true)
						{
							//	数値をセット
							data.charNum = charNum;
							data.servID = SERV_ID;
							data.type = DATA_TYPE_ENTRY;

							//	ユーザー情報をセット
							userInfo[charNum].fromaddr = recvAdd;
							userInfo[charNum].fromaddr.sin_port = htons(3000);
							userInfo[charNum].entryFlag = true;

							//	「エントリーした」という情報をマルチキャストで送信
							sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&sendAdd, sizeof(sendAdd));
						}
					}
					else
					{
						data.type = DATA_TYPE_EMPTY;
						data.servID = SERV_ID;

						//	ポートを再設定
						recvAdd.sin_port = htons(3000);

						//	部屋がいっぱいと送る
						sendto(sendSock, (char*)&data, sizeof(data), 0, (sockaddr*)&recvAdd, sizeof(recvAdd));
					}

					break;
				}
				}
			}
			else
			{
				printf("networkID Not Equal!!\n");
			}
		}
	}

	CloseHandle(ai);
	// ソケット終了
	closesocket(recvSock);

	// WinSock終了処理
	WSACleanup();

	AI::Finalize();
}

//=============================================================================
//	AI用更新処理関数
//=============================================================================
unsigned __stdcall aiUpdate(void *p)
{
	unsigned int CurrentTime = 0;//DWORD dwCurrentTime;
	unsigned int PrevTime = 0;	 //DWORD dwExecLastTime;
	unsigned int FPSLastTime = 0;//DWORD dwFPSLastTime;
	unsigned int FrameCount = 0; //DWORD dwFrameCount;

	timeBeginPeriod(1);				// 分解能を設定

	FPSLastTime =
		CurrentTime = timeGetTime();

	PrevTime = 0;

	FrameCount = 0;

	//	クライアント側とFPSをそろえる
	for (;;)
	{
		CurrentTime = timeGetTime();
		if ((CurrentTime - FPSLastTime) >= 500)	// 0.5秒ごとに実行
		{
			FPSLastTime = CurrentTime;
			FrameCount = 0;
		}

		if (CurrentTime - PrevTime >= 1000 / 60)
		{


			//	AI更新処理
			AI::SetUserInfo(userInfo);
			AI::UpdateAll();


			PrevTime = CurrentTime;
		}
	}
}

VECTOR3 GetRockPos(int index)
{
	if (index >= 0 && index < 20)
	{
		return ROCK_POSITION_LIST[index];
	}

	return VECTOR3(-1.0f,-1.0f,-1.0f);
}

//	EOF