#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>  // Necesario para std::istringstream
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>  // Necesario para std::ifstream y std::ofstream
#include <sys/stat.h>

void handle_client(int client_fd, const std::string& base_directory) {
  // Buffer to store the request
  char buffer[1024] = {0};
  int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
  if (bytes_read < 0) {
    std::cerr << "Failed to read from client\n";
    close(client_fd);
    return;
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

  // Extract headers and body
  size_t headers_end = request.find("\r\n\r\n");
  std::string headers = request.substr(0, headers_end);
  std::string body = request.substr(headers_end + 4);

  // Initialize variables for User-Agent header
  std::string user_agent;

  // Find the User-Agent header
  size_t user_agent_pos = headers.find("User-Agent: ");
  if (user_agent_pos != std::string::npos) {
    size_t user_agent_end = headers.find("\r\n", user_agent_pos);
    user_agent = headers.substr(user_agent_pos + 12, user_agent_end - user_agent_pos - 12);
  }

  // Determine the response based on the URL
  std::string http_response;

  // Check if the request is for /files/{filename}
  if (url.find("/files/") == 0) {
    std::string filename = url.substr(7); // Extract the filename after /files/
    std::string filepath = base_directory + "/" + filename;

    if (method == "GET") {
      std::ifstream file(filepath, std::ios::binary);
      if (file) {
        // Get the size of the file
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read the file content
        std::string file_content(file_size, '\0');
        file.read(&file_content[0], file_size);

        http_response = 
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/octet-stream\r\n"
          "Content-Length: " + std::to_string(file_size) + "\r\n"
          "\r\n" + 
          file_content;
      } else {
        http_response = 
          "HTTP/1.1 404 Not Found\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      }
    } else if (method == "POST") {
      std::ofstream file(filepath, std::ios::binary);
      if (file) {
        file.write(body.c_str(), body.size());
        http_response = 
          "HTTP/1.1 201 Created\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      } else {
        http_response = 
          "HTTP/1.1 500 Internal Server Error\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      }
    }
  } else if (url == "/" || url == "/index.html") {
    http_response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n";
  } else if (url.find("/echo/") == 0 && method == "GET") {
    std::string echo_str = url.substr(6); // Extract the string after /echo/
    std::string content_length = std::to_string(echo_str.length());

    http_response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: " + content_length + "\r\n"
      "\r\n" + 
      echo_str;
  } else if (url == "/user-agent" && method == "GET") {
    std::string content_length = std::to_string(user_agent.length());

    http_response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: " + content_length + "\r\n"
      "\r\n" + 
      user_agent;
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
    return;
  }

  std::cout << "Response sent\n";

  close(client_fd);
}

int main(int argc, char **argv) 
{
  std::string base_directory = "/tmp";  // Default directory

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--directory" && i + 1 < argc) {
      base_directory = argv[++i];
    }
  }

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

    pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "Failed to fork\n";
      close(client_fd);
      continue; // Continúa esperando nuevas conexiones
    }

    if (pid == 0) {  // Proceso hijo
      close(server_fd);  // El proceso hijo no necesita el socket del servidor
      handle_client(client_fd, base_directory);
      exit(0);  // Termina el proceso hijo después de manejar el cliente
    } else {  // Proceso padre
      close(client_fd);  // El proceso padre no necesita el socket del cliente
    }
  }

  close(server_fd);
  return 0;
}
