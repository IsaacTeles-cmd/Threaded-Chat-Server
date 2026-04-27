#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 4096;

void receiveMessages(SOCKET sock){
    vector<char> buffer(BUFFER_SIZE);

   while(true){
        fill(buffer.begin(), buffer.end(), 0);
        
        int bytesReceived = recv(sock, buffer.data(), BUFFER_SIZE - 1, 0);

        if (bytesReceived <= 0) {
            cout << "\n[SYSTEM Connection lost.\n";
            break;
        }

        string msg(buffer.data(), bytesReceived);

        cout << "\r" << msg << "\n> " << flush;
    }
}

int main(){
    WSADATA wsaData;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sock == INVALID_SOCKET){
        cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if(connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        cerr << "Connection failed";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "======================================\n";
    cout << "    CONNECTED TO CHAT SERVER    \n";
    cout << "======================================\n";
    cout << "Commandss: LOGIN|Name , MSG|Name|Text\n";
    cout << "Others: ADD, LIST, REMOVE, HISTORY\n";
    cout << "Type 'EXIT' to quit.\n\n";

    thread receiver(receiveMessages, sock);
    receiver.detach();

    string input;
    while (true) {
        cout << "> ";
        getline(cin, input);

        if (input == "EXIT" || input == "exit") {
            break;
        }

        if (input.empty()) continue;

        // Send commands to server.
        string msg = input + "\n";
        int sendResult = send(sock, msg.c_str(), (int)msg.size(), 0);
        
        if (sendResult == SOCKET_ERROR) {
            cout << "[ERROR] Failed to send data.\n";
            break;
        }
    }


    closesocket(sock);
    WSACleanup();

    cout << "[SySTEM] Closing client...\n";
    return 0;
}