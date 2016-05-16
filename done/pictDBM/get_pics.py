import os
from sys import argv
from subprocess import getstatusoutput
from random import randint

#source = "http://placekitten.com/"
source = "https://unsplash.it/"
dest_folder = argv[1]
how_many_pics = int(argv[2])

for pic in range(how_many_pics):
    resX = str(randint(50, 1280))
    resY = str(randint(50, 1280))
    url = source + resX + "/" + resY
    filename = resX + "x" + resY + ".jpg "
    assert getstatusoutput("wget -O " + dest_folder + "/" + filename + url)[0] == 0, \
            "Could not download " + url

    print(str(pic + 1) + "/" + str(how_many_pics))

