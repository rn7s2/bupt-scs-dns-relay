# bupt-scs-dns-relay

北京邮电大学计算机学院计算机网络课程设计

DNS 中继服务器 （POSIX 实现）

- POSIX 兼容系统均支持，MacOS, BSD, Linux...
- 多线程服务器，采用线程池实现，自动检测最优线程数
- LRU Cache 实现 DNS 记录缓存
- 双 buffer 支持 DNS 规则文件热重载，无需重启服务器即可修改规则
- Trie 树实现高效域名匹配
- 基于独立线程和缓冲队列的 Log 系统，带有时间、等级和色彩提示
- 基于 UNIX 信号检测的 graceful shutdown 支持
- ...描述待续
