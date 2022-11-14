#pragma once

#include <iostream>
#include <string>

class Greeting
{
public:
  Greeting(std::string f_message)
  : m_message(f_message) {}

  const std::string getMessage() const {
    return m_message;
  }

  void greeting() const {
    std::cout << m_message << std::endl;
  }

private:
  std::string m_message;
};