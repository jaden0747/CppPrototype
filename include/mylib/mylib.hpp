#pragma once

#include <iostream>
#include <string>

class Greeting
{
public:
    Greeting(std::string f_message)
        : m_message(f_message)
    {
    }

    const std::string getMessage() const;

    void greeting() const;

private:
    std::string m_message;
};