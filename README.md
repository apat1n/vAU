# vAU
Мессенджер с исходным кодом на C++ и на основе фреймворка Qt (дополнительно используется QWebSocket и QSqlDatabase)

Используемые библиотеки:
- qt-default
- libqt5websockets5-dev
- libqt5sql5-psql

Сборка сервера:
```
cd server
mkdir build && cd build
qmake .. && make && ./server -d
```

Сборка клиента:
```
cd server
mkdir build && cd build
qmake .. && make && ./client
```

Параметры запуска сервера:
- `-d | --debug` – отображать ли debug информацию в консоли (по умолчанию false)
- `--port | -p` – открытый порт сервера (по умолчанию 1234)

Параметры запуска клиента:
- `--ip` – ip-адрес сервера (по умолчанию localhost)
- `--port | -p` – открытый порт сервера (по умолчанию 1234)
