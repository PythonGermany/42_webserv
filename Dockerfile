FROM alpine:latest AS build-stage

RUN apk add --no-cache make g++

COPY . /app

RUN make -C /app fclean custom ARG="-static -O3"

#FROM alpine:latest AS production-stage
FROM alpine:latest AS production-stage

COPY --from=build-stage /app/bin/webserv /usr/bin/webserv
COPY --from=build-stage /app/conf/webserv.conf /etc/webserv/
COPY --from=build-stage /app/conf/mime.types /etc/webserv/
COPY --from=build-stage /app/conf/sites-available/default.conf /etc/webserv/sites-available/

RUN mkdir /etc/webserv/sites-enabled
RUN ln -s /etc/webserv/sites-available/default.conf /etc/webserv/sites-enabled/

RUN mkdir -p /var/www/html
COPY --from=build-stage /app/websites/default/index.html /var/www/html/

EXPOSE 8080

CMD ["/usr/bin/webserv"]
