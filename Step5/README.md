Read header

In this stage, you'll implement the /user-agent endpoint, which reads the User-Agent request header and returns it in the response body.

The User-Agent header
The User-Agent header describes the client's user agent.

Your /user-agent (https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/User-Agent) endpoint must read the User-Agent header, 
and return it in your response body. Here's an example of a /user-agent request:

// Request line
GET
/user-agent
HTTP/1.1
\r\n

// Headers
Host: localhost:4221\r\n
User-Agent: foobar/1.2.3\r\n  // Read this value
Accept: */*\r\n
\r\n

// Request body (empty)

Here is the expected response:

// Status line
HTTP/1.1 200 OK               // Status code must be 200
\r\n

// Headers
Content-Type: text/plain\r\n
Content-Length: 12\r\n
\r\n

// Response body
foobar/1.2.3                  // The value of `User-Agent`
Tests
The tester will execute your program like this:

$ ./your_program.sh
The tester will then send a GET request to the /user-agent endpoint on your server. The request will have a User-Agent header.

$ curl -v --header "User-Agent: foobar/1.2.3" http://localhost:4221/user-agent
Your server must respond with a 200 response that contains the following parts:

Content-Type header set to text/plain.
Content-Length header set to the length of the User-Agent value.
Message body set to the User-Agent value.
HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nfoobar/1.2.3
Notes
Header names are case-insensitive (https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax).

Para implementar el endpoint `/user-agent`, que lee el encabezado de solicitud `User-Agent` y lo devuelve en el cuerpo de la respuesta, necesitamos hacer los siguientes cambios en el código del servidor:

1. **Leer la cabecera `User-Agent` de la solicitud.**
2. **Responder con un código 200 y el valor del `User-Agent` en el cuerpo de la respuesta.**

### Código del Servidor Actualizado (mi_servidor.cpp)

Aquí está el código actualizado para manejar el endpoint `/user-agent`:

```cpp
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

    // Initialize variables for User-Agent header
    std::string user_agent;

    // Find the User-Agent header
    size_t user_agent_pos = request.find("User-Agent: ");
    if (user_agent_pos != std::string::npos) {
      size_t user_agent_end = request.find("\r\n", user_agent_pos);
      user_agent = request.substr(user_agent_pos + 12, user_agent_end - user_agent_pos - 12);
    }

    // Determine the response based on the URL
    std::string http_response;

    // Check if the request is for /user-agent
    if (url == "/user-agent" && method == "GET") {
      std::string content_length = std::to_string(user_agent.length());

      http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + content_length + "\r\n"
        "\r\n" + 
        user_agent;
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
```

### Explicación de los Cambios

1. **Extracción de la cabecera `User-Agent`:**
   ```cpp
   // Initialize variables for User-Agent header
   std::string user_agent;

   // Find the User-Agent header
   size_t user_agent_pos = request.find("User-Agent: ");
   if (user_agent_pos != std::string::npos) {
     size_t user_agent_end = request.find("\r\n", user_agent_pos);
     user_agent = request.substr(user_agent_pos + 12, user_agent_end - user_agent_pos - 12);
   }
   ```
   - Buscamos la cabecera `User-Agent` en la solicitud y la extraemos.

2. **Respuesta basada en la URL `/user-agent`:**
   ```cpp
   // Check if the request is for /user-agent
   if (url == "/user-agent" && method == "GET") {
     std::string content_length = std::to_string(user_agent.length());

     http_response = 
       "HTTP/1.1 200 OK\r\n"
       "Content-Type: text/plain\r\n"
       "Content-Length: " + content_length + "\r\n"
       "\r\n" + 
       user_agent;
   }
   ```
   - Si la URL es `/user-agent` y el método es `GET`, construimos una respuesta HTTP 200 con el valor de `User-Agent` en el cuerpo de la respuesta.

### Compilación y Ejecución

1. **Compila el servidor:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ```

2. **Ejecuta el servidor en una terminal:**
   ```sh
   ./mi_servidor
   ```

3. **Probar la conexión con `curl`:**
   - Para probar el endpoint `/user-agent`:
     ```sh
     curl -v http://localhost:4221/user-agent
     ```
     Deberías ver una respuesta HTTP 200 OK con el valor de la cabecera `User-Agent` en el cuerpo de la respuesta.

   - Para la ruta raíz (`/`):
     ```sh
     curl -v http://localhost:4221/
     ```
     Deberías ver una respuesta HTTP 200 OK.

   - Para una ruta válida (`/index.html`):
     ```sh
     curl -v http://localhost:4221/index.html
     ```
     Deberías ver una respuesta HTTP 200 OK.

   - Para una ruta no válida:
     ```sh
     curl -v http://localhost:4221/invalid.html
     ```
     Deberías ver una respuesta HTTP 404 Not Found.

Con estos cambios, tu servidor debería ahora manejar correctamente el endpoint `/user-agent` y responder con el valor de la cabecera `User-Agent` en el cuerpo de la respuesta.
