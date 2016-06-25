import subprocess
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read
import sys
from time import sleep
from errno import EAGAIN

size = 13
config = 'config.htp'
mohex = ['../src/mohex/mohex', '--boardsize', str(size), '--config', config]
books = {
        'l12': 'cg2016-book-l12.db.230',
        # TODO
        }


def rotate(move):
    x = ord(move[0]) - ord('a')
    y = int(move[1:])
    x = size - x
    y = size - y
    return chr(ord('a') + x - 1) + str(y + 1)

assert(rotate('l12') == 'b2')
assert(rotate('b2') == 'l12')
assert(rotate('a1') == 'm13')
assert(rotate('m13') == 'a1')
assert(rotate('g7') == 'g7')

process = subprocess.Popen(mohex, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
flags = fcntl(process.stdout, F_GETFL)
fcntl(process.stdout, F_SETFL, flags | O_NONBLOCK)
flags = fcntl(sys.stdin, F_GETFL)
fcntl(sys.stdin, F_SETFL, flags | O_NONBLOCK)
opening = None
playRotate = False
genmove = False
while not process.poll():
    # handle input
    try:
        i = sys.stdin.readline()

        # open book
        if not opening:
            if i.startswith("play b"):
                opening = i.split(' ')[2].strip()
                if opening in books:
                    process.stdin.write('book-open ' + books[opening] + '\n')
                if rotate(opening) in books:
                    playRotate = True
                    opening = rotate(opening)
                    process.stdin.write('book-open ' + books[opening] + '\n')

        # handle play with rotated book
        if playRotate and i.startswith('play '):
            t = i[:7] + rotate(i[7:].strip()) + '\n'
            process.stdin.write(t)

        # handle genmovee with rotated book
        elif playRotate and i.startswith('genmove '):
            genmove = True
            process.stdin.write(i)

        # else pass through
        else:
            process.stdin.write(i)
    except IOError, e:
        if e.errno != EAGAIN:
            raise

    # handle output
    try:
        o = process.stdout.readline()

        # handle genmvoe with rotated book
        if playRotate and genmove and o.startswith('= '):
            t = o[:2] + rotate(o[2:].strip()) + '\n'
            sys.stdout.write(t)
            genmove = False

        # else pass through
        else:
            sys.stdout.write(o)
    except IOError:
        if e.errno != EAGAIN:
            raise

    # to avoid threading issues, we just poll the process.
    # don't do it too often, it eats cpu
    sleep(0.01)
