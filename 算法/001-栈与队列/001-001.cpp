#include <iostream>
#include <stack>
using namespace std;
class Stack{
public: 
    void push(int val);
    int pop();
    int getMin();
private:
    stack<int> s1;
    stack<int> s2;
};

void Stack::push(int val){
    if(s1.empty()){
        s2.push(val);
    }
    else if(val <= getMin()){
        s2.push(val);
    }
#ifdef METHOD2
    else{
        s2.push(getMin());
    }
#endif
    s1.push(val);
}
int Stack::pop(){
    int ret = s1.top();
    s1.pop();
#ifdef METHOD2
    s2.pop();
#else
    if(ret == getMin()){
        s2.pop();
    }
#endif
    return ret;
}

int Stack::getMin(){
    return s2.top();
}

void test(){
    Stack s;
    s.push(3);
    s.push(4);
    s.push(5);
    s.push(1);
    s.push(2);
    s.push(1);
    for(int i =0; i < 6; ++i){
        cout<<s.getMin()<<endl;
        s.pop();
    }
}

int main(){
    test();
}