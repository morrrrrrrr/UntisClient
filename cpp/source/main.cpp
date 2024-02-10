
#include <iostream>
#include "curl/curl.h"
#include "nlohmann/json.hpp"
#include "untis.h"

int main( void ) 
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    untis::Client client;
    client.init("url", "school", "username", "password");

    client.login();

    std::cout << client.getTimetableForWeek().dump(2) << "\n";

    client.logout();

    curl_global_cleanup();
}
