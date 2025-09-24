FROM alpine:latest AS build-stage

RUN apk add --no-cache make g++

COPY . /app

RUN make -C /app fclean custom ARG="-static -O3"

RUN mkdir -p /etc/webserv/sites-enabled

FROM gcr.io/distroless/static-debian12:latest AS production-stage

COPY --from=build-stage /app/bin/webserv /usr/bin/webserv
COPY --from=build-stage /app/conf/webserv.conf /etc/webserv/
COPY --from=build-stage /app/conf/mime.types /etc/webserv/
COPY --from=build-stage /app/conf/sites-available/default.conf /etc/webserv/sites-enabled/
COPY --from=build-stage /app/websites/default/index.html /usr/share/webserv/html/

EXPOSE 80

CMD ["/usr/bin/webserv", "-o", "on"]
