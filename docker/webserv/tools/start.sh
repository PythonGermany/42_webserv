set -e


folder="/webserv/tests/websites/wordpress"

if [ ! -d "$folder" ] || [ -z "$(ls -A "$folder")" ]; then
  mkdir -p $folder
  wp core download --path=$folder
fi

if ! wp core is-installed --path=$folder; then
  echo "Installing and setting up wordpress..."
  # Install wordpress site
  wp config create --path=$folder --dbhost=$MYSQL_HOST \
    --dbname=$MYSQL_DATABASE --dbuser=$MYSQL_USER --dbpass=$MYSQL_PASSWORD --skip-check
  wp core install --path=$folder --url=$$WORDPRESS_URL --title=$WORDPRESS_TITLE \
    --admin_user=$WORDPRESS_USER --admin_password=$WORDPRESS_PASSWORD --admin_email=$WORDPRESS_EMAIL
fi

make && ./webserv $WEBSERV_ARGS