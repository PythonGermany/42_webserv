set -e

folder="/webserv/websites/wordpress"
if [ ! -d "$folder" ] || [ -z "$(ls -A "$folder")" ]; then
  mkdir -p $folder
  wp core download --path=$folder
fi

# Time to wait between checks (in seconds)
WAIT_TIME=5

# Maximum number of retries
MAX_RETRIES=30

# Counter for retries
retries=0

# Check if the MariaDB database is online
while [ $retries -lt $MAX_RETRIES ]; do
    if mariadb -h "$MYSQL_HOST" -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" -e "SELECT 1;" 2>/dev/null; then
        # Database is online, proceed with WordPress installation
        echo "MariaDB database is online. Proceeding with WordPress installation."
        break
    else
        retries=$((retries + 1))
        if [ $retries -ge $MAX_RETRIES ]; then
            echo "Max retries reached. Unable to connect to the MariaDB database."
            exit 1
        fi
        echo "MariaDB database is not online yet. Retrying in $WAIT_TIME seconds..."
        sleep "$WAIT_TIME"
    fi
done

if ! wp core is-installed --path=$folder; then
  echo "Installing and setting up wordpress..."
  # Install wordpress site
  wp config create --path=$folder --dbhost=$MYSQL_HOST \
    --dbname=$MYSQL_DATABASE --dbuser=$MYSQL_USER --dbpass=$MYSQL_PASSWORD --skip-check
  wp core install --path=$folder --url=$WORDPRESS_URL --title=$WORDPRESS_TITLE \
    --admin_user=$WORDPRESS_USER --admin_password=$WORDPRESS_PASSWORD --admin_email=$WORDPRESS_EMAIL
fi

make && ./bin/webserv $WEBSERV_ARGS