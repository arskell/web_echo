#include <iostream>
#include <array>
#include <cstdlib>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;

int main(int argc, char**argv){
  if(argc < 2){
    std::cerr<<"ERROR: too few arguments\ntip: use argument to setup port"<<std::endl;
    return -1;
  }
  try{
    boost::asio::io_context ioc;
    tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(),atoi(argv[1])));
    std::array<char, 8192> buffer;
    for(;;){
      tcp::socket socket(ioc);
      acceptor.accept(socket);
      std::cout<<"accepted to "<<socket.remote_endpoint().address().to_string() <<std::endl;
      boost::system::error_code err;
      auto len = socket.receive(boost::asio::buffer(buffer),NULL, err);
      socket.send(boost::asio::buffer(buffer, len));
      socket.close();
    }
  }catch(std::exception& ex){
    std::cerr<<ex.what()<<std::endl;
  }
  return 0;
}
