#!/usr/bin/env python3
import sys

# id 값을 받아와 10으로 나눈 나머지 값을 구합니다.
id_value = int(sys.argv[1])
id_remainder = id_value % 10

# 변환할 문자열을 받아와서 해싱합니다.
string_value = sys.argv[2]
hashed_string = ""

for char in string_value:
    # 문자를 ASCII 코드 값으로 변환한 뒤, id_remainder 값을 더합니다.
    hashed_ascii = ord(char) + id_remainder
    # ASCII 코드 값을 다시 문자로 변환합니다.
    # hashed_char = chr(hashed_ascii)
    hashed_binary = bin(hashed_ascii)[2:].zfill(8)
    hashed_string += hashed_binary

# HTTP 응답으로 해싱된 문자열을 출력합니다.
print("Content-type: text/plain")
print()
print(hashed_string)