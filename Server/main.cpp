#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN


#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include<iostream>
using namespace std;

#pragma comment(lib,"WS2_32.lib")

#define DEFAULT_PORT		"27015"
#define BUFFER_LENGT		1460

int main()
{
	setlocale(LC_ALL, "RU");
	DWORD dwLastError = 0;
	INT iResult = 0;

	//0) Инициализирум WinSock
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		dwLastError = WSAGetLastError();
		cout << "WSA init failed with: " << dwLastError << endl;
		return dwLastError;
	}

	//1) 
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//2) Задаем параметры сокета 
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		dwLastError = WSAGetLastError();
		cout << "getaddrinfo faild with error: " << dwLastError << endl;
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//3) Создаем сокет для прослушивания сервера 
	SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		dwLastError = WSAGetLastError();
		cout << "Socket creation failed with error: " << dwLastError << endl;
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//4) Связываем сокет с IP 
	iResult = bind(listen_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		dwLastError = WSAGetLastError();
		cout << "Bind failed with error : " << dwLastError << endl;
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//5) Запускаем прослушивание сокета 
	iResult = listen(listen_socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		dwLastError = WSAGetLastError();
		cout << "Listen failed with error: " << dwLastError << endl;
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//6) Обработка запросов от клиентов 
	SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
	{
		dwLastError = WSAGetLastError();
		cout << "Accept failed: " << dwLastError << endl;
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return dwLastError;
	}

	//7) Получение запросов от клиентов 
	CHAR send_buffer[BUFFER_LENGT] = "Привет клиент";
	CHAR recv_buffer[BUFFER_LENGT] = {};
	INT iSendResult = 0;
	do
	{
		iResult = recv(client_socket, recv_buffer, BUFFER_LENGT, 0);
		if (iResult > 0)
		{
			cout << "Bytes received: " << recv_buffer << endl;
			iSendResult = send(client_socket, send_buffer, sizeof(send_buffer), 0);
			if (iSendResult == SOCKET_ERROR)
			{
				dwLastError = WSAGetLastError();
				cout << "Send failed with error: " << dwLastError << endl;
				break;
			}
			cout << "Byte sent: " << iSendResult << endl;
		}
		else if (iResult == 0) cout << "Connection closing" << endl;
		else
		{
			dwLastError = WSAGetLastError();
			cout << "Recive failed with error: " << dwLastError << endl;
		}
	} while (iResult > 0);
	closesocket(client_socket);
	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();
	


	
	return dwLastError;
}