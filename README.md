# P2P-Over-MiddleBoxes-Demo
A simple demo of P2P communication over middle boxes such as NAT

## compile:
    
    make

## run:

    ./server host port

    ./client
    >>> help

# FAQ

## It doesn't work 
UDP hole punching works only if both of the peers' NAT types are `Cone NAT`.

## How to check whether my NAT is cone NAT
There're tools for manually check the external NAT's type in [tests](tests).
For example:

- 1) cd tests && make
- 2.a) run `nc -u -lvp 2222` and `nc -u -lvp 3333` on your public server.
- 2.b) if you don't have nc(netcat) on your server, run `tests/udp_server <port>` instead.
- 3) run `tests/udp_client` on your client
- 4) (`udp_client`) sendto server:2222 text
- 5) (`udp_client`) sendto server:3333 text
- 6) check your server output to see the output.

example server output:

```
$ ./tests/udp_server 3333
UDP bind on 0.0.0.0:3333
recv 4 bytes from [172.16.47.71:14781]: text
```

```
$ ./tests/udp_server 2222
UDP bind on 0.0.0.0:2222
recv 4 bytes from [172.16.47.71:14781]: text
```

For cone NAT, the `from` part should be the same.

## My NAT is cone NAT, but it still doesn't work
If your two peers are behind the same NAT, this NAT must support `LOOPBACK TRANSMISSION`
to forward messages. You can test it by using the utils(`udp_server/udp_client`) in [tests](tests)

# related post (in Chinese)

[https://www.pppan.net/2015/10/31/p2p-over-middle-box/](https://www.pppan.net/2015/10/31/p2p-over-middle-box/)
