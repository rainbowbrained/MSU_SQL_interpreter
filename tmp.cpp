#include <iostream>
#include "dbms.hpp"

int main()
{
    // testing ITableStruct methods
    ITableStruct mytable("MyTable");
    mytable.AddText("text1");
    mytable.AddLong("long1");
    mytable.AddText("t2");
    try
    {
        mytable.AddText("text1");
    }
    catch (Xception &x)
    {
        x.report();
    }

    try
    {
        mytable.SetTableName("NewName");
        mytable.SetFieldName("newlong1", "long1");
        std::cout << "get table name " << mytable.get_table_name() << std::endl;
        std::cout << "get fields name " << mytable.get_fields_names() << std::endl;
        std::cout << "get_num_fields " << mytable.get_num_fields() << std::endl;
        std::cout << "fields_length " << mytable.get_fields_length() << std::endl;
        mytable.print();
        mytable.DeleteField("text1", TEXT);
        mytable.print();
        ITable myitable(mytable);
        myitable.Add_line();
        myitable.Add_line();
        myitable.Add_line();
        myitable.Add_line();
        myitable.Add_line();
        myitable.Print_table();
        myitable.AddText("kill", "t2", 0);
        myitable.AddText("me", "t2", 1);
        myitable.AddLong(123, "newlong1", 2);
        myitable.Print_table();

        std::cout << "cell 2 from column newlong1 = " << myitable.GetLongField("newlong1", 2) << std::endl;
        std::cout << "cell 0 from column t2 = " << myitable.GetTextField("t2", 0) << std::endl;
        myitable.GetLongField("newlong1", 2) = 0;
        std::cout << "cell 2 from column newlong1 after assigning 0 = " << myitable.GetLongField("newlong1", 2) << std::endl;

        myitable.Add_Column(TEXT, "txt");
        myitable.Print_table();
        myitable.ToFile("table.txt");
        myitable.Delete_Column(TEXT, "txt"); 
        myitable.Print_table();
        //myitable.Delete_Column(TEXT, "something");
        ITable newtable ("table.txt");
        newtable.Print_table();
        newtable.Delete_Column(TEXT, "something");
    }
    catch (Xception &x)
    {
        x.report();
    }
};