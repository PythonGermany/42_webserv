#ifndef ADDRESS_HPP
# define ADDRESS_HPP

# include <arpa/inet.h> //inet_*
# include <sys/socket.h> //AF_INET
# include <netinet/in.h>
# include <stdexcept>
# include <cerrno>
# include <cstring>
# include <ostream> //overload
# include <cstdlib> //atoi

/**
 * @brief stores either an ipv4 or an ipv6 address with port
 * @throw std::runtime_error() can be thrown by Constructor and stream insertion operator
*/
class Address
{
public:
    Address();
    Address(char const *const src);
    Address(Address const &other);
    Address &operator=(Address const &other);
    ~Address();
    sa_family_t family() const;
    void const *addr() const;
    sockaddr *data();
    sockaddr const *data() const;
    socklen_t size() const;
    void port(in_port_t port);
    in_port_t port() const;
private:
    union AddressStruct
    {
        struct  sockaddr_in v4;
        struct  sockaddr_in6 v6;
    };
    AddressStruct   ip;
};

std::ostream &operator<<(std::ostream &os, Address const &addr);

#endif //ADDRESS_HPP
