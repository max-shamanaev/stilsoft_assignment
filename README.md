# Задание
1.	Необходимо написать программу (HTTP-клиент) на языке C++<br> 
2.	При написании программы разрешается использовать только WinAPI, STL<br> 
3.	Программа должна получать данные посредством HTTP запроса через socket от любого сервера сети Интернет<br> 
4.	В программе необходимо использовать два потока<br> 
5.	Основной (первый) поток должен осуществлять: постоянный контроль за наличием считанных из сокета данных; вывод считанных данных в консоль или в окно; удаление выведенных данных из контейнера std::vector<br> 
6.	Второй поток должен осуществлять бесконечную периодическую отправку HTTP запросов посредством сокета на web-сервер и получение результата HTTP запроса из сокета, с максимально возможной скоростью<br> 
7.	Результат HTTP запроса должен помещается в std::vector только после полного считывания ответа на HTTP запрос, при этом, элементом контейнера std::vector должен являться массив байт данных считанных из сокета – результат HTTP запроса<br> 
8.	Программа должна продолжать выполнение до нажатия ESC, после чего второй поток должен корректно завершиться<br> 
9.	Программа не должна допускать переполнения памяти, т.е. необходимо удалять элементы контейнера std::vector после их вывода на экран<br> 
