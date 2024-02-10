#pragma once

/*
* The class units::Client (introduced here) handles connections and requests to the webuntis json rpc
*/

#include <string>
#include <chrono>
#include "nlohmann/json.hpp"

namespace untis
{

    using time_point = std::chrono::_V2::system_clock::time_point;

    class Client
    {
    public:
        Client() = default;
        Client(const Client& ) = delete;
        virtual ~Client() = default;

    public:
        void init(const std::string& url, const std::string& school, const std::string& username, const std::string& password);

    public:
        void login();
        void logout();

        nlohmann::json getTimetableForToday();
        nlohmann::json getTimetableForWeek();

    private:
        nlohmann::json getTimetable(const time_point& from, const time_point& to);
    
    private:
        nlohmann::json request(const std::string& method, const nlohmann::json& parameter);
        std::string buildCookie();

    private:
        std::string convertToUntisTime(const time_point& timepoint);

    private:
        std::string m_id;
        std::string m_url;
        std::string m_school;
        std::string m_school64;
        std::string m_username;
        std::string m_password;
        std::string m_cookie;

        nlohmann::json m_session_info;

    };

} // namespace untis
