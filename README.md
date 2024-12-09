# Лабораторная работа №2

`Шевченко Дарья P3330`

`Вариант: Not Recently Used`

## Задание

В данной лабораторной работе необходимо реализовать блочный кэш в пространстве пользователя в виде динамической библиотеки. Политика вытеснения страниц - Not recently used.

При выполнении работы необходимо реализовать простой API для работы с файлами, предоставляющий пользователю следующие возможности:

Открытие файла по заданному пути файла, доступного для чтения.
Закрытие файла по хэндлу.
Чтение данных из файла.
Запись данных в файл.
Перестановка позиции указателя на данные файла.
Синхронизация данных из кэша с диском.

---

Реализация block_cache -- [block_cache.cpp](https://github.com/secs-dev-os-course/diy-page-cache-dariayo/blob/lab-2/app/block_cache.cpp)

Измененный bench1 -- [utils.cpp](https://github.com/secs-dev-os-course/diy-page-cache-dariayo/blob/lab-2/app/utils.cpp)

## Краткий обзор кода

Операции с файлами (open, close, read, write, lseek, fsync):

- open: Открывает файл с флагами O_RDWR и O_DIRECT, что означает прямой доступ к файлу без использования кэша операционной системы.
- close: Закрывает файл, удаляя соответствующий дескриптор и данные в кэше.
- read: Читает данные из файла в буфер, проверяя, есть ли эти данные уже в кэше. Если их нет, данные загружаются с диска и сохраняются в кэше.
- write: Записывает данные в файл, проверяя, есть ли блок в кэше. Если блока нет, он сначала считывается в кэш, а затем данные записываются.
- lseek: Перемещает смещение файла и обновляет указатель на соответствующий файл в кэше.
- fsync: Записывает измененные данные обратно в файл, чтобы гарантировать сохранение всех изменений.

Кэш управляется с использованием стратегии NRU: страницы делятся на две категории: недавно использованные (с флагом referenced) и не использованные (с флагом referenced == false).
Когда кэш переполнен, удаляются страницы, которые не использовались (с флагом referenced == false), а для страниц, которые использовались, флаг referenced сбрасывается на false.

## Данные о работе программы-нагрузчика до и после внедрения своего page cache

Было

```zsh
real 1.11
user 0.45
sys 1.08
```

Стало

```zsh
time -p ./bench1 search README.md 2

real 0.18
user 0.08
sys 0.09
```

```zsh
perf stat ./bench1 search CMakeCache.txt 2

Файл найден: "/workspaces/diy-page-cache-dariayo/build/CMakeCache.txt"
Файл найден: "/workspaces/diy-page-cache-dariayo/build/CMakeCache.txt"
Общее время выполнения: 0.225 секунд

 Performance counter stats for './bench1 search CMakeCache.txt 2':

            207.58 msec task-clock:u                     #    0.908 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
               157      page-faults:u                    #  756.338 /sec                      
                                                  
       0.228589541 seconds time elapsed

       0.092900000 seconds user
       0.116125000 seconds sys
```

## Тесты

![alt text](img/image.png)

## Заключение

- Внедрение блочного кэша с политикой вытеснения NRU улучшило производительность, снижая количество операций ввода-вывода и ускоряя доступ к часто используемым данным. (page-faults в perf stat)

- Время выполнения операции поиска заметно снизилось благодаря кэшированию. (elapsed time в perf stat)

- Политика NRU эффективно управляет памятью, сохраняя в кэше только недавно использованные блоки, что предотвращает хранение устаревших данных.

- Программа-нагрузчик продемонстрировала улучшения производительности и снижения системных вызовов. (sys time)
