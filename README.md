# P2P-Over-MiddleBoxes-Demo
A simple demo of P2P communication over middle boxes such as NAT

# p2pchat

## build:
    
    make p2pchat

## run:

    ./p2pchat/server port

    ./p2pchat/client server:port
    >>> help

# test

## build:

    make test

## run:
    
    ./run_test.sh

# FAQ

## It doesn't work?
UDP hole punching works only if both of the peers' NAT types are `full cone NAT` or 
`(address) restrict cone NAT`, and doesn't work on `port restrict cone NAT` since we
have no idea which port of the NAT the peer will be mapped to.

## How to verify whether my NAT is cone NAT
There're tools for manually check the external NAT's type in [tools](tools).
For example:

```
1) make tool
2) run `tools/udp_server 2222` and `tools/udp_server 3333` on your public server.
3) run `tools/udp_client` on your client
4) (`udp_client`) sendto server:2222 text
5) (`udp_client`) sendto server:3333 text
6) check your server output to see the output.
```

example server output:

```
$ ./tools/udp_server 3333
UDP bind on 0.0.0.0:3333
recv 4 bytes from [172.16.47.71:14781]: text
```

```
$ ./tools/udp_server 2222
UDP bind on 0.0.0.0:2222
recv 4 bytes from [172.16.47.71:14781]: text
```

For cone NAT, the `from` part(external ip and port) should be the same.

## My NAT is full/restrict cone NAT, but it still doesn't work
If two of your peers are both behind the same NAT, this NAT must support `LOOPBACK TRANSMISSION`
to forward messages. You can test it by using the utils(`udp_server/udp_client`) in [tools](tools)

# related post (in Chinese)

- [https://www.pppan.net/blog/detail/2017-12-16-p2p-over-middle-box(up to date)][django]
- [http://jekyll.pppan.net/2015/10/31/p2p-over-middle-box/(original post)][jekyll]

> NOTE: This is just a proof of concept project. If you want to build a stable
> P2P application, please refer to STUN/TURN and ICE protocol as well.

[jekyll]:http://jekyll.pppan.net/2015/10/31/p2p-over-middle-box/
[django]:https://www.pppan.net/blog/detail/2017-12-16-p2p-over-middle-box
