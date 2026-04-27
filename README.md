# Multi-Client Chat Server (C++ / Winsock)

A terminal-based multi-client chat system built in C++ using Winsock.  
The application supports real-time messaging, contact management, and persistent chat history, with a custom text-based protocol.

## Features

- Multi-client support using TCP sockets
- Concurrent client handling with multithreading
- Command-based protocol (e.g., LOGIN, MSG, LIST)
- Contact management system (add, remove, update, list)
- Persistent message history stored in files
- Timestamped messaging system
- Structured and colorized server logs
- Input sanitization for security (basic path traversal prevention)
- Help command for user guidance

## Technologies

- C++
- Winsock2 (Windows Sockets API)
- Multithreading (`std::thread`)
- File system (`<filesystem>`, `<fstream>`)
- Synchronization primitives (`std::mutex`)

## Architecture Overview

The system follows a client-server architecture:

- The server handles multiple clients concurrently using threads.
- Each client communicates with the server through a TCP connection.
- Commands are sent as plain text using a custom protocol:
- COMMAND|arg1|arg2

## Example Commands

LOGIN|Guest
ADD|Guest2
LIST
MSG|Guest2|Hello
HISTORY|Guest2

## How It Works

- The server listens on port `54000`
- Each client connection is handled in a separate thread
- Incoming data is buffered and processed line-by-line (`\n` delimiter)
- Commands are parsed and executed via a centralized handler
- Message history is stored in `storage/history/` as text files

## Logging System

The server includes a structured logging system with:

- Timestamps
- Log levels (INFO, REQUEST, RESPONSE, ERROR, MSG)
- Colorized output for better readability

Example:

[2026-04-27 16:30:00] [REQUEST] Guest1 -> LOGIN|Guest1
[2026-04-27 16:30:00] [RESPONSE] Guest1 <- DONE: Logged in as Guest1

## Available Commands

LOGIN|name -> Login with a username
ADD|name -> Add a contact
LIST -> List all contacts
REMOVE|name -> Remove a contact
UPDATE|old|new -> Update a contact name
MSG|user|message -> Send a message
HISTORY|user -> View message history
HELP -> Show available commands
EXIT -> Disconnect

## Project structure

/server
└── server.cpp
/client
└── client.cpp
/storage
└── history/

---

# Servidor de Chat Multi-Cliente (C++ / Winsock)

Um sistema de chat multi-cliente baseado em terminal, desenvolvido em C++ utilizando Winsock.  
A aplicação suporta mensagens em tempo real, gerenciamento de contatos e histórico persistente, utilizando um protocolo de comunicação baseado em texto.

## Funcionalidades

- Suporte a múltiplos clientes via TCP
- Processamento concorrente com multithreading
- Protocolo baseado em comandos (LOGIN, MSG, LIST, etc.)
- Sistema de gerenciamento de contatos
- Histórico de mensagens persistente em arquivos
- Sistema de mensagens com timestamp
- Logs estruturados e coloridos no servidor
- Sanitização de entrada para segurança básica
- Comando HELP para auxílio ao usuário

## Tecnologias

- C++
- Winsock2 (API de sockets do Windows)
- Multithreading (`std::thread`)
- Manipulação de arquivos (`<filesystem>`, `<fstream>`)
- Sincronização com `std::mutex`

## Visão Geral da Arquitetura

O sistema segue o modelo cliente-servidor:

- O servidor gerencia múltiplos clientes simultaneamente usando threads
- Cada cliente se conecta via TCP
- A comunicação utiliza um protocolo textual:
- COMMAND|arg1|arg2

### Exemplos de Comandos

LOGIN|Guest
ADD|Guest2
LIST
MSG|Guest2|Olá
HISTORY|Guest2

## Funcionamento

- O servidor escuta na porta `54000`
- Cada cliente é tratado em uma thread separada
- Os dados recebidos são acumulados e processados por linha (`\n`)
- Os comandos são interpretados por um handler central
- O histórico é salvo em `storage/history/`

## Sistema de Logs

O servidor possui logs estruturados com:

- Timestamp
- Níveis de log (INFO, REQUEST, RESPONSE, ERROR, MSG)
- Saída colorida para melhor leitura

Exemplo:

[2026-04-27 16:30:00] [REQUEST] Guest1 -> LOGIN|Guest1
[2026-04-27 16:30:00] [RESPONSE] Guest1 <- DONE: Logged in as Guest1

## Comandos Disponíveis

LOGIN|name -> Fazer login
ADD|name -> Adicionar contato
LIST -> Listar contatos
REMOVE|name -> Remover contato
UPDATE|old|new -> Atualizar contato
MSG|user|message -> Enviar mensagem
HISTORY|user -> Ver histórico
HELP -> Mostrar comandos
EXIT -> Desconectar

## Estrutura do Projeto

/server
└── server.cpp
/client
└── client.cpp
/storage
└── history/

## Demo

![Demo](assets/ProgramDemo.mp4)