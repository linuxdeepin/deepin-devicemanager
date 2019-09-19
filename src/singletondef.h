#pragma once

#pragma once

/*  Best Singleton
    noncopyable
    use stack, not new from heap
    easy use
*/

#define  Declare_Singleton(classname) \
public: \
static classname& Instance() \
{ \
    static classname _instance; \
    return _instance; \
} \
protected: \
    classname() = default; \
    ~classname() = default; \
                            \
    classname(const classname& rInstance); \
    classname& operator = (const classname& rInstance) \
