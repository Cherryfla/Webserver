#include<list>
#include<cstdio>
template<typename Object>
class Objectpool{
    private:
        size_t Objnum;
        std::list<Object *>Objpool;
    public:
        Objectpool(size_t num);
        ~Objectpool();
        Object* get_object();
        void return_object(Object *pObj);
};
