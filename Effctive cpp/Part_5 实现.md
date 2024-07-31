#### 26.尽可能延长变量定义的时间
```cpp
//考虑下面代码
std::string encryptPassword(const std::string&password){//密码加密函数
    using namespace std;
    string encrypted;
    if(password.length()<MinPasswordLength){//判断长度是否符合要求
        throw logic_error("Password is too short!");
    }
    //...  
    return encrypted;
}```
对于上述函数，它在string encrypted处便调用构造函数，如果密码的长度不符合要求，函数异常退出，那么这个string对象的构造和析构函数就白白调用了一次，降低了效率
```cpp
std::string encryptPassword(const std::string&password){
    if(password.length()<MinPasswordLength){
        throw logic_error("Password is too short!");
    }
    std::string encrypted;
    encrypte=password;
    encrypt(encrypted);
    //...  
    return encrypted;
}
```
该函数先是调用一个default构造函数构造encrypted，而后才利用copying函数重新赋值，这也浪费了时间，降低了效率
```cpp
std::string encryptPassword(const std::string&password){
    if(password.length()<MinPasswordLength){
        throw logic_error("Password is too short!");
    }
    std::string encrypted(password);//利用copy构造函数构造
    encrypt(encrypted);
    //...  
    return encrypted;
}
```
由上可见，我们对定义的变量应该尽量延后到能使用到该变量且该变量被赋予初值参数为止。

那么对于循环中的自定义类型该怎么办：
```cpp
//A：
Widget w;
for(int i=0;i<n;i++){
    w=;//...
}
//B：
for(int i=0;i<n;i++){
    Widget w=;//...
}
```
对于A和B应该使用哪种形式呢？
A：一次构造+一次析构+n次赋值
B：n次构造+n次析构
除非赋值比构造和析构成本低或追求极致效率，否则B是不二选择

#### 27.尽量少做转型动作
（1）转型会减少代码的可移植性，例如在一个derived-class中，创建一个base-class指针来指向derived-class的指针，如果了解c++的多态性，我们可以知道这两个指针的值是不一样的，它和编译器给定的程序的布局与结构有关，当代码进行移植，会发现这并不适用于别的编译器。
（2）会导致某些似是而非的结果：
```cpp
class Window{
    public:
    virtual void onresize(){//...
    }
};
class SpecialWindow{
    public:
    virtual void onresize(){
        static_cast<Window>(*this).onresize();
        //....
    }
}
```
对于上述函数的onresize函数是不可行的， 其中的specialwindow类的onresize函数调用的*this的基类成分的一个副本，如果在onresize上需要修改某些成分，那么special的原意也是修改某些成分，在它调用的时候，由 *this 创建出来的一个基类部分的副本去调用onresize函数，这时候修改的却是副件，onresize并没有得到想要的结果，只修改了derive部分而未修改base部分
（3）dynamic的调用会降低效率，特别是对那些多重继承的类来说
（4）新式转型比旧式转型更安全，更专业
（5）如果转型必要则隐藏它于某函数后

#### 27.避免返回对象内部成分的handles
返回handles(引用、迭代器)会降低对象的封装性，用户可以通过该引用修改private对象：
```cpp
class Point{
public:
    Point(int x,int y){}
    //...
    void setx(int x);
    void sety(int y);
}
struct RectData{
    Point ulhc;
    Point rlhc;
}
class Rectangle{
public:
//...
    Point& upperLeft()const {return pData->ulhc; }
    Point& lowerRight()const {return pData->rlhc;}
private:
    std::tr1::shared_ptr<RectData> pData;
}

Point coord1(0,0);
Point coord2(100,100);
const Rectangle rec(coord1,coord2);
rec.upperLeft().setx(10);//修改了原本不能修改的x，const是符合的，因为指向该对象并没有变，而是对象的内容变了。
```

返回引用还会造成返回的handles悬空、虚吊现象：
```cpp
class GUIobect{};
const Rectangle boundingBox(const GUIobject& obj);

GUIobject * pgo;//创建指向指向GUIobject
//。。。
const Point* pUpperLeft=&(boundingBox(*pgo).upperLeft());
//利用函数取得指针指向外框左上点
//boundingBox(*pgo)产生的是一个temp对象，upperLeft()作用于temp对象上，当函数调用完毕，temp对象就消亡了，这时获得的指针就是悬空的、虚吊的，指向了未知内存
```
#### 29.为异常安全而努力是值得的
异常安全函数应该提供的保证是以下之一：
（1）基本承诺：异常抛出后程序内的事务仍能保持有效，即数据和对象都不会被破坏（虚吊或未释放）
（2）强烈保证：异常抛出后程序的状态和调用函数前一致或恢复到调用前
（3）不抛掷保证：函数在执行期间总能完成任务并且不会抛出异常
同时，在异常被抛出时，异常安全函数会保证：
（1）不泄露资源，无法delete heap区对象，无法释放stack区参数
（2）不允许数据败坏，产生虚吊指针，数据与实义不符
```cpp
class PrettyMenu{//一个带有背景图案的GUI菜单
public:
    void changeBackground(std::istraem& imgsrc);
private:
    Mutex mutex;//用于多线程控制的互斥变量
    Image* bgImage;//目前背景
    int imageChanges;//改变次数
}
void PrettyMenu::changeBackground(std::istream& imgsrc){
    lock(&mutex);
    delete bgImage;
    ++imageChanges;
    bgImage=new Image(imgsrc);
    unlock(&mutex);    
}
```
上述代码可能发生以下错误，在lock(&mutex)后出现异常，那就导致mutex永远处于上锁状态，当new出现错误，bgImage的数据就指向一个被删除的内存，imageChange也增加了不该增加的1
异常安全函数会：
```cpp
class PrettyMenu{
    std::tr1::shared_ptr<Image>bgImage;//使用智能指针管理
    //...
}
void PrettyMenu::changeBackground(std::istream& imgsrc){
    Lock m1(&mutex);//使用类管理对象
    bgImage.reset(new Image(imgsrc));//使用安全的函数置换指针
    ++imageChanges;
}//由于如果imgsrc构造失败就会导致输入流的读取记号被移走，因此该函数只能保证基本异常安全
```
上述函数只能完成基本保证，强烈保证往往使用copy-and-write来实现，把原件复制得到copy件再对copy件进行操作，没出问题再交换，当然这将浪费一定的空间和效率，但为了强烈保证我们不得不牺牲部分东西
最后，异常安全函数的安全保证和函数内最弱的函数安全保证相关，因为函数所调用的函数级别越弱，也会影响到调用的函数。

#### 30.了解inline的里里外外
inline函数的底层原理：把对函数的调用过程直接以函数本地调换
如果调用次数少，函数本体特别小，那么这样的inline将可以提高效率，因为它跳过了传入参数跳到函数地址再返回函数的过程，但对于函数代码较多或频繁调用，这会大大增加目标码的数量
inline函数只是对编译器的申请，而不是强制命令，它可以隐式提出，也可以显示提出
```cpp
class Person {//隐式声明，于类内的成员函数和friend函数
public:
	//...
	int age()const { return theAge; }
	//一个隐喻的inline申请，age被定义于class定义式内。
	//...
private:
	int theAge;
};
template<class T>
inline const T& std::max(const T& a, const T& b)//明确声明使用inline
{
	return a < b ? b : a;
}
```
内联函数的主要目的是减少函数调用的开销。编译器可以根据函数的复杂性和调用次数等实际因素来决定是否将一个函数内联。编译器并不是总是将一个函数内联。如果一个函数非常复杂或者调用次数不多，编译器可能会忽略内联请求。
<br/>
template与inline无关，template一旦被使用，编译器为了将它具现化，需要知道它长什么样子，但即使有时候它带有inline声明，他也不一定是一个inline函数，编译器可能还是会生成函数本地

#### 31.将文件间的编译依存关系降到最低
文件编译依存关系的产生：接口与实现的未分离
```cpp
class Person{ 
public: 
    Person(const std::string& name, const Date& birthday, const Address& addr); 
    std::string name() const; 
    std::string birthDate() const; 
    std::string address() const; 
    ... 
private: 
    std::string theName;        //实现细目 
    Date theBirthDate;          //实现细目 
    Address theAddress;         //实现细目 
};
```
上述类一经修改就会发现设计person的所有文件都需要重新编译，为此我们分离声明和实现：
```cpp
#include <string>
#include "date.h"
#include "address.h"
namespace std {
	class string;    // 前置声明
    //不正确，string不是类
}
class Date;          // 前置声明
class Address;       // 前置声明
//对于前置声明，编译器需要知道大小
class Person {
public:
	Person(const std::string& name, const Date& birthday, const Address& addr);
	std::string name() const;
	std::string birthDate() const;
	std::string address() const;
	//...
};```
利用pImpl类解决这个问题：
```cpp
#include <string>
#include <memory>    // 此乃为了tr1::shared_ptr而含入
class PersonImpl;    // Person实现类的前置声明
class Date;          // Person接口用到的classes的前置声明 
class Address;       
class Person {
public:
	Person(const std::string$ name, const Date& birthday, const Address& addr);
	std::string name() const;
	std::string birthDate() const;
	std::string address() const;
	...
private:
	std::tr1::shared_ptr<PersonImpl> pImpl;  // 指针，指向实现物：std::tr1::shared_ptr 见条款13
};
```
因此解决依赖问题的关键在于用声明依赖性替换定义依赖性（声明实现分离），尽量：
>用对象指针或引用替换对象
>用class声明式替换定义式
>为声明式和定义式提供不同的头文件
通常解决方法是使用handle classes：其一便是上述提到的pImpl类，把对象放于指针，其二便是创建interface class：
interface class：

```cpp
class Person{ 
public: 
    virtual Person(); 
    virtual std::string name() const=0; 
    virtual std::string birthDate() const=0; 
    virtual std::string address() const=0; //使用纯虚函数设置基类 
    static std::tr1::shared_ptr<Person>  create(const std::string& name, const Date& birthday, const Address& addr); 
};
//客户使用他们像这样
std::string name; 
Date dateBirth; 
Address address; 
std::tr1::shared_ptr<Person> pp(Person::create(name, dateBirth, address)); 
... 
std::cout << pp->name() 
            << "was born on " 
            << PP->birthDate() 
            << " and now lives at " 
            << pp->address(); 
//继承类
class RealPerson : public Person{ 
public: 
    RealPerson(const std::string& name, const Date& birthday, const Address& addr) 
    : theName(name), theBirthDate(birthday), theAddress(addr) 
    {} 
    virtual ~RealPerson(){} 

    std::string name() const; 
    std::string birthDate() const; 
    std::string address() const; 

private: 
    std::string theName; 
    Date theBirthDate; 
    Address theAddress; 
};
```
两种做法都有一定的开销，我们需要在文件依赖性和效率上权衡这一点：
handle class：通过pImpl对象访问数据，会受格外指针，额外动态分配，多层构造析构函数的开销。
Interface class：
virtual函数带来的额外跳转指令以及对象内的vptr（virtual）。
