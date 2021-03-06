/*
 *   OBJECT-ORIENTED DATA BASED MANAGEMENT SYSTEM
 *
*/

#ifndef _DBMS_H_
#define _DBMS_H_

#include <vector>
#include <string>
#include <iostream>

#define MAX_LINE_NAME_LEN 15
#define MAX_TITLE_LEN 15
#define MAX_TEXT_LEN 20

enum table_exception_code
{
    X_EXIST,
    X_NEXIST,
    X_REACHEND,
    X_LNEXIST,
    X_EMPTY,
    X_OPEN,
    X_INFO,
    X_CHANGE,
    X_RM
};

class Xception
{
public:
    std::string Message;
    void report();
    Xception(table_exception_code);
    Xception(const std::string &aMessage) : Message(aMessage){};
    Xception(const Xception &xception) : Message(xception.Message){};
    ~Xception() {}
};

enum Type
{
    TEXT,
    LONG
};

class IField
{
    //const Type field_type;
public:
    virtual void Change_field_t(std::string) = 0;
    virtual void Change_field_l(long) = 0;
    virtual bool isInit() = 0;
    virtual ~IField() = default;
};

class ITextField : public IField
{
    bool init;
    std::string field;
    Type field_type;

public:
    ITextField() : init(0), field("\0"), field_type(TEXT){};
    ITextField(std::string val) : init(1), field(val), field_type(TEXT){};
    void Change_field_l(long val) override
    {
        //throw Xception(X_CHANGE);
        return;
    };
    void Change_field_t(std::string val) override
    {
        field = val;
        init = 1;
        return;
    };
    bool isInit() { return init; };
    std::string &GetElem();
};

class ILongField : public IField
{
    bool init;
    long field;
    Type field_type;

public:
    ILongField() : init(0), field(0), field_type(LONG){};
    ILongField(long val) : init(1), field(val), field_type(LONG){};
    long &GetElem();
    void Change_field_l(long val) override
    {
        field = val;
        init = 1;
        return;
    };
    void Change_field_t(std::string val) override
    {
        //throw Xception(X_CHANGE);
        return;
    };
    bool isInit() { return init; };
};

// titles, an array of fields with particular type, name of the table
class ITableStruct
{
    size_t fields_length;
    std::string table_name;
    std::vector<std::pair<std::string, Type>> fields_names;

public:
    ITableStruct() : fields_length(0), table_name("\0"){};
    ITableStruct(const ITableStruct &other) : fields_length(other.fields_length), table_name(other.table_name), fields_names(other.fields_names){};
    ITableStruct(std::string tn) : table_name(tn){};
    ITableStruct(std::string tn, std::vector<std::pair<std::string, Type>> fn, size_t fl) : fields_length(fl), table_name(tn), fields_names(fn){};
    void AddText(const std::string &Name);
    void AddLong(const std::string &Name);
    void DeleteField(const std::string &Name, const Type field_type);
    void SetTableName(const std::string &Name);
    void SetFieldName(const std::string &Name, const std::string &prevName);
    Type GetType(size_t index);

    std::string get_table_name();
    std::string get_fields_names();
    std::string get_field_name(size_t);
    size_t get_num_fields();
    size_t get_num_text_fields();
    size_t get_num_long_fields();
    size_t get_fields_length();
    void print();
};

class ITable
{
    ITableStruct heading;
    std::vector<std::vector<size_t>> fields;
    std::vector<ITextField> text_fields;
    std::vector<size_t> text_fields_sizes;
    std::vector<ILongField> long_fields;

public:
    ITable() {};
    ITable(const ITableStruct &other) : heading(other){};
    ITable(const ITable &other) : heading(other.heading), fields(other.fields), 
            text_fields(other.text_fields), text_fields_sizes(other.text_fields_sizes), 
            long_fields (other.long_fields) {};

    ITable(const std::string filename); //import from file
    ITable Open(const std::string filename);

    void SetName(const std::string &title); // set table's name
    std::string GetName(); // get table's name
    void AddTitle(Type t, const std::string &title, size_t size);

    void AddText(const std::string &val, const std::string &title, const size_t line);
    void AddLong(const long &val, const std::string &title, const size_t line);
    void AddText(const std::string &val, const size_t i); // insert in the last line
    void AddLong(const long &val, const size_t i);// insert in the last line

    std::string &GetTextField(const std::string &Name, const size_t line);
    long &GetLongField(const std::string &Name, const size_t line);
    Type GetFieldType (const std::string &Name);
    Type GetFieldType (size_t i);

    void Add_line();                     // add a line
    void Delete_line(const size_t line); //delete a line
    void Add_Column(const Type, const std::string &Name);
    void Delete_Column(const Type, const std::string &);
    void Delete_Empty ();
    void ToFile(const std::string &filename);

    // utility functions to print the table
    void Print_field_names();
    void Print_line(const size_t);
    void Print_line(const std::vector <std::string> &f_names, const size_t);
    void Print_table();

    bool isTitle (const std::string title);
    size_t GetNumFields (); // num of columns
    size_t GetNumLines (); // num of rows
    size_t GetTextSize (size_t i); // index in the header!
    size_t GetTextSize (const std::string &f_name);

    void Delete_Table (const std::string tname);
};
#endif