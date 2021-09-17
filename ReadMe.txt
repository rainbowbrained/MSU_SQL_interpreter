------------------------------ КАК РАБОТАТЬ С ПРОГРАММОЙ ------------------------------

1. Для компиляции используйте make;
2. Для запуска программы-сервера пропишите в консоли ./server; 
3. Для запуска программы-сервера пропишите в консоли ./client; 
4. Для запуска программы, работающей с базой данных, пропишите в консоли ./spm; 
5. Для чистки файлов пропишите в консоли make clear.


------------------------------ СТРУКТУРА ТАБЛИЦЫ ------------------------------

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
	 
	 
------------------------------ ФУНКЦИОНАЛ ITable ------------------------------

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
	 
------------------------------ ФУНКЦИОНАЛ ITableStruct ------------------------------	 

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
	 
	 
	 
	 
	 
