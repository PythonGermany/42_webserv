# TODO

- [x] chunked POST
- [ ] fcntl subject
- [ ] remove inet_ntop


- [x] Implement and fix chunked request trailer parsing
- [x] Figure out why resolving symlinks doesnt work
- [x] Replace c function for wildcard pattern matching with own function
- [x] Figure out why login for wordpress doesn't work
- [x] Implement cgi support
- [x] Fix issue of cases like /uploaded/../listing/index.php where restrictions of one context can be bypassed
- [x] Implement resolving of . and .. for request uri (also updates context accordingly since this will happen before context selection)
- [x] Fix file uploading
- [x] Implement support for multilistendirective or change config
- [x] Implement chunked transfer encoding (https://datatracker.ietf.org/doc/html/rfc2616#section-3.6 
                                          && https://datatracker.ietf.org/doc/html/rfc2616#section-19.4.6)

- [ ] <del>Is is possible for client to send data whilst cgi is processing?</del>
- [ ] <del>(Optional) Fix problem of no server response in case of connection being destructed due to timeout, in every case but especially if cgi is active</del>
- [ ] <del>(Optional) Implement try_files or similar</del>
- [ ] <del>(Does not occur anymore without having actively tried to fix it) Investigate random closing of program without any notice after sending a first request</del>
- [ ] <del>Maybe use streambufs instead of streams https://gcc.gnu.org/onlinedocs/libstdc++/manual/fstreams.html#std.io.filestreams.binary</del>
- [ ] <del>Maybe implement configurable default mime</del>