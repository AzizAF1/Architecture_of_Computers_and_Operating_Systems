### Управление состоянием светодиодов клавиатуры
===========================================

Этот модуль позволяет программно устанавливать и сбрасывать
светодиоды NumLock, CapsLock и ScrollLock через интерфейс sysfs.

Файлы
----
  ├── Makefile      — правила сборки модуля
  ├── kbled.c       — исходник модуля
  └── README.ld     — документация (это вы читаете)

Сборка
----
В каталоге модуля выполните:
  make

Сборка создаст файл `kbled.ko`.

Установка
----
1. Загрузите модуль (требуется root):
     sudo insmod kbled.ko

2. Проверьте наличие каталога:
     ls /sys/kernel/kbled

Управление
----
Параметр `state` задаёт битовую маску светодиодов:
  • 0x1 — NumLock  
  • 0x2 — CapsLock  
  • 0x4 — ScrollLock  

Примеры:
  • Включить CapsLock:
      echo 0x2 | sudo tee /sys/kernel/kbled/state

  • Включить NumLock и ScrollLock:
      echo 0x5 | sudo tee /sys/kernel/kbled/state

  • Выключить все светодиоды:
      echo 0x0 | sudo tee /sys/kernel/kbled/state

Удаление
----
  sudo rmmod kbled

Безопасность
----
После удаления модуля все светодиоды сбрасываются в 0.

### end
