#!/usr/bin/python

'''
Following instructions will create a disk image with following disk geometry parameters:
* size in megabytes = 64
* amount of cylinders = 130
* amount of headers = 16
* amount of sectors per track = 63
'''

import os, subprocess
import shlex
import sys
import re
import platform
import subprocess

class color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def info(c, s):
    print c + s + color.ENDC


def panic(s):
    print color.BOLD + color.FAIL + s + color.ENDC
    sys.exit(1)

def run(cmd):
    if os.system(cmd) != 0:
        panic ("%s executed with error. exit." % cmd)

def exe(cmd):
    proc = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    return out

def shell(cmd):
    proc = subprocess.Popen(shlex.split(cmd), stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    return proc

def grep(s, pattern):
    return '\n'.join(re.findall(r'^.*%s.*?$'%pattern,s,flags=re.M))

class OS:
    Unknown = 0
    Mac = 1
    Linux = 2

def ostype():
    return {
    'Darwin': OS.Mac,
    'Linux': OS.Linux,
    }.get(platform.system (), OS.Unknown)

info (color.HEADER, 'Building Certikos Image...')

info (color.HEADER, '\ncreating disk...')
run(('dd if=/dev/zero of=certikos.img bs=512 count=%d' % (130 * 16 * 63)))

if ostype() == OS.Linux:
    run('parted -s certikos.img \"mktable msdos mkpart primary 2048s -1s set 1 boot on\"')
elif ostype() == OS.Mac:
    # mp = exe('hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount certikos.img').strip()
    # if mp.startswith('/dev/disk'):
    #     di = exe('diskutil info %s' % mp)
    #     if '67.1 MB' in grep(di, 'Disk Size:') and 'Disk Image' in grep(di, 'Device / Media Name:'):
    #        info(color.HEADER, 'Build disk partition and file system.')
    #        run('diskutil eraseDisk fuse-ext2 certikos MBR %s' % mp)
    #        fd = shell('fdisk -e certikos.img')
    #        fd.stdin.write('flag 1\n')
    #        fd.stdin.write('write\n')
    #        fd.stdin.write('quit\n')
    #     else:
    #         panic('Error: fail to attach certikos.img')
    # run('diskutil eject %s' % mp)
    exe('dd if=misc/partition_table of=certikos.img bs=512 count=1 conv=notrunc');
else:
    panic('Unknown host system. Abort')

info (color.OKGREEN, 'done.')

info (color.HEADER,  '\nwriting mbr...')
run('dd if=obj/boot/boot0 of=certikos.img bs=446 count=1 conv=notrunc')
run('dd if=obj/boot/boot1 of=certikos.img bs=512 count=62 seek=1 conv=notrunc')
info (color.OKGREEN, 'done.')

info (color.HEADER,  '\ncopying kernel files...')
if ostype() == OS.Linux:
    loc = int(grep(exe('fdisk -l certikos.img'), 'certikos.img1').split()[2])
elif ostype() == OS.Mac:
    loc = int(grep(exe('fdisk -d certikos.img'), '0x83').split(',')[0])
else:
    panic('Unknown host system. Abort')

if loc == 0:
    panic ("cannot find valid partition.");

info (color.OKBLUE, 'kernel starts at sector %d' % loc)
run('dd if=obj/kern/kernel of=certikos.img bs=512 seek=%d conv=notrunc' % loc)

info (color.OKGREEN + color.BOLD, '\nAll done.')
sys.exit(0)


