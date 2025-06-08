#include "mylib/mylib.hpp"

const std::string Greeting::getMessage() const
{
    return m_message;
}

void Greeting::greeting() const
{
    std::cout << m_message << std::endl;
}