

Apache Benchmarking tool with 1 Million request with 10K concurrency on my linux laptop. 

```
ab -r -n 100000 -c 10000 http://localhost:80/
This is ApacheBench, Version 2.3 <$Revision: 655654 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            80

Document Path:          /
Document Length:        11 bytes

Concurrency Level:      10000
Time taken for tests:   10.617 seconds
Complete requests:      100000
Failed requests:        0
Write errors:           0
Total transferred:      4900000 bytes
HTML transferred:       1100000 bytes
Requests per second:    9418.73 [#/sec] (mean)
Time per request:       1061.714 [ms] (mean)
Time per request:       0.106 [ms] (mean, across all concurrent requests)
Transfer rate:          450.70 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:      117  579 683.4    416    7497
Processing:   118  392 137.4    399    7036
Waiting:       16  236 156.3    214    7003
Total:        345  971 715.9    813   10371

Percentage of the requests served within a certain time (ms)
  50%    813
  66%    907
  75%    969
  80%    987
  90%   1649
  95%   1723
  98%   2136
  99%   3797
 100%  10371 (longest request)

```

