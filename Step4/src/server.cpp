#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>  // Necesario para std::istringstream
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv) 
{
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  std::cout << "Logs from your program will appear here!\n";

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }
  
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    close(server_fd);
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    close(server_fd);
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    close(server_fd);
    return 1;
  }

  std::cout << "Waiting for clients to connect...\n";

  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
    if (client_fd < 0) {
      std::cerr << "Failed to accept client connection\n";
      continue; // Continúa esperando nuevas conexiones
    }
    
    std::cout << "Client connected\n";

    // Buffer to store the request
    char buffer[1024] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
      std::cerr << "Failed to read from client\n";
      close(client_fd);
      continue; // Continúa esperando nuevas conexiones
    }

    std::cout << "Received request:\n" << buffer << "\n";

    // Extract the request line
    std::string request(buffer);
    std::string request_line = request.substr(0, request.find("\r\n"));
    std::cout << "Request line: " << request_line << "\n";

    // Parse the request line
    std::istringstream request_stream(request_line);
    std::string method, url, version;
    request_stream >> method >> url >> version;

    // Determine the response based on the URL
    std::string http_response;

    // Check if the request is for /echo/{str}
    if (url.find("/echo/") == 0 && method == "GET") {
      std::string echo_str = url.substr(6); // Extract the string after /echo/
      std::string content_length = std::to_string(echo_str.length());

      http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + content_length + "\r\n"
        "\r\n" + 
        echo_str;
    } else if (url == "/" || url == "/index.html") {
      http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    } else {
      http_response = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n"
        "\r\n";
    }

    int bytes_sent = send(client_fd, http_response.c_str(), http_response.length(), 0);
    if (bytes_sent < 0) {
      std::cerr << "Failed to send response to client\n";
      close(client_fd);
      continue; // Continúa esperando nuevas conexiones
    }

    std::cout << "Response sent\n";

    close(client_fd);
  }

  close(server_fd);
  return 0;
}
