#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <mutex>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "ContactManager.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;
namespace fs = std::filesystem;

// Config consts.
const int PORT = 54000;
const int BUFFER_SIZE = 4096;

ContactManager contactManager;
mutex managerMutex;

map<string, SOCKET> connectedClients;
mutex clientsMutex;

map<string, map<string, vector<string>>> messageHistory;
mutex historyMutex;

void setColor(int color){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Trying to avoid Path Traversal...
string sanitize(string data){
    data.erase(remove_if(data.begin(), data.end(), [](char c){
        return !(isalnum(c) || c == '_' || c == '-');
    }), data.end());
    return data;
}

// TimeStamp func, added waaaay later (forgot).
string getTimesStamp(){
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", &ltm);
    return string(buffer);
}

void log(const string& level, const string& message){
    int color = 7;

    if(level == "INFO") color = 9;  // Light blue.
    else if(level == "REQUEST") color = 14; // Yellow.
    else if(level == "RESPONSE") color = 10; // Green.
    else if(level == "ERROR") color = 12; // Red.
    else if(level == "MSG") color = 11; // Cyan.

    setColor(0); // Timestamp gray.
    cout << "[" << getTimesStamp() << "] ";

    setColor(color);
    cout << "[" << level << "] ";

    setColor(7); // White message.
    cout << message << "\n";
}

string getHistoryPath(const string& u1, const string& u2){
    string user1 = sanitize(u1);
    string user2 = sanitize(u2);
    string filename = (user1 < user2) ? (user1 + "_" + user2) : (user2 + "_" + user1);
    return "storage/history/" + filename + ".txt";
}

void saveMessageToFile(const string& user1, const string& user2, const string& msg){
    string filename = getHistoryPath(user1, user2);

    ofstream out(filename, ios::app); // Append.

    if(!out.is_open()){
        cerr << "Error saving history file\n";
        return;
    }

    out << msg << "\n";
}

void loadAllHistory(){
    if(!fs::exists("storage/history")) return;

    for(const auto& entry : fs::directory_iterator("storage/history")){
        string filename = entry.path().filename().string();

        // Example: User1_User2.txt (PLEASE DONT FORGET THE PATTERN).
        size_t underscore = filename.find('_');
        size_t dot = filename.find('.');

        if(underscore == string::npos || dot == string::npos) continue;

        string user1 = filename.substr(0, underscore);
        string user2 = filename.substr(underscore + 1, dot - underscore - 1);

        ifstream in(entry.path());
        if(!in.is_open()) continue;

        string line;

        while(getline(in, line)){
            // Saves for both users.
            messageHistory[user1][user2].push_back(line);
            messageHistory[user2][user1].push_back(line);
        }
    }

    cout << "[HISTORY LOADED WITH SUCCESS]\n";
}

string processCommand(const string& request, SOCKET clientSocket, string& currentUser){
    stringstream ss(request);
    string command, arg1, arg2;

    if(!getline(ss, command, '|')) return "ERROR: Invalid format\n";

    if(command == "HELP"){
    return 
    "Available Commands:\n"
    "---------------------------------\n"
    "LOGIN|name            -> Login with your username\n"
    "ADD|name              -> Add a contact\n"
    "LIST                  -> List all contacts\n"
    "REMOVE|name           -> Remove a contact\n"
    "UPDATE|old|new        -> Update contact name\n"
    "MSG|user|message      -> Send message to user\n"
    "HISTORY|user          -> Show conversation history\n"
    "HELP                  -> Show this help menu\n"
    "EXIT                  -> Disconnect\n";
    } else if(command == "LOGIN"){
        getline(ss, arg1);
        arg1 = sanitize(arg1);
        if(arg1.empty()) return "ERROR: Invalid name\n";

        lock_guard<mutex> lock(clientsMutex);
        connectedClients[arg1] = clientSocket;
        currentUser = arg1;
        return "DONE: Logged in as " + currentUser;
    }

    if(currentUser.empty()) return "ERROR: Must LOGIN first\n";

    if(command == "ADD"){
        getline(ss, arg1);
        lock_guard<mutex> lock(managerMutex);
        return contactManager.add(arg1) ? "DONE: Added\n" : "ERROR: Already exists\n";

    } else if(command == "LIST"){
        auto contacts = contactManager.list();

        if(contacts.empty()) return "No contacts\n";

        string response = "Contacts:\n";
        for(const auto& c : contacts){
            response += "- " + c.name + "\n";
        }
        return response;

    } else if(command == "REMOVE"){
        string name;
        getline(ss, name);

        if(contactManager.remove(name)){
            return "DONE: Contact removed\n";
        } else {
            return "ERROR: Contact not found\n";
        }
    } else if(command == "UPDATE"){
        string oldName, newName;

        getline(ss, oldName, '|');
        getline(ss, newName);

        if(contactManager.update(oldName, newName)){
            return "DONE: Contact updated\n";
        } else {
            return "ERROR: Contact not found\n";

        }
    } else if(command == "MSG"){
        getline(ss, arg1, '|'); // User to send message.
        getline(ss, arg2); // Message.

        string target = sanitize(arg1);
        SOCKET targetSocket = INVALID_SOCKET;

        {
            lock_guard<mutex> lock(clientsMutex);
            if(connectedClients.count(target)) targetSocket = connectedClients[target];
        }

        if(targetSocket != INVALID_SOCKET){
            string ts = getTimesStamp();
            string displayMsg = ts + " " + currentUser + ": " + arg2 + "\n";
            send(targetSocket, displayMsg.c_str(), (int)displayMsg.size(), 0);

            lock_guard<mutex> hlock(historyMutex);
            string path = getHistoryPath(currentUser, target);
            ofstream out(path, ios::app);
            if(out.is_open()) out << displayMsg;

            return "DONE: Sent\n";
        }
        return "ERROR: User offline\n";

    } else if(command == "HISTORY"){
       string contact;
       getline(ss, contact);

        if(currentUser.empty()){
            return "ERROR: you must LOGIN first\n";
        }

        string filename = getHistoryPath(currentUser, contact);

        ifstream in(filename);

        if(!in.is_open()){
            return "No messages\n";
        }

        string response = "Conversation with " + contact + ":\n";
        string line;

        while(getline(in, line)){
            response += "- " + line + "\n";
        }

       return response;
    }

    return "ERROR: Unknown command\n";
}

void handleClient(SOCKET clientSocket){
    vector<char> buffer(BUFFER_SIZE);
    string currentUser = "";
    string accumulatedData = "";

    log("INFO", "New client connected");

    while(true){
        int bytesReceived = recv(clientSocket, buffer.data(), BUFFER_SIZE - 1, 0);
        if(bytesReceived <= 0) break;

        accumulatedData.append(buffer.data(), bytesReceived);

        size_t pos;
        while((pos = accumulatedData.find('\n')) != string::npos){
            string request = accumulatedData.substr(0, pos);
            accumulatedData.erase(0, pos + 1);

            if(!request.empty() && request.back() == '\r'){
                request.pop_back();
            }

            //LOGIN doesn't break logs.
            string userLabel = currentUser.empty() ? "Unknown" : currentUser;

            log("REQUEST", userLabel + " -> " + request);

            if(request.rfind("LOGIN|", 0) == 0){
                string name = request.substr(6);
                log("INFO", "Login attempt: " + name);
            }

            if(request.rfind("MSG|", 0) == 0){
                stringstream ss(request);
                string cmd, target, msg;
                getline(ss, cmd, '|');
                getline(ss, target, '|');
                getline(ss, msg);

                log("MSG", userLabel + " -> " + target + ": " + msg);
            }

            string response = processCommand(request, clientSocket, currentUser);
            response += "\n";

            if(response.find("ERROR") != string::npos){
                log("ERROR", userLabel + " <- " + response);
            } else{
                log("RESPONSE", userLabel + " <- " + response);
            }

            send(clientSocket, response.c_str(), (int)response.size(), 0);
        }
    }

    if(!currentUser.empty()){
        lock_guard<mutex> lock(clientsMutex);
        connectedClients.erase(currentUser);
        log("INFO", currentUser + " disconnected");
    } else {
        log("INFO", "Unknown client disconnected");
    }

    closesocket(clientSocket);
}


int main(){
    fs::create_directories("storage/history");
    loadAllHistory();
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR ||
        listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        return 1;
    }

    cout << "[SERVER RUNNING] Port " << PORT << "\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            thread(handleClient, clientSocket).detach();
        }
    }

    WSACleanup();
    return 0;
}
