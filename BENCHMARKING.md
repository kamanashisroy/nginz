

Apache Benchmarking tool with 1 Million request with 10K concurrency on my linux laptop. 

```
ab -r -n 1000000 -c 10000 http://localhost:80/
This is ApacheBench, Version 2.3 <$Revision: 655654 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 100000 requests
Completed 200000 requests
Completed 300000 requests
Completed 400000 requests
Completed 500000 requests
Completed 600000 requests
Completed 700000 requests
Completed 800000 requests
Completed 900000 requests
Completed 1000000 requests
Finished 1000000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            80

Document Path:          /
Document Length:        11 bytes

Concurrency Level:      10000
Time taken for tests:   259.240 seconds
Complete requests:      1000000
Failed requests:        2454
   (Connect: 0, Receive: 818, Length: 818, Exceptions: 818)
Write errors:           0
Total transferred:      48987946 bytes
HTML transferred:       10997294 bytes
Requests per second:    3857.42 [#/sec] (mean)
Time per request:       2592.404 [ms] (mean)
Time per request:       0.259 [ms] (mean, across all concurrent requests)
Transfer rate:          184.54 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0 1721 2515.5    889   32106
Processing:    62  844 1515.9    807   63958
Waiting:        0  631 140.7    628    4817
Total:        209 2566 2929.1   1747   63958

Percentage of the requests served within a certain time (ms)
  50%   1747
  66%   2037
  75%   2594
  80%   2733
  90%   4542
  95%   4877
  98%   8683
  99%  16549
 100%  63958 (longest request)
```

We did some perfomance update
==============================
ab -r -n 1000000 -c 12000 http://localhost:80/

```
Concurrency Level:      12000
Time taken for tests:   224.543 seconds
Complete requests:      1000000
Failed requests:        2409
   (Connect: 0, Receive: 803, Length: 803, Exceptions: 803)
Write errors:           0
Total transferred:      48993238 bytes
HTML transferred:       10998482 bytes
Requests per second:    4453.49 [#/sec] (mean)
Time per request:       2694.516 [ms] (mean)
Time per request:       0.225 [ms] (mean, across all concurrent requests)
Transfer rate:          213.08 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0 1644 1828.4   1140   32307
Processing:    99 1032 1666.8    989   64408
Waiting:        0  741 332.8    731   36514
Total:        315 2676 2483.7   2169   64408

Percentage of the requests served within a certain time (ms)
  50%   2169
  66%   2437
  75%   2872
  80%   3066
  90%   3608
  95%   4963
  98%   8785
  99%   9217
 100%  64408 (longest request)
```
