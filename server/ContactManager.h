#ifndef CONTACTMANAGER_H
#define CONTACTMANAGER_H

#include <vector>
#include <string>
#include <mutex>


struct Contact{
    std::string name;
};

class ContactManager{
    private:
        std::vector<Contact> contacts;
        std::mutex mtx;
        std::string sanitizeInput(std::string data);

        void saveToFileInternal();
        void loadFromFile();
        
        public:
        ContactManager();

        bool add(const std::string& name);
        bool remove(const std::string& name);
        bool update(const std::string& oldName, const std::string& newName);
        std::vector<Contact> list();
};

#endif