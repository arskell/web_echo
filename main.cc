#include <iostream>
#include <array>
#include <cstdlib>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <boost/asio.hpp>

#include <memory>

using tcp = boost::asio::ip::tcp;



std::stringstream log_accept(tcp::socket& socket){
  std::stringstream msg;
  msg << "accepted to "<<socket.remote_endpoint().address().to_string();
  return msg;
}

std::stringstream log_create(int port){
  std::stringstream msg;
  msg<<"server is listening on port "<< port;
  return msg;
}

std::stringstream log_error(std::exception& exc){
  std::stringstream msg;
  msg<<"CRITICAL ERROR: "<< exc.what()<<std::endl;
  return msg;
}

void create_log_message(std::stringstream&& message, std::ostream& stream = std::cout){
  auto time =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  stream<<std::put_time(std::localtime(&time),  "%b %d %Y %X")<<"| "
        <<message.str()<<std::endl;
  return;
}



int main(int argc, char**argv){
  if(argc < 2){
    std::cerr<<"ERROR: too few arguments\ntip: use argument to setup port"<<std::endl;
    return -1;
  }
  try{
    boost::asio::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(),atoi(argv[1])));
    create_log_message(log_create(atoi(argv[1])));
    std::array<char, 8192> buffer;
    for(;;){
      tcp::socket socket(ioc);
      acceptor.accept(socket);
      create_log_message(log_accept(socket));
      auto len = socket.receive(boost::asio::buffer(buffer),NULL);
      socket.send(boost::asio::buffer(buffer, len));
      socket.close();
    }
  }catch(std::exception& ex){
    create_log_message(log_error(ex));
  }
  return 0;
}
