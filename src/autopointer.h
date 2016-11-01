#ifndef AUTOPOINTER_H
#define AUTOPOINTER_H

template <class F, class P>
class AutoPointer
{
public:
    AutoPointer(F f)
    {
        function = f;
        pointer = NULL;
    }
    AutoPointer(P p)
    {
        pointer = a;
    }
    ~AutoPointer()
    {
        if (function && pointer)
            function(pointer);
    }
    P()
    {
        return pointer;
    }
private:
    P pointer;
    F function;
};

#endif // AUTOPOINTER_H
