import os
from sys import argv
from subprocess import getstatusoutput
import random

executable = argv[1]
pics_path = argv[2]
db = "dbTEST" #+ datetime.datetime.now().strftime("%Y%m%d-%H%M%S") 

allpics = [pics_path + "/" + filename for filename in os.listdir(pics_path) if filename.endswith(".jpg")]

assert getstatusoutput("./" + executable + " create " + db + " -max_files 10000")[0] == 0
print("Database created")

def insert_cmd(pict_id, filename):
    return getstatusoutput("./" + executable + " insert " + db + " " + pict_id + " " + filename)[0]

def read_cmd(pict_id, resolution):
    return getstatusoutput("./" + executable + " read " + db + " " + pict_id + " " + resolution)[0]

def addAll(pics):
    for index, image in enumerate(pics):
        exit_code = insert_cmd(str(index), image)
        assert exit_code == 0, "Could not add "+ image + " with id " + str(index) + " error code: " + str(exit_code)
    print("Added all pictures in folder correctly")

def resizeAll(allpics):
    for index, image in enumerate(allpics):
        for resolution in ["thumb", "small"]:
            exit_code = read_cmd(str(index), resolution)
    assert exit_code == 0, "Could not read " + str(index) + " from db, exit code: " + output
    getstatusoutput("rm *small.jpg")
    getstatusoutput("rm *thumb.jpg")
    print("Resized all pictures to all resolutions correctly")

def addAlreadyPresentId():
    for i in range(0, random.randint(0, len(allpics))):
        assert insert_cmd(str(random.randint(0, len(allpics) - 1)), random.choice(allpics)) != 0, "Error: dded image with id already in db!"
    print("Inserting a random number of already present id returns error")

def addAlreadyPresentimage(pics):
    l = len(pics)
    for i in range(1, random.randint(1, 20)):
        length_before_add = os.stat(db).st_size
        random_pic = random.choice(pics)
        output = insert_cmd(str(random.randint(l + 1, l + 100)), random_pic)
        assert output == 0, "Error: adding already present image returns error"
        assert os.stat(db).st_size < length_before_add + os.stat(random_pic).st_size
    print("Adding already present image does not return error")
    print("Adding already present image deduplicates")

addAll(allpics)
resizeAll(allpics)
addAlreadyPresentId()
addAlreadyPresentimage(allpics)
