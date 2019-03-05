#include <cpr/cpr.h>
#include <iostream>

int main(int argc, char** argv) {
    auto r = cpr::Get(cpr::Url{"https://192.168.1.103/redfish/v1"},cpr::VerifySsl{false});
    std::cout << r.status_code << std::endl;                  // 200
    std::cout << r.header["content-type"] << std::endl;       // application/json; charset=utf-8
    std::cout << r.text << std::endl;                         // JSON text string
}
