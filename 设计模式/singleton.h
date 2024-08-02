#include <memory>

template <class T>
class Singleton{
private:
    Singleton();
public:
    static std::shared_ptr<T> getInstance() {
        static std::shared_ptr<T> instance = std::make_shared<T>();
        return instance;
    }
}
