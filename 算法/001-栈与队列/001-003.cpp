#include <stack>
#include <iostream>
using namespace std;
void display(stack<int> s){
    cout<<"自顶向下:";
    while(!s.empty()){
        cout<<s.top()<<" ";
        s.pop();
    }
    cout<<endl;
}

int getLastAndRemove(stack<int> &s){
    int result = s.top();
    s.pop();
    if(s.empty()){
        return result;
    }else{
        int last = getLastAndRemove(s);
        s.push(result);
        return last;
    }
}

void reverse(stack<int> &s){
    if(s.empty()){
        return;
    }else{
        int last = getLastAndRemove(s);
        reverse(s);
        s.push(last);
    }
}

void test(){
    stack<int> s;
    for(int i=1;i<6;++i){
        s.push(i);
    }
    display(s);
    reverse(s);
    display(s);
}

int main(){
    test();
}