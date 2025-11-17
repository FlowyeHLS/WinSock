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
#define MAX_CLIENTS			3
#define g_sz_SORRY			"Error: Количество подключений превышено"
#define IP_STR_MAX_LENGTH	16

INT n = 0;//Kolovo akctive klient 
SOCKET clietn_sockets[MAX_CLIENTS] = {};
DWORD threadIDs[MAX_CLIENTS] = {};
HANDLE hThreads[MAX_CLIENTS] = {};

VOID WINAPI HandleClient(SOCKET client_socket);


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
	
	cout << "Accept client connections....";
	do
	{
		SOCKET client_socket = accept(listen_socket, NULL, NULL);
		/*if (clietn_sockets[n] == INVALID_SOCKET)
		{
			dwLastError = WSAGetLastError();
			cout << "Accept failed: " << dwLastError << endl;
			closesocket(listen_socket);
			freeaddrinfo(result);
			WSACleanup();
			return dwLastError;
		}*/
			//HandleClient(clietn_socket);
		if (n<MAX_CLIENTS)
		{
			clietn_sockets[n] = client_socket;
			hThreads[n] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, (LPVOID)clietn_sockets[n], 0, threadIDs + n);
			n++;
		}
		else
		{
			CHAR recv_buffer[BUFFER_LENGT] = {};
			INT iResult = recv(client_socket, recv_buffer, BUFFER_LENGT, 0);
			if (iResult > 0)
			{
				cout << "Bytes received: " << iResult << endl;
				cout << "Message: " << recv_buffer << endl;
				INT iSendResult = send(client_socket, g_sz_SORRY, strlen(g_sz_SORRY), 0);
				closesocket(client_socket);
			}
		}
	} while (true);

	WaitForMultipleObjects(MAX_CLIENTS, hThreads, TRUE, INFINITE);

	//7) Получение запросов от клиентов 

	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();


	return dwLastError;
}

INT GetSlotIndex(DWORD dwID)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (threadIDs[i] == dwID)return i;
	}
}

VOID Shift(INT start)
{
	for (INT i = start; i < MAX_CLIENTS; i++)
	{
		clietn_sockets[i] = clietn_sockets[i + 1];
		threadIDs[i] = threadIDs[i + 1];
		hThreads[i] = hThreads[i + 1];
	}
	clietn_sockets[MAX_CLIENTS - 1] = NULL;
	threadIDs[MAX_CLIENTS - 1] = NULL;
	hThreads[MAX_CLIENTS - 1] = NULL;
	n--;
}

/*DWORD WINAPI*/VOID WINAPI HandleClient(SOCKET client_socket)
{
	SOCKADDR_IN peer;
	CHAR address[IP_STR_MAX_LENGTH] = {};
	INT address_length = IP_STR_MAX_LENGTH;
	getpeername(client_socket, (SOCKADDR*)&peer, &address_length);
	inet_ntop(AF_INET, &peer.sin_addr, address, address_length);
	INT port = ((peer.sin_port & 0xFF) << 8) + (peer.sin_port >> 8);
	cout << address << ":" << port << endl;
	////////////////////////////////////////////////////////////////////////////
	INT iResult = 0;
	DWORD dwLastError = 0;
	/*SOCKET client_socket = *(SOCKET*)clients_socket;*/
		CHAR send_buffer[BUFFER_LENGT] = "Привет клиент";
		CHAR recv_buffer[BUFFER_LENGT] = {};
	do
	{
		INT iSendResult = 0;
		ZeroMemory(send_buffer, BUFFER_LENGT);
		ZeroMemory(recv_buffer, BUFFER_LENGT);
		iResult = recv(client_socket, recv_buffer, BUFFER_LENGT, 0);
		if (iResult > 0)
		{
			cout << "Bytes received " << address << ":"<<port << "-" << recv_buffer << endl;
			iSendResult = send(client_socket, recv_buffer, strlen(recv_buffer), 0);
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
	} while (iResult > 0 && !strstr(recv_buffer,"quit"));
	DWORD dwID = GetCurrentThreadId();
	Shift(GetSlotIndex(dwID));
	cout << address << ":" << port << "leaved" << endl;
	ExitThread(0);
	closesocket(client_socket);
	/*return 0;*/
}