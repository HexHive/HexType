#include <stdlib.h>

struct BaseType {
    long long longMember = 0;
    int array[20];
};

#if defined(CAST_BASIC) || defined(CAST_PHANTOM) || defined(CAST_PHANTOM_DEEP)
struct BaseDerivedType : BaseType {
    char c1 = 0;
    char c2 = 0;
    char c3 = 0;
    char c4 = 0;
    char c5 = 0;
    char c6 = 0;
    char c7 = 0;
    char c8 = 0;
};
#endif

#ifdef CAST_INHERITANCE_MULTI
struct OtherBaseType {
    char cb1 = 0;
    char cb2 = 0;
    char cb3 = 0;
    char cb4 = 0;
    char cb5 = 0;
    char cb6 = 0;
    char cb7 = 0;
    char cb8 = 0;
};
struct BaseDerivedType : OtherBaseType, BaseType {
    char c1 = 0;
    char c2 = 0;
    char c3 = 0;
    char c4 = 0;
    char c5 = 0;
    char c6 = 0;
    char c7 = 0;
    char c8 = 0;
};
#endif

struct FakeDerivedType : BaseDerivedType {
};

struct ReallyFakeDerivedType : FakeDerivedType {
    int fake;
};

struct ReallyReallyFakeDerivedType : ReallyFakeDerivedType {
    int reallyFake;
};

#if defined(CAST_PHANTOM) || defined(CAST_PHANTOM_DEEP)
struct FakePhantomType : BaseDerivedType {
};
struct FakeDerivedPhantomType : FakePhantomType {
};
#endif

void usetypeSecond() {
    ReallyReallyFakeDerivedType objDerived;
    if (static_cast<BaseType*>(&objDerived) == NULL) {
        exit(-1);
    }
#if defined(CAST_PHANTOM) || defined(CAST_PHANTOM_DEEP)
    FakeDerivedPhantomType objPhantom;
#endif
}
