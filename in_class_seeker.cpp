#include "Connection.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    Client client("15466.courses.cs.cmu.edu", "15466"); // connect to a local server at port 1337

    { // send handshake message:
        Connection &con = client.connection;
        con.send_buffer.emplace_back('H');
        con.send_buffer.emplace_back(14);
        con.send_buffer.emplace_back('s');
        for (char c : std::string("c176d4"))
        {
            con.send_buffer.emplace_back(c);
        }
        for (char c : std::string("jiyoonp"))
        {
            con.send_buffer.emplace_back(c);
        }
    }

    while (true)
    {
        client.poll([](Connection *connection, Connection::Event evt) {},
                    0.0 // timeout (in seconds)
        );

        Connection &con = client.connection;

        while (true)
        {
            if (con.recv_buffer.size() < 2)
                break;
            char type = char(con.recv_buffer[0]);
            size_t length = size_t(con.recv_buffer[1]);
            if (con.recv_buffer.size() < 2 + length)
                break;
            std::cout << "Type: " << type << std::endl;
            if (type == 'T')
            {
                char *as_chars = reinterpret_cast<char *>(con.recv_buffer.data());
                std::string message(as_chars + 2, as_chars + 2 + length);

                std::cout << "T: '" << message << "'" << std::endl;


            }
            else if (type == 'V')
            {
                char *as_chars = reinterpret_cast<char *>(con.recv_buffer.data());
                std::string message(as_chars + 2, as_chars + length);

                std::cout << "V: '" << message << "'" << std::endl;
            }
            else
            {
                std::cout << "Ignored a " << type << " message of length " << length << "." << std::endl;
            }

            con.recv_buffer.erase(con.recv_buffer.begin(), con.recv_buffer.begin() + 2 + length);
            { // send handshake message:
                Connection &con = client.connection;
                con.send_buffer.emplace_back('M');
                con.send_buffer.emplace_back(1);
                con.send_buffer.emplace_back('N');
            }
        }
    }
}