# Publish-Subscribe System

This project implements a basic publish-subscribe communication infrastructure using **Boost.Asio** for networking. The system consists of a server and a client that communicate over TCP/IP. The server manages client connections and forwards messages to subscribers, while the client can publish, subscribe, and unsubscribe from topics.

## Features

- **Server**:
  - Accepts client connections on a specified port.
  - Forwards `PUBLISH` messages to clients subscribed to the topic.
  - Logs client connections, disconnections, and message handling.

- **Client**:
  - Connects to the server and sends `CONNECT`, `PUBLISH`, `SUBSCRIBE`, and `UNSUBSCRIBE` messages.
  - Receives and displays messages from subscribed topics.
  - Logs all actions and errors using standard I/O (`std::cout` and `std::cerr`).

## Prerequisites

- **Boost Library**: Ensure Boost is installed on your system.
- **CMake**: Required for building the project.
- **C++ Compiler**: Supports C++17 or later.

## Building the Project

1. Clone the repository:
   ```bash
   https://github.com/enemtsev/pubsub.git
   cd pubsub