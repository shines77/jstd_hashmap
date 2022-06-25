
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <string.h>

#include <iostream>
#include <iomanip>  // std::setw(), std::setfill(), std::setprecision().
#include <sstream>
#include <string>
#include <utility>

#include <jstd/basic/inttypes.h>
#include <jstd/string/string_view.h>
#include <jstd/type_traits.h>
#include <jstd/system/Console.h>
#include <jstd/test/Test.h>

struct pair_int_int {
    typedef int first_type;
    typedef int second_type;

    int first;
    int second;

    pair_int_int(int f, int s) : first(f), second(s) {}
};

struct pair_size_size {
    typedef size_t first_type;
    typedef size_t second_type;

    size_t first;
    size_t second;

    pair_size_size(size_t f, size_t s) : first(f), second(s) {}
};

struct pair_str_str {
    typedef std::string first_type;
    typedef std::string second_type;

    std::string first;
    std::string second;

    pair_str_str(const std::string & f, const std::string & s)
        : first(f), second(s) {}
};

struct pair_sv_sv {
    typedef jstd::string_view first_type;
    typedef jstd::string_view second_type;

    jstd::string_view first;
    jstd::string_view second;

    pair_sv_sv(const jstd::string_view & f, const jstd::string_view & s)
        : first(f), second(s) {}
};

template <typename T>
void display_constructible(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is constructible: ";
    std::cout << std::is_constructible<T>::value << '\n';
    std::cout << "is default constructible: ";
    std::cout << std::is_default_constructible<T>::value << '\n';
    std::cout << "is trivially constructible: ";
    std::cout << std::is_trivially_constructible<T>::value << '\n';
    std::cout << "is trivially default constructible: ";
    std::cout << std::is_trivially_default_constructible<T>::value << '\n';
    std::cout << "is nothrow constructible: ";
    std::cout << std::is_nothrow_constructible<T>::value << '\n';
    std::cout << "is nothrow default constructible: ";
    std::cout << std::is_nothrow_default_constructible<T>::value << "\n\n";
}

template <typename T>
void display_copy_constructible(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is copy-constructible: ";
    std::cout << std::is_copy_constructible<T>::value << '\n';
    std::cout << "is trivially copy-constructible: ";
    std::cout << std::is_trivially_copy_constructible<T>::value << '\n';
    std::cout << "is nothrow copy-constructible: ";
    std::cout << std::is_nothrow_copy_constructible<T>::value << "\n\n";
}

template <typename T>
void display_copy_assignable(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is copy-assignable: ";
    std::cout << std::is_copy_constructible<T>::value << '\n';
    std::cout << "is trivially copy-assignable: ";
    std::cout << std::is_trivially_copy_constructible<T>::value << '\n';
    std::cout << "is nothrow copy-assignable: ";
    std::cout << std::is_nothrow_copy_constructible<T>::value << "\n\n";
}

template <typename T>
void display_move_constructible(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is move-constructible: ";
    std::cout << std::is_move_constructible<T>::value << '\n';
    std::cout << "is trivially move-constructible: ";
    std::cout << std::is_trivially_move_constructible<T>::value << '\n';
    std::cout << "is nothrow move-constructible: ";
    std::cout << std::is_nothrow_move_constructible<T>::value << "\n\n";
}

template <typename T>
void display_move_assignable(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is move-assignable: ";
    std::cout << std::is_move_assignable<T>::value << '\n';
    std::cout << "is trivially move-assignable: ";
    std::cout << std::is_trivially_move_assignable<T>::value << '\n';
    std::cout << "is nothrow move-assignable: ";
    std::cout << std::is_nothrow_move_assignable<T>::value << "\n\n";
}

template <typename T>
void display_destructible(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << '\n';

    std::cout << "is destructible: ";
    std::cout << std::is_destructible<T>::value << '\n';
    std::cout << "is virtual destructor: ";
    std::cout << std::has_virtual_destructor<T>::value << '\n';   
    std::cout << "is trivially destructible: ";
    std::cout << std::is_trivially_destructible<T>::value << '\n';
    std::cout << "is nothrow destructible: ";
    std::cout << std::is_nothrow_destructible<T>::value << "\n\n";
}

template <typename T>
void display_copy_and_move_impl()
{
    std::cout << "is constructible: ";
    std::cout << std::is_constructible<T>::value << '\n';
    std::cout << "is default constructible: ";
    std::cout << std::is_default_constructible<T>::value << '\n';
    std::cout << "is trivially constructible: ";
    std::cout << std::is_trivially_constructible<T>::value << '\n';
    std::cout << "is trivially default constructible: ";
    std::cout << std::is_trivially_default_constructible<T>::value << '\n';
    std::cout << "is nothrow constructible: ";
    std::cout << std::is_nothrow_constructible<T>::value << '\n';
    std::cout << "is nothrow default constructible: ";
    std::cout << std::is_nothrow_default_constructible<T>::value << "\n\n";

    std::cout << "is copy-constructible: ";
    std::cout << std::is_copy_constructible<T>::value << '\n';
    std::cout << "is trivially copy-constructible: ";
    std::cout << std::is_trivially_copy_constructible<T>::value << '\n';
    std::cout << "is nothrow copy-constructible: ";
    std::cout << std::is_nothrow_copy_constructible<T>::value << "\n\n";

    std::cout << "is copy-assignable: ";
    std::cout << std::is_copy_constructible<T>::value << '\n';
    std::cout << "is trivially copy-assignable: ";
    std::cout << std::is_trivially_copy_constructible<T>::value << '\n';
    std::cout << "is nothrow copy-assignable: ";
    std::cout << std::is_nothrow_copy_constructible<T>::value << "\n\n";

    std::cout << "is move-constructible: ";
    std::cout << std::is_move_constructible<T>::value << '\n';
    std::cout << "is trivially move-constructible: ";
    std::cout << std::is_trivially_move_constructible<T>::value << '\n';
    std::cout << "is nothrow move-constructible: ";
    std::cout << std::is_nothrow_move_constructible<T>::value << "\n\n";

    std::cout << "is move-assignable: ";
    std::cout << std::is_move_assignable<T>::value << '\n';
    std::cout << "is trivially move-assignable: ";
    std::cout << std::is_trivially_move_assignable<T>::value << '\n';
    std::cout << "is nothrow move-assignable: ";
    std::cout << std::is_nothrow_move_assignable<T>::value << "\n\n";

    std::cout << "is destructible: ";
    std::cout << std::is_destructible<T>::value << '\n';
    std::cout << "is virtual destructor: ";
    std::cout << std::has_virtual_destructor<T>::value << '\n';   
    std::cout << "is trivially destructible: ";
    std::cout << std::is_trivially_destructible<T>::value << '\n';
    std::cout << "is nothrow destructible: ";
    std::cout << std::is_nothrow_destructible<T>::value << "\n\n";
}

template <typename T>
void display_copy_and_move(const std::string & name)
{
    std::cout << std::boolalpha;
    std::cout << name << ":" << "\n\n";

    display_copy_and_move_impl<T>();
}

template <typename T>
void display_pair_copy_and_move(const std::string & name)
{
    typedef typename T::first_type  first_type;
    typedef typename T::second_type second_type;

    std::cout << std::boolalpha;
    std::cout << name << ":" << "\n\n";

    std::cout << "First  type is pod<first_type>:  ";
    std::cout << std::is_pod<first_type>::value << '\n';
    std::cout << "Second type is pod<second_type>: ";
    std::cout << std::is_pod<second_type>::value << "\n\n";

    display_copy_and_move_impl<T>();
}

template <typename T>
void display_pair_class_trivial(const std::string & name)
{
    display_pair_copy_and_move<T>(name);
}

void class_trivial_test()
{
    display_copy_and_move<std::string>("std::string");
    display_copy_and_move<jstd::string_view>("jstd::string_view");

    printf("--------------------------------------------------------\n\n");

    display_pair_class_trivial<pair_int_int>("struct pair_int_int()");
    display_pair_class_trivial<pair_size_size>("struct pair_size_size()");
    display_pair_class_trivial<pair_str_str>("struct pair_str_str()");
    display_pair_class_trivial<pair_sv_sv>("struct pair_sv_sv()");

    printf("--------------------------------------------------------\n\n");

    display_pair_class_trivial<std::pair<int, int>>("std::pair<int, int>");
    display_pair_class_trivial<std::pair<size_t, size_t>>("std::pair<size_t, size_t>");
    display_pair_class_trivial<std::pair<std::string, std::string>>
                              ("std::pair<std::string, std::string>");
    display_pair_class_trivial<std::pair<jstd::string_view, jstd::string_view>>
                              ("std::pair<jstd::string_view, jstd::string_view>");

    printf("--------------------------------------------------------\n\n");
}

int main(int argc, char * argv[])
{
    class_trivial_test();

    //jstd::Console::ReadKey();
    return 0;
}
