# crshell
basic remote shell made in C
# Compile and Usage
- client  
`cc -lreadline -o client client.c ../lib/easysocket.c`  
`Usage: ./client [ip] [port]`
- server  
`cc -o server server.c ../lib/easysocket.c`  
`Usage: ./server [ip] [port]`
# Reserved command
- exit  
> will close the connection with the server
> but the server will still be listening
- STOP  
> will close the connection with the server
> and stop the server
