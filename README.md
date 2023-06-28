# Echo Servers

Simple TCP/UDP echo servers and clients to measure ping-pong style roundtrip time.

# Building
```
mkdir echo-build
cd echo-build
export CC=gcc-13
export CXX=gcc-13
export CFLAGS="-Wall -O3"
cmake ../echo
make
```

# Executing
```
one machine ./tcp_blocking_server
other machine ./tcp_blocking_client <IP> 8080 100000
```
## loopback interface latency
isolate CPU cores 10 and 11
```
taskset -c 10 ./udp_spinning_server 127.0.0.1 8080
taskset -c 11 ./udp_spinning_client 127.0.0.1 8080 5000000
```
**The user space networking solution with dedicated network hardware is required for achieving ultimate roundtrip latencies.**
