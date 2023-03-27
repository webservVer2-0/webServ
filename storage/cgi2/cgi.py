#!/usr/bin/env python3

import sys

# 입력으로 받은 id 값을 id_value 변수에 저장합니다.
id_value = int(sys.argv[1])

# id_value 값을 10으로 나눈 나머지 값을 id_remainder 변수에 저장합니다.
id_remainder = id_value % 10

# 입력으로 받은 이진수 값을 binary_string 변수에 저장합니다.
binary_string = sys.argv[2]

# 8자리씩 나누어 각각을 hashed_ascii 값으로 변환합니다.
hashed_string = ""
for i in range(0, len(binary_string), 8):
    # 8자리 이진수 값을 10진수 정수 값으로 변환합니다.
    binary_chunk = binary_string[i:i+8]
    hashed_ascii = int(binary_chunk, 2)
    # id_remainder 값을 뺀 값을 ASCII 코드 값으로 변환합니다.
    hashed_ascii -= id_remainder
    # ASCII 코드 값을 다시 문자로 변환하여 hashed_string 변수에 이어붙입니다.
    hashed_char = chr(hashed_ascii)
    hashed_string += hashed_char

# 해싱된 문자열을 출력합니다.
print(hashed_string)


# import pyfiglet

# ## CGI 2 
# ## pyfiglet 을 활용한 ASCII 값의 변환 
# # while True:
#     # get user input
#     # text = input("Enter some text: ")
# text = input()

#     # generate ASCII art using pyfiglet
# ascii_art = pyfiglet.figlet_format(text, "doh")

#     # print the ASCII art
# print(ascii_art)
