#### 49.了解new-handle的行为
对于一个new操作，如果内存不足以满足new操作，它就会抛出异常，并返回一个null指针，但在抛异常以前，会先调用一个错误处理函数：new-handle
```cpp
namespace std{
    typedef void(*new_handler)();
    new_handler (new_handler p) throw();
}
```
typedef定义一个函数指针，该指针无参数无返回值；
set_new_handler是一个设置错误处理函数，它的参数是new失败时所执行的函数，返回值是set_new_handler被调用前正在执行的那个new-handler函数，throw()表示不会抛出异常；
```cpp
void outOfMem(){
    std::cerr<<"Unable to satisfy request for memoryn";
    std::abort();
}
int main(){
    std::set_new_handler(outOfMem);
    int *pBigDataArray=new int[100000000L];
}//当new无法分配100000000个内存时，outOfMem就会被调用
```
当new失败时，new_handle会一直被调用直到找到足够内存，new_handle必须能对这个事件做出反应：
（1）使得下次new调用成功率更高：
**如果无法分配即释放**：每次进行new操作时，分配一大块内存，当new-handle第一次被调用时（意味着内存不够）就释放
**安装另一个new-handle**：把一个分配成功概率更高的new-handle替换本身（变法：该变自身的行为，如static数据、namespace数据或global数据）
（2）异常处理：
**卸载new-handle**：null指针传入set_new_handle的参数，new失败会直接抛出异常
**抛出bad_alloc的相关异常**：把该异常传播到内存申请处
**不返回**：abort或exit

在c++如果想为自己的类设计new和new-handle函数，那么我们需要：
（1）声明并定义set_new_handle函数
（2）声明并定义new操作符
（3）声明并定义new_handle函数
```cpp
class Widget{
public:
    static std::new_handler set_new_handler(std::new_handler p) throw();
    static void* operator new(std::size_t size) throw(std::bad_alloc);
private:
    static std::new_handler currentHandler;
};
//初始化static函数
std::new_handler Widget::currentHandler=0;
std::new_handler Widget::set_new_handler(std::new_handler p) throw(){
    std::new_handler oldHandler=currentHandler;
    currentHandler=p;
    reutrn oldHandler;
}
```
那么对于widget类的new过程如下：
（1）调用set_new_handle函数安装类内的new_handle
（2）执行operator new，若成功返回分配的内存的指针，并恢复原来的new_handle（恢复默认）
（3）如果执行失败，调用刚刚安装的new_handle，如果还是分配失败，则恢复到原本的new_handle（恢复默认）并抛出bad_alloc异常

我们把这个类模板化，使得变为一个基类能被任意类public继承，这样使得它能完成相应功能。
```cpp
 template<typename T>
class NewHandlerSupport{
public:
    static std::new_handler set_new_handler((std::new_handler p) throw();)
    static void* operator new(std::size_t size) throw(std::bad_alloc);
    ……
private:
    static std::new_handler currentHandler;
};
template<typename T> std::new_handler
NewHandlerSupport<T>::set_new_handler(std::new_handler p) throw(){
    std::new_handler oldHandler=currentHandler;
    currentHandler=p;
    return oldHandler;
}
template<typename T> void* NewHandlerSupport<T>::operator new(std::size_t size) throw(std::bad_alloc){
    NewHandlerHolder h(std::set_new_handler(currentHandler));
    return ::operator new(size);
}
//将每一个currentHandler初始化为null
template<typename T>std::new_handler NewHandlerSupport<T>::currentHandler=0;
//有了这个class template，为Widget添加set_new_handler就容易了
class Widget:public NewHandlerSupport<Widget>
{……};
```

对于旧时new分配失败的处理，都是返回null，但是现在都是抛出bad_alloc异常，因此c++标准委员协会兼容两者，设计了nothrow（在< new >头文件中）：
```cpp
Widget* pw1=new Widget;//分配失败，抛出bad_alloc
if(pw1==null)//判断是否分配成功。但是这个测试失败
Widget* pw2=new(std::nothrow)Widget;//分配失败，返回null
if(pw2==null)//可以侦测
```
nothrow对异常的强制保证性并不强,它只能保证operator new不抛异常，但不保证new的对象的构造函数不抛异常，因此没有使用它的必要。

#### 50.了解new和delete的合理交替时机
替换new和delete的作用：
（1）为了检测运行时的错误：可以在分配的内存之前加入超前元素，之后加入超尾元素，在发生错误时查看两个元素的固定值是否改变
（2）为了收集动态分配内存所使用上的统计数据：用于测定哪一种形式的机制更适合我们本身的类，获取数据并根据数据分析
（3）为了增加分配和归还速度：缺省分配器为满足所有的分配，一定会比定制性的分配器慢，设置自己的分配器可以找到最适合自己的定制性分配器
（4）为了降低空间开销：同上
（5）为了弥补非最佳齐位：如果我们把指针按字节对齐，将会得到更快速的访问效率
（6）为了将相关对象成簇集中：把某一数据结构创于heap中可以减少缺页率
（7）为了获得非传统行为：做一些缺省分配器不会做的事

#### 51.编写new和delete时要固守常规
operator new应该*返回指针或抛出异常，内存不足时必须不断调用new-handling函数，要有对付零内存需求的准备，避免不慎掩盖正常形式的new*
伪码：
```cpp
void* operator new(std::size_t size) throw(std::bad_alloc)
{
    using namespace std;
    if(size==0){//处理0-byte申请
        size=1;
    }
    while(true){
    //循环调用new_handle函数直接内存被分配或抛异常，如果不那么做将永远卡在这
        尝试分配size bytes;
        if(分配成功)
            return 指向分配得来的内存的指针;
        //分配失败，找到当前的new-handling函数
        new_handler globalHandler=set_new_handler(0);
        set_new_handler(globalHandler);
        //无法直接获取new_handle，通过此方法获取
        if(globalHandler) (*globalHandler)();
        else throw std::bad_alloc();
        //当new_handle为空时就抛出异常
    }
}
```
上述operator new成员函数可能会被derived classes继承，我们并未考虑继承，继承时可以发现new已经不适用了，若它是专属new就应该：
```cpp
void* Base::operator new(std::size_t size) throw(std::bad_alloc){
    if(size!=sizeof(Base))
        return ::operator new(size);
    //若size为0或者继承类大小，使用标准的operator new
    ……
}
```
如果我们还打算管理该类对象的new操作，那也要实现operator new[]。

operator delete应该保证删除null指针永远安全：
```cpp
void Base::operator delete(void rawMemory, std::size_t size) throw()
{
    if(rawMemory==0) return;
    if(size!=sizeof(Base)){
        ::operator delete(rawMemory);
        rexxiturn ;
    }
    归还rawMemory所指内存;
    return ;
}
```
另外，如果派生类来自某个基类没有virtual析构函数，那么if语句中的size可能有问题，导致重新使用全局delete。

#### 52.写了placement new也要写placement delete
当我们使用new创建一个对象时
>Widget* pw=new Widget;
有两个函数被调用，第一个函数就是operator new用以分配内存，第二个是Widget的default构造函数。若构造函数调用失败，运行期系统会调用第一个函数operator new所对应的operator delete版本。但是如果使用的是由参数的operator new，那么就无法知道哪一个delete是对应版本了。

如果operator new接受的参数除了必有的size_t之外还有其他，这便是placement new。
```cpp
void* operator new(std::size_t, void* pMemory) throw(); //placement new
//该placement new已被纳入C++标准程序库<new>中
class Widget{
public:
    ……
    static void* operator new(std::size_t size, std::ostream& logStream)//非正常形式的new
    throw(std::bad_alloc);
    static void operator delete(void* pMemory, std::size_t size)//正常的class专属delete
    throw();
    ……
};
Widget* pw=new (std:cerr) Widget;//调用operator new，并传递cerr作为ostream实参，但这如果在widget的构造函数出现异常，将会导致内存泄漏
//因此应该声明delete的placement版本
void operator delete(void*, std::ostream&) throw();
```

当一个带额外参数的new没有对应的带额外参数的delete，那么当new异常需要取消并恢复时就没有delete被调用，因此有必要声明并定义一个placement的delete版本去对应new。

当声明placement new和placement delete时有可能会遮掩他们的正常版本，即让
>delete pw;
这样的语句失效。因此在声明placement版本时不要落下原始版本
```cpp
class StadardNewDeleteForms{
public:
    //normal
    static void* operator new(std::size_t size) throw(std::bad_alloc)
    {return ::operator new(size);}
    static void operator delete(void* pMemory) throw()
    {::operator delete(pMemory);}
    //placement
    static void* operator new(std::size_t size, void* ptr) throw(std::bad_alloc)
    {return ::operator new(size, ptr);}
    static void operator delete(void* pMemory, void* ptr) throw()
    {::operator delete(pMemory, ptr);}
    //nothrow
    static void* operator new(std::size_t size, const std::nothrow_t& nt) throw(std::bad_alloc)
    {return ::operator new(size,nt);}
    static void operator delete(void* pMemory,const std::nothrow_t&) throw()
    {::operator delete(pMemory);}
};
```
