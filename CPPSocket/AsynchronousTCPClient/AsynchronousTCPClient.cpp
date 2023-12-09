#include <SDKDDKVer.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>


const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 31400;


class TCP_Client 
{
public:
    TCP_Client(boost::asio::io_context& io_service)
        : m_io_service(io_service),
          m_Socket(io_service),
		  m_nSeqNumber(0)
    {}

    void Connect( boost::asio::ip::tcp::endpoint& endpoint )
    {
        m_Socket.async_connect( endpoint,
								boost::bind(&TCP_Client::handle_connect, 
											this, 
											boost::asio::placeholders::error)
								);
    }
	   
	
private:
	void PostWrite()
	{
		if( m_Socket.is_open() == false )
		{
			return;
		}

		if( m_nSeqNumber > 7 )
		{
			m_Socket.close();
			return;
		}


		++m_nSeqNumber;

		char szMessage[128] = {0,};
		sprintf_s( szMessage, 128-1, "%d - Send Message", m_nSeqNumber );

		m_WriteMessage = szMessage;

		boost::asio::async_write(m_Socket, boost::asio::buffer(m_WriteMessage),
							boost::bind( &TCP_Client::handle_write, this,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred )
							);

			
		PostReceive();
	}

	void PostReceive()
	{
		memset( &m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer) );

		m_Socket.async_read_some( boost::asio::buffer(m_ReceiveBuffer), 
									boost::bind( &TCP_Client::handle_receive, this, 
												boost::asio::placeholders::error, 
												boost::asio::placeholders::bytes_transferred ) 
								); 
							
	}


	void handle_connect(const boost::system::error_code& error)
    {
        if (error) 
		{
            std::cout << "connect failed : " << error.message() << std::endl;
        }
        else 
		{
            std::cout << "connected" << std::endl;
			
			PostWrite();
        }
    }

	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
	{
	}

	void handle_receive( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( error )
		{
			if( error == boost::asio::error::eof )
			{
				std::cout << "disconnected from server" << std::endl;
			}
			else 
			{
				std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
			}
		}
		else
		{
			const std::string strRecvMessage = m_ReceiveBuffer.data();
			std::cout << "received message: " << strRecvMessage << ", size received: " << bytes_transferred << std::endl;

			PostWrite(); 
		}
	}

	

	boost::asio::io_context& io_service;
        boost::asio::ip::tcp::socket socket;

	int seqnumber;
	std::array<char, 128> m_ReceiveBuffer;
	std::string m_WriteMessage;
};



int main()
{
    boost::asio::io_context io_service;

	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(SERVER_IP), PORT_NUMBER);

    TCP_Client client(io_service);

    client.Connect( endpoint );

    io_service.run();

	
	std:: cout << "receiving..." << std::endl;

	getchar();
	return 0;
}
