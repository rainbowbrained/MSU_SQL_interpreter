
 #include "sock_wrap.h"
#include "dbms.hpp"
#include "interpret.hpp"
#include <fstream>

using namespace ModelSQL;

class MyServerSocket : public ServerSocket
{
public:
    MyServerSocket() : ServerSocket(default_address) {}
    void OnAccept(BaseSocket *pConn)
    {
        // sending the request for the file name
        pConn->PutString(server_invitation);
        std::string str;
        std::string f_name;
        f_name = pConn->GetString();
        f_name.pop_back();

        pConn->PutString(server_stop);

        while ((str = pConn->GetString()) != "END\n")
        {
            //change standard output for .h-files working
            std::ofstream out(f_name);
            std::streambuf *coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
            try
            {
                std::cerr << "'" << str << "'" << std::endl;
                Interpreter obj(str);
                pConn->PutString("OK"); // get an answer
            }
            catch (SocketException &e)
            {
                // error --- input to the screen
                e.report();
                pConn->PutString (e.Message);
            }
            catch (SQL_Xception &e)
            {
                // error --- input to the screen
                e.report();
                pConn->PutString (e.Message);
            }
            catch (Xception &e)
            {
                // error --- input to the screen
                e.report();
                pConn->PutString (e.Message);
            }
            catch (...)
            {
                std::cerr << "OnAccept: unknown error" << std::endl;
            }
            std::cout.rdbuf(coutbuf);
        }
        pConn->PutString("END");
        delete pConn;
        remove (default_address);
        // delete mysocket
    }
};

int main()
{
    try
    {
        MyServerSocket sock;
        sock.OnAccept(sock.Accept());
    }
    catch (SocketException &e)
    {
        // error --- input to the screen
        e.report();
    }
    return 0;
}
