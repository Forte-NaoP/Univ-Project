import sys
import random



if __name__ == "__main__":

    if len(sys.argv) != 2:
        SIZE = 5 * 1024
    elif sys.argv[1] == "P1":
        SIZE = 5 * 1024
    elif sys.argv[1] == "P2":
        SIZE = 1024 * 1024
    else:
        SIZE = 50 * 1024 * 1024

    f = open(f"./{sys.argv[1]}/test_file.txt", "w")

    for i in range(0, SIZE):
        f.write(str(random.randrange(0, 9)))

    f.close()

