Вариант 2
Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp). Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
Замечание. Для передачи поддерева файловой системы (или одного файла) рекомендуется на стороне клиента использовать вызов (через fork) команды tar с перенаправлением ее вывода непосредственно в сокет. На стороне сервера также рекомендуется использовать tar с перенаправлением ее ввода непосредственно из сокет.


Сервер:
    компиляция:
        make srv
    запуск:
        ./srv
        
    Остановка сервера производится отправкой сигнара SIGSTOP
    
Клиент: 
    компиляция:
        make clt
    запуск:
        ./clt path.to.src.file host@path.to.dst.dir
        
    Пример запуска:
        ./clt test_file.txt 127.0.0.1@./test_dir/
        
        