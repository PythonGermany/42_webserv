FROM debian:latest

COPY tools/ /

RUN apt-get update && apt-get upgrade -y && apt-get install -y make clang php-cgi php-mysql php-curl php-dom php-imagick php-mbstring php-zip php-gd php-intl

EXPOSE 4242
EXPOSE 8079
EXPOSE 8080
EXPOSE 8081
EXPOSE 8082

CMD ["/usr/bin/bash", "start.sh"]