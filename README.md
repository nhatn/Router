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



**Packet Flow**
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

Currently,the Router is unable to release inactive virtual paths. The problem is if there are too many inactive connections (virtual paths), there may not be enough the resource for the new connection.The issue is not related to how long the virtual path will be used but the number of unused connections. 
However,this Router won't be running for a long time with too many connections, such leaks is still acceptable.
With current computer resource, the Router will be able to handle at least several thousand virtual paths.

There is no problem if client retries to use its own address. In such case, the existing virtual path will be reused & refreshed.

**Usage**
---------

### **Router Argument**

You can specific the arguments for Router application. Here is its usage:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Router.exe --routerport 5000 --servername localhost --serverport 5001 --delay 5 --drop 5 --loglevel debug
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With:

-   *routerport *is the port that the router will be listening on

-   *servername *is the host name of the server application

-   *serverport *is the port number of the server application

-   *delay *is the delay percentage

-   *drop *is the drop percentage

-   *loglevel *is either DEBUG, INFO or ERROR

The *servername*, *delay *& *drop *parameters are mandatory, thus if they are
not provided, you will be asked to provide them through console



### **Logging**

In order to use the Log facility, you have to following these steps

1.  Include the Log.h & Logstream.h headers to your application

2.  Add the Log.cpp & Logstream.cpp implementation to your application

3.  Configure the Logging System

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    LogManager::SharedManager().SetLogFileName("Router.log")
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The log file that the log will be written to

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    LogManager::SharedManager().SetLogConsole(true)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Show log on console or not

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    LogManager::SharedManager().SetLogLevel(parameter->loglevel); 
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The log level can be LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_INFO or
    LOG_LEVEL_DEBUG

    

4.  Using the Macros to log

    -   LOG_DEBUG

    -   LOG_INFO

    -   LOG_ERROR



### **UDPSocket**

The simple implementation of UDPSocket can be found at UDPSocket.h &
UDPSocket.cpp. You can use this implementation for any purpose

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool Bind(int localPort)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Bind the UDP Socket to a specific port, so that you can receive packets that
arrived on that port



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int Send(const charbuffer,unsigned int sz, SOCKADDR_IN destination)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Send packet to peer (specified by destination address)



~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int Receive(char buffer,unsigned int sz, SOCKADDR_IN sender, int timeout = -1)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Receive packet at the binding port, the sender address is out parameter (will be
filled out). Time out is in milliseconds
