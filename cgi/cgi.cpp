#include <iostream>
#include <unistd.h>

int main(int, char **argv, char **env)
{
    if (isatty(0) == false)
    {
        std::cout << "cin: " << std::endl;
        while (std::cin.eof() == false)
        {
            char tmp[4096];
            ssize_t len = read(0, tmp, 4096);
            if (len == 0)
                break;
            if (len == -1)
                return -1;
            write(1, tmp, len);
        }
    }
    std::cout << "arg: " << std::endl;
    for (;*argv;++argv)
    {
        std::cout << *argv << std::endl;
    }
    std::cout << "env: " << std::endl;
    for (;*env;++env)
        std::cout << *env << std::endl;
}
