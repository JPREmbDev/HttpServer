Respond with body

In this stage, you'll implement the /echo/{str} endpoint, which accepts a string and returns it in the response body.

Response body
A response body is used to return content to the client. This content may be an entire web page, a file, a string, or anything else that can be represented with bytes.

Your /echo/{str} endpoint must return a 200 response, with the response body set to given string, and with a Content-Type and Content-Length header.

Here's an example of an /echo/{str} request:

GET /echo/abc HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n
And here's the expected response:

HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\nabc
Here's a breakdown of the response:

// Status line
HTTP/1.1 200 OK
\r\n                          // CRLF that marks the end of the status line

// Headers
Content-Type: text/plain\r\n  // Header that specifies the format of the response body
Content-Length: 3\r\n         // Header that specifies the size of the response body, in bytes
\r\n                          // CRLF that marks the end of the headers

// Response body
abc                           // The string from the request
The two headers are required for the client to be able to parse the response body. Note that each header ends in a CRLF, and the entire header section also ends in a CRLF.

Tests
The tester will execute your program like this:

$ ./your_program.sh
The tester will then send a GET request to the /echo/{str} endpoint on your server, with some random string.

$ curl -v http://localhost:4221/echo/abc
Your server must respond with a 200 response that contains the following parts:

Content-Type header set to text/plain.
Content-Length header set to the length of the given string.
Response body set to the given string.
HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\nabc
Notes
For more information about HTTP responses, see the MDN Web Docs on HTTP responses
(https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages#http_responses)
or the HTTP/1.1 specification (https://datatracker.ietf.org/doc/html/rfc9112#name-message).

Para implementar el endpoint `/echo/{str}` que acepta una cadena y la devuelve en el cuerpo de la respuesta con una respuesta HTTP 200, necesitamos modificar el código del servidor para extraer la cadena de la URL y luego construir una respuesta con las cabeceras `Content-Type` y `Content-Length`.

### Código del Servidor Actualizado (mi_servidor.cpp)

Vamos a actualizar el código del servidor para manejar el endpoint `/echo/{str}`.

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
```

### Explicación de los Cambios

1. **Extracción de la cadena después de `/echo/`:**
   ```cpp
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
   ```
   - Si la URL comienza con `/echo/` y el método es `GET`, extraemos la cadena después de `/echo/`.
   - Construimos la respuesta HTTP 200 con `Content-Type: text/plain` y `Content-Length` igual a la longitud de la cadena extraída.
   - Si la URL es `/` o `/index.html`, respondemos con un 200 OK.
   - Para cualquier otra URL, respondemos con un 404 Not Found.

### Compilación y Ejecución

1. **Compilar el servidor:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ```

2. **Ejecutar el servidor en una terminal:**
   ```sh
   ./mi_servidor
   ```

3. **Probar la conexión con `curl`:**
   - Para probar el endpoint `/echo/{str}`:
     ```sh
     curl -v http://localhost:4221/echo/abc
     ```
     Deberías ver una respuesta HTTP 200 OK con el cuerpo `abc`.

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

Con estos cambios, tu servidor debería ahora manejar correctamente el endpoint `/echo/{str}` y responder con la cadena dada en el cuerpo de la respuesta.

**Sponsored**
```
[API response here]
```