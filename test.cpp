#include <iostream>
#include <typeinfo>

int main()
{
    int a = 0;
    bool b = false;
    double c = 334.5;
    std::string str = "Nirmal";

    std::cout << "Type of a : " << typeid(a).name() << "\n";
    std::cout << "Type of b : " << typeid(b).name() << "\n";
    std::cout << "Type of c : " << typeid(c).name() << "\n";
    std::cout << "Type of str : " << typeid(str).name() << "\n";

    return 0;
}