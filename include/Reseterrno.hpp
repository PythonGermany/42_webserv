#ifndef RESETERRNO_HPP
# define RESETERRNO_HPP

# include <cerrno>

struct Reseterrno
{
    Reseterrno()
    {
        olderrno = errno;
    }
    ~Reseterrno()
    {
        errno = olderrno;
    }
    int olderrno;
};

#endif //RESETERRNO_HPP
