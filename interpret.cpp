#ifndef _SQL_H_
#define _SQL_H_

#include "dbms.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <regex>
#include <set>
#include <string>
#include <vector>

// Interpreter --- SQL-interpreter class
class Interpreter
{
private:
    void select_sentence (std::string &);
    void insert_sentence (std::string &);
    void update_sensence (std::string &);
    void delete_sentence (std::string &);
    void create_sentence (std::string &);
    void drop_sentence (std::string &);
    void field_description (std::string &);
    std::vector <unsigned long> where_clause (std::string &);
    ITable bd_table;
public:
    Interpreter (std::string &);
    ~ Interpreter () {}
};

/*--------------------------------------------------------------------*/

// read one word from line
std::string read_word (std::string & str)
{
    std::string word;
    int i = 0;
    while (!isspace (str.c_str() [i]) && !(str.empty()))
    {
        word = word + str.c_str() [i];
        str.erase (0, 1);
    }
    while (isspace (str.c_str() [i]) && !(str.empty()))
    {
        str.erase (0, 1);
    }
    return word;
}

/*--------------------------------------------------------------------*/

// lexical and syntactic parsers for long-expressions
enum long_type_t 
{
    START,
    PLUS,   // +
    MINUS,  // -
    MULT,   // *
    DIV,    // /
    MOD,    // %
    OPEN,   // (
    CLOSE,  // )
    NUMBER, // set of numbers
    L_NAME, // name of the field with type LONG
    END
};

/*
LONG_EXPR   = LONG_SUM { S_OP LONG_SUM }
S_OP        = + | -
LONG_SUM    = LONG_MULT { M_OP LONG_MULT }
M_OP        = * | \ | %
LONG_MULT   = LONG_VAL | (LONG_EXPR)
LONG_VAL = id | longint
*/
namespace lexer_long_expr 
{
    enum long_type_t cur_lex_type;
    std::string cur_lex_text;
    std::string c;
    
    void init (std::string & str)
    {
        c = read_word (str);
        cur_lex_type = START;
    }

    void next (std::string & str)
    {
        cur_lex_text.resize(0);
        long_type_t state = START;
        while (state != END)
        {
            switch (state)
            {
                case START:
                    if (c == "+")
                    {
                        state = PLUS; // plus
                    }
                    else if (c == "-")
                    {
                        state = MINUS; // mius
                    }
                    else if (c == "*")
                    {
                        state = MULT; // multiply
                    }
                    else if (c == "/")
                    {
                        state = DIV; // div
                    }
                    else if (c == "%")
                    {
                        state = MOD; // mod
                    }
                    else if (c == "(")
                    {
                        state = OPEN; // open bracket
                    }
                    else if (c == ")")
                    {
                        state = CLOSE;  // close bracket
                    }
                    // words, that can follow long-expression
                    else if ((c == "NOT") || (c == "IN") || 
                             (c == "WHERE") || (c == "=") ||
                             (c == ">")  || (c == "<") ||
                             (c == ">=") || (c == "<=") ||
                             (c == "!=") || (c == "AND") ||
                             (c == "OR") || c.empty() )
                    {
                        cur_lex_type = END;
                        state = END; //OK
                    }
                    else
                    {
                        unsigned int i = 0;
                        state = NUMBER;
                        while ((i < c.length()))
                        {
                            if (!isdigit(c[i]))
                            {
                                // not long, it is a long_name
                                i = c.length();
                                state = L_NAME;
                            }
                            i++;
                        }
                    }
                    break;

                case PLUS:
                    cur_lex_type = PLUS;
                    state = END;
                    break;
                    
                case MINUS:
                    cur_lex_type = MINUS;
                    state = END;
                    break;

                case MULT:
                    cur_lex_type = MULT;
                    state = END;
                    break;
                    
                case DIV:
                    cur_lex_type = DIV;
                    state = END;
                    break;
                    
                case MOD:
                    cur_lex_type = MOD;
                    state = END;
                    break;

                case OPEN:
                    cur_lex_type = OPEN;
                    state = END;
                    break;

                case CLOSE:
                    cur_lex_type = CLOSE;
                    state = END;
                    break;
                
                case NUMBER:
                    cur_lex_type = NUMBER;
                    state = END;
                    break;

                case L_NAME:
                    cur_lex_type = L_NAME;
                    state = END;
                    break;

                case END:
                    break;
            }

            if (state != END)
            {
                if (cur_lex_type != END)
                {
                    cur_lex_text = c;
                }
                c = read_word (str);
            }
        }
    }
} 

namespace parser_long_expr 
{

    void init (std::string & str)
    {
        lexer_long_expr :: init (str);
        lexer_long_expr :: next (str);
    }
    
    // functions for syntactic parser
    std::string A (std::string &);
    std::string B (std::string &);
    std::string C (std::string &);
    
    // functions to calculate the value
    long A (std::string &, ITable);
    long B (std::string &, ITable);
    long C (std::string &, ITable);

    std::string A (std::string & str)
    {
        std::string s;
        s = B (str);
        while ( (lexer_long_expr::cur_lex_type == PLUS) ||
                (lexer_long_expr::cur_lex_type == MINUS) )
        {
            if (lexer_long_expr::cur_lex_type == PLUS)
            {
                lexer_long_expr::next(str);
                s = s + "+ "; 
                s = s + B (str);
            }
            else if (lexer_long_expr::cur_lex_type == MINUS)
            {
                lexer_long_expr::next(str);
                s = s + "- "; 
                s = s + B (str);
            }
        }
        return s;
    }

    std::string B (std::string & str)
    {
        std::string s;
        s = C (str);
        while ( (lexer_long_expr::cur_lex_type == MULT) ||
                (lexer_long_expr::cur_lex_type == DIV) ||
                (lexer_long_expr::cur_lex_type == MOD) )
        {
            if (lexer_long_expr::cur_lex_type == MULT)
            {
                lexer_long_expr::next(str);
                s = s + "* "; 
                s = s + C (str);
            }
            else if (lexer_long_expr::cur_lex_type == DIV)
            {
                lexer_long_expr::next(str);
                s = s + "/ "; 
                s = s + C (str);
            }
            else if (lexer_long_expr::cur_lex_type == MOD)
            {
                lexer_long_expr::next(str);
                s = s + "% "; 
                s = s + C (str);
            }
        }
        return s;
    }

    std::string C (std::string & str)
    {
        std::string s;
        if (lexer_long_expr::cur_lex_type == OPEN)
        {
            lexer_long_expr::next(str);
            s = "( "; 
            s = s + A (str);
            if (lexer_long_expr::cur_lex_type != CLOSE)
            {
                throw "ex";
            }
            s = s + ") "; 
            lexer_long_expr::next (str);
        }
        else if (lexer_long_expr::cur_lex_type == NUMBER)
        {
            s = lexer_long_expr::cur_lex_text;
            s = s + " ";
            lexer_long_expr::next (str);
        }
        else if (lexer_long_expr::cur_lex_type == L_NAME)
        {
            s = lexer_long_expr::cur_lex_text;
            s = s + " ";
            lexer_long_expr::next (str);
        }
        else
        {
            throw "ex";
        }
        return s;
    }
    
    
    
    long A (std::string & str, ITable line)
    {
        long num;
        num = B (str, line);
        while ( (lexer_long_expr::cur_lex_type == PLUS) ||
                (lexer_long_expr::cur_lex_type == MINUS) )
        {
            if (lexer_long_expr::cur_lex_type == PLUS)
            {
                lexer_long_expr::next(str);
                num = num + B (str, line);
            }
            else if (lexer_long_expr::cur_lex_type == MINUS)
            {
                lexer_long_expr::next(str);
                num = num - B (str, line);
            }
        }
        return num;
    }

    long B (std::string & str, ITable line)
    {
        long num;
        num = C (str, line);
        while ( (lexer_long_expr::cur_lex_type == MULT) ||
                (lexer_long_expr::cur_lex_type == DIV) ||
                (lexer_long_expr::cur_lex_type == MOD) )
        {
            if (lexer_long_expr::cur_lex_type == MULT)
            {
                lexer_long_expr::next(str);
                num = num * C (str, line);
            }
            else if (lexer_long_expr::cur_lex_type == DIV)
            {
                lexer_long_expr::next(str);
                num = num / C (str, line);
            }
            else if (lexer_long_expr::cur_lex_type == MOD)
            {
                lexer_long_expr::next(str);
                num = num % C (str, line);
            }
        }
        return num;
    }

    long C (std::string & str, ITable line)
    {
        long num;
        if (lexer_long_expr::cur_lex_type == OPEN)
        {
            lexer_long_expr::next(str);
            num = A (str, line);
            if (lexer_long_expr::cur_lex_type != CLOSE)
            {
                throw "ex";
            }
            lexer_long_expr::next (str);
        }
        else if (lexer_long_expr::cur_lex_type == NUMBER)
        {
            try
            {
                // convert string to long
                num = stol (lexer_long_expr::cur_lex_text); 
            }
            catch (...)
            {
                throw "ex";
            }
            lexer_long_expr::next (str);
        }
        else if (lexer_long_expr::cur_lex_type == L_NAME)
        {
            // get value from the table
            //num = line.GetLongField(lexer_long_expr::cur_lex_text.c_str(), const size_t line);
            //field_struct * f = line.get_field 
            //(lexer_long_expr::cur_lex_text.c_str());
            //num = f -> l_num;
            lexer_long_expr::next (str);
        }
        else
        {
            throw "ex";
        }
        return num;
    }
} // end of namespace parser_long_expr
/*--------------------------------------------------------------------*/

// lexical and syntactic parsers for where-clause
//  WHERE   = 'WHERE' TEXT_ID ['NOT'] 'LIKE' STR | EXPR ['NOT'] 'IN' (CONSTS) | 
//            'WHERE' LOG | 'WHERE ALL' 
enum where_type_t 
{
    START_w,
    PLUS_w,     // +
    MINUS_w,    // -
    OR_w,       // OR
    MULT_w,     // *
    DIV_w,      // /
    MOD_w,      // %
    AND_w,      // AND
    NOT_w,      // NOT
    OPEN_w,     // (
    CLOSE_w,    // )
    REL_w,      // =, >, <, !=, >=, <=
    NUMBER_w,   // set of numbers
    L_NAME_w,   // name of the field with the type LONG
    T_NAME_w,   // name of the field with the type TEXT
    STR_w,      // line
    LIKE_w,     // LIKE
    IN_w,       // IN
    COM_w,      // ,
    ALL_w,      // ALL
    END_w
};

namespace lexer_where
{
    enum where_type_t cur_lex_type_w;
    std::string cur_lex_text_w;
    std::string c_w;
    
    void init (std::string & str)
    {
        c_w = read_word (str);
        cur_lex_type_w = START_w;
    }

    void next (std::string & str,ITable bd)
    {
        cur_lex_text_w.resize(0);
        where_type_t state_w = START_w;
        while (state_w != END_w)
        {
            switch (state_w)
            {
                case START_w:
                    if (c_w == "+")
                    {
                        state_w = PLUS_w;
                    }
                    else if (c_w == "-")
                    {
                        state_w = MINUS_w;
                    }
                    else if (c_w == "OR")
                    {
                        state_w = OR_w;
                    }
                    else if (c_w == "*")
                    {
                        state_w = MULT_w;
                    }
                    else if (c_w == "/")
                    {
                        state_w = DIV_w;
                    }
                    else if (c_w == "%")
                    {
                        state_w = MOD_w;
                    }
                    else if (c_w == "AND")
                    {
                        state_w = AND_w;
                    }
                    else if (c_w == "NOT")
                    {
                        state_w = NOT_w;
                    }
                    else if (c_w == "(")
                    {
                        state_w = OPEN_w;
                    }
                    else if (c_w == ")")
                    {
                        state_w = CLOSE_w;
                    }
                    else if ( (c_w == "=") || (c_w == "!=") || 
                              (c_w == ">") || (c_w == ">=") ||
                              (c_w == "<") || (c_w == "<=") )
                    {
                        state_w = REL_w;
                    }
                    else if (c_w == "LIKE")
                    {
                        state_w = LIKE_w;
                    }
                    else if (c_w == "IN")
                    {
                        state_w = IN_w;
                    }
                    else if (c_w == ",")
                    {
                        state_w = COM_w;
                    }
                    else if (c_w == "ALL")
                    {
                        state_w = ALL_w;
                    }
                    else if (c_w.empty())
                    {
                        cur_lex_type_w = END_w;
                        state_w = END_w;
                    }
                    else
                    {
                        unsigned int i = 0;
                        int flag = 1;
                        while ((i < c_w.length()) && flag)
                        {
                            if (!isdigit(c_w[i]))
                            {
                                // not long
                                flag = 0;
                            }
                            i++;
                        }
                        if (flag)
                        {
                            state_w = NUMBER_w;
                        }
                        
                        else if (c_w[0] == '\'') // line
                        {
                            state_w = STR_w;
                            if (c_w[c_w.length() - 1] == '\'')
                            {
                                cur_lex_type_w = STR_w;
                                cur_lex_text_w = cur_lex_text_w + c_w;
                                cur_lex_text_w = cur_lex_text_w + " ";
                                c_w = read_word (str);
                                state_w = END_w;
                            }
                        }
                        else 
                        {
                            // check if the word is the field and its type 
                            Type t;
                            try
                            {
                                t = bd.GetFieldType (c_w.c_str());
                            }
                            catch (...)
                            {
                                throw "ex";
                            }
                            if (t == TEXT)
                            {
                                state_w = T_NAME_w;
                            }
                            else
                            {
                                state_w = L_NAME_w;
                            }
                        }
                    }
                    break;

                case PLUS_w:
                    cur_lex_type_w = PLUS_w;
                    state_w = END_w;
                    break;
                    
                case MINUS_w:
                    cur_lex_type_w = MINUS_w;
                    state_w = END_w;
                    break;
                    
                case OR_w:
                    cur_lex_type_w = OR_w;
                    state_w = END_w;
                    break;

                case MULT_w:
                    cur_lex_type_w = MULT_w;
                    state_w = END_w;
                    break;
                    
                case DIV_w:
                    cur_lex_type_w = DIV_w;
                    state_w = END_w;
                    break;
                    
                case MOD_w:
                    cur_lex_type_w = MOD_w;
                    state_w = END_w;
                    break;
                
                case AND_w:
                    cur_lex_type_w = AND_w;
                    state_w = END_w;
                    break;
                
                case NOT_w:
                    cur_lex_type_w = NOT_w;
                    state_w = END_w;
                    break;

                case OPEN_w:
                    cur_lex_type_w = OPEN_w;
                    state_w = END_w;
                    break;

                case CLOSE_w:
                    cur_lex_type_w = CLOSE_w;
                    state_w = END_w;
                    break;
                
                case REL_w:
                    cur_lex_type_w = REL_w;
                    state_w = END_w;
                    break;
                
                case NUMBER_w:
                    cur_lex_type_w = NUMBER_w;
                    state_w = END_w;
                    break;

                case L_NAME_w:
                    cur_lex_type_w = L_NAME_w;
                    state_w = END_w;
                    break;
                    
                case T_NAME_w:
                    cur_lex_type_w = T_NAME_w;
                    state_w = END_w;
                    break;
                    
                case STR_w:
                    // searching fot the line end
                    if (c_w.empty())
                    {
                        throw "ex";
                    }
                    if (c_w[c_w.length() - 1] != '\'')
                    {
                        //stay in S
                    }
                    else
                    {
                        cur_lex_type_w = STR_w;
                        cur_lex_text_w = cur_lex_text_w + c_w;
                        cur_lex_text_w = cur_lex_text_w + " ";
                        c_w = read_word (str);
                        state_w = END_w;
                    }
                    break;
                    
                case LIKE_w:
                    cur_lex_type_w = LIKE_w;
                    state_w = END_w;
                    break;
                
                case IN_w:
                    cur_lex_type_w = IN_w;
                    state_w = END_w;
                    break;
                
                case COM_w:
                    cur_lex_type_w = COM_w;
                    state_w = END_w;
                    break;
                    
                case ALL_w:
                    cur_lex_type_w = ALL_w;
                    state_w = END_w;
                    break;

                case END_w:
                    break;
            }

            if (state_w != END_w)
            {
                if (cur_lex_type_w != END_w)
                {
                    cur_lex_text_w = cur_lex_text_w + c_w;
                    cur_lex_text_w = cur_lex_text_w + " ";
                }
                c_w = read_word (str);
            }
        }
    }
} 

// modes of where_clause
enum mode_type
{
    LIKE_alt,
    IN_alt_T,
    IN_alt_L,
    LOG_alt,
    ALL_alt
};

namespace parser_where
{
    int flag_log = 0;
    int flag_expr = 0;
    enum mode_type mode;
    std::multiset <long> mst_l;
    std::multiset <std::string> mst_s;

    void init (std::string & str, ITable bd)
    {
        lexer_where :: init (str);
        lexer_where :: next (str, bd);
    }
    
    // functions for sintactic parser
    
} // end of namespace parser_where



/*---------------Interpreter---------------*/
Interpreter :: Interpreter (std::string & str)
{
    std::string cur_word;
    cur_word = read_word (str); // operation
    if (cur_word == "SELECT")
    {
        select_sentence (str);
    }
    else if (cur_word == "INSERT")
    {
        insert_sentence (str);
    }
    else if (cur_word == "UPDATE")
    {
        update_sensence (str);
    }
    else if (cur_word == "DELETE")
    {
        delete_sentence (str);
    }
    else if (cur_word == "CREATE")
    {
        create_sentence (str);
    }
    else if (cur_word == "DROP")
    {
        drop_sentence (str);
    }
    else
    {
        throw "ex";
    }
}

void Interpreter :: select_sentence (std::string & str)
{
}

void Interpreter :: insert_sentence (std::string & str)
{
}

void Interpreter :: update_sensence (std::string & str)
{
}

void Interpreter :: delete_sentence (std::string & str)
{
}

void Interpreter :: create_sentence (std::string & str)
{
}

void Interpreter :: drop_sentence (std::string & str)
{
    std::string cur_word;
    std::string t_name;
    cur_word = read_word (str);
    if (cur_word != "TABLE")
    {
        throw "ex";
    }
    t_name = read_word (str); // table_name
    cur_word.clear();
    // check if it is the end of the comand
    cur_word = read_word (str);
    if (!cur_word.empty())
    {
        throw "ex";
    }
    //bd_table.DeleteTable (t_name); 
    std::cout << "The table " << t_name << " was deleted" << std::endl;
}

void Interpreter :: field_description (std::string & str)
{
}

// list of record numbers need to be treated
std::vector <unsigned long> Interpreter :: where_clause (std::string & str)
{
    return {0};
}

#endif