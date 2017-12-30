#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from __future__ import print_function, absolute_import

import sys
import struct
import socket

def serialize(type, data):
    data = data.encode('utf-8')
    #return struct.pack('!H', 0x8964)\
    #        + struct.pack('!H', type)\
    #        + struct.pack('!I', len(data))\
    #        + data
    return struct.pack('!HHI{}s'.format(len(data)), 0x8964, type, len(data), data)

def deserialize(buf):
    head = 8
    m_magic, m_type, m_length = struct.unpack('!HHI', buf[:head])
    m_data = buf[head: head+m_length]
    return '(0x{:x}, {}, {}): {}'.format(m_magic, m_type, m_length, m_data.decode('utf-8'))

def main():
    if len(sys.argv) < 2:
        print('Usage %s type [text...]' % sys.argv[0])
        return
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server = ('127.0.0.1', 9999)
    res = sock.sendto(serialize(int(sys.argv[1]), ' '.join(sys.argv[2:])), server)
    data, addr = sock.recvfrom(1024)
    print('{}: {}'.format(addr, deserialize(data)))

if __name__ == '__main__':
    main()
