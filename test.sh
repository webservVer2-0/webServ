#!/bin/bash

#GET METHOD
curl http://localhost
usleep(5000)

#POST METHOD
curl -X POST http://localhost/post -H "Content-length=5" -H "Content-Type=application/x-www-form-urlencoded" -d "hello"
usleep(5000)
curl -X POST http://localhost/post -H "Content-length=14452" -H "Content-Type=application/x-www-form-urlencoded" -d "hello"
usleep(5000)
curl -X POST -H "Transfer-Encoding: chunked" --data-binary $"5\r\nhello\r\n6\r\nworld!"
usleep(5000)
curl -X POST -F "file=@/Users/jinyoung/Desktop/0323web/storage/static/asset/home.png" http://localhost/post -H "Content-length=897"



#DELETE METHOD
curl -X DELETE “http://localhost/delete?delete_target=896544303_userdata.dat”
curl -X DELETE “http://localhost/image.png"

#ERROR
curl -X GET http://localhost/ -H "Content-length=5" -H "Content-Type=application/x-www-form-urlencoded" -d "hello"
curl -X GET http://localhost/indux.html -H "Content-length=5" -H "Content-Type=application/x-www-form-urlencoded" -d "hello"


