Extract URL path

In this stage, your server will extract the URL path from an HTTP request, and respond with either a 200 or 404, depending on the path.

HTTP request
An HTTP request is made up of three parts, each separated by a CRLF (\r\n):

1.- Request line.
2.- Zero or more headers, each ending with a CRLF.
3.- Optional request body.

Here's an example of an HTTP request:

GET /index.html HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n

Here's a breakdown of the request:

// Request line
GET                          // HTTP method
/index.html                  // Request target
HTTP/1.1                     // HTTP version
\r\n                         // CRLF that marks the end of the request line

// Headers
Host: localhost:4221\r\n     // Header that specifies the server's host and port
User-Agent: curl/7.64.1\r\n  // Header that describes the client's user agent
Accept: */*\r\n              // Header that specifies which media types the client can accept
\r\n                         // CRLF that marks the end of the headers

// Request body (empty)
The "request target" specifies the URL path for this request. In this example, the URL path is /index.html.

Note that each header ends in a CRLF, and the entire header section also ends in a CRLF.

Tests
The tester will execute your program like this:

$ ./your_program.sh
The tester will then send two HTTP requests to your server.

First, the tester will send a GET request, with a random string as the path:

$ curl -v http://localhost:4221/abcdefg
Your server must respond to this request with a 404 response:

HTTP/1.1 404 Not Found\r\n\r\n
Then, the tester will send a GET request, with the path /:

$ curl -v http://localhost:4221
Your server must respond to this request with a 200 response:

HTTP/1.1 200 OK\r\n\r\n
Notes
You can ignore the headers for now. You'll learn about parsing headers in a later stage.
In this stage, the request target is written as a URL path. But the request target actually has four possible formats.
(https://datatracker.ietf.org/doc/html/rfc9112#section-3.2)
The URL path format is called the "origin form," and it's the most commonly used format. The other formats are used for more niche scenarios, 
like sending a request through a proxy.
For more information about HTTP requests, see the MDN Web Docs on HTTP requests
(https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages#http_requests)
or the HTTP/1.1 specification (https://datatracker.ietf.org/doc/html/rfc9112#name-message).


Para desarrollar la funcionalidad que mencionas, necesitamos extraer la ruta URL de una petición HTTP y responder con un código 200 si la ruta es `/index.html` o con un código 404 si no lo es. 

### Pasos a Seguir

1. **Leer la solicitud del cliente y extraer la línea de petición.**
2. **Analizar la línea de petición para obtener la ruta URL.**
3. **Responder con un código 200 si la ruta es `/index.html` o con un código 404 en caso contrario.**

### Código del Servidor Actualizado

Aquí está el código completo actualizado con las nuevas funcionalidades:

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

    // Determine the response based on the URL
    std::string http_response;
    if (url == "/" || url == "/index.html") {
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

```

### Explicación del Código

1. **Lectura de la solicitud del cliente:**
   ```cpp
   char buffer[1024] = {0};
   int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
   ```
   - Leemos la solicitud del cliente en un buffer.

2. **Extracción de la línea de petición:**
   ```cpp
   std::string request(buffer);
   std::string request_line = request.substr(0, request.find("\r\n"));
   std::cout << "Request line: " << request_line << "\n";
   ```
   - Convertimos el buffer en un `std::string` y extraemos la línea de petición.

3. **Análisis de la línea de petición:**
   ```cpp
   std::istringstream request_stream(request_line);
   std::string method, url, version;
   request_stream >> method >> url >> version;
   ```
   - Utilizamos un `std::istringstream` para analizar la línea de petición y extraer el método, la URL y la versión HTTP.

4. **Determinación de la respuesta en función de la URL:**
   ```cpp
   std::string http_response;
   if (url == "/index.html") {
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
   ```
   - Dependiendo de la URL, preparamos la respuesta HTTP correspondiente.

5. **Envío de la respuesta al cliente:**
   ```cpp
   int bytes_sent = send(client_fd, http_response.c_str(), http_response.length(), 0);
   if (bytes_sent < 0) {
     std::cerr << "Failed to send response to client\n";
     close(client_fd);
     continue; // Continúa esperando nuevas conexiones
   }

   std::cout << "Response sent\n";
   ```
   - Enviamos la respuesta al cliente y cerramos la conexión.

6. **Cierre de la conexión:**
   ```cpp
   close(client_fd);
   ```
   - Cerramos la conexión con el cliente y volvemos a esperar nuevas conexiones.

### Compilación y Ejecución

1. **Compilar el servidor:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ```

2. **Ejecutar el servidor en una terminal:**
   ```sh
   ./mi_servidor
   ```
   Deberías ver "Logs from your program will appear here!" seguido de "Waiting for clients to connect...".

3. **Probar la conexión con `curl`:**
   - Para una ruta válida:
     ```sh
     curl -v http://localhost:4221/index.html
     ```
     Deberías ver una respuesta HTTP 200 OK.

   - Para una ruta no válida:
     ```sh
     curl -v http://localhost:4221/invalid.html
     ```
     Deberías ver una respuesta HTTP 404 Not Found.

Este código ahora debería manejar correctamente las solicitudes HTTP y responder con el código de estado adecuado dependiendo de la ruta solicitada.
