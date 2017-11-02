// The type at the allocation type
// Includes a sub-struct of type BaseType on BADCAST tests
// Includes a sub-struct of type DerivedType (derived from BaseType) on GOODCAST tests
struct AllocType;
// The root-type for down-casting
// Pointers are passed along with type BaseType*
struct BaseType;
// Derived from BaseType and the target type for the down-cast
struct BaseDerivedType;
// Trivially derived from BaseDerivedType to avoid hash-equivalence tests on GOODCAST
struct DerivedType;

#ifdef DO_PASSING
#define ALLOC_MEMBER_TYPE DerivedType
#else
#define ALLOC_MEMBER_TYPE BaseType
#endif

#ifdef CAST_PHANTOM
#define CAST_TYPE PhantomType
#elif CAST_PHANTOM_DEEP
#define CAST_TYPE DerivedPhantomType
#else
#define CAST_TYPE BaseDerivedType
#endif

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

#if defined(CAST_PHANTOM) || defined(CAST_PHANTOM_DEEP)
struct PhantomType : BaseDerivedType {
};
#endif

#ifdef CAST_PHANTOM_DEEP
struct DerivedPhantomType : PhantomType {
};
#endif

struct DerivedType : BaseDerivedType {
};

#if defined(ALLOC_OVERLOADED_NEW) || defined(ALLOC_OVERLOADED_NEW_ARRAY) || defined(ALLOC_OVERLOADED_NEW_VLA)
#define NEW_OPERATOR_OVERLOAD                                       \
    void *operator new(size_t size) { return new char[size]; }      \
    void *operator new[](size_t size) { return new char[size]; }
#else
#define NEW_OPERATOR_OVERLOAD
#endif

#ifdef BASE_BASIC
struct AllocType {
    ALLOC_MEMBER_TYPE baseMember;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_NESTED0
struct AllocType {
    ALLOC_MEMBER_TYPE baseMember;
    int longMemberEnd = 0;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_NESTED
struct AllocType {
    long long longMember;
    ALLOC_MEMBER_TYPE baseMember;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_NESTED_ARRAY

struct AllocType {
    long long longMember;
    ALLOC_MEMBER_TYPE baseMember[10];
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_NESTED_DEEP
struct IntermediateType2 {
    long long longMember;
    ALLOC_MEMBER_TYPE baseMember;
};
struct IntermediateType {
    long long longMember;
    IntermediateType2 intermediate2;
};
struct AllocType {
    long long longMember;
    IntermediateType intermediate;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_NESTED_ARRAY_DEEP
struct IntermediateType2 {
    long long longMember;
    ALLOC_MEMBER_TYPE baseMember[10];
};
struct IntermediateType {
    long long longMember;
    IntermediateType2 intermediate2[10];
};
struct AllocType {
    long long longMember;
    IntermediateType intermediate[10];
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_INHERITANCE
struct AllocType : ALLOC_MEMBER_TYPE {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_VINHERITANCE
struct AllocType : virtual ALLOC_MEMBER_TYPE {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_INHERITANCE_MULTI
struct IntermediateType {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct AllocType : IntermediateType, ALLOC_MEMBER_TYPE {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_VINHERITANCE_MULTI
struct IntermediateType {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct AllocType : IntermediateType, virtual ALLOC_MEMBER_TYPE {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_INHERITANCE_MULTI_DEEP
struct IntermediateType {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct IntermediateType2 {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct IntermediateBaseType : IntermediateType2, ALLOC_MEMBER_TYPE {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct AllocType : IntermediateType, IntermediateBaseType {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif

#ifdef BASE_VINHERITANCE_MULTI_DEEP
struct IntermediateType {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct IntermediateType2 {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct IntermediateBaseType : IntermediateType2, virtual ALLOC_MEMBER_TYPE {
    int intMember1 = 0;
    int intMember2 = 0;
};
struct AllocType : IntermediateType, IntermediateBaseType {
    long long longMemberEnd;
    NEW_OPERATOR_OVERLOAD
};
#endif 
