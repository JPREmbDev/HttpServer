
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

