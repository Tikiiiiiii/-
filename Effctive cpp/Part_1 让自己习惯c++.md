#### 1.视c++为一个语言联邦：
把c++看成由多种语言组（c语法基础、面向对象编程c++、泛型编程template、STL库）形成的联邦，每个语言组有每个语言组的守则，但c++可以视情况使用其中一种或几种混合使用。

#### 2.宁可const、enum、inline也不#define
#define 宏定义是由预处理器完成的不属于语言的一部分，也就是说它过程中的变量是不会被记录在符号表中的，但如果这个事情让编译器做就会记录相应的符合，应该尽可能满足这个原则

（1）编译器处理会比预处理器处理得到的目标码更小，且不会出现冗杂目标码
```cpp
#define effective 1.635
```
#define 宏定义仅仅只是把所有的effective替换成1.635，这就会导致程序中会出现1.635的多个副本，而利用关键字const定义变量可以让他的目标码固定
```cpp
const double effective=1.635
```

（2）define不重视作用域，无法创建一个类的专属不可变常量
```cpp
class myclass{
private: 
static const int data=5; 
int array [data]; 
}
```
上述的是声明式，定义式一般要在类的实现文件中，因为声明描述了如何分配内存，但不分配内存。程序可能将头文件包括在其他文件中。如果在头文件中进行初始化，将出现多个初始化语句副本，从而引发错误
有的旧版编译器不允许在声明时赋初值，那这时就可以把初值放在实现文件的定义式中，这时enum的作用就出来了，我们可以
```cpp
class myclass{
private: 
enum{data=5}; 
int array [data]; 
}
```
enum很类似define但又不想const，enum不允许像const那样被获取地址，另外enum也不会像define那样浪费空间

（3）宏定义函数导致意外错误
```cpp
#define x 2+2
cout<<x-x; //得到的是2+2-2+2=4而不是0
```
或许我们可以为其中的每个变量加上括号，但又会引出某些无法避免的错误
```cpp
#define max(a,b) f((a)>(b)?(a):(b))
```
给每个变量加上括号麻烦已经是一方面了，而另一方面：
```cpp
a=5，b=0
Max(++a, b)//将导致a自增两次
而max(++a, b+10)//只会自增一次
```
这显然不是想要的结果，因此我们可以利用inline和const来保证可预料行为和类型安全性
```cpp
 template<typename T>
inline void max(const T&a, const T&b){
f(a>b?a:b); 
}
```
#### 3.尽可能使用const
const在*左侧指被指物是常量
const在*右侧指指针是常量
```cpp
std::vector<int> vec;
const std::vector::iterator iter=vec.begin();//作用同T* const
*iter=10;//没问题，可以改变所指之物
iter++;//错误，不可以改变指针位置
std::vector::const_iterator citer=vec.begin();//作用同const T*
*iter=10;//错误
iter++;//正确
```
const可以和参数返回值函数本身关联，const与返回值关联可以避免用户错误
对于一个自定义用户类型Myclass，我们应该令它的函数返回一个常量，这是为了防止用户在使用该自定义类型比较时漏写=导致错误难以查到
```cpp
class Myclass{
    //...
    const Myclass& operator*();
}
Myclass a,b,c;
//if(a*b=c);//编译器报错，应该是a*b==c
```

const成员函数：
在对自定义类型加入const限定符，好处有（1）易理解，不修改值的成员函数（2）使操作const对象变得可能；接下来将逐步分析，好处便体现其中
对于一个成员函数，c++可以根据他的常量性constness不同而给予不同的重载
```cpp
class TextBlock {
    //...
public:
const char& operator[] (std::size t position) const//operator[] for( return text[position];) //const对象.
char& operator[] (std::size_t position) //operator[] for f return text [position];) //non-const 对象.
private:
std::string text;
};
TextBlock tb("Hel1o") ;
std::cout << tb[0]; //调用non-const TextBlock::operator[] 
const TextBlock ctb("World");
std::cout << ctb[0]; //调用 const TextBlock::operator[]
tb[0]='x';//正确
//ctb[0]='x';//错误，写一个const char&对象是不允许的，删去const后合法
```
学术界对于const的定义有两类，bitwise constness认为上述const成员函数不应该修改类成员的任何1bit，即对于上述删去const后仍不成立。但如果对于一个const成员函数返回值是引用类型，那么它就不符合constness，对于
```cpp
const TextBlock cctb("hello");
char*pc =&cctb[0];
*pc='J';//那么cctb就变成了Jello
```
而logical constness认为这是合理的，只要客户端侦测不出错误。但假如我们需要在某个成员函数内去动态修改某些数据，这就需要mutable的引入。
```cpp
class CTextBlock {
public:
//...
    std::size_t length() const;
private:
    char* pText;
    //std::size_t textLength;
    //bool lengthIsValid;
    mutable std::size_t textLength; //这些成员变量可能总是
    mutable bool lengthIsValid; //会被更改，即使在const成员函数内。
}
std::size_t CTextBlock::length() const{
    if (!lengthIsValid) {
    textLength = std::strlen(pText); //引入mutable之前不得行
    lengthIsValid = true;//现在可以
    } 
    return textLength;
}
```

对于同一个类，如果我们需要实现其中的功能，我们需要重复编写两份一模一样的代码，即：
```cpp
class TextBlock{
public:
//...
    const char& operator[](std::size_t position) const
{ 
    //... //边界检验（bounds checking)
    //··· //志记数据访问（log access data）
    //··· //检验数据完整性(verify data integrity)
    return text [position];
}
    char& operator[](std::size t position)
{
    //··· //边界检验（bounds checking）
    //··· //志记数据访问（log access data)
    //··· //检验数据完整性（verify data integrity)
    return text [position];
}
private:
    std::string text;
};
```
我们可以利用non-const成员函数的特性，它可以修改类成员的值去省略代码
```cpp
class TextBlock{
public:
//...
    const char& operator[](std::size_t position) const
{ 
    //... //边界检验（bounds checking)
    //··· //志记数据访问（log access data）
    //··· //检验数据完整性(verify data integrity)
    return text [position];
}
    char& operator[](std::size t position)
{
    return const_cast<char&>(static_cast<const TextBlock&>*this[position]);
}
private:
    std::string text;
};
```

#### 4.确保对象使用前已被初始化
对于c++内置类型：在使用对象之前先初始化
对于自定义类类型：使用成员初值列对类对象初始化（类对象在创建时会先调用default构造函数，即用初值列初始化对象的类成员，再调用构造函数，因此初值列比构造函数内的赋值更快，但这点对于内置类型是不成立的）
```cpp
Student::Student():
    TheName(sname),//调用TheName的default构造函数
    ThePartM(sno),//同上
    Age(20)//由于age是int类型，显式说明成20，效率与在构造函数内是一样的
    {//Age=20等同于上面的赋值
    }
```

reference-returning函数：
跨编译单元的non-local static对象的次序是不能确定的，这会导致某些不可确定的结果，即在不同源文件的某个类的static对象，它只是声明式，而未定义或未初始化，当我们在别的源文件使用了该对象时，由于他们的编译顺序是不确定的，会导致使用的对象是一个不知所以然的对象。我们可以使用函数调用来改善这种情况，那就是使用reference-returning函数：
```cpp
class FileSystem {}//... 
FileSystem& tfs() //这个函数用来替换tfs对象；它在
{                       //FileSystem class中可能是个static。
static FileSystem fs;   //定义并初始化一个local static对象，
return fs;              //返回一个reference指向上述对象。
}
class Directory {};
Directory::Directory( params) //同前，但原本的 reference to tfs
{       //现在改为tfs()
//...
std::size t disks =tfs().numDisks();
}
Directorys tempDir()
{                    //这个 用来替换tempDir对象；
                     //它在Directory class中可能是个static。
static Directory td; //定义并初始化local static对象，
return td;           //返回一个reference指向上述对象。
}
```
