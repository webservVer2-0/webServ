#!/usr/bin/env python
import cgi
import io
from PIL import Image
import pytesseract

print("Content-Type: text/plain")
print()

form = cgi.FieldStorage()
if "image" not in form:
    print("Error: No image uploaded")
else:
    # read the image file
    fileitem = form["image"]
    imgfile = io.BytesIO(fileitem.file.read())
    
    # open the image using PIL
    img = Image.open(imgfile)
    
    # extract text from the image using Tesseract OCR
    text = pytesseract.image_to_string(img)
    
    # print the extracted text
    print(text)
