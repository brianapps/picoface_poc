import pyperclip


text = pyperclip.paste()


with open("data.scr", "wb") as fp:
    for l in text.split("\r\n"):
        for i in range(0, len(l), 2):
            
            b = int(l[i:i+2], base=16)
            fp.write(bytes([b]))

    