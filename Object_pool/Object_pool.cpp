#include"Object_pool.h"


template<typename Object>
Objectpool<Object>::Objectpool(size_t num):Objnum(num){
    for(size_t i=0;i<Objnum;i++){
        Objpool.push_back(new Object());
    }
}
template<typename Object>
Objectpool<Object>::~Objectpool(){
    auto it=Objpool.begin();
    while(it!=Objpool.end()){
        delete(*it);
        ++it;
    }
    Objnum=0;
}
template<typename Object>
Object* Objectpool<Object>::get_object(){
    Object * pObj=nullptr;
    if(0==Objnum){
        pObj=new Object();
    }
    else{
        pObj=Objpool.front();
        Objpool.pop_front();
        --Objnum;
    }
    return pObj;
}
template<typename Object>
void Objectpool<Object>::return_object(Object * pObj){
    Objpool.push_back(pObj);
    ++Objnum;
}
 
