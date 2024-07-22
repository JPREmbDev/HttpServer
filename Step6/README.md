Concurrent connections 

In this stage, you'll add support for concurrent connections.

Tests
The tester will execute your program like this:

$ ./your_program.sh
Then, the tester will create multiple concurrent TCP connections to your server. (The exact number of connections is determined at random.) After that, the tester will send a single GET request through each of the connections.

$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
$ (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
Your server must respond to each request with the following response:

HTTP/1.1 200 OK\r\n\r\n


Para manejar conexiones concurrentes en tu servidor, una manera efectiva es utilizar el modelo de multiprocesamiento o multihilos. Para esta implementación, vamos a utilizar el enfoque de multiprocesamiento, donde cada conexión de cliente se maneja en un proceso hijo separado. 

### Código del Servidor Actualizado para Conexiones Concurrentes

Aquí tienes el código del servidor actualizado para manejar conexiones concurrentes usando `fork` para crear un proceso hijo para cada conexión entrante:

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

void handle_client(int client_fd) {
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
    return;
  }

  std::cout << "Response sent\n";

  close(client_fd);
}

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

    pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "Failed to fork\n";
      close(client_fd);
      continue; // Continúa esperando nuevas conexiones
    }

    if (pid == 0) {  // Proceso hijo
      close(server_fd);  // El proceso hijo no necesita el socket del servidor
      handle_client(client_fd);
      exit(0);  // Termina el proceso hijo después de manejar el cliente
    } else {  // Proceso padre
      close(client_fd);  // El proceso padre no necesita el socket del cliente
    }
  }

  close(server_fd);
  return 0;
}
```

### Explicación de los Cambios

1. **Función `handle_client`:**
   - Esta función maneja la interacción con un cliente, lee la solicitud y envía la respuesta. Es la misma lógica que teníamos antes pero separada en una función para manejar la concurrencia.

2. **Uso de `fork`:**
   ```cpp
   pid_t pid = fork();
   if (pid < 0) {
     std::cerr << "Failed to fork\n";
     close(client_fd);
     continue; // Continúa esperando nuevas conexiones
   }

   if (pid == 0) {  // Proceso hijo
     close(server_fd);  // El proceso hijo no necesita el socket del servidor
     handle_client(client_fd);
     exit(0);  // Termina el proceso hijo después de manejar el cliente
   } else {  // Proceso padre
     close(client_fd);  // El proceso padre no necesita el socket del cliente
   }
   ```
   - `fork` crea un nuevo proceso. El proceso hijo maneja la conexión del cliente, mientras que el proceso padre continúa esperando nuevas conexiones.
   - En el proceso hijo, cerramos el socket del servidor porque no es necesario en este contexto.
   - En el proceso padre, cerramos el socket del cliente porque el cliente está siendo manejado por el proceso hijo.

### Compilación y Ejecución

1. **Compila el servidor:**
   ```sh
   g++ -o mi_servidor mi_servidor.cpp
   ```

2. **Ejecuta el servidor en una terminal:**
   ```sh
   ./mi_servidor
   ```

3. **Probar con conexiones concurrentes:**
   - Usa el siguiente comando para probar múltiples conexiones simultáneamente:
     ```sh
     (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
     (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
     (sleep 3 && printf "GET / HTTP/1.1\r\n\r\n") | nc localhost 4221 &
     ```
   - Esto simula tres clientes conectándose al servidor de forma concurrente y enviando solicitudes GET.

Con estos cambios, tu servidor debería ser capaz de manejar múltiples conexiones concurrentes, respondiendo correctamente a cada una con una respuesta HTTP 200 OK.
