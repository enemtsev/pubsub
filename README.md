# Publish-Subscribe Communication Infrastructure

This project implements a basic publish-subscribe communication protocol over TCP/IP using C++ and the Boost.Asio library. The system consists of a server application and a client application that facilitate message exchange based on topic subscriptions.

## Overview

The publish-subscribe architecture is commonly used in distributed systems for efficient data exchange. In this implementation:

- **Server Application**: Manages client connections, handles subscriptions, and forwards messages to the appropriate clients based on their subscribed topics.
- **Client Application**: Connects to the server, subscribes to topics, publishes messages, and receives messages from subscribed topics.

## Features

### Server Application
- Accepts client connections on a specified port.
- Logs client connections and disconnections.
- Handles client subscriptions and unsubscriptions to topics.
- Forwards published messages to clients subscribed to the relevant topics.

### Client Application
- Connects to the server using the `CONNECT` command.
- Disconnects from the server using the `DISCONNECT` command.
- Subscribes to topics using the `SUBSCRIBE` command.
- Unsubscribes from topics using the `UNSUBSCRIBE` command.
- Publishes messages to topics using the `PUBLISH` command.
- Receives and displays messages from subscribed topics.

## Prerequisites

- **C++ Compiler**: Ensure you have a C++ compiler that supports C++20 or later.
- **Boost Library**: Install the Boost library, specifically Boost.Asio and Boost.Log.

## Building the Project

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/enemtsev/pubsub.git
    cd pubsub
    ```

2. **Build Applications**:
    ```bash
    cmake .
    make
    ```

## Running the Applications

### Server Application
Run the server application by specifying the port number:

```bash
./server_app <port>
```

# Example:

```bash
./server_app 12345
```

### Client Application
Run the client application and use the following commands to interact with the server:

# Connect to the server:

```bash
CONNECT <port> <client_name>
```

Example:

```bash
CONNECT 12345 client1
```

# Subscribe to a topic:

```bash
SUBSCRIBE <topic_name>
```

Example:

```bash
SUBSCRIBE weather
```

# Publish a message to a topic:

```bash
PUBLISH <topic_name> <data>
```

Example:

```bash
PUBLISH weather "Sunny with a chance of rain"
```

# Unsubscribe from a topic:

```bash
UNSUBSCRIBE <topic_name>
```

Example:

```bash
UNSUBSCRIBE weather
```

# Disconnect from the server:

```bash
DISCONNECT
```

Display help:

```bash
HELP
```

### Example Usage
Start the server on port 12345:

```bash
./server_app 12345
```

Start the client and connect to the server:

```bash
./client_app
CONNECT 12345 client1
```

Subscribe to a topic:

```bash
SUBSCRIBE weather
```

Publish a message to the topic:

```bash
PUBLISH weather Sunny weather
```

The client will receive and display the message:

```bash
[Message] Topic: weather Data: Sunny weather
```

Unsubscribe from the topic:

```bash
UNSUBSCRIBE weather
```

Disconnect from the server:

```bash
DISCONNECT
```

## Logging
Both the server and client applications use Boost.Log for logging. Logs are printed to the console with timestamps and severity levels.

## Assumptions
- Topic names and data are in ASCII format.
- Topic names do not contain spaces.
- Messages are delimited by a newline character (\n).

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments
- Boost.Asio for network communication.
- Boost.Log for logging.

[![CMake on a single platform](https://github.com/enemtsev/pubsub/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/enemtsev/pubsub/actions/workflows/cmake-single-platform.yml)
