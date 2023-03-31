import pyfiglet

## CGI 2 
## pyfiglet 을 활용한 ASCII 값의 변환 
# while True:
    # get user input
    # text = input("Enter some text: ")
text = input()

    # generate ASCII art using pyfiglet
ascii_art = pyfiglet.figlet_format(text, "doh")

    # print the ASCII art
print(ascii_art)