# P2P-Over-MiddleBoxes-Demo

一个简单的P2P通信示例

[English README](README_en.md)

# p2pchat

一个P2P聊天程序，使用UDP打洞创建链接。

## 编译
    
    make p2pchat

## 运行

    ./p2pchat/server <服务器端口号>

    ./p2pchat/client <服务器IP>:<服务器端口号>
    >>> help

# 测试：

## 编译

    make test

## 运行
    
    ./run_test.sh

# 常见问题

## 通信不正常/打洞不成功？
该UDP打洞示例仅支持锥形地址转换器（Cone NAT），如果两个客户端都在同一个公网结点下，需要确保出口路由器支持**回环传输（LOOPBACK TRANSMISSION）**。

## 我咋知道我的NAT是什么类型？
在[stun目录下](stun)有个简单的Python脚本，用RFC3489（经典STUN协议）的示例来检测NAT类型。
运行：
```
cd stun
python3 classic_stun_client.py [本地IP]
```

运行结果示例如下：
```
INFO:root:running test I with stun.ideasip.com:3478
INFO:root:MAPPED_ADDRESS: 220.181.57.217:46208
INFO:root:running test II with stun.ideasip.com:3478
INFO:root:running test I with 217.116.122.138:3479
INFO:root:MAPPED_ADDRESS: 220.181.57.217:2732
NAT_TYPE: Symmetric NAT
```


# 相关介绍文章

- [https://evilpan.com/2015/10/31/p2p-over-middle-box/][blog]

> 注: 本项目只是一个简单的UDP打洞示例，如果想构建成熟的P2P应用，可以接着参考STUN/TURN以及ICE等协议。

[jekyll]:http://jekyll.pppan.net/2015/10/31/p2p-over-middle-box/
[django]:https://www.pppan.net/blog/detail/2017-12-16-p2p-over-middle-box
[blog]: https://evilpan.com/2015/10/31/p2p-over-middle-box/
