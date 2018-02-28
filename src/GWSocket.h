//
// Created by Fredy on 25.02.2018.
//
#ifndef GWSOCKETS_GWSOCKET_H
#define GWSOCKETS_GWSOCKET_H
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <GWSocket.h>
#include <mutex>
#include <vector>
#include <memory>
#include "BlockingQueue.h"

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

enum GWSMessageType { TYPE_CONNECTED, TYPE_MESSAGE, TYPE_ERROR };

class GWSocketMessage {
public:
	GWSMessageType type;
	std::string message;
	GWSocketMessage(GWSMessageType type, std::string message): type(type), message(message) {
	}
};

enum SocketState {
	STATE_CONNECTING,
	STATE_CONNECTED,
	STATE_DISCONNECTING,
	STATE_DISCONNECTED,
};

class GWSocket {
public:
	void connect(std::string host, std::string path, unsigned short port);
	void connect();
	void onDisconnected(const boost::system::error_code & ec);
	void close();
	void closeNow();
	void write(std::string message);
	BlockingQueue<GWSocketMessage> messageQueue;
	bool isConnected() { return state == STATE_CONNECTED; };
	bool canBeDeleted() { return state == STATE_DISCONNECTED; };
	std::string path = "";
	std::string host = "";
	unsigned int port = 0;
	std::atomic<SocketState> state{ STATE_DISCONNECTED };

	//static boost::asio::io_context ioc;
	static std::unique_ptr<boost::asio::io_context> ioc;
private:
	void errorConnection(std::string errorMessage);
	void onRead(const boost::system::error_code &ec, size_t readSize);
	void onWrite(const boost::system::error_code &ec, size_t bytesTransferred);
	void checkWriting();
	void handshakeStep(const boost::system::error_code &ec);
	void connectedStep(const boost::system::error_code& ec, tcp::resolver::iterator i);
	void hostResolvedStep(const boost::system::error_code &ec, tcp::resolver::iterator it);
	bool writing = { false };
	websocket::stream<tcp::socket> ws { *ioc };
	tcp::resolver resolver{ *ioc };
	boost::beast::multi_buffer readBuffer;
	std::vector<std::string> writeQueue;
	std::mutex queueMutex;
};


#endif //GWSOCKETS_GWSOCKET_H