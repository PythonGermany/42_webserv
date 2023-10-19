# TODO

- [ ] Needed? -> Implement so that and alias updating an uri also updates context
- [ ] Investigate NS_ERROR_NET_RESET error for put request 404 and 413 response
- [ ] Figure out why resolving symlinks doesnt work

- [ ] (Does not occur anymore without having actively tried to fix it) Investigate random closing of program without any notice after sending a first request

- [x] Replace c function for wildcard pattern matching with own function
- [x] Figure out why login for wordpress doesn't work
- [x] Implement cgi support
- [x] Fix issue of cases like /uploaded/../listing/index.php where restrictions of one context can be bypassed
- [x] Implement resolving of . and .. for request uri (also updates context accordingly since this will happen before context selection)
- [x] Fix file uploading
- [x] Implement support for multilistendirective or change config
- [x] Implement chunked transfer encoding (https://datatracker.ietf.org/doc/html/rfc2616#section-3.6 
                                          && https://datatracker.ietf.org/doc/html/rfc2616#section-19.4.6)

- [ ] <del>Maybe use streambufs instead of streams https://gcc.gnu.org/onlinedocs/libstdc++/manual/fstreams.html#std.io.filestreams.binary</del>
- [ ] <del>Maybe implement configurable default mime</del>

- [ ] Remove unwanted submodules and config once ready for evaluation