# MSU_SQL_interpreter

Модельный SQL-интерпретатор 
=====================

Данный проект содержит:
##### 1. Реализацию архитектуры «Клиент — Сервер».
##### 2. Реализацию SQL-интерпретатора. 
  Интерпретатор анализирует и выполняет некоторое подмножество предложений языка SQL. 
  Синтаксический  анализ  SQL-предложений проводится методом рекурсивного спуска.
##### 3. Реализацию  модельного  SQL-сервера на базе разработанного интерпретатора 
и предоставленной библиотеки классов для работы с файлами данных. 
  Процесс-клиент, принимает запрос к реляционной  базе  данных,  записанный  на  языке  SQL.  
  Клиентский  процесс  передает запрос процессу-серверу, который осуществляет поиск в базе 
  данных в соответствии с запросом и передает результат поиска клиенту.
##### 4. Реализацию интерфейса пользователя с модельным SQL-сервером. 
  Клиент получает от пользователя запрос на модельном SQL и, не анализируя 
  его, передает серверу. Сервер анализирует запрос и в случае его 
  корректности  выполняет  запрос  и  передает  клиенту  ответ.  Если  же  запрос 
  некорректен, сервер передает клиенту код ошибки. Клиент выдает 
  пользователю либо ответ на его запрос, либо сообщение об ошибке.
##### 5. Реализацию  модельной  базы  данных,  демонстрирующей  работоспособность 
программ. 
***

РАБОТА С ПРОГРАММОЙ
=====================
### Подготовка к работе
0. Для компиляции напишите "make" в командной строке.
1. Перед началом работы программы убедитесь, что в папке с исполняемыми файлами client и server
отсутствует файл "mysocket". При корректной работе (без прерывания работы сервера по ctrl+C) 
файл ysocket удаляется автоматически
2. Запустите сначала процесс сервер с помощью ./server, затем в другом параллельном процессе 
запустите клиента ./client. В начале введите название файла, в который будет записываться 
результат работы интерпретатора.
3. Для работы с базой данных* пишите нужные команды** в процессе клиенте. 
4. Для просмотра результатов работы базы данных введите в командной строке cat имя_файла.
5. Для работы с базой данных без клиент-серверного взаимодейтсвия, а посредством команд в 
программе tmp.cpp, пропишите в командной строке ./spm.
6. Для чистки файлов пропишите в консоли make clear.

* для ознакомления с прицнипом работы базы данных прочтите ReadMe.txt
** примеры команд есть в файле tests.txt

### Принцип работы 
1. Сервер получает от клиента строку, содержащую команду, и запускает интерпретатор с помощью
конструктора: Interpreter obj (str);
2. Кoнструктор считывает первое слово из строки: если это слово SELECT, INSERT, UPDATE, DELETE, 
CREATE или DROP, то вызыаются соответствующие методы интерпретатора (select_sentence() и т.д.),
иначе генерируется сообщение об ошибке.
3. Предложения без WHERE-клаузы (create, insert, drop) не требуют дополнительного парсинга и 
анализируются в цикле непосредственно в командах create_sentence() и т.д.
4. В WHERE-клаузах требуются лексический и синтаксический анализаторы для обработки самих 
where-клауз, long-выражений (используются в IN-альтернативе и логических выражений), и для 
регулярных выражений в LIKE-альтернативе. Для каждого случая создан отдельный namespace со своим
набором типов лексем и т.д.
5. После обработки выполняются нужные действия, результат записывается в указанный клиентом файл.

ВНИМАНИЕ!
В результате работы команд INSERT, UPDATE, DELETE, CREATE и DROP исходная таблица обновляется!

### Реализованные предложения модельного языка SQL
Модельный  язык  SQL  будет  включать  лишь  шесть  основных  предложений  стандарта 
языка SQL, возможности которых также будут существенно ограничены. Это: 
SELECT — выбрать из БД данные, соответствующие запросу; 
INSERT — вставить новую строку данных в таблицу; 
UPDATE — обновить значения данных в существующей строке; 
DELETE — удалить строку из таблицы; 
CREATE — создать таблицу; 
DROP — уничтожить таблицу. 

***

Моделирование архитектуры «Клиент — Сервер»
=====================

Когда вызывается программа сервер, она запрашивает у ОС сокет (socket). После получения
сокета он  связывает  с  ним  некоторое  стандартное  имя,  чтобы  программа-клиент 
могла общаться с сервером через данный сокет по этому имени. 
После  присваивания  имени  сокету,  сервер  « слушает»  на  этом  сокете  требования  связи 
(запросы)  от  процессов-клиентов.  Когда  запрос  появляется,  сервер  может  допустить 
или запретить связь клиента с сервером. Если связь допустима, ОС соединяет клиента с 
сервером,  и  сервер  получает  возможность  получать  сообщения  от  клиента  и  посылать 
ему ответы так же, как и при использовании механизма информационных каналов. 

Клиент  также  запрашивает  у  ОС  свой  сокет  для  взаимодействия  с  другим  процессом 
(сервером), а затем сообщает имя сокета, с которым он хотел бы связаться. ОС 
пытается  найти  сокет  c  заданным  именем  и,  если  находит  его,  посылает  серверу, 
слушающему этот сокет, запрос связи. Если сервер допускает связь, ОС создает 
специальный сокет, соединяющий два процесса, и клиент получает возможность 
посылать  и  получать  данные  от  сервера  так  же,  как  и  при  использовании  механизма 
информационных каналов. 

***

База данных и средства управления
=====================

Каждая таблица описывается одним файлом данных. Этот файл состоит из двух частей: 
заголовка, в котором описывается структура таблицы, и собственно записей. Заголовок 
содержит  информацию  о  числе  полей  и  для  каждого  поля —  его  тип  ( строковый  или 
длинный целый). Для строковых полей, кроме того, хранится их длина.  

Для работы с таблицами реализована следующая функциональность:
—  создание новой таблицы, 
—  удаление существующей таблицы, 
—  открытие существующей таблицы, 
—  перемещение по записям таблицы, 
—  редактирование отдельных записей таблицы, 
—  добавление новых записей в таблицу, 
—  удаление существующих записей из таблицы. 

### СТРУКТУРА ТАБЛИЦЫ 

В библиотеке dbms.hpp реализованы следующие структуры: 
	class IField 				базовый класс для полей разных типов;
	class ITextField : public IField	класс потомок для полей типа текст;
	class ILongField : public IField	класс потомок для полей типа long;
	class IShortField : public IField	класс потомок для полей типа short;
	class ITableStruct 			содержит вектор из типов. По сути 
					    выполняет роль заголовка таблицы, где 
					    хранятся имена колонок и тип колонки;			    
	class ITable				основной класс, содержащий информацию
					    в таблице в виде вектора векторов указателей
					    на поля IField, в которых содержится ссылка
					    на нужный класс-потомок.
					    
Класс ITable содержит: 
	1. Массив text_fields полей ITextField.
	2. Массив long_fields полей ILongField.
	3. Массив short_fields полей IShortField.
	4. Массив массивов fields с номерами. Если мы хотим получить доступ к значению в 
	клетке [i][j], мы должны посмотреть значение fields[i][j], затем посмотреть тип 
	j-го столбца с помощью метода header.GetType(). Далее нужно обратиться к 
	соответствующему массиву по индексу fields[i][j].
	
	Пример:
	header | text1 : TEXT | text2 : TEXT | long1 : LONG | short1 : SHORT | text3 : TEXT
	     0 |      ---     |      ---     |      123     |       ---      |     hello
	     1 |      kill    |      ---     |      ---     |        1       |     world 
	     2 |      ---     |       me     |     987654   |       -1       |      ---
	     3 |      \0      |       \0     |       0      |       ---      |     please
	
	       
	 fields of elements in the text_field array: 
	 "\0"* | "\0"* | "hello" | "kill" | "\0"* | "world" | "\0"* | "me" | "\0"* | "\0" |  "\0" |  "please"
	 
	 fields of elements in the long_field array: 
	 123 | 0* | 987654 | 0
	 
	 fields of elements in the short_field array: 
	 0* | 1 | -1 | 0*
	 
	 *isInit() = 0, то есть поле не инициализировано
	 	 
	 fields:
	 0 1  0 0 2 
	 3 4  1 1 5
	 6 7  2 2 8
	 9 10 3 3 11
	 __________________________________________________________________
	 
	 При удалении столбца (пример со столбцом text2):
	header | text1 : TEXT | long1 : LONG | short1 : SHORT | text3 : TEXT
	     0 |      ---     |      123     |       ---      |     hello
	     1 |      kill    |      ---     |        1       |     world 
	     2 |      ---     |     987654   |       -1       |      ---
	     3 |      \0      |       0      |       ---      |     please
	       
	 fields of elements in the text_field array: 
	 "\0"* | "hello" | "kill" | "world" | "\0"* | "\0"* | "\0" |  "please"
	 
	 fields of elements in the long_field array (не меняются): 
	 123 | 0* | 987654 | 0
	 
	 fields of elements in the short_field array (не меняются): 
	 0* | 1 | -1 | 0*
	 
	 *isInit() = 0, то есть поле не инициализировано
	 	 
	 fields: удалили столбец i типа t. Элементы столбцов типа != t не меняются. В первой строке
	 элементы столбцов типа t с номером >= i уменьшаются на 1. В строке >1 элементы столбцов 
	 типа t уменьшаются на число, равное порядковому номеру строки.
	  
	 0 1  0 0 2       0 0 0 1
	 3 4  1 1 5    => 2 1 1 3
	 6 7  2 2 8       4 2 2 5
	 9 10 3 3 11      6 3 3 7
	 ___________________________________________________________________
   
	 При добавлении столбца (пример со столбцом newshort) :
	header | text1 : TEXT | long1 : LONG | short1 : SHORT | text3 : TEXT | newshort : SHORT 
	     0 |      ---     |      123     |       ---      |     hello    |          1       
	     1 |      kill    |      ---     |        1       |     world    |          2       
	     2 |      ---     |    987654    |       -1       |      ---     |          3       
	     3 |      \0      |       0      |       ---      |     please   |          4      
	       
	 fields of elements in the text_field array (не меняются): 
	 "\0"* | "hello" | "kill" | "world" | "\0"* | "\0"* | "\0" |  "please"
	 
	 fields of elements in the long_field array (не меняются): 
	 123 | 0* | 987654 | 0
	 
	 fields of elements in the short_field array (места 1, 3, 5, 7): 
	 0* | 1 | 1 | 2 | -1 | 3 | 0* | 3
	 Из примера видно, что если мы хотим вставить колонку с типом t, которой предшествует
	 k колонок такого же типа t, то мы вставляем поля на k-е место, а потом на каждое (i*n)+k
	 место, где n - количество колонок типа t в таблице, i - номер линии.
	 
	 *isInit() = 0, то есть поле не инициализировано
	 	 
	 fields: на 1й строке появляется лишь дополнительный элемент, равный k. В строках >1 элементы
	 столбцов типа t увеличиваются на число, равное порядковому номеру строки (нумерация с 0).
	 0 0 0 1        0 0 0 1 1
	 2 1 1 3    =>  2 1 2 3 3
	 4 2 2 5        4 2 4 5 5
	 6 3 3 7        6 3 6 7 7
	 
	 
### ФУНКЦИОНАЛ ITable 

1. Инициализация: 
	пустой таблицы ITable();
	пустой таблицы с заданной структурой ITable(const ITableStruct &other);
	конструктор копирования ITable(const ITable &other);
	импорт из файла ITable(const std::string filename);

2. Импорт таблицы из файла ITable Open(const std::string filename);

3. Добавление текста в ячейку на строке line в колонку с названием title
void AddText(const std::string &val, const std::string &title, const size_t line);

4. Добавление числа типа long в ячейку на строке line в колонку с названием title
void AddLong(const std::string &val, const std::string &title, const size_t line);

5. Добавление числа типа short в ячейку на строке line в колонку с названием title
void AddShort(const std::string &val, const std::string &title, const size_t line);

6. Получение ссылки на элемент текстовой ячейки на строке line в колонке с названием title
std::string &GetTextField(const std::string &Name, const size_t line);

7. Получение ссылки на элемент ячейки типа long на строке line в колонке с названием title
long &GetLongField(const std::string &Name, const size_t line);
    
8. Получение ссылки на элемент ячейки типа short на строке line в колонке с названием title
short &GetShortField(const std::string &Name, const size_t line);

9. Добавление линии в таблицу 		void Add_line();

10. Удаление линии из колонки 		void Delete_line(const size_t line);

11. Добавление колонки в таблицу с заданным типом и названием Name 
void Add_Column(const Type, const std::string &Name);

12. Удаление колонки из таблицы с заданным типом и названием Name 
void Delete_Column(const Type, const std::string &);

13. Конвертация в файл 		void ToFile(const std::string &filename);

14. Перемещение в начало таблицы 	void ReadFirst();

15. Перемещение на следующую строчку в таблице 	void ReadNext();

16. Печать заголовка таблицы 	void Print_field_names();

17. Печать заданной линии 	void Print_line(const size_t);

18. Печать текущей линии 	void Print_line();
    
19. Печать таблицы 		void Print_table();
	 
### ФУНКЦИОНАЛ ITableStruct 	 

1. Инициализация заголовка таблицы: 
	Пустой заголовок ITableStruct();
	Конструктор копирования ITableStruct(const ITableStruct &other);
	Инициализация заголовка с заданным именем ITableStruct(std::string tn);
	Инициализация заголовка с заданным вектором и массивом названий и типов колонок
		ITableStruct(std::string tn, std::vector<std::pair<std::string, Type>> fn, size_t fl);
		
2. Добавление текстового поля 	 void AddText(const std::string &Name);
    
3. Добавление поля типа long	 void AddLong(const std::string &Name);
    
4. Добавление поля типа short 	 void AddShort(const std::string &Name);
    
5. Удаление поля данного типа с данным названием
void DeleteField(const std::string &Name, const Type field_type);
    
6. Добавление имени таблицы	 void SetTableName(const std::string &Name);
    
7. Изменение заголовка таблицы   void SetFieldName(const std::string &Name, const std::string &prevName);
    
8. Возврат типа поля по индексу 	Type GetType(size_t index);

9. Возврат имени таблицы 	 std::string get_table_name();
    
10. Возврат строки с заголовками таблицы 	std::string get_fields_names();
    
11. Возврат заголовка таблицы по индексу 	std::string get_field_name(size_t);
    
12. Возврат количества полей в таблице 	size_t get_num_fields();
    
13. Возврат количества текстовых полей в таблице 	size_t get_num_text_fields();

14. Возврат количества полей типа long в таблице 	size_t get_num_long_fields();

15. Возврат количества полей типа short в таблице 	size_t get_num_short_fields();
    
16. Возврат количества битов, которые занимает заголовок таблицы 	size_t get_fields_length();
    
17. Печать заголовка таблицы 	void print();

***

Грамматика языка
=====================
	 
S       = SEL | INS | UPD | DEL | CR | DR

SEL     = 'SELECT' FIELDS 'FROM' id WHERE           создает новую таблицу из отобранных строк и столбцов
FIELDS  = id {, id} | *

INS     = 'INSERT INTO' id (F_VAL {, F_VAL})        вставка в конец таблицы
F_VAL   = STR | longint
STR     = symbol {symbol}

UPD     = 'UPDATE' id 'SET' id '=' EXPR WHERE       замена всех строк, удовлетворяющих where-клаузе

DEL     = 'DELETE FROM' id WHERE                    удаление всех строк, удовлетворяющих where-клаузе

CR      = 'CREATE TABLE' id (DESCR)                 создание заголовка таблицы
DESCR   = DES_F {, DESC_F}
DESC_F  = id TYPE
TYPE    = TEXT (unsigned_int) | LONG

DR      = 'DROP TABLE' id          		     удаление из файловой системы


WHERE   = 'WHERE' TEXT_ID ['NOT'] 'LIKE' STR | EXPR ['NOT'] 'IN' (CONSTS) | 'WHERE' LOG | 'WHERE ALL' 

EXPR    = LONG_EXPR | TEXT_EXPR
CONSTS  = STR {, STR} | longint {, longint}

LONG_EXPR   = LONG_SUM { S_OP LONG_SUM }
S_OP        = + | -
LONG_SUM    = LONG_MULT { M_OP LONG_MULT }
M_OP        = * | \ | %
LONG_MULT   = LONG_VAL | (LONG_EXPR)
LONG_VAL = id | longint

TEXT_EXPR   = id | STR

LOG = LOG_SUM {'OR' LOG_SUM}
LOG_SUM = LOG_MULT = 'NOT' LOG_MULT | (LOG) | (RELATION)
RELATION = TEXT_REL | LONG_REL
TEXT_REL = TEXT_EXPR EVAL TEXT_EXPR
LONG_REL = LONG_EXPR EVAL LONG_EXPR
EVAL = '=' | '<' | '>' | '>=' | '<=' | '!='

	 
