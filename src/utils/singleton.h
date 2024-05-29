#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

namespace implayer
{
    template <typename T>
    class Singleton
    {
    public:
        static T &getInstance()
        {
            static T t;
            return t;
        }

        // 禁止拷贝和禁止移动
        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;
        Singleton(Singleton &&other) = delete;
        Singleton &operator=(Singleton &&other) = delete;

    protected:
        Singleton() = default;
        ~Singleton() = default;
    };
}

#endif
