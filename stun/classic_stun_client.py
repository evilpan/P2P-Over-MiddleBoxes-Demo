#!/usr/bin/env python3
import io
import struct
import socket
import logging
from enum import Enum, unique
from random import randint

@unique
class MessageType(Enum):
    BINDING_REQUEST         = 0x0001
    BINDING_RESPONSE        = 0x0101
    BINDING_ERROR_RESPONSE  = 0x0111
    SHARED_SECRET_REQUEST   = 0x0002
    SHARED_SECRET_RESPONSE  = 0x0102
    SHARED_SECRET_ERROR     = 0x0112

@unique
class AttributeType(Enum):
    MAPPED_ADDRESS      = 0x0001
    RESPONSE_ADDRESS    = 0x0002
    CHANGE_REQUEST      = 0x0003
    SOURCE_ADDRESS      = 0x0004
    CHANGED_ADDRESS     = 0x0005
    USERNAME            = 0x0006
    PASSWORD            = 0x0007
    MESSAGE_INTEGRITY   = 0x0008
    ERROR_CODE          = 0x0009
    UNKNOWN_ATTRIBUTES  = 0x000a
    REFLECTED_FROM      = 0x000b
    XOR_MAPPED_ADDRESS  = 0x8020
    SERVER              = 0x8022
    SECONDARY_ADDRESS   = 0x8050

@unique
class NAT(Enum):
    PUBLIC      = 'The open Internet'
    UDP_BLOCKED = 'Firewall that blocks UDP'
    SYMMETRIC_UDP_FIREWALL = 'Firewall that allows UDP out, and responses have to come back to the source of the request'
    FULL_CONE = 'Full Cone NAT'
    SYMMETRIC = 'Symmetric NAT'
    PORT_RISTRICT  = 'Port Rristrict NAT'
    ADDR_RISTRICT  = '(Address) Rristrict NAT'

class StunHeader(object):
    """ 20 bytes header """
    def __init__(self, **kwargs):
        # 16 bits
        self.type = kwargs.pop('type', None)
        # 16 bits body length(excluding 20 bytes header)
        self.length = kwargs.pop('length', 0)
        # 128 bits
        self.transactionId = kwargs.pop('transactionId', randint(0, 0xFFFFFFFFFFFFFFFF))
        if len(kwargs) != 0:
            raise ValueError('unknown kwargs: {}'.format(kwargs))
    def to_bytes(self):
        return struct.pack('!HH', self.type.value, self.length) + \
                self.transactionId.to_bytes(16, 'big')
    @classmethod
    def from_bytes(cls, data):
        assert len(data) == 20
        _type, _len, _tid = struct.unpack('!HH16s', data)
        return cls(
                type=MessageType(_type),
                length=_len,
                transactionId = int.from_bytes(_tid, 'big'))
    def __str__(self):
        return '<{}|{}|{:X}>'.format(
                self.type.name if self.type else None,
                self.length, self.transactionId)

class StunAttribute(object):
    HEADER_LENGTH = 4
    def __init__(self, **kwargs):
        self.type = kwargs.pop('type', None)
        self.length = kwargs.pop('length', 0)
        self.value = kwargs.pop('value', b'')
        if len(kwargs) != 0:
            raise ValueError('unknown param: {}'.format(kwargs))
    @classmethod
    def change_request(cls, change_addr=False, change_port=False):
        change_addr = '1' if change_addr else '0'
        change_port = '1' if change_addr else '0'
        # padding is unnecessary
        v = int('0' * 29 + change_addr + change_port + '0', 2)
        _binary = struct.pack('!I', v)
        return cls(type=AttributeType.CHANGE_REQUEST,
                length=len(_binary),
                value=_binary)

    def to_bytes(self):
        self.length = len(self.value)
        return struct.pack('!HH', self.type.value, self.length) + self.value
    def is_address(self):
        return self.length == 8 and self.type in [
                AttributeType.MAPPED_ADDRESS,
                AttributeType.RESPONSE_ADDRESS,
                AttributeType.CHANGED_ADDRESS]
    @property
    def address(self):
        if self.is_address():
            _, _family, port, ip = struct.unpack('!cBHI', self.value)
            return socket.inet_ntoa(struct.pack('!I', ip)), port
    def __str__(self):
        if self.is_address():
            return '<Attr {}|{}:{}>'.format(self.type.name,
                    self.address[0], self.address[1])
        else:
            return '<Attr {}>'.format(self.type.name if self.type else None)

class Message(object):
    def __init__(self, **kwargs):
        self.header = kwargs.pop('header', None)
        self.attributes = kwargs.pop('attributes', [])
    def to_bytes(self):
        # network order (big endian)
        _header = b''
        _body = b''
        for attr in self.attributes:
            _body += attr.to_bytes()
        self.header.length = len(_body)
        _header = self.header.to_bytes()
        return _header + _body
    @classmethod
    def from_bytes(cls, data):
        header = StunHeader.from_bytes(data[:20])
        attributes = []
        datalen = header.length
        f = io.BytesIO(data[20:])
        while datalen > 0:
            _type, _len = struct.unpack('!HH', f.read(StunAttribute.HEADER_LENGTH))
            _value = f.read(_len)
            attributes.append(StunAttribute(
                type=AttributeType(_type),
                length=_len,
                value=_value))
            datalen -= StunAttribute.HEADER_LENGTH + _len
        return cls(header=header, attributes=attributes)
    def __str__(self):
        return '{}: [{}]'.format(self.header,
                ','.join(map(str, self.attributes)))

class StunTimeout(Exception):
    pass

class TestResult(object):
    def __init__(self):
        self.timeout = True
        self.ip = '255.255.255.255'
        self.port = 0
    def host_same(self, host):
        return host == (self.ip, self.port)

def test_I(sock, stun_server):
    logging.info('running test I')
    result = TestResult()
    binding_request = Message(header=StunHeader(type=MessageType.BINDING_REQUEST))
    logging.debug('SEND: {}'.format(binding_request))
    sock.sendto(binding_request.to_bytes(), stun_server)
    try:
        data, addr = sock.recvfrom(4096)
    except socket.timeout as e:
        return result
    response = Message.from_bytes(data)
    logging.debug('RECV: {}'.format(response))
    result.timeout = False
    for attr in response.attributes:
        if attr.type is AttributeType.MAPPED_ADDRESS:
            result.ip, result.port = attr.address
            break
    return result

def test_II(sock, stun_server):
    logging.info('running test II')
    result = TestResult()
    binding_request = Message(header=StunHeader(type=MessageType.BINDING_REQUEST))
    change = StunAttribute.change_request(False, True)
    binding_request.attributes.append(change)
    logging.debug('SEND: {}'.format(binding_request))
    sock.sendto(binding_request.to_bytes(), stun_server)
    try:
        data, addr = sock.recvfrom(4096)
    except socket.timeout:
        return result
    response = Message.from_bytes(data)
    logging.debug('RECV: {}'.format(response))
    for attr in response.attributes:
        if attr.type is AttributeType.MAPPED_ADDRESS:
            result.ip, result.port = attr.address
            break
    return result

def test_III(sock, stun_server):
    logging.info('running test III')
    result = TestResult()
    result = TestResult()
    binding_request = Message(header=StunHeader(type=MessageType.BINDING_REQUEST))
    change = StunAttribute.change_request(True, True)
    binding_request.attributes.append(change)
    logging.debug('SEND: {}'.format(binding_request))
    sock.sendto(binding_request.to_bytes(), stun_server)
    try:
        data, addr = sock.recvfrom(4096)
    except socket.timeout as e:
        return result
    response = Message.from_bytes(data)
    logging.debug('RECV: {}'.format(response))
    for attr in response.attributes:
        if attr.type is AttributeType.MAPPED_ADDRESS:
            result.ip, result.port = attr.address
            break
    return result

def test_nat(sock, stun_server, local_ip='', local_port=0):
    # Please 
    source = (local_ip, local_port)
    ret = test_I(sock, stun_server)
    if ret.timeout:
        return NAT.UDP_BLOCKED
    if ret.host_same(source):
        ret = test_II(sock, stun_server)
        if ret.timeout:
            return NAT.SYMMETRIC_UDP_FIREWALL
        return NAT.PUBLIC
    source = (ret.ip, ret.port)
    logging.info('MAPPED_ADDRESS: {}:{}'.format(ret.ip, ret.port))
    ret = test_II(sock, stun_server)
    if not ret.timeout:
        return NAT.FULL_CONE
    ret = test_I(sock, stun_server)
    logging.info('MAPPED_ADDRESS: {}:{}'.format(ret.ip, ret.port))
    if not ret.host_same(source):
        return NAT.SYMMETRIC
    ret = test_III(sock, stun_server)
    if ret.timeout:
        return NAT.PORT_RISTRICT
    else:
        return NAT.ADDR_RISTRICT

def main():
    logging.basicConfig(level=logging.INFO)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2.0)
    ntype = test_nat(sock, ('stun.ekiga.net', 3478))
    print('Your NAT type is: ' + ntype.value)

if __name__ == '__main__':
    main()
