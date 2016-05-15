import os
from sys import argv
from subprocess import getstatusoutput
import random
from datetime import datetime

executable = argv[1]
pics_path = argv[2]
db = "dbTEST" #+ datetime.datetime.now().strftime("%Y%m%d-%H%M%S") 

allpics = [pics_path + "/" + filename for filename in os.listdir(pics_path) if filename.endswith(".jpg")]

assert getstatusoutput("./" + executable + " create " + db + " -max_files 1000")[0] == 0

def insert_cmd(pict_id, filename):
    return getstatusoutput("./" + executable + " insert " + db + " " + pict_id + " " + filename)[0]

def read_cmd(pict_id, resolution):
    return getstatusoutput("./" + executable + " read " + db + " " + pict_id + " " + resolution)[0]

def addAll(pics):
    success = 0
    for index, image in enumerate(pics):
        success += insert_cmd(str(index), image)
    assert success == 0
    print("Added all pictures in folder correctly")

def resizeAll(allpics):
    success = 0
    for index, image in enumerate(allpics):
        for resolution in ["thumb", "small"]:
            success += read_cmd(str(index), resolution)
    assert success == 0
    getstatusoutput("rm *small.jpg")
    getstatusoutput("rm *thumb.jpg")
    print("Resized all pictures to all resolutions correctly")

def addAlreadyPresentId():
    for i in range(0, random.randint(0, len(allpics))):
        assert insert_cmd(str(random.randint(0, len(allpics) - 1)), random.choice(allpics)) != 0
    print("Inserting a random number of already present id returns error")

def addAlreadyPresentimage(pics):
    l = len(pics)
    length_before_add = os.stat(db).st_size
    random_pic = random.choice(pics)
    output = insert_cmd(str(random.randint(l + 1, l + 100)), random_pic)
    assert output == 0
    print("Adding already present image does not return error")
    assert os.stat(db).st_size < length_before_add + os.stat(random_pic).st_size
    print("Adding already present image deduplicates")

addAll(allpics)
resizeAll(allpics)
addAlreadyPresentId()
addAlreadyPresentimage(allpics)
