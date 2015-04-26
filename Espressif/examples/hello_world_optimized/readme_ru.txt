Оптимизация скорости и размера доступного IRAM.

Обсуждение на форуме http://esp8266.ru/forum/threads/udk-obschie-razgovory.185/page-4

Оптимизация заключается в добавлении опции -mno-serialize-volatile в CFLAGS (см. Makefile)

When this option is enabled, GCC inserts MEMW instructions before volatile memory references to guarantee
sequential consistency. The default is -mserialize-volatile. Use -mno-serialize-volatile to omit the
MEMW instructions.
