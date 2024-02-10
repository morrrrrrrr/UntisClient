
import requests
import json
import base64
import sys

from datetime import datetime, date, time, timedelta

def serialize(name, val):
    return f"{name}={val}"

class UntisClient:
    def __init__(self, url, school, username, password):
        self._url = url
        self._school = school
        self._school64 = base64.b64encode(school.encode('utf-8'))
        self._username = username
        self._password = password

        self._session_info = {}

        self._id = 'awesome'

        self._headers = {
            "Content-Type": "application/json",
            "Cache-Control": "no-cache",
            "Pragma": "no-cache",
            "X-Requested-With": "XMLHttpRequest",
            "User-Agent": "Chrome/61.0.3163.79"
        }
    
    def _buildCookies(self):
        cookies = serialize("JSESSIONID", self._session_info['session_id'])
        cookies += "; " + serialize("schoolname", self._school64)
        return cookies
    
    def _request(self, method, parameter):
        params = {
            'school': self._school
        }
        headers = {
            'Cookie': self._buildCookies()
        }
        data = {
            'id': self._id,
            'method': method,
            'params': parameter,
            'jsonrpc': "2.0"
        }

        return requests.post('https://' + self._url + '/WebUntis/jsonrpc.do', data=json.dumps(data), params=params, headers=headers).json()

    def _datetime_to_untis(self, date: datetime):
        return (
            str(date.year) +
            (f"0{date.month}" if date.month < 10 else str(date.month)) +
            (f"0{date.day}" if date.day < 10 else str(date.day))
        )

    def _get_year_start(self):
        return self._datetime_to_untis(datetime(year=2023, month=7, day=1))

    def _timetable_request(self, start_date: datetime, end_date: datetime):
        parameter = {
            'options': {
                'id': datetime.now().second,
                'element': {
                    'id': self._session_info['person_id'],
                    'type': self._session_info['person_type']
                },
                'startDate': self._datetime_to_untis(start_date),
                'endDate': self._datetime_to_untis(end_date),
                'showLsText': True,
                'showStudentgroup': True,
                'showLsNumber': True,
                'showSubstText': True,
                'showInfo': True,
                'showBooking': True,
                'klasseFields': ["id", "name", "longname", "externalkey"],
                'roomFields': ["id", "name", "longname", "externalkey"],
                'subjectFields': ["id", "name", "longname", "externalkey"],
                'teacherFields': ["id", "name", "longname", "externalkey"]
            }
        }
        
        return self._request("getTimetable", parameter)
    
    def get_timetable_for_day(self, date: datetime):
        return self._timetable_request(date, date)

    def get_absensces(self, excuse_status_id = -1):
        params = {
            'startDate': self._get_year_start(),
            'endDate': self._datetime_to_untis(datetime.now()),
            'studentId': self._session_info['person_id'],
            'excuseStatusId': excuse_status_id
        }
        headers = {
            'Cookie': self._buildCookies()
        }

        response = requests.get('https://' + self._url + '/WebUntis/api/classreg/absences/students', params=params, headers=headers)
        if response.status_code == 200:
            return response.json()
        return 'error: ' + response.text
    
    def get_unexcused_absences(self):
        absences = self.get_absensces()['data']['absences']

        unexcused = []
        for absence in absences:
            if not absence['isExcused']:
                unexcused.append(absence)

        return unexcused
    
    def get_absence_pdf_download_url(self, lateness = True, absences = True, excuse_status_id = -1, excuseGroup = 2):
        params = {
            'name': "Excuse",
            'format': "pdf",
            'rpt_sd': self._get_year_start(),
            'rpt_ed': self._datetime_to_untis(datetime.now()),
            'excuseStatusId': excuse_status_id,
            'studentId': self._session_info['person_id'],
            'withLateness': lateness,
            'withAbsences': absences,
            'execuseGroup': excuseGroup
        }
        headers = {
            'Cookie': self._buildCookies()
        }

        response = requests.get('https://' + self._url + '/WebUntis/reports.do', params=params, headers=headers)

        data = response.json()['data']

        if response.status_code == 200:
            return 'https://' + self._url + '/WebUntis/reports.do?msgId=' + str(data['messageId']) + '&' + data['reportParams']
        return 'error: ' + response.text

    def download_pdf(self, url):
        
        headers = {
            'Cookie': self._buildCookies()
        }
        response = requests.get(url, headers=headers)

        if response.status_code != 200:
            print('error at downloading pdf from url: ' + url + '(code = ' + response.status_code + ')')
            return 'error'
        
        with open('Excuse.pdf', 'wb') as file:
            file.write(response.content)


    def login(self):
        params = {
            'school': self._school
        }
        data = {
            'id': self._id,
            'method': 'authenticate',
            'params': {
                'user': self._username,
                'password': self._password,
                'client': self._id
            },
            'jsonrpc': '2.0'
        }

        response = requests.post('https://' + self._url + '/WebUntis/jsonrpc.do', data=json.dumps(data), params=params, headers=self._headers)
        
        if response.status_code != 200:
            print("Error loggin in")
            sys.exit(-1)

        try:
            data = response.json()
            self._session_info['session_id'] = data['result']['sessionId']
            self._session_info['person_id'] = data['result']['personId']
            self._session_info['person_type'] = data['result']['personType']
            self._session_info['klasse_id'] = data['result']['klasseId']
        except:
            print("Error when logging in")
    
    def logout(self):
        params = {
            'school': self._school
        }
        data = {
            'id': self._id,
            'method': 'logout',
            'params': { },
            'jsonrpc': '2.0'
        }
        response = requests.post('https://' + self._url + '/WebUntis/jsonrpc.do', data=json.dumps(data), params=params, headers=self._headers)
        self._session_info = {}
