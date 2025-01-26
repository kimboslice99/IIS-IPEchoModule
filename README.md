# IPEcho Module

## Overview
The **IPEcho Module** is an IIS module designed to echo the remote IP address of a client back to the client as a simple plaintext response.

## Features
- Retrieves the client's IP address using the `REMOTE_ADDR` server variable.
- Returns the IP address in the response body as plaintext.

```
Statistics        Avg      Stdev        Max
  Reqs/sec    143903.67   12410.24  159803.20
  Latency        0.86ms    64.10us    10.32ms
  HTTP codes:
    1xx - 0, 2xx - 1438746, 3xx - 0, 4xx - 0, 5xx - 0
    others - 0
  Throughput:    30.32MB/s
```

## Requirements
- IIS 7.0 or higher.
