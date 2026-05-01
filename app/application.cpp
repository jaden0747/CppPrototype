#include <iostream>

#include "mylib/mylib.hpp"

class TestArray
{
public:
    TestArray()
    {
        for (int i = 0; i < 5; ++i)
            m_data[i] = i + 1;
    }
    int* begin()
    {
        return m_data;
    }

    int* end()
    {
        return m_data + 5;
    }

private:
    int m_data[5];
};

int main(int argc, char** argv)
{
    TestArray arr;
    std::cout << "for (auto data : arr)" << "\n";
    for (auto data : arr)
    {
        std::cout << data << " ";
    }
    std::cout << std::endl;

    std::cout << "for (auto i = arr.begin(); i != arr.end(); ++i)" << "\n";
    for (auto i = arr.begin(); i != arr.end(); ++i)
    {
        std::cout << *i << " ";
    }
    std::cout << std::endl;

    std::vector<int> std_arr{1, 2, 3, 4, 5};

    std::cout << "for (std::vector<int>::iterator it = std_arr.begin(); it != std_arr.end(); ++it)" << "\n";
    for (std::vector<int>::iterator it = std_arr.begin(); it != std_arr.end(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << std::endl;
    // Main::run();
    return 0;
}
