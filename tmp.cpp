#include <iostream>
#include "dbms.hpp"
#include "interpret.hpp"

int main()
{
    try
    {
        std::cout << "_________________________test 1_______________________________" << std::endl;
        std::string str = "SELECT * FROM Students WHERE Age + ( 2 * Age - 8 ) % 3 IN (18, 19, 20) ";
        Interpreter obj(str);
        std::cout << "_________________________test 2_______________________________" << std::endl;
        str = "SELECT * FROM Students WHERE First_name LIKE '[ABC][^mno]_x%'";
        Interpreter obj1(str);
        std::cout << "_________________________test 3_______________________________" << std::endl;
        str = "SELECT Surname, Phone FROM Students WHERE (Age > 19) OR (Surname = 'Ivanov') ";
        Interpreter obj2(str);
        std::cout << "_________________________test 4_______________________________" << std::endl;
        str = "UPDATE Students SET First_name = 'Unknown' WHERE Surname LIKE 'text%'  ";
        Interpreter obj3(str);
        std::cout << "_________________________test 5_______________________________" << std::endl;
        str = "DELETE FROM Students WHERE  Surname IN ('Ivanov', 'Sidorov') ";
        Interpreter obj4(str);
        str = "UPDATE Students SET Age = Age + 1 WHERE ALL  ";
        Interpreter obj5(str);
    }
    catch (SQL_Xception err)
    {
        err.report();
    }
    catch (char const *msg)
    {
        std::cout << msg;
    }

    try
    {
        std::cout << "_________________________testing other table methods_______________________________" << std::endl;
        ITable tabl;
        tabl = ITable("Students.txt");
        tabl.Print_table();

        tabl.Delete_line(4);
        tabl.Print_table();
        tabl.Delete_line(1);
        tabl.Print_table();
        tabl.Delete_line(0);
        tabl.Print_table();

        tabl.Add_line();
        tabl.Add_line();
        tabl.Add_line();
        tabl.AddLong(19, "Age", 0);
        tabl.AddText("text1", "First_name", 0);
        tabl.AddText("text2", "Surname", 0);
        tabl.AddText("text3", "Phone", 0);
        tabl.AddLong(20, "Age", 1);
        tabl.AddText("text4", "First_name", 1);
        tabl.AddText("text5", "Surname", 1);
        tabl.AddText("text6", "Phone", 1);
        tabl.AddLong(21, "Age", 2);
        tabl.AddText("TEXT1", "First_name", 2);
        tabl.AddText("text7", "Surname", 2);
        tabl.AddText("text10", "Phone", 2);
        tabl.Print_table();
        tabl.Delete_Empty();
        tabl.Print_table();
    }
    catch (Xception &x)
    {
        x.report();
    }

    //tabl.ToFile("Students.txt");
    //std::cout << "tabl.GetNumFields() = " << tabl.GetNumFields() << std::endl;

    // testing ITableStruct methods
    /*
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
    }*/
};