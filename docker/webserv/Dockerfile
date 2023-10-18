FROM debian:latest

COPY tools/ /

RUN apt-get update && apt-get upgrade -y && bash setup.sh

EXPOSE 4242
EXPOSE 8079
EXPOSE 8080
EXPOSE 8081
EXPOSE 8082

WORKDIR /webserv
USER webserv

CMD ["/usr/bin/bash", "/start.sh"]