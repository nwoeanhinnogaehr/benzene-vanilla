import subprocess
from fcntl import fcntl, F_GETFL, F_SETFL
from os import O_NONBLOCK, read, getcwd
import sys
from time import sleep
from errno import EAGAIN
import Queue
import threading

size = 13
config = 'config.htp'
mohex = ['../src/mohex/mohex', '--boardsize', str(size), '--config', config]
books11 = {
        'a2': '11-a2.db',
        'a6': '11-a6.db'
        }
books13 = {
        'a7': '13-a7.db',
        'a8': '13-a8.db',
        'b2': '13-b2.db',
        'l12': '13-l12.db'
        }
books = books13
basedir = getcwd() + '/../../mohex-books/'


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

process = subprocess.Popen(mohex, stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=100000)

flags = fcntl(process.stdout, F_GETFL)
fcntl(process.stdout, F_SETFL, flags | O_NONBLOCK)
flags = fcntl(sys.stdin, F_GETFL)
fcntl(sys.stdin, F_SETFL, flags | O_NONBLOCK)
opening = None
playRotate = False
genmove = False
supress = False
while not process.poll():
    # handle input
    try:
        i = sys.stdin.read()
        sys.stderr.write("BEGIN" + i + "END")

        # open book
        if not opening:
            if i.startswith("play black"):
                opening = i.split(' ')[2].strip()
                if opening in books:
                    process.stdin.write('book-open ' + basedir + books[opening] + '\n')
                    process.stdin.flush()
                    supress = True
                elif rotate(opening) in books:
                    playRotate = True
                    opening = rotate(opening)
                    process.stdin.write('book-open ' + basedir + books[opening] + '\n')
                    process.stdin.flush()
                    supress = True

        # handle play with rotated book
        if playRotate and i.startswith('play '):
            t = i[:11] + rotate(i[11:].strip()) + '\n\n'
            process.stdin.write(t)
            process.stdin.flush()

        # handle genmovee with rotated book
        elif playRotate and i.startswith('genmove '):
            genmove = True
            process.stdin.write(i)
            process.stdin.flush()

        # else pass through
        else:
            process.stdin.write(i)
            process.stdin.flush()
    except IOError, e:
        if e.errno != EAGAIN:
            raise
    # handle output
    try:
        o = process.stdout.read()
        sys.stderr.write("BEGIN" + o + "END")

        if not supress:
            # handle genmvoe with rotated book
            if playRotate and genmove and o.startswith('= '):
                t = o[:2] + rotate(o[2:].strip()) + '\n\n'
                sys.stdout.write(t)
                sys.stdout.flush()
                genmove = False

            # else pass through
            else:
                sys.stdout.write(o)
                sys.stdout.flush()
        else:
            sys.stdout.write('= \n\n')
            sys.stdout.flush()
        supress = False
    except IOError, e:
        if e.errno != EAGAIN:
            raise

    # to avoid threading issues, we just poll the process.
    # don't do it too often, it eats cpu
    sleep(0.01)
