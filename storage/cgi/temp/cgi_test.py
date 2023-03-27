#!/usr/bin/env python3

import cgi
import requests

# 입력 데이터 받기
form = cgi.FieldStorage()
ascii_data = form.getvalue('ascii-data')

# 요청 보내기
response = requests.get('localhost/cgi', params={'ascii-data': ascii_data})
response_data = response.text

# 출력 데이터 생성
output = '입력한 ASCII 데이터: {}\n'.format(ascii_data)
output += '외부 API로부터 받은 데이터: {}'.format(response_data)

# 출력
print('Content-Type: text/html')
print()
print('<html><head><title>ASCII 데이터 전송 결과</title></head><body>')
print('<h1>ASCII 데이터 전송 결과</h1>')
print('<p>{}</p>'.format(output))
print('</body></html>')