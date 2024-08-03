#include <iostream>
#include <stack>
class Queue{
public:
    void push(int val);
    void pop();
    int front();
private:
    stack<int> pushStack;
    stack<int> popStack;
};

void Queue::push(int val){
    pushStack.push(val);
}

void Queue::pop(){
    if(!popStack.empty()){
        popStack.pop();
    }
    else{
        while(!pushStack.empty()){
            popStack.push(pushStack.top());
            pushStack.pop();
        }
        popStack.pop();
    }
}

int Queue::front(){
    if(popStack.empty()){
        while(!pushStack.empty()){
            popStack.push(pushStack.top());
            pushStack.pop();
        }
    }
    return popStack.top();
}