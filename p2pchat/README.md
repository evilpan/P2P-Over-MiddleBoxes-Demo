This p2pchat demo is based on UDP protocol, and works only when

1) both of peers are behind `Cone NAT`.
2) if both peers are behind the same NAT, then this NAT must support `loopback transmission` to forward message.

It's a proof-of-concept project, and not importing some standard protocol like STUN or TURN yet.
