
In this stage, you'll create a TCP server that listens on port 4221.

TCP[https://www.cloudflare.com/en-ca/learning/ddos/glossary/tcp-ip/] is the underlying protocol used by HTTP servers.

Tests
The tester will execute your program like this:

$ ./your_program.sh
Then, the tester will try to connect to your server on port 4221. The connection must succeed for you to pass this stage.



[![progress-banner](https://backend.codecrafters.io/progress/http-server/2dd9a1db-0e87-45b4-a734-7d8266fad123)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

This is a starting point for C++ solutions to the
["Build Your Own HTTP server" Challenge](https://app.codecrafters.io/courses/http-server/overview).

[HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol) is the
protocol that powers the web. In this challenge, you'll build a HTTP/1.1 server
that is capable of serving multiple clients.

Along the way you'll learn about TCP servers,
[HTTP request syntax](https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html),
and more.

**Note**: If you're viewing this repo on GitHub, head over to
[codecrafters.io](https://codecrafters.io) to try the challenge.

# Passing the first stage

The entry point for your HTTP server implementation is in `src/server.cpp`.
Study and uncomment the relevant code, and push your changes to pass the first
stage:

```sh
git add .
git commit -m "pass 1st stage" # any msg
git push origin master
```

Time to move on to the next stage!


Vamos a analizar y comentar detalladamente el código proporcionado. Este programa en C++ configura un servidor básico que escucha en un puerto determinado y acepta una conexión de cliente.

### Análisis del Código ###

```cpp
    #include <iostream>
    #include <cstdlib>
    #include <string>
    #include <cstring>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
```
Estos son los encabezados necesarios:
    - `iostream`: Para operaciones de entrada y salida.
    - `cstdlib`: Para funciones generales como `exit`.
    - `string`: Para utilizar la clase `std::string`.
    - `cstring`: Para operaciones con cadenas de estilo C.
    - `unistd.h`: Para funciones POSIX como `close`.
    - `sys/types.h`, `sys/socket.h`, `arpa/inet.h`, `netdb.h`: Para operaciones de sockets.

```cpp
int main(int argc, char **argv)
{
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
```
- `std::unitbuf` asegura que `std::cout` y `std::cerr` se vacíen (flush) después de cada operación, útil para ver los mensajes de depuración inmediatamente.

```cpp
  std::cout << "Logs from your program will appear here!\n";
```
- Mensaje de depuración para confirmar que el programa ha iniciado.

```cpp
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
```
- Crea un socket usando IPv4 (`AF_INET`) y TCP (`SOCK_STREAM`). Si `socket` retorna un valor negativo, indica un error y el programa termina.

```cpp
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
```
- Configura el socket para reutilizar la dirección (`SO_REUSEADDR`), lo cual es útil para evitar errores de "dirección ya en uso" al reiniciar el programa frecuentemente.

```cpp
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
```
- Define la estructura `sockaddr_in` para el servidor, configurándola para aceptar conexiones en cualquier dirección (`INADDR_ANY`) y en el puerto 4221 (`htons` convierte el número de puerto al orden de bytes de red).

```cpp
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
```
- Asocia el socket con la dirección y puerto especificados. Si `bind` falla, el programa termina.

```cpp
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
```
- Marca el socket como un socket pasivo que aceptará conexiones entrantes, con un backlog de 5 conexiones pendientes.

```cpp
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";
```
- Espera y acepta una conexión de cliente. La llamada `accept` es bloqueante hasta que un cliente se conecta. Imprime un mensaje cuando un cliente se conecta.

```cpp
  close(server_fd);

  return 0;
}
```
- Cierra el socket del servidor y finaliza el programa.

### Resumen
Este código configura un servidor básico en C++ que:
1. Crea un socket TCP.
2. Configura opciones del socket para reutilización de la dirección.
3. Asocia el socket a una dirección y puerto específicos.
4. Marca el socket como pasivo para aceptar conexiones.
5. Espera y acepta una conexión de cliente.
6. Cierra el socket y termina.

