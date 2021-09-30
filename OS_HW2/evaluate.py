ground_truth_file = open("01.txt", "r").readlines()
ground_truth = []

for line in ground_truth_file:
    ground_truth.append(line)
    
output_file = open("01.mp4.txt", "r").readlines()
output = []

for line in output_file:
    output.append(line)

# TP
TP = 0
for o in output:
    for g in ground_truth:
        if o == g:
            TP += 1

N_output = []
for i in range(1,5401):
    flag = 0
    for o in output:
        if i == int(o):
            flag = 1
    if flag == 0:
        N_output.append(i)


N_ground_truth = []
for i in range(1,5401):
    flag = 0
    for g in ground_truth:
        if i == int(g):
            flag = 1
    if flag == 0:
        N_ground_truth.append(i)

TN = 0
for o in N_output:
    for g in N_ground_truth:
        if o == g:
            TN += 1

print("TP: ", TP, "TN: ", TN)
print("Accuracy: ", (TP + TN )/5400)