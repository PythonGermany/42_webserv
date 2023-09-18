#ifndef REQUESTPIPE_HPP
# define REQUESTPIPE_HPP

# include "IFileDescriptor.hpp"
# include "AConnection.hpp"

class RequestPipe : public IFileDescriptor
{
public:
	RequestPipe(AConnection *src);
	RequestPipe(RequestPipe const &other);
	~RequestPipe();
	RequestPipe &operator=(RequestPipe const &other);

	void onPollEvent(struct pollfd &pollfd);
private:
	AConnection *callbackObject;
	RequestPipe();
};

#endif //REQUESTPIPE_HPP
