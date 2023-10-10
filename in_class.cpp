#include "Connection.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    Client client("15466.courses.cs.cmu.edu", "15466"); // connect to a local server at port 1337

    { // send handshake message:
        Connection &con = client.connection;
        con.send_buffer.emplace_back('H');
        con.send_buffer.emplace_back(14);
        con.send_buffer.emplace_back('g');
        for (char c : std::string("tlenet"))
        {
            con.send_buffer.emplace_back(c);
        }
        for (char c : std::string("jiyoonp"))
        {
            con.send_buffer.emplace_back(c);
        }
    }

    std::vector<char> maze_map;

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
                char *as_chars = reinterpret_cast< char *>(con.recv_buffer.data());
                std::string message(as_chars + 2, as_chars + 2 + length);

                std::cout << "T: '" << message << "'" << std::endl;

            }
            else if (type == 'V')
            {
                char *as_chars = reinterpret_cast<char *>(con.recv_buffer.data());
                std::string message(as_chars + 2, as_chars +length);
                std::cout << "V: "<< std::endl;
                for (int i = 0; i < 3; ++i)
                {
                    for (int j = 0; j < 3; ++j)
                    {
                        std::cout << message[i * 3 + j] << " ";
                    }
                    std::cout << std::endl;
                }

                for (char c : message)
                {
                    maze_map.push_back(c);
                }
                {
                    // send handshake message:
                    Connection &con = client.connection;
                    con.send_buffer.emplace_back('M');
                    con.send_buffer.emplace_back(1);

                    // Accept user input
                    std::cout << "Enter a character and press Enter: ";
                    char userInput;
                    std::cin >> userInput; // Capture the character

                    // if (std::string("^[[A").find(userInput) != std::string::npos)
                    // {
                    //     con.send_buffer.emplace_back('N'); // Send the user input
                    // }
                    // else if (std::string("^[[B").find(userInput) != std::string::npos)
                    // {
                    //     con.send_buffer.emplace_back('S'); // Send the user input
                    // }
                    // else if (std::string("^[[C").find(userInput) != std::string::npos)
                    // {
                    //     con.send_buffer.emplace_back('E'); // Send the user input
                    // }
                    // else if (std::string("^[[D").find(userInput) != std::string::npos)
                    // {
                    //     con.send_buffer.emplace_back('W'); // Send the user input
                    // }

                    if (userInput=='w'){
                        userInput = 'N';
                    }
                    else if (userInput=='s'){
                        userInput = 'S';
                    }
                    else if (userInput=='d'){
                        userInput = 'E';
                    }
                    else if (userInput=='a'){
                        userInput = 'W';
                    }
                    else if (userInput=='q'){
                        userInput = 'f';
                    }
                        con.send_buffer.emplace_back(userInput); // Send the user input
                }
            }
            else
            {
                std::cout << "Ignored a " << type << " message of length " << length << "." << std::endl;
            }

            con.recv_buffer.erase(con.recv_buffer.begin(), con.recv_buffer.begin() + 2 + length);
        }
    }
}