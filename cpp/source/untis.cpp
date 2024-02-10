#include "curl/curl.h"
#include "base64.h"

#include "untis.h"

// all of these are for time string
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) 
{
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string post(std::string url, const nlohmann::json& data, const nlohmann::json& header, const nlohmann::json& params)
{
    CURL* handle;
    CURLcode result;
    std::string response;
    
    handle = curl_easy_init();

    if (!handle)
    {
        // error handling
        return "";
    }

    // start with the url parameters
    url.push_back('?');    
    for (const auto& element : params.items())
    {
        url = url + element.key() + "=" + element.value().get<std::string>() + "&";
    }
    
    // remove potential last & char
    if (url.back() == '&') 
        url.erase(url.end() - 1);

    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

    struct curl_slist* headers = nullptr;
    for (const auto& element : header.items())
    {
        headers = curl_slist_append(headers, std::string(element.key() + ": " + element.value().get<std::string>()).c_str());
    }

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);


    std::string data_str = data.dump(-1, 32, true);

    // add the post data
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data_str.c_str());

    // load ca certificate
    curl_easy_setopt(handle, CURLOPT_CAINFO, "curl-ca-bundle.crt");

    // set writeback function
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

    result = curl_easy_perform(handle);

    // cleanup
    curl_easy_cleanup(handle);
    curl_slist_free_all(headers);

    // check result after cleanup
    if (result != CURLE_OK)
    {
        // some error happened?
        return "";
    }

    // everything went CURLE_OK
    return response;
}

std::string untis::Client::buildCookie()
{
    return 
        "JSESSIONID=" + m_session_info["sessionId"].get<std::string>() + "; " +
        "schoolname=" + m_school64;
}

nlohmann::json untis::Client::request(const std::string& method, const nlohmann::json& parameter)
{
    nlohmann::json params{
        { "school", m_school }
    };

    nlohmann::json headers{
        { "Content-Type", "application/json" },
        { "Cookie", m_cookie }
    };

    nlohmann::json data{
        { "id", m_id },
        { "method", method },
        { "params", parameter },
        { "jsonrpc", "2.0" }
    };

    return nlohmann::json::parse(post("https://" + m_url + "/WebUntis/jsonrpc.do", data, headers, params));
}

std::string untis::Client::convertToUntisTime(const std::chrono::_V2::system_clock::time_point& timepoint)
{
    // first convert to std::time_t
    std::time_t tp_time_t = std::chrono::system_clock::to_time_t(timepoint);

    // then convert to a tm struct
    std::tm* tp_tm = std::localtime(&tp_time_t);

    // format the tm struct into a string
    std::stringstream ss;
    ss << std::put_time(tp_tm, "%Y%m%d");

    return ss.str();
}

void untis::Client::init(
    const std::string& url, 
    const std::string& school, 
    const std::string& username, 
    const std::string& password)
{
    m_id = "awesome";

    m_url = url;
    m_school = school;
    
    // base 64-encode m_school64
    m_school64 = base64_encode(school);

    m_username = username;
    m_password = password;
}

void untis::Client::login()
{
    nlohmann::json params{
        { "school", m_school }
    };

    nlohmann::json headers{
        { "Content-Type", "application/json" },
        { "Cache-Control", "no-cache" },
        { "Pragma", "no-cache" },
        { "X-Requested-With", "XMLHttpRequest" },
        { "User-Agent", "Chrome/61.0.3163.79" }
    };

    nlohmann::json data{
        { "id", m_id },
        { "method", "authenticate" },
        { "params", 
            {
                { "user", m_username },
                { "password", m_password },
                { "client", m_id }
            } 
        },
        { "jsonrpc", "2.0" }
    };

    nlohmann::json answer = nlohmann::json::parse(
        post("https://" + m_url + "/WebUntis/jsonrpc.do", data, headers, params)
    );

    m_session_info["sessionId"] = answer["result"]["sessionId"];
    m_session_info["personId"] = answer["result"]["personId"];
    m_session_info["personType"] = answer["result"]["personType"];
    m_session_info["klasseId"] = answer["result"]["klasseId"];

    // build the session cookie
    m_cookie = buildCookie();
}

void untis::Client::logout()
{
    nlohmann::json params{
        { "school", m_school }
    };

    nlohmann::json headers{
        { "Content-Type", "application/json" },
        { "Cache-Control", "no-cache" },
        { "Pragma", "no-cache" },
        { "X-Requested-With", "XMLHttpRequest" },
        { "User-Agent", "Chrome/61.0.3163.79" }
    };

    nlohmann::json data{
        { "id", m_id },
        { "method", "logout" },
        // { "params", 
        //     {
                
        //     } 
        // },
        { "jsonrpc", "2.0" }
    };

    // clear session info
    m_session_info = {};

    post("https://" + m_url + "/WebUntis/jsonrpc.do", data, headers, params);
}



// TIMETABLE:
nlohmann::json untis::Client::getTimetable(const time_point& from, const time_point& to)
{
    nlohmann::json parameter;
    nlohmann::json options;

    options["id"] = clock(); // some random number
    options["element"] = nlohmann::json{
        { "id", m_session_info["personId"] },
        { "type", m_session_info["personType"] }
    };
    options["startDate"] = convertToUntisTime(from);
    options["endDate"] = convertToUntisTime(to);
    options["showLsText"] = true;
    options["showStudentgroup"] = true;
    options["showLsNumber"] = true;
    options["showSubstText"] = true;
    options["showInfo"] = true;
    options["showBooking"] = true;
    options["klasseFields"] = nlohmann::json{
        "id", "name", "longname", "externalkey"
    };
    options["roomFields"] = nlohmann::json{
        "id", "name", "longname", "externalkey"
    };
    options["subjectFields"] = nlohmann::json{
        "id", "name", "longname", "externalkey"
    };
    options["teacherFields"] = nlohmann::json{
        "id", "name", "longname", "externalkey"
    };

    parameter["options"] = options;

    return request("getTimetable", parameter);
}

nlohmann::json untis::Client::getTimetableForToday()
{
    return getTimetable(std::chrono::system_clock::now(), std::chrono::system_clock::now());
}
nlohmann::json untis::Client::getTimetableForWeek()
{
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Get the current date
    auto today = std::chrono::system_clock::to_time_t(now);
    std::tm current_tm = *std::localtime(&today);

    // Calculate the time point for Monday of the current week
    int days_to_monday = current_tm.tm_wday == 0 ? 6 : current_tm.tm_wday - 1;
    if (days_to_monday >= 5)
    {
        // saturday and sonday
        days_to_monday -= 7;
    }

    auto monday = now - std::chrono::hours(24 * days_to_monday);
    auto friday = monday + std::chrono::hours(24 * 5);

    return getTimetable(monday, friday);
}
