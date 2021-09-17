#ifndef __SOCK_WRAP_H__
#define __SOCK_WRAP_H__

#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

namespace ModelSQL 
{    
    const char *default_address = "mysocket";
    const char *server_invitation = "Enter the name of the file that will record the result of the work";
    const char *server_stop = "Input 'END' to stop";
    // SocketException --- Exception class
    class SocketException 
    {
    public:
        std :: string Message;
        enum socket_exception_code 
        {
            ESE_SOCKCREATE,
            ESE_SOCKCONN,
            ESE_SOCKSEND,
            ESE_SOCKRECV,
            ESE_SOCKBIND,
            ESE_SOCKLISTEN,
            ESE_SOCKACCEPT
        };
        SocketException (socket_exception_code);
        void report ();
    };
    
    // BaseSocket --- basic class fot sockets
    class BaseSocket 
    {
    protected:
        int m_Socket;
        struct sockaddr_un m_SockAddr;
    public:
        explicit BaseSocket (int sd = -1, const char *address = NULL);
        
        // sending info
        void Write (void *, int);
        void PutChar (char);
        void PutString (const char *);
        void PutString (const std::string &);
        
        // recieving info
        int Read (void *, int);
        char GetChar ();
        std::string GetString ();
        
        int GetSockDescriptor ();
        ~ BaseSocket () {}
    };
    
    // ServerSocket --- class for server sockets
    class ServerSocket : public BaseSocket 
    {
    public:
        ServerSocket (const char *);
        void Bind ();
        void Listen (const int);
        BaseSocket * Accept ();
        ~ ServerSocket ();
    };

    // ClientSocket --- class for client sockets
    class ClientSocket: public BaseSocket 
    {
    public:
        ClientSocket (const char *);
        void Connect ();
        ~ ClientSocket ();
    };

//------------------------------SocketException methods ------------------------------
    SocketException :: SocketException (socket_exception_code errcode)
    {
        switch (errcode) {
            case ESE_SOCKCREATE:
                Message = "Socket error: server - unsuccessful 'socket'";
                break;
            case ESE_SOCKCONN:
                Message = "Socket error: client - unsuccessful 'connect'";
                break;
            case ESE_SOCKSEND:
                Message = "Socket error: unsuccessful 'send'";
                break;
            case ESE_SOCKRECV:
                Message = "Socket error: unsuccessful 'recv'";
                break;
            case ESE_SOCKBIND:
                Message = "Socket error: server - unsuccessful 'bind'";
                break;
            case ESE_SOCKLISTEN:
                Message = "Socket error errorOR: server - unsuccessful 'listen'";
                break;
            case ESE_SOCKACCEPT:
                Message = "Socket error: server - unsuccessful 'accept'";
                break;
        }
    }
    
    void SocketException :: report ()
    {
        std :: cout << Message << std :: endl << "Code: " << errno << std :: endl;
        return;
    }
    
    
//------------------------------ BaseSocket methods ------------------------------
    BaseSocket :: BaseSocket (int sd, const char * address)
    {
        m_Socket = sd;
        if (address != NULL)
        {
            m_SockAddr.sun_family = AF_UNIX;
            strcpy (m_SockAddr.sun_path, address);
        }
    }
    
    void BaseSocket :: Write (void * buf, int len)
    {
        int s_send = send (m_Socket, buf, len, 0);
        if (s_send == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKSEND);
        }
    }
    
    void BaseSocket :: PutChar (char sym)
    {
        int s_send = send (m_Socket, (char *) &sym, sizeof (char), 0);
        if (s_send == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKSEND);
        }
    }
    
    void BaseSocket :: PutString (const char * str)
    {
        int s_send = send (m_Socket, str, strlen (str) * sizeof (char), 0);
        if (s_send == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKSEND);
        }
        if (str[strlen (str) - 1] != '\n')
            BaseSocket :: PutChar ('\n');
    }
    
    void BaseSocket :: PutString (const std::string &str)
    {
        BaseSocket :: PutString (str.c_str());
    }
    
    int BaseSocket :: Read (void * buf, int len)
    {
        int s_recv = 0;
        while (s_recv == 0)
        {
            s_recv = recv (m_Socket, buf, len, 0);
        }
        if (s_recv == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKRECV);
        }
        return s_recv;
    }
    
    char BaseSocket :: GetChar ()
    {
        char sym;
        int s_recv = 0;
        while (s_recv == 0)
        {
            s_recv = recv (m_Socket, (char *) &sym, sizeof (char), 0);
        }
        if (s_recv == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKRECV);
        }
        return sym;
    }
    
    std::string BaseSocket :: GetString ()
    {
        std :: string str;
        char sym = ' ';
        while ((sym != EOF) && (sym != '\n'))
        {
            sym = BaseSocket :: GetChar ();
            str.push_back (sym);
        }
        return str;
    }
    
    int BaseSocket :: GetSockDescriptor ()
    {
        return m_Socket;
    }
    
//------------------------------ ServerSocket methods ------------------------------
    ServerSocket :: ServerSocket (const char * address)
    {
        // create sockets for one computer
        int sock_fd = socket (AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKCREATE);
        }
        m_Socket = sock_fd;
        m_SockAddr.sun_family = AF_UNIX;
        strcpy (m_SockAddr.sun_path, address);
        Bind ();
        Listen (1);
    }
    
    void ServerSocket :: Bind ()
    {
        int s_bind = bind (m_Socket, (struct sockaddr *) &m_SockAddr, sizeof (m_SockAddr));
        if (s_bind == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKBIND);
        }
    }
    
    void ServerSocket :: Listen (const int backlog)
    {
        int s_listen = listen (m_Socket, backlog);
        if (s_listen == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKLISTEN);
        }
    }
    
    BaseSocket * ServerSocket :: Accept () 
    {
        int sock_fd = accept (m_Socket, NULL, NULL);
        if (sock_fd == -1) 
        {
            throw SocketException(SocketException::ESE_SOCKACCEPT);
        }
        BaseSocket * p_sock = new BaseSocket (sock_fd);
        return p_sock;
    }
    
    ServerSocket :: ~ ServerSocket ()
    {
        // unlink with socket
        shutdown (m_Socket, 2);
        close (m_Socket);
    }


// ------------------------------ ClientSocket methods ------------------------------
    ClientSocket :: ClientSocket (const char * address)
    {
        int sock_fd = socket (AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKCREATE);
        }
        m_Socket = sock_fd;
        m_SockAddr.sun_family = AF_UNIX;
        strcpy (m_SockAddr.sun_path, address);
        Connect ();
    }
    
    void ClientSocket :: Connect ()
    {
        int s_connect = connect (m_Socket, (struct sockaddr *) &m_SockAddr, sizeof (m_SockAddr));
        if (s_connect == -1)
        {
            throw SocketException (SocketException :: ESE_SOCKCONN);
        }
    }
    
    ClientSocket :: ~ ClientSocket ()
    {
        // unlink with socket
        shutdown (m_Socket, 2);
        close (m_Socket);
    }

}; // the end of namespace ModelSQL

#endif
