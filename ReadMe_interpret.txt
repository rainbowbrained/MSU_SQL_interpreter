0. Для компиляции напишите "make" в командной строке.
1. Перед началом работы программы убедитесь, что в папке с исполняемыми файлами client и server
отсутствует файл "mysocket". 
2. Запустите сначала процесс сервер с помощью ./Server, затем в другом параллельном процессе 
запустите клиента ./Client
3. Для работы с БД пишите нужные команды в процессе клиенте. 






----------------------------------- ПРИНЦИП РАБОТЫ -----------------------------------
1. Сервер получает от клиента строку, содержащую команду, и запускает интерпретатор с помощью
конструктора: Interpreter obj (str);
2. КОнструктор считывает первое слово из строки: если это слово SELECT, INSERT, UPDATE, DELETE, 
CREATE или DROP, то вызыаются соответствующие методы интерпретатора (select_sentence() и т.д.),
иначе генерируется сообщение об ошибке











----------------------------------- ЧТО ГОТОВО -----------------------------------
1. Скелет программы
2. Парсеры для long выражений
3. Парсеры для where-кляузы

----------------------------------- ЧТО НЕ ГОТОВО -----------------------------------
1. Методы класса interpret
2. Пространство имен parser_where
3. Классы исключений
4. Нужно доработать некоторые методы таблицы (конструирование заголовка из строки и тд)

----------------------------------- ГРАММАТИКА -----------------------------------

S       = SEL | INS | UPD | DEL | CR | DR

SEL     = 'SELECT' FIELDS 'FROM' id WHERE           создает новую таблицу из отобранных строк и столбцов
FIELDS  = id {, id} | *

вставляет в конец таблицы <имятаблицы> строку с перечисленным значением полей. 
Количество полей и их значения должны соответствовать структуре таблицы. В противном 
случае выдается сообщение об ошибке. 
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
		
		3 functions that call each other recurrently and generate a sentence which will be calculated
		in the next 3 functions (LONG_SUM, LONG_MULT, LONG_VAL)
		- std::string A (std::string &);	// operates with + and minus 
		- std::string B (std::string &);	// operates with *,  %, 
	  	- std::string C (std::string &);	// operates with brackets, number or long field name
		 
		- long A (std::string &, ITable);
		- long B (std::string &, ITable);
		- long C (std::string &, IT/,able);
	
	4. namespace lexer_where - для лексических и синтаксических анализаторов where-кляузы
		
		- enum where_type_t cur_lex_type_w; // тип 
		- std::string cur_lex_text_w;	    // current lexem
		- std::string c_w;		    // 
		    
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
	    - std::multiset <long> mst_l;
	    - std::multiset <std::string> mst_s;

	    void init (string & str, ITable bd)
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

