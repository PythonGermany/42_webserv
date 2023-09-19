#ifndef ACONNECTION_HPP
# define ACONNECTION_HPP

# define BUFFER_SIZE 4096

# include "IFileDescriptor.hpp"
# include "Address.hpp"
# include "timeval.hpp"

# include <string>
# include <vector>

class AConnection : public IFileDescriptor
{
public:
	AConnection();
	AConnection(AConnection const &other);
	virtual ~AConnection();
	AConnection &operator=(AConnection const &other);

	virtual void OnCgiRecv(std::string msg) = 0;
	virtual void OnCgiTimeout() = 0;
	void onPipePollOut(struct pollfd &pollfd);
protected:
	Address client;
	Address host;
	std::string::size_type msgsizelimit;
	std::string::size_type msgsize;
	std::string	msgdelimiter;

	virtual void OnHeadRecv(std::string msg) = 0;
	virtual void OnBodyRecv(std::string msg) = 0;
	void send(std::string msg);
	void cgiSend(std::string msg);
	void runCGI(std::string program, std::vector<std::string> &arg, std::vector<std::string> &env);
	void closeConnection();
private:
	std::string _writeBuffer;
	std::string _readBuffer;
	std::string _cgiWriteBuffer;

	struct timeval lastTimeActive;

	void onPollEvent(struct pollfd &pollfd);
	void onPollOut(struct pollfd &pollfd);
	void onPollIn(struct pollfd &pollfd);
	void onNoPollEvent(struct pollfd &pollfd);
	void passReadBuffer(struct pollfd &pollfd);
};

#endif //ACONNECTION_HPP
