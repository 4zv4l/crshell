# crshell

basic remote shell made in C

# Compile and Usage

- client  
`cc -lreadline -o client client.c`  
`Usage: ./client [ip] [port]`

- server  
`cc -o server server.c`  
`Usage: ./server [ip] [port]`

# reserved command

`exit` will close the connection  
`stop` will close the connection and stop remote server  
