import socket
import numpy as np
import cv2
import sys

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 8080        # The port used by the server

image = cv2.imread("plates/1.png")
encode_param = [int(cv2.IMWRITE_JPEG_QUALITY),90]
_, img_encode = cv2.imencode(".jpeg", image, encode_param)
data = np.array(img_encode)
stringData = data.tostring()

image2 = cv2.imread("./road.png")
_, img_encode2 = cv2.imencode(".jpeg", image2, encode_param)
data2 = np.array(img_encode2)
stringData2 = data2.tostring()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    file_name = sys.argv[1]
    s.send(file_name.encode())
    for i in range(100):
        s.send(str(len(stringData)).ljust(16).encode())
        s.send(stringData)
    s.send(str(len(stringData2)).ljust(16).encode())
    s.send(stringData2)
    s.close()