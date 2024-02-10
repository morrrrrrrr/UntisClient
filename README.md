# UntisClient

Webuntis Client code from https://github.com/SchoolUtils/WebUntis

translated into C++ and Python

# Python

The python implementation is the bigger of the two:

## supports:

* getting the timetable for today and for the week
* getting the absences
* printing the absence letter

## dependencies:

requests
json
base64
datetime

See the main.py file in the python folder for usage

# C++

The C++ implementation is very small and simple

## supports:

* getting the timetable for today and for the week

## dependencies:

* libcurl: https://curl.se/libcurl/
* json parser: https://github.com/nlohmann/json
* base64 encoder: https://github.com/ReneNyffenegger/cpp-base64

see the main.cpp file in the c++/source folder for usage

# Notice

I am not affiliated with Untis GmbH. Use this at your own risk.
