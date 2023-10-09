#include "Connection.hpp"
#include <iostream>

int main(int argc, char **argv) {
	Client client("15466.courses.cs.cmu.edu", "15466"); //connect to a local server at port 1337

	{ //send handshake message:
		Connection &con = client.connection;
		con.send_buffer.emplace_back('H');
		con.send_buffer.emplace_back( 14 );
		con.send_buffer.emplace_back('g');
		for (char c : std::string("lerdue")) {
			con.send_buffer.emplace_back(c);
		}
		for (char c : std::string("aliloia")) {
			con.send_buffer.emplace_back(c);
		}

	}

	
    // bool worm = true;
	while (true) {
		client.poll([](Connection *connection, Connection::Event evt){
		},
		0.0 //timeout (in seconds)
		);

		Connection &con = client.connection;

		while (true) {
			if (con.recv_buffer.size() < 2) break;
			char type = char(con.recv_buffer[0]);
			size_t length = size_t(con.recv_buffer[1]);
			if (con.recv_buffer.size() < 2 + length) break;

			if (type == 'T') {
				const char *as_chars = reinterpret_cast< const char * >(con.recv_buffer.data());
				std::string message(as_chars + 2, as_chars + 2 + length);

				std::cout << "T: '" << message << "'" << std::endl;
			} else if (type == 'V'){
				const char *as_chars = reinterpret_cast< const char * >(con.recv_buffer.data());
				std::string message(as_chars + 2, as_chars + 2 + length);

				std::cout << "V: '" << message << "'" << std::endl;
                std::cout << message[4] << std::endl;
                // srand(time(NULL));
                // int randNum = (rand() % 3) + 1;
                
                if (message[1] == ' '){
                    con.send_buffer.emplace_back('M');
                    con.send_buffer.emplace_back(1);
                    con.send_buffer.emplace_back('N');
                    std::cout << "received " << std::endl;
                }
                else if (message[3] == ' '){
                    con.send_buffer.emplace_back('M');
                    con.send_buffer.emplace_back(1);
                    con.send_buffer.emplace_back('W');
                    std::cout << "received " << std::endl;
                }
                
                else if (message[5] == ' '){
                    con.send_buffer.emplace_back('M');
                    con.send_buffer.emplace_back(1);
                    con.send_buffer.emplace_back('E');
                    std::cout << "received " << std::endl;
                }
                else if (message[7] == ' '){
                    con.send_buffer.emplace_back('M');
                    con.send_buffer.emplace_back(1);
                    con.send_buffer.emplace_back('S');
                    std::cout << "received " << std::endl;
                }
                
                
                
                
                
                
                
                
			}else{
                std::cout << "Ignored a " << type << " message of length " << length << "." << std::endl;
            }

			con.recv_buffer.erase(con.recv_buffer.begin(), con.recv_buffer.begin() + 2 + length);
		}
	}
}
