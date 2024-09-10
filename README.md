# Webserv

Welcome to **Webserv**, a basic HTTP web server implemented in C++98 as part of the 42 School curriculum. This project provides a foundational understanding of web server operations, including request handling, response generation, and basic network communication.

## Table of Contents

- [About the Project](#about-the-project)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

## About the Project

**Webserv** is a fundamental project designed to deepen understanding of how web servers work by implementing one from scratch in C++98. The objective is to handle basic HTTP requests and serve static files, mimicking fundamental web server functionalities.

### Key Objectives:
- **Request Handling**: Parse and process HTTP GET POST and DELETE requests.
- **Static File Serving**: Serve static files from a specified directory.
- **Socket Programming**: Handle client connections over TCP.
- **C++98 Compliance**: Develop using the C++98 standard to ensure compatibility with older C++ compilers.

## Features

- **Basic HTTP Handling**: Supports HTTP GET POST and DELETE requests.
- **Static File Serving**: Serve files from a directory on the server.
- **Socket Communication**: Handles TCP connections with clients.
- **Request Parsing**: Basic parsing of HTTP requests and generation of responses.
- **Simple Multi-threading**: Handles multiple client connections (if applicable).
- **CGI implementation**: Support CGI scripts.

## Installation

To build and run **Webserv**, follow these steps:

1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/mywebserver.git
    cd webserv
    ```

2. **Compile the project**:
    ```bash
    make
    ```

3. **Run the server**:
    ```bash
    ./webserv <config file>
    ```

## Usage

After running the server, you can test it by sending HTTP requests using `curl` or a web browser. Example `curl` command:

```bash
curl -i -X GET http://localhost:4545/ -H "Host: easy.com"
