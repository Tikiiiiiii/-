#### 18.让接口更容易正确使用，不易误使用
对于一个函数接口，我们应该
（1）防止客户传入错误次序的参数：
（2）防止客户传入一个无效或无意义的参数：
```cpp
//E.g:
//Date类：used to describe date
class Date{
public:
    Date(int month,int day,int year);
}
//wrong sequence
Date d1(30,3,2023);//mean：2023-30-2
//true meaning：2023-2-30
//invalid argument
Date d2(2,30,2023);//2023-2-30(not exist)
```
解决办法：使用一个struct去描述Date的属性（即**导入一个新类型规范化接口**）
```cpp
struct Day{
    explict Day(int d):day(d){};//explict防止隐式转换
    int day;
}
struct Month{
    explict Month(int d):month(d){};//explict防止隐式转换
    int month;
}
struct Year{
    explict Year(int d):Year(d){};//explict防止隐式转换
    int Year;
}
class Date{
public:
    Date(const Month&m,const Day&d,cont Year& y);
}
```
上述方法解决参数次数的问题，对于无效或无意义参数，可以外加参数范围去限制其值（**其中一种办法就是利用函数列举值（适用于值少的情况**）：
```cpp
class Month{
public:
    static Month Jan(){return Month(1);}//为什么用referenc-returning函数参见条款4
    //...
    static Month Dec(){return Month(12);}
private:
    explicit Month (int m);
}
Date d(Month::Jan(),Day(30),Year(2023));
```
（3）限制类型内什么事可做，什么事不可做，如利用const防止比较操作写出赋值操作
>a*b=c          //见条款3
（4）提供一致性接口：如同STL中的函数一样，他们都提供一个size()返回容器的大小，size总是这个接口的名字，我们应该设置一致的接口减轻用户记忆的负担
（5）接口要求用户记住做某事是不正确的：如delete对应资源：
```cpp
Investment* createInvestment();
std::tr1::shared_ptr<Investment> createInvestment();//由于createInvestment返回的是一个指针，需要delete，为避免资源泄漏，使用shared_ptr管理避免用户误删
//但是如果用户删除多次同一指针该如何做：
std::tr1::shared_ptr<Investment> pInv(0,getRidOfInvestmetn);
//使用shared_ptr的删除器绑定delete，当delete时就调用该函数
//但上述语句有误，0不是一个有效指针
std::tr1::shared_ptr<Investment> createInvestment(){
    std::tr1::shared_ptr<Investment> return_res(static_cast<Investment>(0),getRidOfInvestment);
    //利用static_cast将0转为指针，并绑定删除器
    return_res=();//正确对象
    return return_res;
}
```
（6）解决cross-DLL-problem（跨动态连接程序库（DLL）在一个程序new而在另一个程序delete，而shared_ptr也刚好可用解决这个问题：shared_ptr的删除器是来自创建它的DLL中的

#### 19.设计class如同设计type
在设计一个class之前先回答以下问题：
（1）新type对象应该如何创建和销毁：*构造函数和析构函数，new和delete系列函数*：
（2）新type对象是否会被继承：*析构函数是否需要变成virtual*
（3）新type对象的初始化和赋值区别：*copying函数：赋值构造函数和赋值运算符*
（4）新type对象如果被pass-by-value意味着什么：*赋值构造函数* --条款20
（5）新type对象的操作符和函数：*成员函数和成员变量*
（6）什么样的标准函数应该驳回：*成员函数和成员变量的private*
（7）谁能取用type的成员：*private、public、protected、friends*
（8）新type对象需要作哪些类型转换：*成员函数operator Type Or Explicit构造函数*
（9）新type对象的合法值：*成员变量的合法性检查和异常抛出*
（10）新type的未声明接口：*应减少定义虚函数的多态接口*
```cpp
class Base {  
public:  
    virtual void foo() = 0; // 纯虚函数  
};  
class Derived : public Base {  
public:  
    void foo() override {  //这就是未声明接口
        // 实现接口中的方法  
    }  
};
```
新的未声明接口，它如何为效率、异常安全性和资源运用提供保证，借此来确定它的必要性：
效率：如果为类型定义了新的接口，那么每次对该类型的操作都可能需要执行额外的开销，例如虚函数调用、动态内存分配等。这可能导致性能下降，尤其是在需要大量操作该类型的情况下。

异常安全性：在C++中，异常安全性通常与资源管理有关。如果一个类型定义了新的接口，那么在处理该类型的对象时，程序员需要考虑更多的细节，例如如何正确地释放资源。这增加了编程的复杂性，并可能导致资源泄漏或异常。

资源运用：定义新的接口可能会导致资源的过度使用或错误使用。例如，如果一个接口公开了过多的内部细节，那么其他代码可能会尝试使用这些内部细节，从而导致资源的不必要消耗或错误操作。
因此，在定义接口时需要谨慎考虑其可能的影响，以确保程序的效率、异常安全性和资源运用得到保障。
（11）新type的一般性：*模板*泛化type
（12）确认新type的必要性:有时*non-member or template*都可以达到同样的效果

#### 20.宁以pass-by-reference替换pass-by-value
实际上，当我们pass-by-value时，函数参数用到的是以实参为副本处于栈的复件（这个复件是调用复制构造函数所得到的，因此效率可能会降低），另外，对于多态的实现，我们pass-by-value可能导致内容的切割，一个本是派生类的对象被切割成基类的对象
取而代之，如果我们使用pass-by-reference-const来代替就不会由效率问题，并且安全问题由const关键字来保障

以上规则不针对内置类型、STL的迭代器和函数对象

#### 21.返回对象时勿返回其引用
在函数执行过程中，返回值通常在函数体内被创建，退出函数后被销毁，如果返回值pass-by-reference，那可能导致我们获得了一个指向不知名栈段的指针，因此这是行不通的；另外如果我们把它创建到堆区就是new一个新对象并返回其引用，这又引出了一个新的问题，如何释放该堆区的资源，当我们退出函数返回后，就无法再进行delete了。

#### 22.将成员声明为private
（1）为什么不把成员声明为public：
语法一致性，public中都是一些函数，调用函数需要用()，访问成员不需要，两者语法需要客户记忆
（2）使用private和成员函数可以实现控制权限
（3）封装性，把函数声明为public，把成员声明为private可以屏蔽实现细节，平衡效率和空间的关系
（4）当取消某个public或protected变量时，所有的客户代码和derived classes都会被影响

#### 23.宁以non-member函数和non-friend替换member函数
增加封装性：
考虑一个浏览器
```cpp
class WebBrowser {
    public:
    void clearCache();
    void clearHistory();
    void clearCookies();
}
//用户想一键清空
class WebBrowser {
    public:
    void clearAll();//clear，all调用前面三个函数
}
//non-member函数形式：
void ClearBrowser(WebBrowser w){
    w.clearCache();
    w.clearHistory();
    w.clearCookiers();
}
```
对于上述两种函数，我们考虑封装性，对于一个数据，如果越多的数据能够访问它那他的封装性就越差，而member函数可以访问private的一切，相反non-member函数却不行，相比较之下，non-member函数封装性更好。
提升包裹弹性：
用户往往需要的只是某一功能的完整组件，如果把所有功能都封装到一个类中，或许用户并不会真正使用，反而增加了类的大小，一般都会定义一个相关类功能的头文件
>#include < bookmark >
>#include < cookies >
而member成员函数可以封装入这些对应的头文件中，提升包裹弹性
提升机能扩充性：
允许用户自主包装，用户写一个non-member函数把他们需要的函数包括起来。

#### 24.若所有参数均需要类型转换，设为non-member函数
当一个函数是类的成员函数时，编译器会为它生成一个隐式的this指针，这个指针会作为第一个参数传递给该函数。如果这个函数的所有参数都需要类型转换，那么编译器生成的this指针的类型也会需要转换。这可能会导致代码难以理解和维护。

此外，如果一个函数是类的成员函数，那么它的参数类型可能会因为类的不同而不同。这使得代码难以移植和维护。如果一个函数是非成员函数，那么它的参数类型就与类的类型无关，这使得代码更加通用和可移植。

#### 25.考虑支持不抛异常的swap函数
STL中提供的swap函数：
```cpp
namespace std{
    template<typename T>
    void swap(T&a,T&b){//swap模板
        T temp(a);
        a=b;
        b=temp
    }

}
```
对于上述的交换函数，只要它有copying函数，那么就可以使用该缺省swap函数，但我们进入其底层实现的细节，它会复制所赋予它的对象的所有内容，这大大降低了效率：
```cpp
//考虑pImpl（以指针指向对象，对象内才是完整的内容）
class WidgetImpl{
public:
    //...
private:
    int a,b,c;
    std::vector<double> v;//假设很多很多内容
    //...
}
class Widget{
    //采用pimpl方法
public:
    Widget(const Widget& rhs);
    Widget& operator =(const Widget&rhs)  //copying函数
    {
        //...
        *pImpl=*(rhs.pImpl);//调用内容的copying函数
    }  
private:
    WidgetImpl* pImpl;//指向对象，真正含数据的对象
}
```
当我们使用缺省的swap函数交换内容的时候，swap并不只是简单交换两者的指针，而是把对象的成员内容利用赋值构造函数再重新构造一遍，这对效率问题无疑是一个巨大的冲击。
解决上述问题的方法便是特例化，对Widget这一对象的特例化：
```cpp
namespace std{
    template<>
    void swap<Widget>(Widget&a,Widget&b){
        swap(a.pImpl,b.pImpl);
    }//但上述程序不会通过，因为pImpl是private对象，无法访问
}
class Widget{
public:
    //...
    //在类中定义一个swap成员函数
    void swap(Widget& other){
        swap(pImpl,other.pImpl);
    }
}
template<>
    void swap<Widget>(Widget&a,Widget&b){//特例化版本
        a.swap(b);
    }
//我们把这一过程泛化：
template<typename T>
class WidgetImpl{}
template<typename T>
class Widget{}
// namespace std{
//     template<typename T>
//     void swap< Widget<T> >(Widget<T>&a,Widget<T>&b){
//         a.swap(b);
//     }//但这不合法，因为我们企图特例化一个函数模板而不是类模板这是不被允许的
// }
//因此只能重载STL的swap函数：
namespace std{
    template<typename T>
    void swap(Widget<T>&a,Widget<T>&b){
        a.swap(b);
    }
}//但这也不合法，std的规则不允许别的新类型进入命名空间中
//于是我们在自建的命名空间创建swap
namespace WidgetStuff{
    //...//同之前的类定义
    template<typename T>
    void swap(Widget<T>&a,Widget<T>&b){
        a.swap(b);
    }
}
//这时widget的swap会自动调用相应命名空间的swap函数，如果无法匹配再调用std空间的swap函数
```
总结：
（1）当std::swap的效率低时，自行编写一个swap函数并确保不会抛出异常
（2）如果有了member-swap函数，也应该有一个non-member-swap函数调用它，对于classes，请特化swap函数
（3）调用swap时不带空间修饰词以调用特化版
