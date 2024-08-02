#### 5.了解编译器默默编写并调用哪些函数
编译器会为每个空类创建default构造函数，copy函数构造函数，copy赋值运算符以及析构函数，这些函数只有在被使用时才被编译器创建
另外，若一个类中有引用成员或const成员等不支持修改的成员，c++将会拒绝生成赋值操作符；若一个类继承了了一个base类，base类中存在private成员，那么编译器也拒绝生产赋值操作符，因为他无权修改

#### 6.明确拒绝编译器自动生成的函数
如果想要屏蔽赋值构造和赋值运算符，需要继承一个赋制构造和赋值运算符是private属性的基类，原因是当我们仅在本类中使用private属性创建赋值构造和赋值运算符时，编译器不再会自动生成相应函数，但连接器会报错，而friend函数以及成员函数仍然可以使用里面的赋值构造函数，这不是我们想要的结果，但当我们把它写成所继承的空基类，那么就可以避免这些问题，在编译阶段便能发现错误

#### 7.为多态基类声明virtual析构函数
当一个类要作为基类并设置指针指向它的派生类对象时，应把基类的析构函数写成virtual形式，否则在用基类指针指向派生类对象实现多态的析构过程中会导致只释放了派生对象的继承部分，派生部分内存泄漏。
当一个类不作为基类也不作多态使用时，不应随意增加virtual析构函数，这会导致内存的无意义浪费

#### 8.别让异常逃离析构函数
```cpp
class DBconnection{
public:
    //...
    static DBconnection::create();//返回一个DBconncetion对象
    void close();
}
//防止用户忘记关闭，创建一个管理对象，自动调用
class DBc{
public:
    ~DBc(){db.close();} 
    //...
private:
    DBconnection db;
}
//对于用户代码：
{
    DBc dbc(DBconnection::create);
    //...
    //结束后自动销毁
}
```
对于上述情况符合我们的现实需求，但是如果这个类在析构时出现了异常，那么类也会因此析构，异常也就从析构函数中出来了，如果是多个同一种异常从析构函数出来，c++编译器就会报不明确行为，它无法判别谁是谁，因此这并不是我们想要的的结果，我们应该阻止异常从析构函数中出来影响后面的代码，解决方法：
（1）在遇到异常时立即终止程序：
```cpp
~DBc(){
    try{db.close();}
    catch(...){
        //...
        std::abort();
    }
} 
```
(2)吞下异常，不管：
```cpp
~DBc(){
    try{db.close();}
    catch(...){
        //...记录异常
    }
} 
```
而最正确的应该是：
```cpp
class DBc{
public:
    void close()
    {
        db.close();
        closed=true;
    }
    ~DBc()
    {
        if(!closed){
        try{
            db.close();
        }
        catch(...){
        //...记录异常，强制关闭或不管
        }
        }
    } 
private:
    DBconnection db;
    bool closed;
}
//对于用户代码：
{
    DBc dbc(DBconnection::create);
    //...
    //结束后自动销毁
}
```
这时关闭的负担就交给了客户，客户调用了close函数产生了异常也有机会反映并且该异常并不是从析构函数抛出。

#### 9.绝不在构造和析构中调用virtual函数
在一个构造函数中调用virtual函数会导致这样一个过程，在我们创建这个基类的派生类对象时，派生类对象会率先调用基类构造函数，而基类构造函数中带有纯虚函数，这个函数此时还无定义，那将会出错。
```cpp
class Transaction {//所有交易的 base class
public:
    Transaction();
    virtual void logTransaction() const = 0; //做出一份因类型不同而不同
                                            //的日志记录(logentry)
    //...
}
Transaction::Transaction () //base class构造函数之实现
{
    //...
    logTransaction(); //最后动作是志记这笔交易
}
class BuyTransaction: public Transaction {//derived class
public:
    virtual void logTransaction() const; //志记（log）此型交易
    //...
};
class SellTransaction: public Transaction { //derived class
public:
    virtual void logTransaction() const; //志记（log）此型交易
    //···
};
BuyTransaction b;//出错
```
当然，如果把基类的构造函数改为init()，并把虚函数放在其中也是不可行的:
```cpp
Transaction::Transaction () //base class构造函数之实现
{
    //...
    logTransaction(); //最后动作是志记这笔交易
}
```
也就是说我们必须保证**virtual函数不应该构造函数、析构函数以及构造函数析构函数调用的子函数集中**

#### 10.令operator = 返回 a reference to *this
在赋值操作时是允许a=b=c这样的连续赋值操作，自定义对象也应该返回一个引用来支持该操作


#### 11.令operator = 完成自我赋值
在类无资源管理的情况下，w=w,a[i]=a[j],* px= *py这些自我赋值的操作都是安全的
但涉及到资源管理的类的自我赋值就有可能出现错误：
```cpp
class Bitmap{};
class Widget{
    //...
private:
    Bitmap*pb;
}
Widget& Widget::operator= (const Widget& rhs){
    delete pb;
    pb=new Bitmap(*rhs.pb);
    return *this;
}
```
考虑上面自我赋值的情况，pb被删去后已经为空，仍然被赋值给自己，导致原来的数据丢失。如果我们在赋值前加入一个自我判断if(this==&rhs)确实可以解决自我赋值安全性的问题，但是如果new Bitmap出现异常，那最终还是导致pb指向一块奇奇怪怪的内存。因此，应该如此做：
```cpp
Widget& Widget::operator= (const Widget& rhs){
    Bitmap*temp=pb;    //记住原先的pb
    pb=new Bitmap(*rhs.pb);//令pb指向rhs的pb副本
    delete temp;    //删去pb
    return *this;
}
```
对于上述的代码，新建副本，删去暂时指针都是由程序员手工排序的，但我们也可以使用封装好的copy and swap技术去保证异常安全性，即：
```cpp
Widget& Widget::operator= (const Widget& rhs){
   Widget temp(rhs);//创建副本
   swap(temp);//把*this和副本对换
    return *this;
}
//但如果参数是by value，那就不需要副本了，在传参时已经创造了一份副本
Widget& Widget::operator= (Widget rhs){
    swap(rhs);//把*this和副本对换
    return *this;
}//把copying动作从本体移至函数参数构造阶段
```
那么通过上述方法，也就能确保**任何函数操作一个以上对象，而对象是同一个对象时仍然正确**

#### 12.赋值对象时勿忘其成分
当编写copying函数时确保（1）复制新派生的成员的值（2）调用其继承基类的copying函数（原因很简单，如果我们自己编写了copying函数将屏蔽编译器的copying函数，那么对于那些忽略的成分编译器只会初始化，而不会copy）
不该用copy赋值运算符操作调用copy构造函数，反之也成立，正确做法应该是使用一个private的init函数供两者去完成相同的操作

