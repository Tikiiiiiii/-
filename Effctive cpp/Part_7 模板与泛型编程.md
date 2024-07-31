#### 41.了解隐式接口和编译期多态
在面向对象编程设计中，显式接口和运行时多态是很常见的。
```cpp
class Widget {//一个无实义的类
public:
    Widget();
    virtual ~Widget();
    virtual std::size_t size() const;
    virtual void normalize();
    void swap(Widget& other); //见条款25
};

void doProcessing(Widget& w)
{
    if (w.size() > 10 && w != someNastyWidget) {
        Widget temp(w);
        w.normalize();
        temp.swap(w);
    }
} 
```
对于w，它的类型是确定的，对于类中所有的接口在代码中都是明确可见的，因此称为**显式接口（：类型确定，代码可见）**
**运行时多态**：对于某些virtual函数，*它的调用取决于运行时指针指向对象的类型*，也就是运行期根据运行类型确定的（条款37）

在泛型编程世界则与OOP不同，显式接口和运行多态仍然存在，但增加了隐式接口和编译期多态
```cpp
template<typename T>
void doProcessing(T& w)
{
    if (w.size() > 10 && w != someNastyWidget) {
        T temp(w);
        temp.normalize();
        temp.swap(w);
    }
}
```
对于w，它的类型并未确定，并且要求它在doProcessing支持size，normalize，swap构造函数和赋值构造函数，比较运算符等操作，然而这样一组操作并未确定，因此这是**隐式接口**
对于关于w的调用，可能使模板具现化（生成一个该类型的函数），*这个具体化过程发生于编译期。用不同的类型调用会导致不同的函数*，这就是**编译器多态**

显式接口：由函数的签名式（函数名称、参数类型、返回类型）构成。
隐式接口：其并不基于函数签名式，而是由有效表达式组成。（隐式接口还需要满足一定约束）
#### 42.了解typename的双重意义
```cpp
template<class T>
class Demo;
template<typename T>
class Demo;
```
在声明模板时，class和typename有什么区别？
当声明 template 类型参数时，class和typename的意义相同，但两者并不等价。
在template中：
*如果某个类型依赖于模板的参数T，那我们就称之为从属名称*，*如果从属名称在 class 内呈嵌套状，我们就称之为嵌套从属名称*
*如果某个类型不依赖于任何 template 参数。我们便称之为非从属名称*
```cpp
//以下代码是无法通过编译器的
template<typename T>
void PrintContainer(const T& Container){
    if (Container.Size()>0) {
        T::const_iterator iter// T::const_iterator就是一个嵌套从属名称
        (Container.begin()); 
        ++iter;
        int value=*iter; //value不依赖模板参数，因此是非从属名称
        std::cout<<value<<"\n";
    }
}
```

嵌套从属名称可能导致解析（parsing）困难：
如果解析器在 template 中遭遇到一个嵌套从属名称，它便假设这个名称不是类型，而是变量
```cpp
template<typename C>
void print2nd(const C& container){
    C::const_iterator* x;
    //...
}
//假如T有一个 static 成员变量碰巧被命名为 const_iterator，或恰巧是个全局变量，那么上述的代码便不再是声明一个指针，而是一个乘法算式
```
解决方式：
**当想要在template使用嵌套从属类型名称，就在其前面加上typename关键字，但typename不能使用在 base class list （基类列）和 member initialization list（成员初值列）中**
```cpp
template<typename T>
class Derived:public Base<T>::Nested{//不可以用typename
public:
    explict Derived(int x):Base<T>::Nested(x){//不可以用typename
    typename Base<T>::Nested temp;
    //可以用typename
    //...
    }
//...
}
```

#### 43.学习处理模板化基类内的名称
在模板继承中存在一个问题，由于base class template可能被特化，而特化版本可能不提供和一般性template模板相同的接口。因此编译器往往拒绝在templated base class（模板基类）内寻找继承而来的名称
```cpp
class CompanyA{
public:
    void sendCleartext(const std::string& msg);
    void sendEncryted(const std::string& msg);
    };
class CompanyB{
public:
    void sendCleartext(const std::string& msg);
    void sendEncryted(const std::string& msg);
    };
//还有一些公司
class MsgInfo{……};//用来保存信息，以备将来产生信息
template<typename Company>
class MsgSender{
public:
    ……//构造析构等函数
    void sendClear(const MsgInfo& info)
{
    std::string msg;
    //根据info产生信息
    Company c;
    c.sendCleartext(msg);
}
    void sendSecart(const MsgInfo& info){……}
};
//记录日志类
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
    ……//析构构造等
    void SendClearMsg(const MsgInfo& info){
    //发送前的信息写到log
    sendClear(info);//调用base class函数，这段代码无法通过编译
    //传送后信息写到log
}
};
//特化类，只做加密通话：
class CompanyZ{
pubic:
    void sendEncryted(const std::sting& msg);
    ……
};
template<>
class MsgSender<CompanyZ>{
public:
    void sendSecret(const MsgInfo& infof){……}
    ……
}；//我们会发现这个模板对于上述的loggingMsgSender是冲突的，它没有sendclear，也就无法调用它，编译器为了防止特化版本接口的特殊性，拒绝在模板化基类中寻找继承的接口
```
解决方法：
（1）使用this->：
```cpp    
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
    ……//析构构造等
    void SendClearMsg(const MsgInfo& info){
    //发送前的信息写到log
    this->sendClear(info);//假设被继承
    //传送后信息写到log
}
};
```
（2）使用using声明式：
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
    ……//析构构造等
    uinsg MsgSender<Company>::sendClear;//告诉编译器，假设sendClear位于base class内
    void SendClearMsg(const MsgInfo& info){
    //发送前的信息写到log
    sendClear(info);//可以编译通过，假设sendClear将被继承
    //传送后信息写到log
}
};
```
上述情况与条款33不同，条款33是因为base class被derive class屏蔽，而这里是因为编译器不进入base class的作用域查找函数，using可以显式声明让编译器那么做
（3）明白指出被调用的函数位于base内：
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company>
{
public:
    ……//析构构造等
    void SendClearMsg(const MsgInfo& info){
    //...
    MsgSender<Company>::sendClear(info);//可以编译通过，假设sendClear将被继承
    //...
}
};
```
此方法不好，使用了明确资格修饰符（explicit qualification），这将会关闭virtual绑定行为

#### 44.将与参数无关的代码抽离template
templates 是节省时间和避免代码重复的奇方妙法，但也很容易使得代码膨胀（code bloat），我们通过：共性与变形分析（commonality and variability analysis）来避免代码膨胀（即把两份代码相同的部分提取出来形成一个函数，变化部分留下）
```cpp
template<typename T,size_t N>
class SquareMatrix {
public:
    void Invert() const{
        //...
    }
};
SquareMatrix<int,5>SquareMatrixFive;
SquareMatrixFive.Invert();
SquareMatrix<int,10>SquareMatrixTen;
SquareMatrixTen.Invert();
```
上述代码因为 template 参数不同，编译器会具现化两份函数，即使他们除了N处处相同
```cpp
template<typename T>
class SquareMatrixBase {
public:
    void Invert(std::size_t InSize) {
        //...
    }
};
template<typename T,size_t N>
class SquareMatrix:private SquareMatrixBase<T>{
public:
    using SquareMatrixBase<T>::Invert;  // 避免遮掩继承自 base 版的 Invert 函数
    void Invert(){
        this->Invert(N);
    }
};
```
所有T相同的共享这对应 class 内的 N 不同的 Invert()。
另一个问题产生了，derived class知道数据的位置，但base class的invert不知道数据在何处，derived class该如何联络base class？
方法一：给invert加入指针参数指向内存位置，但是需要derived class一次又一次的告诉 base class 需要操作的数据，这似乎不太好
方法二：在base类中加入指针指向数据，derive在构造时传入数据位置指针
```cpp
template <typename T>
class SquareMatrixBase {
public:
    SquareMatrixBase(std::size_t InN, T* InData): N(InN), Data(InData) {}
    void Invert() const {
        std::cout << N << "\n";
    }
private:
    std::size_t N;
    T* Data;
};
template <typename T, size_t N>
class SquareMatrix : private SquareMatrixBase<T> {
public:
    SquareMatrix()
        : SquareMatrixBase<T>(N, Data) {
        Data = new T[N * N];
    }
    void Invert() {
        SquareMatrixBase<T>::Invert();
    }
private:
    T* Data;
};
```
但注意，之前直接使用模板参数的 Invert 函数，有可能产出比上面共享版本更好的代码，因为模板尺寸是一个编译器常量，因此可以像常量那样被直接生成到指令中成为立即操作数，达到更优化。
另外一个角度来看，拥有共享的 Invert 函数，可减少执行文件的大小，降低了所需的内存，也提高了高速缓存命中率>
哪一个影响占主要地位？需要进行实际的平台测试和观察面对代表性数据的行为。
类型参数（type parameters）也会导致代码膨胀: vector < int > 和 vector< long > 有着相同的代码实现,某些链接器（linkers）会合并完全相同的函数实现码，但有些不会，后者意味着某些 templates 将具现化为 int 和 long 两个版本，从而造成代码膨胀

#### 45.运用成员函数模板接受所有兼容类型
指针的一大特性是支持隐式转换：
```cpp
class Top {};
class Middle :public Top {};
class Bottom :public Middle {};
Top* pt1 = new Middle; //将Miffle*转换为Top*
Top* pt2 = new Bottom; //将Bottom*转换为Top*
const Top* pct2 = pt1; //将Top*转换为const Top*
```
我们希望自行设置的指针也能做到如此，即
```cpp
//自己设计的智能指针类
template<typename T>
class SmartPtr{
public:
    explicit SmartPtr(T* realPtr);
};
//下面是我们希望能完成的，但是还没有实现
SmartPtr<Top> pt1 = SmartPtr<Middle>(new Middle);  
SmartPtr<Top> pt2 = SmartPtr<Bottom>(new Bottom);
SmartPtr<const Top> pct2 = pt1;
return 0;
```
为了避免每有一个继承类就要写一个构造函数，我们使用member function templates：
```cpp
template<typename T>
class SmartPtr 
{
public:
    //拷贝构造函数，是一个成员函数模板
    typename<typename U> SmartPtr(const SmartPtr<U>& other);
    //不能带有explicit
};
```
我们称之为泛化copy构造函数，但它并未进行约束，即可以从top转型为bottom指针，可以从int转型为bottom指针
```cpp
//可以为自己的智能指针类提供一个类似于shared_ptr的get()成员函数，这个函数返回智能指针锁封装的那个原始指针
template<typename T>
class SmartPtr{
public:
    typename<typename U> SmartPtr(const SmartPtr<U>& other):heldPtr(other.get()){}
    T* get()const {
        return heldPtr;
    }
private:
    T* heldPtr;
};
```
另外，一个类如果没有提供构造函数、拷贝构造函数、拷贝赋值运算符，那么编译器会自动为类提供合成/默认的版本，这一规则同样适用于模板类，如果我们想控制模板类的copying行为，我们必须同时声明泛化copy构造函数和普通copy构造函数。对于赋值操作也是同理。

#### 46.需要类型转换时请为模板定义非成员函数
```cpp
template <typename T>
class Rational {
public:
    Rational(const T& numerator = 0, const T& denominator = 1);
    const T numerator() const;
    const T denominator() const;
    //...
};
template<typename T>
const Rational<T> operator*(const Rational<T>& lhs,const Rational<T>& rhs){/*...*/ }
Rational<int> oneHalf(1, 2);
Rational<int> result = oneHalf * 2;//错误，无法通过编译
```
调用函数前必须知道函数本体，但为了知道函数本地，必须先根据模板推导参数类型但template实参推导过程中不将隐式转换考虑其中，因此上述代码无法通过编译，理由是编译器无法知道调用的函数
```cpp
template <typename T>
class Rational {
public:
    //...
    //声明operator*函数
    friend const Rational<T> operator* (const Rational<T>& lhs, const Rational<T>& rhs);
};
//定义operator*函数
template<typename T>
const Rational<T> operator*(const Rational<T>& lhs,const Rational<T>& rhs){/*...*/ }
```
把operator*声明为template的模板函数，当Rational< int >的对象被声明时，对应的operator *的Rational< int >版本就被具现化出来了，这时调用的是一个已知函数，编译器会自动完成隐式转换
但上述代码无法连接，连接器无法定位它的位置
一种方法是把代码写入类中
```cpp
template <typename T>
class Rational {
public:
    //...
    friend const Rational<T> operator* (const Rational<T>& lhs, const Rational<T>& rhs){
    return Rational(lhs.numerator() * rhs.numberator(), lhs.denominator() * rhs.denominator());
    }
    //...
};
```
这就解决了连接器的问题

当我们定义class template时，如果它提供的函数需要支持参数的隐式类型转换，请写为类模板内部的friend函数。

#### 47.请使用萃取类表现类型信息
当我们在程序中需要取得类型的某些信息时，trait能保证我们我们在编译期就能获取。
```cpp
//设计一个用于计算stl迭代器的移动距离的stl工具类
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d);  //将迭代器向前移动d个单位，d<0则向后移动。
/*STL迭代器分类的专属的卷标结构（tag struct)*/
struct input_iterator_tag{}; 
struct output_iterator_tag{}; 
struct forward_iterator_tag : public input_iterator_tag{}; 
struct bidirectional_iterator_tag : public forward_iterator_tag{}; 
struct random_iterator_tag : public bidirectional_iterator_tag{};
template<typename IterT, typename DistT>

//我们希望的advance函数
void advance(IterT& iter, DistT d) {
	if (iter is a random_access_iterator) iter += d;  //针对random_access迭代器使用迭代器算术运算
	else {
		if (d >= 0) {
			while (d--) ++iter;
		}else {
			while (d++) --iter;
		}
	}
}

//我们设计的traits必须对内置类型和用户自定义类型表现得一样好
template<typename IterT>
struct iterator_traits {
  typedef typename IterT::iterator_category iterator_category;
  ...
};
//iterator_traits通常以结构来定义
//它的工作方式：对于每一个 IterT 类型，在struct（结构体）iterator_traits<IterT> 中声明一个名为  iterator_category 的 typedef。这个 typedef 用来确定 IterT 的 iterator category（迭代器种类）

//但该traits对内置类型指针并没有效，即int*这样的类型是无法进行trait的
//为了支持这种特殊的指针迭代器，我们可以利用模板的全特化版本来解决这一问题
template<typename IterT>
struct iterator_traits<TierT*> {
	typedef random_access_iterator_tag iterator_category;
	...
};

//有了traits类，我们可以把上述伪码转化为真正的代码：
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d){
  if (typeid(typename std::iterator_traits<IterT>::iterator_category) ==
     typeid(std::random_access_iterator_tag))
  ...
}
//IterT 的类型在编译期间是已知的，所以iterator_traits<IterT>::iterator_category 可以在编译期间被确定。但是 if 语句还是要到运行时才能被求值。为什么要到运行时才做我们在编译期间就能做的事情呢？它浪费了时间，而且使我们的执行码膨胀。

template <typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d, random_access_iterator_tag) {
	iter += d;
}
template <typename IterT, typename DistT>
void doAdcance(IterT& iter, DistT d, bidirectionl_iterator_tag) {
	if(d >= 0) {
		while (d--) ++iter;
	}else {
		while (d++) --iter;
	}
}
template <typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d, input_iterator_tag) {
	if (d < 0) {
		throw out_of_range("Negative distance");
	}
	while (d--) ++iter;
}
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d){
  doAdvance(iter,d,typename std::iterator_traits<IterT>::iterator_category()   
  );
}   
//我们所做的是重载函数，编译器便会运用重载解析机制在编译期匹配最佳函数解决我们的问题
```

总结traits的方法：
（1）提供重载函数或函数模板，差异在于traits的参数（上述的doadvance）
（2）提供控制函数或函数模板，调用函数并传递traits class信息（上述的advance）

#### 48.认识template元编程
模板元编程（template metaprogramming）TMP：编写 template C++ 程序并执行于编译期的过程
TMP的伟大处在于将很多工作从运行期转移到编译期：
（1）某些错误原本通常在运行期才能检测到，现在可在编译器找出来。
（2）使用 TMP 的 C++ 程序可能在每一方面都更高效：如较小可执行文件，较短运行期，较少内存需求

```cpp
template <typename IteratorType>
void Move(IteratorType& Iterator, int Distance) {
	// 使用类型信息
	if (typeid(IteratorTraits<IteratorType>::IteratorTag) == typeid(RandomAccessIteratorTag)) {   
	Iterator += Distance;
	}
	else {
	    if (Distance >= 0) {
	            while (Distance--)++Iterator;
	    }
	    else {
	        while (Distance++)--Iterator;
	    }
	}
}//对于这段代码，类型测试发生于运行期，也就是会出现在目标码中，但我们使用重载函数的形式匹配目标函数将会使这一过程在编译期间匹配从而减少目标码
```

TMP 已被证明是一个图灵完备（Turing-complete）机器，这意味着它可以计算任何事物，使用 TMP 你可以声明变量，执行循环，编写及调用函数…
TMP的循环：
TMP循环不涉及递归函数调用而是涉及递归模板具现化
```cpp
//计算阶乘的模板
template <unsigned N>
struct Factorial {
    static const int Value = N * Factorial<N - 1>::Value;
};
template <>
struct Factorial<0> {
    static const int Value = 1;
};
void Test() {
    std::cout << Factorial<5>::Value << "\n";//120
    std::cout << Factorial<10>::Value << "\n";//3628800
}
//这个TMP的示范就像helloworld引领我们进入c++一样
```
TMP还可以用于：
（1）确保度量单位的正确：
（2）优化矩阵运算
（3）生成客户定制的设计模式实现品
TMP将是一个很好的方向。
