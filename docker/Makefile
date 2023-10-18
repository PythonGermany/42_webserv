ARG=

all: up

up:
	docker compose up -d

down:
	docker compose down

update:
	docker compose build --no-cache --force-rm $(ARG)