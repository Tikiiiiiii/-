#### 32.确定public继承是is-a关系
is-a并非是存在classes之间的唯一关系，还有has-a is-implement-in-terms-of两种。在public继承中，base类适用的函数对derive类也一定适用，因为每一个derive类是一个base类，我们必须确认这点，谨慎选择继承关系。

#### 33.避免遮掩继承而来的名称
对于一个同名函数，局部变量会遮掩全局变量，derive类也会遮掩base类的同名函数
使用using可以恢复同名函数的使用，也可以使用转交函数：
```cpp
class  Base//using声明
{
private :
     int  x;
public :
     virtual  void  mf1() = 0;
     virtual  void  mf1( int );
     virtual  void  mf2();
     void  mf3();
     void  mf3( double );
     ...
};
class  Derived :  public  Base
{
public :
     using  Base::mf1; //让Base class内名为mf1和mf3的所有东西在Derived作用域都可见
     using  Base::mf3;
     
     virtual  void  mf1();
     void  mf3();
     void  mf4();
     ...
};
Derived d;
int  x;
d.mf1(); //OK
d.mf1(x); //OK，调用Base::mf1
d.md2(); //OK
d.mf3(); //OK
d.mf3(x); //OK，调用Base::mf3
//转交函数：
class  Base
{
public :
     virtual  void  mf1() = 0;
     virtual  void  mf1( int );
     ...
};
class  Derived :  private  Base
{
public :
     virtual  void  mf1() //转交函数暗自为inline
     {    Base::mf1(); }
     ...
};
Derived d;
int  x;
d.mf1(); //OK,调用的是Derived::mf1
d.mf1(x); //Error！Base::mf1()被遮掩
```

#### 34.区分接口继承和实现继承
```cpp
class Shape {
public:
	virtual void draw() const = 0;
	virtual void error(const std::string& msg);
	int objectID() const;
	...
};
class Rectangle: public Shape { ... };
class Ellipse: public Shape { ... };
```
对于virtual void draw()const=0，这是一个纯虚函数，它的目的是让派生类只继承函数接口，他要求用户必须提供实现
对于virtual void error()，这是一个虚函数，它的目的是让派生类继承接口，并在缺省情况下继承实现，这有一大缺点就是把重写函数实现的任务交给了客户，如果客户忘记了覆写这一内容，有可能破坏public的is-a关系
解决方法：将默认实现分离成单独函数
```cpp
class Airplane {
public:
    virtual void fly(const Airport& destination) = 0;
    ...
protected:
    void defaultFly(const Airport& destination);
};
void Airplane::defaultFly(const Airport& destination) {
	//飞机飞往指定的目的地(默认行为)
}
//这样就不会出现破坏is-a关系的情况了，用户必须覆写实现
```
对于int objectID()const这个普通成员函数，目的在于让派生类继承一个函数接口，并且有一个强制的实现，不允许继承类有任何修改
class设计会犯的错误：
（1）把base类设计为全non-virtual函数，non-virtual析构函数会带来问题的同时使得派生类没有多余空间进行特化工作。
（2）把base类设计为全virtual函数，有时是对的，但有时不应该让继承类享有不该有的权利，不变性凌驾于特异性的函数应该及时注明。

#### 35.考虑virtual函数以外的选择
由非虚接口(Non-Virtual Interface)手法实现模板方法(Template Method)模式：
```cpp
class GameCharacter {
public:
    //派生类不应该重新定义它
    int healthValue()const {
        //可以进行锁定互斥器、制造运转日志记录项、验证class约束条件、验证函数先决条件等等
        int retVal = doHealthValue(); 
        //可以进行互斥器解锁、验证函数的事后条件、再次验证class约束条件等等
        return retVal;
    }
private:
    //返回人物的健康指数，派生类可以重新定义它
    virtual int doHealthValue()const { 
    	...
    }
};
```
NVI用法允许派生类重新定义一个虚函数，这使他们可以对如何实现一个功能进行控制，但是基类保有何时调用这个函数的权利。我们将基类的这个非虚函数称为虚函数的外覆器（wrapper）。

藉由Function Pointers实现Strategy模式：
在类对象的构造函数中加入函数指针，指向*全局*函数，通过该指针实现同一类但不同实体的多态性
```cpp
class GameCharacter;	// 前置声明
int defaultHealthCala(const GameCharacter& gc);//默认，计算健康指数
 
class GameCharacter {
public:
    //函数指针别名
    typedef int(*HealthCalcFunc)(const GameCharacter& gc);
    //构造函数
    explicit GameCharacter(HealthCalaFunc hcf = defaultHealthCalc) 
        :healthFunc(hcf) {}    
    int healthValue() {
        //通过函数指针调用函数
        return healthFunc(*this);
    }
private:
	HealthCalcFunc healthFunc; //函数指针
};
//同一个人物类型之间可以有不同的健康计算函数：
class GameCharacter {
public:
    typedef int(*HealthCalcFunc)(const GameCharacter& gc);
    explicit GameCharacter(HealthCalaFunc hcf = defaultHealthCalc) 
        :healthFunc(hcf) {}    
    int healthValue() {
        return healthFunc(*this);
    }
    ...
private:
	HealthCalcFunc healthFunc;
};
class EvilBadGuy :public GameCharacter {
    explicit EvilBadGuy(HealthCalaFunc hcf = defaultHealthCalc)
        :GameCharacter(hcf) {}
	...
};
int loseHealthQuickly(const GameCharacter&);//健康指数计算函数1
int loseHealthSlowly(const GameCharacter&);//健康指数计算函数2
EvilBadGuy ebg1(loseHealthQuickly);//相同类型的人物搭配
EvilBadGuy ebg2(loseHealthSlowly);//不同的健康计算方式
//某已知人物的健康函数可在运行期变更。如：GameCharacter可提供一个成员函数setHealthCalculator，来替换当前的健康指数计算函数。
```
上述方法的缺点是降低封装性，但个健康计算函数涉及到non-public的数据，那这个策略就需要设定friend或public接口甚至不可行。

藉由tr1::function完成Strategy模式：
对于上述的方法，它太过于苛刻和死板，例如返回值只能是int，而不能是可隐式转化为int的类型，要求一定是一个函数指针，而不能是一个类似函数的东西如成员函数成员函数指针等等。
```cpp
 class GameCharacter;//forward declaration
int defaultHealthCalc(const GameCharacter& gc);
class GameCharacter{
public:
    typedef std::tr1::function<int (const GameCharacter&)> HealthCalcFunc;
    explicit GameCharacter(HealthCalFunc hcf=defaultHealthCalc):healthFunc(hcf){}
    int healthValue() const{ return healthFunc(*this); } 
private:
    HealthCalcFunc healthFunc;
};
//tr1::function已经帮我们把这些情况都处理好了，我们可以这样使用它
short calcHealth(const GameCharacter&);//健康计算函数，返回类型不是int
struct HealthCalculator{//为健康计算设计的函数对象
     int operator()(const GameCharacter&)const{}
};
class GameLevel{
public:
    float health(const GameCharacter&) const;//成员函数计算健康，返回不是int
};
class EvilBadGuy:public GameCharacter{//和前面一样
    };
class EyeCandyCharacter: public GameCharacter{//另一个人物，假设其构造函数和EvilBadGuy相同
};
//人物1，使用某个函数计算健康指数
EvilBadGuy edg1(calcHealth);
//人物2，使用函数对象计算健康指数
EyeCandyCharacter ecc1(HealthCalculator());
GameLevel currentLevel；
//人物3，使用某个成员函数计算健康指数
EvilBadGuy ebg2(std::tr1::bind(&GameLevel::health,currentLevel，_1));
```
对于ebg2计算，我们知道成员函数会有一个隐式参数*this指向自己的类对象，而我们的函数只接受一个参数，我们需要把该成员函数转化为单一参数，而我们利用另一种方式，把gamelevel绑定为函数的对象，利用tr1::bind函数去完成这件事

古典的strategy模式：
即把要重载的函数剥离出来成为一个新的抽象体系令原类中加入一个指向该类的指针，指向的抽象体系利用多态性实现不同类别的函数
```cpp
class GameCharacter;//forward declaration
class HealthCalcFunc{
public:
    virtual int calc(const GameCharacter& gc) const{}
};
HealthCalcFunc defaultHealthCalc;
class GameCharacter{
public:
explicit GameCharacter(HealthCalcFunc* phcf=&defaultHealthCalc):pHealthCalc(phcf){}
int healthValue() const{ return pHealthClac->calc(*this); }
private:
HealthCalcFunc* pHealthCalc;
};
```

#### 36.绝不重新定义继承而来的non-virtual函数
首先，在继承类中会屏蔽基类的同名函数，这不符合public继承的is-a关系
其次，base类中的non-virtual意味着必须被继承接口和实现，而derived类会根据指向他指针的类别来选择函数执行
```cpp
class B{
public:
    void mf();
};
class D:public B{
public:
    void mf();  
};
D x;
x.mf();//调用的是D::mf，B::mf被屏蔽了
B *pB = &x;
pB -> mf();//经由该指针调用B::mf
D *pD = &x;
pD -> mf();//经由该指针调用D::mf
```

#### 37.绝不重新定义继承而来的缺省参数值
non-virtual函数是绝对不能重新定义的（条款36），对于继承而来的带有缺省参数值的virtual函数是绝不能重新定义的，理由是它的缺省参数是静态绑定的，而virtual函数是动态绑定的
对象的静态类型就是它在程序中被声明时所采用的类型 ，对象的动态类型指的是目前所指对象的类型，也就是说动态类型可以表现出一个对象将会有什么行为
```cpp
// 一个用以描述几何形状的calss
class Shape {
public:
	enum ShapeColor { Red, Green, Blue };
	//所有形状都必须提供一个函数，用来绘出自己
	virtual void draw(ShapeColor color = Red) const = 0;
	...
};

class Rectangle: public Shape {
public:
	virtual void draw(ShapeColor color = Green) const;
	...
};
class Circle: public Shape {
public:
	virtual void draw(ShapeColor color) const;
	...
};
Shape *ps;         
Shape *pr = new Rectangle;  
ps=pr;
pr->draw();
```
pr的动态类型是Rectangle*，所以调用的虚函数是Rectangle的函数，然而因为pr的静态类型是Shape*,默认参数值来自Shape类，参数值是red，与Rectangle的默认参数是不符的，这并不是我们想要的

#### 38.Model 'has-a' and 'is implemented-in-terms-of' through composition.
复合是类型之间的关系，当某个类型内含其余对象便是复合关系
```cpp
class Address { ... };
class PhoneNumber { ... };
class Person 
{
public:
	...
private:
	std::string name;
	Addresss address;
	PhoneNumber voiceNumber;
	PhoneNumber faxNumber;
};//preson就是一个复合
```
复合的意义和public完全不同。在应用域上，他们体现的是has-a关系，在实现域上则表现is-implemented-in-terms-of。
考虑set的实现，set通常用平衡查找树来实现，它保证了效率但消耗了过多的空间，如果我们的空间比时间紧张，那我们需要自己实现一个set，我们想到set可以由linked list实现，如果我们用public实现：
```cpp
template <typename T>
class Set: public std::list<T> {};     
```
但是这违反了public继承的is-a关系，正确的方式是意识到一个Set对象可以被“implemented in terms of”一个list对象
```cpp
template<class T>                           
class Set {
public:               
	bool member(const T& item) const; 
	void insert(const T& item);            
	void remove(const T& item);         
	std::size_t size() const;                   
private:           
	std::list<T> rep;              
};   
template<typename T>
bool Set<T>::member(const T& item) const
{
	return std::find(rep.begin(), rep.end(), item) != rep.end();
}
template<typename T>
void Set<T>::insert(const T& item)
{
	if (!member(item)) rep.push_back(item);
}
template<typename T>
void Set<T>::remove(const T& item)
{
	typename std::list<T>::iterator it=// see Item 42 for info on
	std::find(rep.begin(), rep.end(), item); // “typename” here
	if (it != rep.end()) rep.erase(it);
}
template<typename T>
std::size_t Set<T>::size() const
{
	return rep.size();
}
```

#### 39.明智慎重选择private继承
(1)private继承不会把derive class自动隐式转化为base class
```cpp
class Person { ... };
class Student: private Person { ... };     // 改用private继承
	void eat(const Person& p);       // 任何人都可以吃
	void study(const Student& s); // 只有学生才在校学习
	Person p;// p是人
	Student s;       // s 是学生
	eat(p);    // 没问题，p是人
	eat(s);    // error! 无法隐式转换
```
（2）private继承而来的所有成员都是private

private的意义是什么？
private继承意味着is-implemented-in-terms-of。如果类D private继承自类B，用意是因为想利用类B中的一些已存在性质，而不是因为类B和类D之前有任何概念上的关系。private继承纯粹只是一种实现技术，且只继承实现，而不继承接口。

private和复合都能体现is-implemented-in-terms-of，该怎么选择？
尽量使用复合（composition），在必须使用private继承的时候才去使用它。何时是必须使用？（1）protected成员或者（和）虚函数被牵扯进来的时候。（2）因为空间原因而不得不使用private继承。（激进情况）

考虑以下情况：我们在设计一个Widgets类，我们决定较好了解如何使用Widgets（想知道Widget成员函数调用频繁度和一段时间调用比例的变化情况），我们修改Widget，让它记录每个成员函数的调用次数，为了达到这个目的，我们会创建一个定时器于是我们可以知道什么时候去收集这些统计信息。
```cpp
class Timer {
public:
	explicit Timer(int tickFrequency);
	virtual void onTick() const;    // 定时器每滴答一次，此函数就自动调用一次
};//这个类符合我们的需求，能按我们的频度，每滴答一次调用一次virtual的ontick函数，我们利用继承使用它

//public继承（错误做法××××××××）
// class Widget: public Timer {
// public:
// 	virtual void onTick() const; 
// }   
//该方法向用户暴露了ontick接口，很容易使用户调用错误，违反条款18并且不满足public的is-a关系，违反了条款32

//private继承：
class Widget: private Timer {
private:
	virtual void onTick() const;  
}//可行，但不如复合

//复合形式
class Widget{
private:
    class WidgetTimer :public Timer {
    public:
        virtual void onTick()const;
    };
    WidgetTimer timer;
};
```

为何复合形式好于private继承呢？
（1）防止Widget的派生类重写onTick()函数：
private继承方式：据条款35，即使无法调用virtual函数也要重新定义它，也就是说widget的派生类对于virtual的重写可能无法阻止
在复合模式下：WidgetTimer类是Widget内部的一个private成员，派生类永远无法访问，也就无法改写virtual函数
（2）可将Widget的编译依存性降至最低
private继承方式必须在Widget的头文件包含#include"Timer.h"
复合模式我们可以使用条款31的指针降低编译依存性。

激进情况：一个base类没有非静态数据成员、虚函数和虚基类
C++使得*独立*对象必须占用空间，所以sizeof(HoldsAnInt2)>sizeof(int)，但当一个对象非独立，即属于某个类的附属、基类，那他就不计算大小，sizeof(HoldsAnInt1)=sizeof(int)，这就是EBO（空白积累最优化机制）
```cpp
class Empty {};
class HoldsAnint1 :private Empty {
private:
    int x;
};
class HoldsAnint1 : {
private:
    int x;
    Empty e;
};
sizeof(HoldsAnint1); //4 
sizeof(HoldsAnint2); //5或8 
```
EBO仅在单一继承可行，对于这些“empty”类，大多都含有typedef，enums，static或non-virtual函数，常用于被继承。

#### 40.明智慎重使用多重继承
对于多重继承需要了解：
（1）基类同名函数带来的二义性
```cpp
class BorrowableItem {   // 图书馆允许你借某些东西
public:                    
	void checkOut();  // 离开进行检查 
};                                     
class ElectronicGadget {    
private:                             
	bool checkOut() const;//注意，此处的为private 
};
//多重继承
class MP3Player:
	public BorrowableItem,
	public ElectronicGadget 
{ ... };              
MP3Player mp;
mp.checkOut();//歧义！调用的是哪个checkOut
```
解决歧义的方法是明确指出调用的是那个基类方法：
>mp.BorrowableItem::checkOut();
(2)钻石继承问题：一个derive类的两个不同基类继承了另一个相同的基类，导致产生同样的副本和访问的二义性：
```cpp
class File { ... };
class InputFile: public File { ... };
class OutputFile: public File { ... };
class IOFile: public InputFile, public OutputFile
{ ... };
```
解决方法是使用virtual继承：
```cpp
class File { ... };
class InputFile: virtual public File { ... };
class OutputFile: virtual public File { ... };
class IOFile: public InputFile, public OutputFile
{ ... };
```
virtual继承的代价：
从正确性观点来看，public继承应该总是virtual的，借此防止重复继承成员数据，但这样的后果是耗费资源：
a.体积比正常大，访问速度变慢
b.继承自虚基类的类如果需要初始化，它们必须意识到虚基类的存在，无论这个虚基类离派生类有多远。
c.当一个派生类被添加到继承体系中的时候，它必须承担初始化虚基类的责任

对于virtual继承的建议：
a.非必要不使用虚继承
b.如果必须使用虚继承，尝试着不放置数据以避免这些类的初始化、赋值的古怪行为

案例：
我们的目的是通过IPerson接口塑膜CPerson这个类
现在我们有一个抽象接口类和一个工作类：
```cpp
class IPerson {//抽象接口类
public:
	virtual ~IPerson();
	virtual std::string name() const = 0;  //返回人的名称
	virtual std::string birthDate() const = 0;  //返回生日
};
//用来生产IPerson指针的工厂函数
std::tr1::shared_ptr<IPerson> makePerson(DatabaseID personIdentifier);
DatabaseID askUserForDatabaseID();
DatabaseID id(askUserForDatabaseID());
std::tr1::shared_ptr<IPerson> pp(makePerson(id)); // create an object
//工作类
class PersonInfo {//打印名字并获取数据库id的类
public:
	explicit PersonInfo(DatabaseID pid);
	virtual ~PersonInfo();
	virtual const char * theName() const;
	virtual const char * theBirthDate() const;
	...
private:
	virtual const char * valueDelimOpen() const; 
	virtual const char * valueDelimClose() const; 
	...
};
//其中的valueDelinmOpen和valueDelinmClose用作打印输出的开始和结束符号
//缺省的虚函数，派生类可以重写
const char* valueDelimOpen()const
{    return "[";}
const char* valueDelimClose()const
{    return "]";}
```
CPerson是我们的目标类，我们必须为继承自IPerson的纯虚函数提供一份实现代码，我们可以从头开始实现这个函数，但是利用现成的组件工作类来对其进行实现更好，这些现成的组件实现了大部分或者全部的必要功能，比如PersonInfo类，但PersonInfo类有个问题在于它的输入总以[开始 ]结束，我们的目标是不带有此种设计，只输出名字，因此我们需要覆写它
那也就是PersonInfo恰好有一些函数使得CPerson的实现更加容易，满足is-implemented-in-terms-of的关系，由于我们需要重新定义虚函数，因此只能是private继承。CPerson同样必须实现IPerson接口，这些接口为public继承所用。因此我们结合两种继承：公有继承于IPerson、私有继承于PersonInfo：
```cpp
class IPerson {
public:
    virtual ~IPerson();
    virtual std::string name()const = 0;
    virtual std::string birthDate()const = 0;
};
class DatabaseID {};
 
class PersonInfo {
public:
    explicit PersonInfo(DatabaseID pid);
    virtual ~PersonInfo();
    virtual const char* theName()const;
    virtual const char* theBirthDate()const;
private:
    virtual const char* valueDelimOpen()const;
    virtual const char* valueDelimClose()const;
};
class CPerson :public IPerson, private PersonInfo {
public:
    explicit CPerson(DatabaseID pid) :PersonInfo(pid) {}
 
    virtual std::string name()const = 0{
        return PersonInfo::theName();
    }
    virtual std::string birthDate()const = 0 {
        return PersonInfo::theBirthDate();
    }
private:
    virtual const char* valueDelimOpen()const {return "";};
    virtual const char* valueDelimClose()const {return "";};
};             
```
