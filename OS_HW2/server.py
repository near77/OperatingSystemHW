import os
import cv2
import time
import socket, threading
import numpy as np
from TOOLS.server_mod import DataProcessor
# from Extraction import LicencePlateDetector as Detect
from TOOLS.LicenceDetector import LicencePlateDetector as Detect
MAX_CLIENT = 5


def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf: return None
        buf += newbuf
        count -= len(newbuf)
    return buf

def write_txt(file_name, frame_no_array):
    filename = file_name.split(".")[-2]
    while os.path.exists(filename):
        tmp_int = 0
        filename = filename + str(tmp_int)
        tmp_int += 1
    f = open(filename+".txt", "w")
    f.close()
    f = open(filename+".txt", "a")
    for frame_no in frame_no_array:
        f.write(str(frame_no)+'\n')
    f.close()

def LicencePlateDetector(conn, client_num):
    DP = DataProcessor()
    if client_num == 1:
        DP.InitImgDir()
    else:
        pass

    with conn:
        file_name = conn.recv(6).decode() # Demo will always be 6 bytes
        print("File name: ",file_name)
        frame_count = 1
        frame_no_array = []
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        video = cv2.VideoWriter(file_name.split(".")[-2] +"tmp.mp4", fourcc, 60, (1280,720))
        while True:
            stime = time.time()
            length = conn.recv(16)
            if not length:
                break
            stringData = recvall(conn, int(length))
            if not stringData:
                break
            data = np.frombuffer(stringData, dtype="uint8")
            decimg = cv2.imdecode(data, 1)
            video.write(decimg)
            # decimg = cv2.resize(decimg, (640, 480))
            if frame_count % 60 == 1:
                have_lic = Detect(decimg)
            # have_lic = Detect(decimg)
            if(have_lic):
                frame_no_array.append(frame_count)
            else:
                pass
            # print("Time per frame: ", time.time() - stime)
            frame_count += 1
        write_txt(file_name, frame_no_array)
        abs_path = os.path.split(os.path.abspath(__file__))[0]
        video_name = os.path.join(abs_path, file_name.split(".")[-2] + "tmp.mp4")
        text_name = os.path.join(abs_path, file_name.split(".")[-2] + ".txt")
        print(video_name)
        print(text_name)
        assert os.path.exists(video_name)
        assert os.path.exists(text_name)
        print("uploading")
        DP.Upload(video_name)
        DP.Upload(text_name)
        print("upload complete")

class ClientThread(threading.Thread):
    def __init__(self,clientAddress,clientsocket, client_num):
        threading.Thread.__init__(self)
        self.csocket = clientsocket
        self.caddr = clientAddress
        self.client_num = client_num
        print ("New connection added: ", clientAddress)
    def run(self):
        print ("Connection from : ", self.caddr)
        LicencePlateDetector(self.csocket, self.client_num)
        print ("Client at ", self.caddr , " disconnected...")


def main():
    LOCALHOST = "127.0.0.1"
    PORT = 9999
    client_number = 0

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((LOCALHOST, PORT))
    print("Server started")
    print("Waiting for client request..")

    while True:
        print("Listening")
        server.listen(1)
        clientsock, clientAddress = server.accept()
        newthread = ClientThread(clientAddress, clientsock, client_number)
        newthread.start()


if __name__ == "__main__":
    main()
