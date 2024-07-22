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