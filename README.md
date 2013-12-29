**Router Simulator**
====================



This simulation is used to verify **Reliable UDP Implementations** such as *Go
Back N* or *Sliding Window*. When it receives packets from a peer (either client
or server), it can drop, delay or deliver the packet to the other side. The goal
of this simulation is to be able to control the network quality without
modifying the implementation at peers.



**How does it work?**
---------------------

1.  The Router will be waiting for UDP packet at a specific port (ROUTER_PORT)

2.  You will be asked to provide the Host Name & Port Number of the UDP Server
    Application

3.  All packets between your server & client will be through the Router instead
    of exchanging directly



**Structure**
-------------

There will be three kinds of socket & the virtual path

1.  **Router Socket:** This socket is on behalf of the Main UDP Server Socket

2.  **Client Socket:** The socket is created when a new packet from Client
    arrives at Router Socket. This socket is simulated the UDP Socket from
    Server Application.

3.  **Server Socket:** The socket is simulated the Client Application Socket

4.  **Virtual Path:** a set of path from Client to Router & Router to Server.
    Each client endpoint will have its own virtual path. (See issue)

**Communication**
-----------------

-   The Client Socket is used to send & receive packets from client

-   The Server Socket is used to send & receive packets from server

-   The Router Socket is used to receive packets from server



**Communication Flow**
----------------------

**1. Router Socket**

For each packet (from client) arrives at the Router Socket, the Router will
check if there is a virtual path for this endpoint or not. If there is no
virtual path for this endpoint, it will create new virtual path for this end
point. Once the virtual path is created (or found if existing),the Router ask
the Virtual Path to process (drop, delay or deliver) the packet to The Main
Server Socket

**2. Client Socket**

For each incoming packets from client, this socket first updates the client
address, then ask the Virtual Path process this packet to the latest server
address

The latest server address can be:

-   Default Server Endpoint if Server uses one endpoint

-   Child Server Endpoint if Server uses multiple endpoints

**3. Server Socket**

For each incoming packets from server, this socket first updates server address,
then ask the Virtual Path process (drop, delay or deliver) this packet to the
latest client address.The latest client address usually is the initialized
address but users can change.



**Issues** 
-----------

1.  Currently,the Router is unable to release inactive virtual paths. However,
    this Router won't be running for a long time, such leaks is still acceptable








