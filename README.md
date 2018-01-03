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
This UDP hole punching demo only works on Cone NAT.

## How to check my NAT type?
There is a simple python script to test your NAT type using RFC3489(the classic STUN protocol) in [stun](stun).
You can simply check it by running:
```
cd stun
python3 classic_stun_client.py [your-local-ip]
```

And the result would be similar with:
```
INFO:root:running test I with stun.ideasip.com:3478
INFO:root:MAPPED_ADDRESS: 220.181.57.217:46208
INFO:root:running test II with stun.ideasip.com:3478
INFO:root:running test I with 217.116.122.138:3479
INFO:root:MAPPED_ADDRESS: 220.181.57.217:2732
NAT_TYPE: Symmetric NAT
```

## My NAT is cone NAT, but it still doesn't work
If two of your peers are both behind the same NAT, this NAT must support `LOOPBACK TRANSMISSION`
to forward messages. You can test it by using the utils(`udp_server/udp_client`) in [tools](tools)

# related post (in Chinese)

- [https://www.pppan.net/blog/detail/2017-12-16-p2p-over-middle-box(up to date)][django]
- [http://jekyll.pppan.net/2015/10/31/p2p-over-middle-box/(original post)][jekyll]

> NOTE: This is just a proof of concept project. If you want to build a stable
> P2P application, please refer to STUN/TURN and ICE protocol as well.

[jekyll]:http://jekyll.pppan.net/2015/10/31/p2p-over-middle-box/
[django]:https://www.pppan.net/blog/detail/2017-12-16-p2p-over-middle-box
