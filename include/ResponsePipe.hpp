#ifndef RESPONSEPIPE_HPP
# define RESPONSEPIPE_HPP

# include "AConnection.hpp"

class ResponsePipe : public IFileDescriptor
{
public:
	ResponsePipe(AConnection *callbackObject, pid_t cgiPid);
	ResponsePipe(ResponsePipe const &other);
	~ResponsePipe();
	ResponsePipe &operator=(ResponsePipe const &other);

	void onPollEvent(struct pollfd &pollfd);
private:
	AConnection *_callbackObject;
	short _callbackObjectFlags;
	std::string _readBuffer;
	struct timeval lastTimeActive;
	pid_t _cgiPid;

	ResponsePipe();
	void onNoPollEvent(struct pollfd &pollfd);
};

#endif //RESPONSEPIPE_HPP
