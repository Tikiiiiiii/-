#### 13.以对象管理资源
所谓资源就是使用后就要还给系统，考虑下列情况：
```cpp
class Class{};
Class* CreatClass(){};
void f(){
    //...
    Class* pt=CreatClass();
    //...
    delete pt;
}
```
在后续的维护和更新过程中，如果f()函数中有return语句或在循环中有break语句，或在f()函数内部抛出了一些异常，那pt这个指针就无法正常释放，就导致了内存泄漏。这就要求：
>许多资源被动态分配与heap中内而被用于单一区块或函数内，它们应该在控制流离开那个区块或函数时被释放。

那么我们可以用auto_ptr来避免这些问题：
```cpp
void f(){
    std::atuo_ptr<Class> ptC(CreateClass());
}//当控制流离开这个区块时atuo_ptr自动删除
```
这可以得到以对象管理资源的两个关键思想：
>获得资源后立刻放进管理对象
>管理对象运用析构函数确保资源被释放
注意：
>auto_ptr不允许多个指针指向同一个对象
>auto_ptr的copying函数会把赋值符改为NULL，被赋值对象拥有资源唯一归属权
```cpp
std::atuo_ptr<Class> ptC1(CreateClass());
std::auto_ptr<Class> ptC2(ptC1);//此时ptC1为空，ptC2指向目标
ptC1=ptC2;//此时ptC2为NULL
```
替换它的方案是使用reference-counting smart pointer(RCSP)：（RCSP通过引用计数的方式管理对象的销毁）
```cpp
std::shared_ptr<Class> ptC1(CreateClass());
std::shared_ptr<Class> ptC2(ptC1);//此时ptC1和ptC2指向目标
ptC1=ptC2;//同上，当他们离开区块时，也会自动销毁
```
但是这样的指针也有一个缺点，那就是它的析构调用的是delete而不是delete[]，这意味着它不适用指向数组

#### 14.留意在资源管理类中的copying行为
当一个资源管理类被复制，即上述的ptC2和ptC1如果不是shared_ptr指针，只是一个普通的管理类，那将会发生两个指针指向同一个资源，如果多次释放将会导致不可意想的结果。对于一个资源取得初始化对象（RAII），我们对于copying函数，我们可以：
>禁止复制，也就是条款6
>在底层使用引用计数法，也就变成了shared_ptr指针
>深拷贝，也就是创建被拷贝资源的所有底部资源
>转移底部资源的控制权，也就是auto_ptr

#### 15.在资源管理类中提供对原始数据的访问
APIs往往需要访问原始资源，那么RAII应该提供获取原始资源的方法：
（1）显式转换：即在资源管理类中提供get()
（2）隐式转换：即重载运算符->和*这样，或是operator Class（）const这样的
但相比下，显式转换比隐式转换安全：
```cpp
ClassControl c1(GetClass());
//。。。
Class c2=c1;//这时会多产生一个从c1转换为底部资源才赋值给c2
//如果c1在其中已经被销毁了，那么c2就可能出很大问题
```

#### 16.成对使用delete和new
new []则使用delete []
new则使用delete
如果new对应delete[]将会析构未知内存
如果new[]对应delete将会内存泄漏
//...例子省略

#### 17.以独立语句将newed对象置入智能指针
以独立语句将newed对象置入智能指针，如果不这么做，异常的抛出将导致某些难以发现的内存泄漏：
```cpp
int priority();
void processWidget(std::tr1::shared_ptr<Widget>pw,int priority);
//假如有以上两个接口函数
//考虑这样调用函数
// processWidget(new Widget,priority());
// 由于shared_ptr是一个禁止隐式转换的explicit函数，所以上面调用是错误的
processWidget(std::tr1::shared_ptr<Widget>(new Widget),priority());
//对于这个函数调用，前者由两部分组成，new Widget和把它置入share_ptr中，以及调用priority()
//对于他们三的顺序可用知道new Widget一定在tr1::shared_ptr<Widget>之前，但priority可以随意
//我们考虑priority第二执行的结果，如果此时出现了异常，那么shared_ptr的指针资源就是处于危险状态，因此单独把newed对象独立语句是个不错的选择
```













