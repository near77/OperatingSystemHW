from TOOLS.server_mod import DataProcessor
import hdfs
video = open("OS_HW2/01.mp4", "rb")

DP = DataProcessor()
DP.DataProcess(video)