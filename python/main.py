
from webuntis import UntisClient
import json
from datetime import datetime

url =       'your url'
school =    'your school'
username =  'your username'
password =  'your password'

client = UntisClient(url, school, username, password)

client.login()

print(json.dumps(client.get_timetable_for_day(datetime.now()), indent=2))

unexcused = client.get_unexcused_absences()

if len(unexcused) > 0:
    
    if input(f"Du hast {len(unexcused)} Fehltage. Entschuldigung ausdrucken? (y/n): ") == 'y':
        excuse_url = client.get_absence_pdf_download_url()
        client.download_pdf(excuse_url)

client.logout()