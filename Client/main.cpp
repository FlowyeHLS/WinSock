#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"WS2_32.lib")

#define DEFAULT_PORT		"27015"
#define BUFFER_LENGHT		1460

int main()
{
	setlocale(LC_ALL, "RU");
	cout << "Hello winSock" << endl;

	INT iResult = 0;
	DWORD dwLastError = 0;

	//0) Инициализация WinSock:
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);			//MAKEWORD(2,2) - задает версию WinSock
	if (iResult != 0)
	{
		cout << "WSAStartup failed: " << iResult << endl;
		return iResult;
	}

	//1) Создаем переменные для подключения сокетов
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//2) Задаем информацию о сервере к которому будем подключаться
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo faild: " << iResult << endl;
		WSACleanup();
		return iResult;
	}

	//3) Создаем сокет для подключения к серверу 
	ptr = result;
	SOCKET connect_scoket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (connect_scoket == INVALID_SOCKET)
	{
		dwLastError = WSAGetLastError();
		cout << "Socket error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//4) Подключаемся к серверу
	iResult = connect(connect_scoket, ptr->ai_addr, (INT)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		dwLastError = WSAGetLastError();
		cout << "Connetction error: " << dwLastError << endl;
		closesocket(connect_scoket);
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//5) Отправляем данные на сервер 
	
	CHAR send_buffer[BUFFER_LENGHT] = "Hello Server, I am client";
	do 
	{
		iResult = send(connect_scoket, send_buffer, strlen(send_buffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			dwLastError = WSAGetLastError();
			cout << "Send failed with error: " << dwLastError;
			closesocket(connect_scoket);
			freeaddrinfo(result);
			WSACleanup();
			return dwLastError;
		}
		cout << iResult << "Bytes sent" << endl;

		//6) Ожидаем ответ от сервера 
		CHAR recv_buffer[BUFFER_LENGHT] = {};
		/*do*/
		{
			iResult = recv(connect_scoket, recv_buffer, BUFFER_LENGHT, 0);
			if (iResult > 0)cout << iResult << "Bytes recived, Message:\t" << recv_buffer << ".\n";
			else if (iResult == 0) cout << "Connetcion closed\n";
			else cout << "Recive faild with error: " << WSAGetLastError() << endl;
		} /*while (iResult > 0);*/
		
		ZeroMemory(send_buffer, BUFFER_LENGHT);
		cout << "Type some text: ";
		SetConsoleCP(1251);
		cin.getline(send_buffer, BUFFER_LENGHT);
		SetConsoleCP(866);
	} while (strstr(send_buffer, "exit") == 0 && strstr(send_buffer, "quit") == 0);
	//} while (strcmp(send_buffer, "exit") != 0 && strcmp(send_buffer, "quit") != 0);

	//7) Отключение от сервера
	iResult = shutdown(connect_scoket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		dwLastError = WSAGetLastError();
		cout << "Shutdown faild with error: " << dwLastError << endl;
	}
	closesocket(connect_scoket);
	freeaddrinfo(result);
	WSACleanup();

	return dwLastError;
}