#include "dbms.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

void Xception::report()
{
    std::cerr << Message << std::endl;
    return;
};

Xception::Xception(table_exception_code errcode)
{
    switch (errcode)
    {
    case X_EXIST:
        Message = "There is already a field with this type and name";
        break;
    case X_NEXIST:
        Message = "There is no such a field with this type and name";
        break;
    case X_REACHEND:
        Message = "Reached the end of the table";
        break;
    case X_LNEXIST:
        Message = "There is no such line in the table";
        break;
    case X_EMPTY:
        Message = "The table is empty";
        break;
    case X_OPEN:
        Message = "Can not open the file";
        break;
    case X_INFO:
        Message = "Not enough information in the file";
        break;
    case X_CHANGE:
        Message = "Can not change a field, because it has different type";
        break;
    default:
        break;
    }
};

// ------- text field -------

std::string &ITextField::GetElem()
{
    std::string *tmp = &field;
    return *tmp;
}

long &ILongField::GetElem()
{
    long *tmp = &field;
    return *tmp;
}

// add a field in the heading
void ITableStruct::AddText(const std::string &Name)
{
    std::vector<std::pair<std::string, Type>>::iterator itt = fields_names.begin();
    while (itt != fields_names.end())
    {
        if ((*itt).first == Name)
        {
            throw Xception(X_EXIST);
        }
        itt++;
    }
    fields_names.push_back({Name, TEXT});
    fields_length += sizeof(ITextField);
    return;
};

void ITableStruct::AddLong(const std::string &Name)
{
    std::vector<std::pair<std::string, Type>>::iterator itt = fields_names.begin();
    while (itt != fields_names.end())
    {
        if ((*itt).first == Name)
        {
            throw Xception(X_EXIST);
        }
        itt++;
    }
    fields_names.push_back({Name, LONG});
    fields_length += sizeof(ILongField);
    return;
};

void ITableStruct::DeleteField(const std::string &Name, const Type field_type)
{
    std::vector<std::pair<std::string, Type>>::iterator itt = fields_names.begin();
    while (itt != fields_names.end())
    {
        if (((*itt).first == Name) && ((*itt).second == field_type))
        {
            fields_names.erase(itt);
            if (field_type == TEXT)
                fields_length -= sizeof(ITextField);
            else
                fields_length -= sizeof(ILongField);
            return;
        }
        itt++;
    }
    if (itt == fields_names.end())
    {
        throw Xception(X_NEXIST);
    }
    return;
};

void ITableStruct::SetTableName(const std::string &Name)
{
    table_name = Name;
    return;
};

void ITableStruct::SetFieldName(const std::string &Name, const std::string &prevName)
{
    std::vector<std::pair<std::string, Type>>::iterator itt = fields_names.begin();
    while (itt != fields_names.end())
    {
        if ((*itt).first == prevName)
        {
            (*itt).first = Name;
            return;
        }
        itt++;
    }
    if (itt == fields_names.end())
    {
        throw Xception(X_NEXIST);
    }
    return;
};

Type ITableStruct::GetType(size_t index) { return fields_names[index].second; };

std::string ITableStruct::get_table_name() { return table_name; };

size_t ITableStruct::get_num_text_fields()
{
    size_t res = 0;
    for (auto it : fields_names)
    {
        if (it.second == TEXT)
            res++;
    }
    return res;
};

size_t ITableStruct::get_num_long_fields()
{
    size_t res = 0;
    for (auto it : fields_names)
    {
        if (it.second == LONG)
            res++;
    }
    return res;
};

std::string ITableStruct::get_fields_names()
{
    std::string tmp;
    for (size_t i = 0; i < fields_names.size() - 1; i++)
    {
        tmp += fields_names[i].first + " | ";
    }
    tmp += fields_names[fields_names.size() - 1].first;
    return tmp;
};

size_t ITableStruct::get_num_fields() { return fields_names.size(); };

size_t ITableStruct::get_fields_length() { return fields_length; };

std::string ITableStruct::get_field_name(size_t i) { return fields_names[i].first; };

void ITableStruct::print()
{
    std::cout << table_name << std::endl
              << "\\ ";
    for (size_t i = 0; i < fields_names.size(); i++)
    {
        std::cout << fields_names[i].first << "\t | ";
    }
    std::cout << std::endl;
}

// ------------------------------ ITABLE METHODS ------------------------------

ITable::ITable(const std::string filename)
{
    cur_line = 0;
    cur_size = 0;
    capacity = 0;
    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        throw Xception(X_OPEN);
    }

    std::string word;
    fin >> word;

    heading.SetTableName(word);
    std::getline(fin, word, '\n');
    std::getline(fin, word, '\n');

    if (word.size() == 0)
    {
        //there is no column, getting the number of lines;
        while (!fin.eof())
        {
            getline(fin, word, '\n');
            Add_line();
        }
        fin.close();
        // count is a nummber of columns in the table
    }
    else
    {
        size_t start = 0, end = 0;
        //header
        while (end < word.size())
        {
            while (word[end] != ':')
                end++;
            std::string tmp;
            tmp.insert(0, word, start, end - start);
            if (word[end + 1] == '0' + TEXT)
            {
                heading.AddText(tmp);
            }
            else
            {
                heading.AddLong(tmp);
            }
            start = end + 3;
            end = start;
        }
        heading.print();
        size_t cur_line_ = 0;
        while (!fin.eof())
        {
            Add_line();
            getline(fin, word, '\n');
            if (word.size())
            {
                size_t cur_cell = 0, i = 0;
                while (cur_cell < heading.get_num_fields())
                {
                    while ((cur_cell < heading.get_num_fields()) && (i < word.size()) && (word[i] == '\t'))
                    {
                        i++;
                        cur_cell++;
                    }
                    if ((cur_cell < heading.get_num_fields()) && (i == word.size() - 1))
                    {
                        throw Xception(X_INFO); // not enough information
                    }
                    if ((i < word.size()) && (word[i] != '\t'))
                    {
                        std::string tmp;
                        size_t j = i;
                        i++;
                        while ((i < word.size()) && (word[i] != '\t'))
                        {
                            //tmp[j] = word[i];
                            i++;
                        }
                        tmp.insert(0, word, j, i - j);
                        if (heading.GetType(cur_cell) == TEXT)
                        {
                            AddText(tmp, heading.get_field_name(cur_cell), cur_line_);
                        }
                        else
                        {
                            long val = atol(tmp.c_str());
                            AddLong(val, heading.get_field_name(cur_cell), cur_line_);
                        }
                        cur_cell++;
                        i++;
                    }
                }
            }
            cur_line_++;
        }
        fin.close();
    }
};

ITable ITable::Open(const std::string filename)
{
    ITable tmp(filename);
    return tmp;
};

void ITable::ReadFirst()
{
    cur_line = 0;
    return;
};

void ITable::ReadNext()
{
    if (cur_line == cur_size)
    {
        throw Xception(X_REACHEND);
        ;
    }
    cur_line++;
    return;
};

void ITable::AddText(const std::string &val, const std::string &title, const size_t line)
{
    if (line > cur_size)
    {
        throw Xception(X_LNEXIST);
    }
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if ((heading.get_field_name(i) == title) && (heading.GetType(i) == TEXT))
        {
            text_fields[fields[line][i]].Change_field_t(val);
            return;
        }
    }
    throw Xception(X_NEXIST);
    return;
};

void ITable::AddLong(const long &val, const std::string &title, const size_t line)
{
    if (line > cur_size)
    {
        throw Xception(X_LNEXIST);
    }
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if ((heading.get_field_name(i) == title) && (heading.GetType(i) == LONG))
        {
            long_fields[fields[line][i]].Change_field_l(val);
            return;
        }
    }
    throw Xception(X_NEXIST);
    return;
};

std::string &ITable::GetTextField(const std::string &Name, const size_t line)
{
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if ((heading.GetType(i) == TEXT) && (heading.get_field_name(i) == Name))
        {
            return text_fields[fields[line][i]].GetElem();
        }
    }
    throw Xception(X_NEXIST);
};

long &ITable::GetLongField(const std::string &Name, const size_t line)
{
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if ((heading.GetType(i) == LONG) && (heading.get_field_name(i) == Name))
        {
            return long_fields[fields[line][i]].GetElem();
        }
    }
    throw Xception(X_NEXIST);
};

Type ITable::GetFieldType(const std::string &Name)
{
    size_t i = 0;
    while ((i < heading.get_num_fields()) && (heading.get_field_name(i) != Name))
        i++;
    if (i < heading.get_num_fields())
        return heading.GetType(i);
    else
        throw Xception(X_NEXIST);
};

void ITable::Add_line()
{ // add a line
    std::vector<size_t> tmp_vec;
    if (cur_size == capacity)
    {
        capacity = (capacity + 1) * 2;
        fields.resize(capacity, tmp_vec);
    }
    fields[cur_size].resize(heading.get_num_fields(), 0);
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if (heading.GetType(i) == TEXT)
        {
            ITextField t;
            text_fields.push_back(t);
            fields[cur_size][i] = text_fields.size() - 1;
        }
        else
        {
            ILongField t;
            long_fields.push_back(t);
            fields[cur_size][i] = long_fields.size() - 1;
        }
    }
    cur_size++;
    return;
};

void ITable::Delete_line(const size_t line)
{ //delete a line
    if (cur_size < line)
    {
        throw Xception(X_LNEXIST);
    }
    if (cur_size == 0)
    {
        throw Xception(X_EMPTY);
    }
    cur_size--;
    size_t text_delete = 0, long_delete = 0;
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if (heading.GetType(i) == TEXT)
        {
            text_fields.erase(text_fields.begin() + fields[line][i]);
            text_delete++;
        }
        else
        {
            long_fields.erase(long_fields.begin() + fields[line][i]);
            long_delete++;
        }
    }
    for (size_t i = line; i < fields.size(); i++)
    {
        for (size_t j = 0; j < heading.get_num_fields(); j++)
        {
            if (heading.GetType(j) == TEXT)
            {
                fields[i][j] -= text_delete;
            }
            else
            {
                fields[i][j] -= long_delete;
            }
        }
    }
    fields.erase(fields.begin() + line);
    return;
};

void ITable::Delete_Column(const Type t, const std::string &Name)
{
    size_t i;

    for (i = 0; i < heading.get_num_fields(); i++)
    {
        if ((heading.GetType(i) == t) && (heading.get_field_name(i) == Name))
        {
            for (size_t j = fields.size() - 2; j > 0; j--)
            {

                if (t == TEXT)
                {
                    text_fields.erase(text_fields.begin() + fields[j][i]);
                }
                else
                {
                    long_fields.erase(long_fields.begin() + fields[j][i]);
                }
                for (int k = heading.get_num_fields() - 1; k >= 0; k--)
                {
                    if (heading.GetType(k) == t)
                        fields[j][k] -= j;
                }
                fields[j].erase(fields[j].begin() + i);
            }
            if (t == TEXT)
            {
                text_fields.erase(text_fields.begin() + fields[0][i]);
            }
            else
            {
                long_fields.erase(long_fields.begin() + fields[0][i]);
            }

            for (size_t j = heading.get_num_fields() - 1; j > i; j--)
            {
                if (heading.GetType(j) == t)
                    fields[0][j]--;
            }

            heading.DeleteField(Name, t);
            // output
            return;
        }
    }
    throw Xception(X_NEXIST);
    return;
};

void ITable::Add_Column(const Type t, const std::string &Name)
{
    size_t pos = 0, line = 0;
    if (t == TEXT)
    {
        size_t num_fields = heading.get_num_text_fields();
        heading.AddText(Name);
        ITextField tmp;
        while (pos < text_fields.size())
        {
            if ((pos == num_fields) || ((line > 0) && (pos == line * (num_fields + 1) + 1)))
            {
                text_fields.insert(text_fields.begin() + pos, tmp);
            }
            pos++;
            if (pos == (num_fields + 1) * (line + 1))
                line++;
        }
        text_fields.push_back(tmp);
        fields[0].push_back(num_fields);
        for (size_t i = 1; i < fields.size(); i++)
        {
            fields[i].push_back((num_fields + 1) * i + 1);
            for (size_t j = 0; j < heading.get_num_fields() - 1; j++)
            {
                if (heading.GetType(j) == TEXT)
                    fields[i][j] += i;
            }
        }
    }
    else
    {
        size_t num_fields = heading.get_num_long_fields();
        heading.AddLong(Name);
        ILongField tmp;
        while (pos < long_fields.size())
        {
            if ((pos == num_fields) || ((line > 0) && (pos == line * (num_fields + 1) + 1)))
            {
                long_fields.insert(long_fields.begin() + pos, tmp);
            }
            pos++;
            if (pos == (num_fields + 1) * (line + 1))
                line++;
        }
        long_fields.push_back(tmp);
        fields[0].push_back(num_fields);
        for (size_t i = 1; i < fields.size(); i++)
        {
            fields[i].push_back((num_fields + 1) * i + 1);
            for (size_t j = 0; j < heading.get_num_fields() - 1; j++)
            {
                if (heading.GetType(j) == LONG)
                    fields[i][j] += i;
            }
        }
    }
    return;
};

void ITable::ToFile(const std::string &filename)
{
    std::ofstream fout(filename);
    if (!fout.is_open())
    {
        throw Xception(X_OPEN);
    }
    fout << heading.get_table_name() << std::endl;
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        fout << heading.get_field_name(i) << ":" << heading.GetType(i) << "\t";
    }
    fout << std::endl;
    for (size_t i = 0; i < fields.size() - 1; i++)
    {
        for (size_t j = 0; j < heading.get_num_fields(); j++)
        {
            //fout << "converting " << fields[i][j] << ":" << heading.GetType(j) << std::endl;
            if ((fields[i][j] < text_fields.size()) && (heading.GetType(j) == TEXT))
            {
                if (text_fields[fields[i][j]].isInit())
                {
                    fout << text_fields[fields[i][j]].GetElem();
                }
                fout << "\t";
            }
            else if ((fields[i][j] < long_fields.size()) && (heading.GetType(j) == LONG))
            {
                if (long_fields[fields[i][j]].isInit())
                {
                    fout << long_fields[fields[i][j]].GetElem();
                }
                fout << "\t";
            }
        }
        fout << std::endl;
    }
    fout.close();
    return;
};

// utility functions to print the table
void ITable::Print_field_names()
{
    heading.print();
    return;
};

void ITable::Print_line(const size_t line)
{
    std::cout << line << "|\t";
    for (size_t i = 0; i < heading.get_num_fields(); i++)
    {
        if ((fields[line][i] < text_fields.size()) && (heading.GetType(i) == TEXT))
        {
            if (text_fields[fields[line][i]].isInit())
            {
                std::cout << text_fields[fields[line][i]].GetElem();
            }
            std::cout << "\t |";
        }
        else if ((fields[line][i] < long_fields.size()) && (heading.GetType(i) == LONG))
        {
            if (long_fields[fields[line][i]].isInit())
            {
                std::cout << long_fields[fields[line][i]].GetElem();
            }
            std::cout << "\t |";
        }
    }
    std::cout << std::endl;
    return;
};

//prints
void ITable::Print_line()
{
    Print_line(cur_line);
    return;
};

void ITable::Print_table()
{
    std::cout << "________________________________________________________" << std::endl;
    std::cout << "Table parameters: " << heading.get_num_fields() << " * " << fields.size() << std::endl;
    heading.print();

    for (size_t i = 0; i < fields.size() - 1; i++)
    {
        Print_line(i);
    }
    std::cout << "________________________________________________________" << std::endl;
    return;
};