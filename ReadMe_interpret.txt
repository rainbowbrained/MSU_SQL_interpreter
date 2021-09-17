----------------------------------- РУКОВОДСТВО ДЛЯ РАБОТЫ -----------------------------------
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

* для ознакомления с прицнипом работы базы данных прочтите ReadMe.txt
** примеры команд есть в файле tests.txt

----------------------------------- ПРИНЦИП РАБОТЫ -----------------------------------
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



----------------------------------- ГРАММАТИКА -----------------------------------

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


В программе используются следующие классы и пространства имен:
	1. class Interpreter - содержит базу данных и методы, работающие с ней:
	private:
	    void select_sentence (std::string &);   // выбрать из БД данные, соответствующие запросу; 
	    void insert_sentence (std::string &);   // вставить новую строку данных в таблицу; 
	    void update_sensence (std::string &);   // обновить значения данных в существующей строке; 
	    void delete_sentence (std::string &);   // удалить строку из таблицы; 
	    void create_sentence (std::string &);   // создать таблицу; 
	    void drop_sentence (std::string &);     // удалить базы данных
	    void field_description (std::string &); // описание поля ????????
	    std::vector <unsigned long> where_clause (std::string &);  // where-кляуза для update, 
	    							       // delete и select предложений
	    ITable bd_table; 			    // база данных
	public:
	    Interpreter (std::string &);
	    ~ Interpreter () {}
	};
	
	2. namespace lexer_long_expr - содержит функции и переменные для лексического и синтаксического
	анализа LONG_EXPR:
	LONG_EXPR   = LONG_SUM { S_OP LONG_SUM }
	S_OP        = + | -
	LONG_SUM    = LONG_MULT { M_OP LONG_MULT }
	M_OP        = * | \ | %
	LONG_MULT   = LONG_VAL | '('LONG_EXPR')'
	LONG_VAL    = id | longint
	
		- enum long_type_t cur_lex_type; // тип текущей лексемы: start, +, -, *, /, %, (, ), 
						 // число, имя поля типа long, end
		- std::string cur_lex_text;      //
		- std::string c;		 // текущее слово 
		- void init (std::string & str)  // функция считывает первое слово из str, присваивает тип start
		- void next (std::string & str)  // анализатор - основная функция
		    	state\ лексема
			START|
			PLUS |	+
			MINUS|	-
			MULT |	*
			DIV  |	/
			MOD  |	%
			OPEN |	(
			CLOSE|	)
		       NUMBER|	long int number 
		       L_NAME|	long int field id
			END  |	correct lexem
			
	3. namespace parser_long_expr - 
		- void init (std::string & str)
		
		Функции, рекуррентно вызывающие друг друга для создания строки, которая затем будет 
		вычислена функция Al(), Bl() и Cl().
		- std::string A(std::string &); // processes + | - (S_OP)
		- std::string B(std::string &); // processes * | / | % (M_OP) and LONG_SUM
		- std::string C(std::string &); // LONG_MULT, longint, LONG id and brackets (calls A())

		Функции для вычисления long выражений (используются для каждой строки)
		- long Al(std::string &, ITable &);
		- long Bl(std::string &, ITable &);
		- long Cl(std::string &, ITable &);
	
	4. namespace lexer_where - для лексических и синтаксических анализаторов where-кляузы
		
		- enum where_type_t cur_lex_type_w; // тип 
		- std::string cur_lex_text_w;	    // предыдущая лексема
		- std::string c_w;		    // текущая лексема
		    
		 -void init (std::string & str)
		    {
			c_w = read_word (str);
			cur_lex_type_w = START_w;
		    }

		 -void next (std::string & str, ITable bd) // анализатор - основная функция
			    START_w	| start 		PLUS_w		| +
			    MINUS_w     | -			OR_w       	| OR
			    MULT_w      | *			DIV_w       	| /
			    MOD_w       | %			AND_w           | AND
			    NOT_w       | NOT			OPEN_w     	| (
			    CLOSE_w     | )			REL_w   	| =, >, <, !=, >=, <=
			    NUMBER_w 	| set of numbers 	L_NAME_w	| LONG field's name 
			    T_NAME_w    | TEXT field's name 	STR_w        	| text line
			    LIKE_w,     | LIKE			IN_w            | IN
			    COM_w,      | ,			ALL_w      	| ALL
			    END_w
			    
	5. namespace parser_where - для анализа каждой отдельной альтернативы where-кляузы
	WHERE = 'WHERE' TEXT_ID ['NOT'] 'LIKE' STR | EXPR ['NOT'] 'IN' (CONSTS) | 'WHERE' LOG | 'WHERE ALL' 

	    - enum mode_type { LIKE_alt, IN_alt_T, IN_alt_L, LOG_alt, ALL_alt }; - состояния
	    - int flag_log = 0;
	    - int flag_expr = 0;
	    - enum mode_type mode;
	    - std::multiset <long> mst_l;	 // список числовых констант
	    - std::multiset <std::string> mst_s; // список строковых констант

	    void init (string & str, ITable bd)
	    
	    Функции для синтаксического анализа:
	    - std::string W0(std::string &, ITable &); // beginning of where-clause
	    - std::string W1(std::string &, ITable &); // LIKE-alternative
	    - std::string W2(std::string &, ITable &); // IN-alternative
	    - std::string W3(std::string &, ITable &); // start processing long or logic expressions, call W4
	    - std::string W4(std::string &, ITable &); // long or logic expressions, call W5 or W7
	    - std::string W5(std::string &, ITable &); // long or logic expressions, call W3 (if in brackets), W6 or W7
	    - std::string W6(std::string &, ITable &); // parsing text expression and relations for logic-expr
	    - std::string W7(std::string &, ITable &); // brackets, call W3
	    - void W8(std::string &, ITable &);        // list of constants

	    Функции для вычисления логических выражений (используются для каждой строки)
	    - long W31(std::string &, ITable &); // start funstion
	    - long W41(std::string &, ITable &);
	    - long W51(std::string &, ITable &);
	    - long W71(std::string &, ITable &); // brackets
	    
	6. namespace myregex - для обработки регулярных выражений, содержащих символы и служебные символы:
		%   - any sequence of >= symbols
		_   - any symbol
		[]  - any symbol from brackets: [abcdef] of [a-f]
		[^] - any symbol not from brackets: [^abcdef] of [^a-f]
		
	    - bool my_regex_match (std::string reference, std::string check)
	    возвращает true, если строка check соответствует строке образцу reference
	
