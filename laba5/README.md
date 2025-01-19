# Руководство

## Docker
Для поднятия СУБД Postgres можно воспользоваться контейнеризацией docker и [docker-compose.yaml](docker-compose.yaml)

[Установка Docker](https://docs.docker.com/engine/install/)

Далее, достаточно написать
> docker compose up -d

Создастся инстанс СУБД, из контейнера пробросится порт 5432 создадутся таблицы при помощи [init.sql](init.sql).


## Виртуальные драйверы для COM портов

Для корректной работы с COM портами необходимо установить виртуальные драйверы.

Для Windows используйте com0com. Установите его и настройте виртуальные COM порты.

Для Linux используйте socat.

> socat -d -d pty,raw,echo=0 pty,raw,echo=0


## Клиентское-веб приложение на python
Для отображения данных с сервера используется приложение на python с использованием фреймворка FastAPI.

Установите fastapi
> pip install fastapi

Установите ASGI веб-сервер uvicorn
> pip install uvicorn 

Запустите сервер:
> cd python_http && uvicorn main:app --reload

## Сборка проекта

Для работы с базой данных используется библиотека [pqxx](https://pqxx.org/development/libpqxx/), сначала установите её.


Для сборки проекта с использованием CMake выполните следующие команды:

> mkdir build && cd build && cmake .. && cmake --build .


Необходимо запустить генератор температуры:

windows:

> ./temperature_generator.exe


linux:

> ./temperature_generator

И после, сам сервер:

windows:

> ./main.exe

linux:

> ./main

