In this stage, your server will respond to an HTTP request with a 200 response.

HTTP response
An HTTP response is made up of three parts, each separated by a CRLF (\r\n):

1.- Status line.
2.- Zero or more headers, each ending with a CRLF.
3.- Optional response body.
In this stage, your server's response will only contain a status line. Here's the response your server must send:

HTTP/1.1 200 OK\r\n\r\n
Here's a breakdown of the response:

// Status line
HTTP/1.1  // HTTP version
200       // Status code
OK        // Optional reason phrase
\r\n      // CRLF that marks the end of the status line

// Headers (empty)
\r\n      // CRLF that marks the end of the headers

// Response body (empty)

Tests
The tester will execute your program like this:

$ ./your_program.sh
The tester will then send an HTTP GET request to your server:

$ curl -v http://localhost:4221
Your server must respond to the request with the following response:

HTTP/1.1 200 OK\r\n\r\n

Notes
You can ignore the contents of the request. We'll cover parsing requests in later stages.
For more information about HTTP responses, see the MDN Web Docs on HTTP responses
(https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages#http_responses) or 
the HTTP/1.1 specification (https://datatracker.ietf.org/doc/html/rfc9112#name-message).
This challenge uses HTTP/1.1.

Para asegurarnos de que el servidor sigue escuchando después de enviar la respuesta HTTP 200 OK, debemos modificar el bucle principal del programa para que siga esperando nuevas conexiones después de cerrar la conexión con un cliente. Aquí tienes una versión mejorada del código que permite al servidor aceptar múltiples conexiones de clientes en lugar de cerrarse después de la primera:

### Código del Servidor (mi_servidor.cpp)

```cpp
#include <iostream>
#include <cstdlib>
#include <cstring>
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

    // HTTP response
    const char *http_response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

    int bytes_sent = send(client_fd, http_response, strlen(http_response), 0);
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

1. **Bucle infinito para aceptar múltiples conexiones:**
   - Se ha añadido un bucle `while (true)` para que el servidor siga esperando y aceptando nuevas conexiones después de cerrar la conexión con un cliente.
   
2. **Manejo de errores sin cerrar el servidor:**
   - En caso de errores al aceptar una conexión (`accept`) o al leer datos (`read`), el servidor imprime un mensaje de error y continúa esperando nuevas conexiones sin cerrarse.
   
3. **Continuar aceptando conexiones después de enviar una respuesta:**
   - Después de enviar la respuesta HTTP 200 OK, el servidor cierra la conexión con el cliente (`close(client_fd)`) y vuelve a esperar nuevas conexiones.

### Ejecución y Prueba

1. **Compila y ejecuta el servidor en una terminal:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ./mi_servidor
   ```

2. **Usa `curl` en otra terminal para enviar solicitudes al servidor:**
   ```sh
   curl -v http://localhost:4221
   ```

El servidor ahora debería seguir ejecutándose y aceptando nuevas conexiones después de cada solicitud, permitiéndote probar múltiples conexiones con `curl` o cualquier otro cliente HTTP.

El resultado del comando `netstat -tuln | grep 4221` indica que tu servidor está escuchando correctamente en el puerto 4221. Ahora, el problema de "Connection refused" debería haberse resuelto, y deberías poder conectar al servidor usando `curl`. 

Si sigues teniendo problemas, podrías probar los siguientes pasos adicionales para asegurarte de que todo funciona correctamente:

### Verificación Adicional

1. **Asegúrate de que el servidor sigue ejecutándose:**
   Confirma que el servidor sigue ejecutándose y está esperando conexiones. Esto lo puedes verificar si ves mensajes como "Waiting for clients to connect..." en la terminal donde ejecutaste `./mi_servidor`.

2. **Prueba de Conexión con `curl`:**
   Intenta conectar nuevamente usando `curl`:
   ```sh
   curl -v http://localhost:4221
   ```
   Deberías obtener una respuesta similar a esta:
   ```
   *   Trying 127.0.0.1:4221...
   * Connected to localhost (127.0.0.1) port 4221 (#0)
   > GET / HTTP/1.1
   > Host: localhost:4221
   > User-Agent: curl/7.68.0
   > Accept: */*
   > 
   < HTTP/1.1 200 OK
   < Content-Length: 0
   < 
   * Connection #0 to host localhost left intact
   ```

### Verifica y Ejecución del Código del Servidor

Asegúrate de que tu archivo de código `mi_servidor.cpp` esté compilado y ejecutándose correctamente. Aquí está el código completo nuevamente para referencia:

```cpp
#include <iostream>
#include <cstdlib>
#include <cstring>
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

    // HTTP response
    const char *http_response = 
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

    int bytes_sent = send(client_fd, http_response, strlen(http_response), 0);
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

### Compilación y Ejecución

1. **Compila el servidor:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ```

2. **Ejecuta el servidor en una terminal:**
   ```sh
   ./mi_servidor
   ```
   Deberías ver "Logs from your program will appear here!" seguido de "Waiting for clients to connect...".

3. **Prueba la conexión con `curl` en otra terminal:**
   ```sh
   curl -v http://localhost:4221
   ```

### Solución Alternativa: Verificar Firewall y Seguridad

Asegúrate de que no haya un firewall o configuración de seguridad que esté bloqueando las conexiones al puerto 4221. Puedes intentar desactivar temporalmente el firewall para probar la conexión.

### Conclusión

Estos pasos deberían ayudarte a diagnosticar y resolver el problema. Si sigues teniendo problemas, revisa cualquier mensaje de error que aparezca en la terminal donde ejecutas el servidor y asegúrate de que el servidor sigue ejecutándose y esperando conexiones.

