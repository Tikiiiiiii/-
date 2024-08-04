#include <iostream>
#include <string>
#include <queue>
using namespace std;
class Pet{
private:
    string type;
public:
    Pet(string t):type(t){};
    virtual string getType(){
        return type;
    }
};
class Cat:public Pet{
    Cat(string t):Pet(t){};
};  
class Dog:public Pet{
    Dog(string t):Pet(t){};
}; 

class PetQueueElement{
private:
    Pet p;
    int count;
public:
    PetQueueElement(Pet pet,int cnt):p(pet),count(cnt){};
    int getCount() { return count; }
    Pet getPet() { return Pet; }

}

class Queue{
private:
    queue<PetQueueElement> dogq;
    queue<PetQueueElement> catq;
    int count = 0;
public:
    Queue(){}
    void add(Pet p){
        if(p.getType() == "dog"){
            dogq.push(new PetQueueElement(p,count));
        }else{
            catq.push(new PetQueueElement(p,count));
        }
    }
    void isEmpty(){
        return dogp.empty() && catq.empty();
    }
    void isDogEmpty(){
        return dogp.empty();
    }
    void isCatEmpty(){
        return catp.empty();
    }
    void popCat(){
        while(!catp.empty()){
            PetQueueElement e = catp.front();
            catp.pop();
            cout<<e.getPet().getType()<<" ";
        }
        cout<<endl;
    }
    void popDog(){
        while(!dogq.empty()){
            PetQueueElement e = dogq.front();
            dogq.pop();
            cout<<e.getPet().getType()<<" ";
        }
        cout<<endl;
    }
    void popAll(){
        while(!dogp.empty()&&!catp.empty()){
            PetQueueElement e1 = dogq.front();
            PetQueueElement e2 = catp.front();
            if(e1.getCount()<e2.getCount()){
                
            }
        }
    }
}