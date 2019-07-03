#include <iostream>
#include <array>
#include <cstdlib>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>

using tcp = boost::asio::ip::tcp;


void create_log_message(std::stringstream&& message, std::ostream& stream = std::cout){
  auto time =std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  stream<<std::put_time(std::localtime(&time),  "%b %d %Y %X")<<"| "
        <<message.str()<<std::endl;
  return;
}

std::stringstream log_accept(const tcp::socket& socket){
  std::stringstream msg;
  msg << "accepted to "<<socket.remote_endpoint().address().to_string();
  return msg;
}

std::stringstream log_create(int port){
  std::stringstream msg;
  msg<<"server is listening on port "<< port;
  return msg;
}

std::stringstream log_crit_error(const std::exception& exc){
  std::stringstream msg;
  msg<<"CRITICAL ERROR: "<< exc.what()<<std::endl;
  return msg;
}

std::stringstream log_error(const std::exception& exc){
  std::stringstream msg;
  msg<<"ERROR: "<< exc.what()<<std::endl;
  return msg;
}


struct tcp_connection: public std::enable_shared_from_this<tcp_connection>{
  using pointer =  std::shared_ptr<tcp_connection>;

  static pointer create(boost::asio::io_context& io_context){
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket(){
    return socket_;
  }
  void start(){
    socket_.async_receive(boost::asio::buffer(buffer),
                          boost::bind(&tcp_connection::handler_read, shared_from_this(),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
  }
private:
  tcp_connection(boost::asio::io_context& ioc):socket_(ioc){

  }
  void handler_read(const boost::system::error_code&, size_t len){
    socket_.send(boost::asio::buffer(buffer,len));
    socket_.close();
  }
  std::array<char, 8192> buffer;
  tcp::socket socket_;
};


struct tcp_server{
public:
  tcp_server(boost::asio::io_context& ioc, int port)
    :acceptor_(ioc, tcp::endpoint(tcp::v4(),port)){
    start_accept();
  }
private:
  void start_accept(){
    tcp_connection::pointer connection = tcp_connection::create(acceptor_.get_executor().context());

    acceptor_.async_accept(connection->socket(),
                           boost::bind(&tcp_server::handle_accept, this, connection,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer con, const boost::system::error_code& error){
    if(!error){
      create_log_message(log_accept(con->socket()));
      con->start();
    }else{
      create_log_message(log_error(boost::system::system_error(error)));
    }
    start_accept();
  }
  tcp::acceptor acceptor_;
};


int main(int argc, char**argv){
  if(argc < 2){
    std::cerr<<"ERROR: too few arguments\ntip: use argument to setup port"<<std::endl;
    return -1;
  }
  try{
    boost::asio::io_context ioc;
    tcp_server serv(ioc, atoi(argv[1]));
    create_log_message(log_create(atoi(argv[1])));
    ioc.run();
  }catch(std::exception& ex){
    create_log_message(log_crit_error(ex));
  }
  return 0;
}

