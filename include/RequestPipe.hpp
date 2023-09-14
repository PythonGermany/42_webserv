#ifndef REQUESTPIPE_HPP
# define REQUESTPIPE_HPP

# include "IFileDescriptor.hpp"
# include "AConnection.hpp"

class RequestPipe : IFileDescriptor
{
public:
	RequestPipe(AConnection *src, int pid);
	RequestPipe(RequestPipe const &other);
	~RequestPipe();
	RequestPipe &operator=(RequestPipe const &other);

	void onPollOut(struct pollfd &pollfd);
	void onPollIn(struct pollfd &pollfd);
	void onNoPollIn(struct pollfd &pollfd);
private:
	AConnection *connection;
	std::string _writeBuffer;
	RequestPipe();
};

#endif //REQUESTPIPE_HPP
