#ifndef RESPONSEPIPE_HPP
# define RESPONSEPIPE_HPP

class ResponsePipe
{
public:
	ResponsePipe();
	ResponsePipe(ResponsePipe const &other);
	~ResponsePipe();
	ResponsePipe &operator=(ResponsePipe const &other);
private:
};

#endif //RESPONSEPIPE_HPP
