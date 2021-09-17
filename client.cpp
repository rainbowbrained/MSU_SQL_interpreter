#include "sock_wrap.h"

using namespace ModelSQL;

int main()
{
    try
    {
        ClientSocket sock(default_address);
        std::cout << "(Client) Read from server: " << sock.GetString();
        std::string str;
        // sending a message to server
        getline(std::cin, str, '\n');
        sock.PutString(str);
        // new message from the server
        std::cout << "(Client) Read from server: " << sock.GetString() << std::endl;

        while (str != "END")
        {
            getline(std::cin, str, '\n');
            sock.PutString(str);
            std::cout << "(Client) Read from server: " << sock.GetString() << std::endl;
        }
    }
    catch (SocketException &e)
    {
        // error --- output the message
        e.report();
    }
    return 0;
}
