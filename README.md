# Client server app for message management
The project aims to implement an application that respects the client-server
model for message management. Thus, it is realized multiplexing TCP and UDP
connections using sockets and using a data type for a predefined protocol
over UDP.

##### Contributor: Dumitrascu Filip-Teodor

## Content
1. [TCP client](#tcp-client)
2. [UDP client](#udp-client)
3. [Server](#server)

## TCP client
It can communicate with the server from which it receives one of the subscribe/
unsubscribe commands, server redirected requests, or exit command to disconnect
from the server. By communicating with the server, it subscribes to various
topics and receives at the time of sending by clients UDP, payloads with data
relevant to the chosen topic. Can subscribe to more topics simultaneously. 

## UDP client
They communicate with the server and send payloads of data to it. The data
is parsed to match the topic to which the tcp clients are subscribed to or
may subscribe to.

## Server
Being the intermediary of all actions, it has a multitude of functionalities
performed on various sockets simultaneously. It communicates with the stdin
from where it can disconnect both all clients and itself, with udp_fd from
where it receives data payloads from udp clnents, with tcp_fd for connecting
other tcp clients to the server and with the connected tcp clients for sending
payloads to the topics to which they are subscribed. Clients cannot connect
over active ids and cannot request invalid requests. The server works with an
unlimited number of clients and messages
