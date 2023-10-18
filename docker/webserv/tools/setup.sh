set -e
useradd -u 1000 webserv
apt-get install -y make clang php-cgi php-mysql \
 php-curl php-dom php-imagick php-mbstring php-zip php-gd php-intl wget unzip

# Install wp-cli
wget -O wp-cli.phar https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar
chmod +x wp-cli.phar
mv wp-cli.phar /usr/local/bin/wp