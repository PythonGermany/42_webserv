#include <iostream>

int main(int, char **argv, char **env)
{
    std::cout << "arg: " << std::endl;
    for (;*argv;++argv)
    {
        std::cout << *argv << std::endl;
    }
    std::cout << "env: " << std::endl;
    for (;*env;++env)
        std::cout << *env << std::endl;
}
