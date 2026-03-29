# Caching Proxy Server

A lightweight HTTP caching proxy written in C++ that forwards requests to an origin server, stores responses in memory, and serves repeated requests directly from cache.

## Overview

This project demonstrates a simple reverse-proxy style cache using:

- low-level POSIX sockets for handling HTTP requests
- `libcurl` for fetching data from the origin server
- an in-memory `unordered_map` as the cache store

The server listens on port `4000`, forwards incoming requests to `http://dummyjson.com`, and adds an `X-Cache` header to indicate whether the response came from cache or the origin.

## Features

- Accepts HTTP requests over a raw TCP socket
- Proxies requests to the configured origin server
- Caches responses in memory by request method and path
- Returns `X-Cache: MISS` for first-time requests
- Returns `X-Cache: HIT` for repeated requests
- Minimal implementation suitable for learning and extension

## How It Works

1. The server listens on port `4000`.
2. A client sends a request such as `GET /products/1`.
3. The server parses the HTTP method and path.
4. It builds a cache key in the format `METHOD:path`.
5. If the key exists in memory, it returns the cached response with `X-Cache: HIT`.
6. Otherwise, it fetches data from `http://dummyjson.com`, stores the response in cache, and returns it with `X-Cache: MISS`.

## Project Structure

```text
.
├── server.cpp   # Source code for the caching proxy
├── server       # Compiled binary
└── README.md
```

## Requirements

- Linux environment
- `g++`
- `libcurl`

## Build

Compile the server with:

```bash
g++ server.cpp -o server -lcurl
```

## Run

Start the proxy server:

```bash
./server
```

Expected startup output:

```text
Server running on port 4000
```

## Usage

Send a request through the proxy:

```bash
curl -i http://127.0.0.1:4000/products/1
```

Example first response headers:

```http
HTTP/1.1 200 OK
Content-Type: application/json
X-Cache: MISS
```

Repeat the same request:

```bash
curl -i http://127.0.0.1:4000/products/1
```

Expected repeated response headers:

```http
HTTP/1.1 200 OK
Content-Type: application/json
X-Cache: HIT
```

## Example Behavior

- First request to `/products/1`: fetched from origin and cached
- Second request to `/products/1`: served from memory cache
- First request to a different path such as `/products/2`: fetched from origin again

## Verification

The current binary was run locally and verified with live requests:

- `GET /products/1` returned `200 OK` with `X-Cache: MISS`
- Repeating `GET /products/1` returned `200 OK` with `X-Cache: HIT`
- `GET /products/2` returned `200 OK` with `X-Cache: MISS`

This confirms that the proxy fetches origin data and caches repeated requests as intended.

## Current Implementation Notes

This project is intentionally minimal. The current implementation:

- uses an in-memory cache with no expiration
- always responds with `200 OK`
- assumes JSON responses
- handles requests sequentially
- does not implement cache invalidation
- does not validate HTTP parsing robustly
- does not support multithreading or persistence

## Limitations

This is a learning/demo project, not a production-ready proxy. Important limitations include:

- no TTL or eviction policy
- no concurrency control
- limited HTTP parsing
- no upstream error handling in the response status
- no configuration via environment variables or CLI flags
- no logging framework or tests

## Possible Improvements

- add configurable origin URL and port
- add cache expiration and size limits
- preserve upstream status codes and headers
- support concurrent clients
- improve request parsing and validation
- add unit and integration tests
- add graceful shutdown and signal handling

## Origin Service

The current origin configured in code is:

```text
http://dummyjson.com
```

Requests are proxied by concatenating the incoming path with this base URL.

## License

No license file is currently included in this repository. Add one if you plan to distribute or publish the project.
