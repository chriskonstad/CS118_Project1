---
title: 'CS118 Project 1'
author:
- Chris Konstad (504275045)
---

# Server Design
The server is designed as a single-threaded, single process server.  A TCP socket
is opened when the server starts and all requests all handled through their own
sockets using a blocking syscall to make the connection.

Each request is then read into a buffer and parsed for the requested file.
The server determines the response content-type by reading the file name
extension of the requested files.  As only HTML, JPEG/JPG, and GIF files are
supposed to be served, all other file type requests generate 403 Forbidden
errors.  If the requested file is not found, the 404 page is returned.

After the content to be returned is read into memory, the header is created and
the header and content are streamed out using `write`.

# Difficulties
% TODO

# Compiling and Running
Please see `README.md` for compilation and running instructions.


# HTTP Samples
## Request
```
GET /test/test.gif HTTP/1.1
Host: localhost:5042
Connection: keep-alive
Accept: image/webp,image/*,*/*;q=0.8
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36
            (KHTML, like Gecko) Chrome/47.0.2526.111 Safari/537.36
Referer: http://localhost:5042/test/test.html
Accept-Encoding: gzip, deflate, sdch
Accept-Language: en-US,en;q=0.8
```
* **GET**: This line has the HTTP request type (GET, POST, etc.), the path to return and the protocol type and version
* **Host**: This is the hostname of the server
* **Connection**: Specify the requested lifetime of the connection
* **User-Agent**: The client application's type (for handling different browsers different, for example)
* **Accept**: The media type the client accepts
* **Referer**: The page that linked to this page
* **Accept-Encoding**: The accepted encodings for the returned media
* **Accept-Language**: The accepted languages for the returned media (human languages)

## Response
```
HTTP/1.1 200 OK
Date: Sat, 23 Jan 2016 20:55:31
Server: CS118 Project 1
Last-Modified: Tue, 19 Jan 2016 01:31:02
Content-Type: text/html
```
* **HTTP/1.1**: The protocol type and version, along with the response code
* **Date**: The time and date the HTTP message was sent
* **Server**: Information about the server that generated the response (usually name, version, etc)
* **Last-Modified**: The last-modified time of the file serverd, used for cache invalidation
* **Content-Type**: The type of the file being returned
