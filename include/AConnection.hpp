#ifndef ACONNECTION_HPP
# define ACONNECTION_HPP

# define BUFFER_SIZE 4096

# include "IFileDescriptor.hpp"
# include "Address.hpp"

# include <string>
# include <vector>
# include <sys/time.h>

class AConnection : public IFileDescriptor
{
public:
	AConnection();
	AConnection(AConnection const &other);
	virtual ~AConnection();
	AConnection &operator=(AConnection const &other);

	virtual void OnHeadRecv(std::string msg) = 0;
	virtual void OnBodyRecv(std::string msg) = 0;
	virtual void OnCgiRecv(std::string msg) = 0;
	void send(std::string msg);
	// void runCGI(std::string program, std::vector<std::string> &arg, std::vector<std::string> &env);
	void onPipePollOut(struct pollfd &pollfd);
	void onPipePollIn(struct pollfd &pollfd);
protected:
	Address client;
	Address host;
	std::string::size_type msgsizelimit;
	std::string::size_type msgsize;
	std::string	msgdelimiter;
private:
	std::string _writeBuffer;
	std::string _readBuffer;
	std::string _cgiWriteBuffer;
	std::string _cgiReadBuffer;

	struct timeval lastTimeActive;

	void onPollOut(struct pollfd &pollfd);
	void onPollIn(struct pollfd &pollfd);
	void onNoPollIn(struct pollfd &pollfd);
	void passReadBuffer();
};

#endif //ACONNECTION_HPP
