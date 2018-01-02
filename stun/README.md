
# Classic STUN ([RFC3489][rfc3489])

The classic STUN datagram structure is as follow(TLV encoded):
All STUN messages consist of a 20 byte header:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      STUN Message Type        |         Message Length        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                         Transaction ID
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                                                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

After the header are 0 or more attributes.  Each attribute is TLV
encoded, with a 16 bit type, 16 bit length, and variable value:

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Type                  |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             Value                             ....
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```



```
Figure 2: Flow for type discovery process

                      +--------+
                      |  Test  |
                      |   I    |
                      +--------+
                           |
                           |
                           V
                          /\              /\
                       N /  \ Y          /  \ Y             +--------+
        UDP     <-------/Resp\--------->/ IP \------------->|  Test  |
        Blocked         \ ?  /          \Same/              |   II   |
                         \  /            \? /               +--------+
                          \/              \/                    |
                                           | N                  |
                                           |                    V
                                           V                    /\
                                       +--------+  Sym.      N /  \
                                       |  Test  |  UDP    <---/Resp\
                                       |   II   |  Firewall   \ ?  /
                                       +--------+              \  /
                                           |                    \/
                                           V                     |Y
                /\                         /\                    |
 Symmetric  N  /  \       +--------+   N  /  \                   V
    NAT  <--- / IP \<-----|  Test  |<--- /Resp\               Open
              \Same/      |   I    |     \ ?  /               Internet
               \? /       +--------+      \  /
                \/                         \/
                |                           |Y
                |                           |
                |                           V
                |                           Full
                |                           Cone
                V              /\
            +--------+        /  \ Y
            |  Test  |------>/Resp\---->Restricted
            |   III  |       \ ?  /
            +--------+        \  /
                               \/
                                |N
                                |       Port
                                +------>Restricted
```


[rfc3489]:https://www.ietf.org/rfc/rfc3489.txt
