# Руководство

## Виртуальные драйверы для COM портов

Для корректной работы с COM портами необходимо установить виртуальные драйверы.

Для Windows используйте com0com. Установите его и настройте виртуальные COM порты.

Для Linux используйте socat.
```sh
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

## Сборка проекта

Для сборки проекта с использованием CMake выполните следующие команды:

```sh
mkdir build && cd build && cmake .. && cmake --build .
```

Необходимо запустить генератор температуры:

windows:
```sh
./temperature_generator.exe
```

linux:
```sh
./temperature_generator
```

И после, сам логгер:

windows:
```sh
./main.exe
```

linux:
```sh
./main
```
