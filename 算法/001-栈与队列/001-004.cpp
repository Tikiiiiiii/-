#include <iostream>
#include <string>
#include <queue>
using namespace std;
class Pet{
private:
    string type;    // 类型
    int count;      // 时间
public:
    Pet(string t, int c):type(t),count(c){};
    string getType(){
        return type;
    }
    int getCount(){
        return count;
    }
};
class Cat:public Pet{
public:
    Cat(int c):Pet("cat",c){};
};  
class Dog:public Pet{
public:
    Dog(int c):Pet("dog",c){};
}; 

class Queue{
private:
    queue<Pet*> dogq;
    queue<Pet*> catq;
    int count = 0;
public:
    Queue(){}
    void add(string t){
        if(t == "dog"){
            dogq.push(new Dog(count));
        }else{
            catq.push(new Cat(count));
        }
        count++;
    }
    bool isEmpty(){
        return dogq.empty() && catq.empty();
    }
    bool isDogEmpty(){
        return dogq.empty();
    }
    bool isCatEmpty(){
        return catq.empty();
    }
    void popCat(){
        while(!catq.empty()){
            Pet *p = catq.front();
            cout<< p->getType()<<" ";
            catq.pop();
        }
    }
    void popDog(){
        while(!dogq.empty()){
            Pet *p = dogq.front();
            cout<< p->getType()<<" ";
            dogq.pop();
        }
    }
    void popAll(){
        while(!dogq.empty()&&!catq.empty()){
            Pet *p1 = dogq.front();
            Pet *p2 = catq.front();
            if(p1->getCount()<p2->getCount()){
                cout<<p1->getType()<<" ";
                dogq.pop();
            }else{
                cout<<p2->getType()<<" ";
                catq.pop();
            }
        }
        popCat();
        popDog();
        cout<<endl;
    }
};

void test(){
    Queue petq;
    petq.add("dog");
    petq.add("dog");
    cout<<petq.isCatEmpty() <<" "<< petq.isDogEmpty()<<endl;
    petq.add("cat");
    petq.add("cat");
    petq.add("dog");
    petq.add("dog");
    petq.add("dog");
    petq.add("dog");
    petq.add("cat");
    cout<<petq.isCatEmpty() <<" "<< petq.isDogEmpty()<<endl;
    petq.popAll();
    petq.add("cat");
    petq.add("dog");
    petq.add("dog");
    petq.add("dog");
    petq.popDog();
    cout<<endl;
}

int main(){
    test();
}