#include "ContactManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

ContactManager::ContactManager(){
    fs::create_directories("storage");
    loadFromFile();
}

std::string ContactManager::sanitizeInput(std::string data){
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
    data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    return data;
}

void ContactManager::loadFromFile(){
    std::lock_guard<std::mutex> lock(mtx);
    contacts.clear();

    std::ifstream in("storage/contacts.txt");
    if(!in.is_open()) return;

    std::string name;
    while(std::getline(in, name)){
        if(!name.empty()){
            contacts.push_back({ name });
        }
    }
}

void ContactManager::saveToFileInternal(){
    std::ofstream out("storage/contacts.txt", std::ios::trunc);

    if (!out.is_open()) {
        std::cerr << "Error saving contacts\n";
        return;
    }

    for(const auto& c : contacts){
        out << c.name << "\n";
    }
}

bool ContactManager::add(const std::string& name){
    std::string safeName = sanitizeInput(name);
    if(safeName.empty()) return false;

    std::lock_guard<std::mutex> lock(mtx);

    // Avoid duplicates.
    for(const auto& c : contacts){
        if(c.name == name) return false;    
    }

    contacts.push_back({ safeName });
    saveToFileInternal();
    return true;
}

std::vector<Contact> ContactManager::list(){
    std::lock_guard<std::mutex> lock(mtx);
    return contacts;
}

bool ContactManager::remove(const std::string& name){
    std::lock_guard<std::mutex> lock(mtx);

    auto it = std::remove_if(contacts.begin(), contacts.end(), [&](const Contact& c){
        return c.name == name;
    });

    if(it != contacts.end()){
        contacts.erase(it, contacts.end());
        saveToFileInternal();
        return true;
    }

    return false;
}

bool ContactManager::update(const std::string& oldName, const std::string& newName){
    std::string safeNewName = sanitizeInput(newName);
    if(safeNewName.empty()) return false;
    
    std::lock_guard<std::mutex> lock(mtx);

    for(auto& c : contacts){
        if(c.name == oldName){
            c.name = safeNewName;
            saveToFileInternal();
            return true;
        }
    }

    return false;
}