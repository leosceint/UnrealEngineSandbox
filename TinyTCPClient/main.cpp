#include "CanData.h"
#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <random>
#include <ctime>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;


int main()
{
    srand(static_cast<unsigned int>(time(0)));

    WSADATA WSAData;
    SOCKET connection;
    SOCKADDR_IN addr;
    WSAStartup(MAKEWORD(2, 0), &WSAData);
    connection =  socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);

    int result = connect(connection, (SOCKADDR*)&addr, sizeof(addr));
    if(result == 0)
    {
        cout << "Connected to server!" << endl;
        CanData Data;
        Data.type = 0x0101;
        Data.pos0 = 234434;
        Data.pos1 = 1256;
        Data.pos2 = 3444;
        Data.time = 0;
        
        int IdxSent = 0;
        while(Data.time < 1800)
        {
            unsigned int type_appendix = static_cast<unsigned int>(rand()%4);
            Data.type = 0x0101 + type_appendix;
            result = send(connection, (char*)&Data, sizeof(CanData), 0);
            if (result == SOCKET_ERROR)
                break;
            cout << "!! Data sent #";
            this_thread::sleep_for(4ms);
            cout << IdxSent << endl;
            ++Data.time;
            ++IdxSent;
        }
        cout << ">> WE SENT "<< IdxSent << " PACKAGES <<" << endl;
    }
    else
        cout << "Not connected to server!" << endl;


    closesocket(connection);
    WSACleanup();
    cout<<"Socket for connection Closed" << endl;

    return 0;
}