#pragma once
#include "AddInDefBase.h"
#include "ComponentBase.h"
#include "IMemoryManager.h"
#include "core_as/str/sstring.h"
#include "core_as/testable_static_assert.h"
#include <utility>
using namespace core_as::str;

inline static stru varToTextU(const tVariant& v) {
    return stru{v.pwstrVal, v.wstrLen};
}

inline static stra varToTextA(const tVariant& v) {
    return stra{v.pstrVal, v.strLen};
}

inline static bool isInteger(const tVariant& v) {
    switch (v.vt) {
    case VTYPE_I1:
    case VTYPE_I2:
    case VTYPE_I4:
    case VTYPE_UI1:
    case VTYPE_UI2:
    case VTYPE_UI4:
    case VTYPE_ERROR:
        return true;
    default:
        return false;
    }
}

inline static int getInteger(const tVariant& v) {
    switch (v.vt) {
    case VTYPE_I1:
        return v.i8Val;
    case VTYPE_I2:
        return v.shortVal;
    case VTYPE_I4:
        return v.intVal;
    case VTYPE_UI1:
        return v.ui8Val;
    case VTYPE_UI2:
        return v.ushortVal;
    case VTYPE_UI4:
        return v.uintVal;
    case VTYPE_ERROR:
        return v.errCode;
    default:
        return 0;
    }
}

class AddinBase : public IComponentBase {
public:
    bool ADDIN_API setMemManager(void* mem) override {
        memoryManager_ = reinterpret_cast<IMemoryManager*>(mem);
        return true;
    }

    long ADDIN_API GetInfo() override {
        return 2000;
    }

    void ADDIN_API SetLocale(const WCHAR_T* loc) override {
        locale_ = stru{loc};
    }

    void ADDIN_API SetUserInterfaceLanguageCode(const WCHAR_T* lang) override {
        uiLanguageCode_ = stru{lang};
    }

    template<typename T>
    T* v8alloc(size_t size) const {
        T* destination = nullptr;
        memoryManager_->AllocMemory(reinterpret_cast<void**>(&destination), (unsigned long)size * sizeof(T));
        return destination;
    }
    WCHAR_T* copyText(auto&& text) const {
        WCHAR_T* destination     = v8alloc<WCHAR_T>(text.length() + 1);
        *text.place(destination) = 0;
        return destination;
    }

protected:
    stru selectLocaleStr(stru rus, stru eng) {
        return uiLanguageCode_.starts_with_ia(u"ru") || locale_.starts_with_ia(u"ru") ? rus : eng;
    }
    IAddInDefBaseEx* v8connection_{};
    IMemoryManager* memoryManager_{};
    stringu locale_;
    stringu uiLanguageCode_;
    stringu lastError_;
    bool throwErrors_{};
};

struct AddinInfo {
    ssu name;
    IComponentBase* (*create)();
    AddinInfo* next;
    inline static AddinInfo* first{nullptr};

    AddinInfo(ssu n, IComponentBase* (*c)()) : name(n), create(c), next(first) {
        first = this;
    }
};

// Основа инфраструктуры работы с описаниями методов/свойств
struct first_def {
    enum {
        val      = -1,
        IsFunc   = -1,
        nParam   = -1,
        IsDefVal = -1,
        IsRead   = -1,
        IsWrite  = -1,
    };
};

// Стратегии поиска номера элемента по его имени

// Стратегия при отсутствии элементов
template<typename T>
struct nofnd_pol {
    static void OnInitFirstObject() {}
    static int find(ssu name) {
        return -1;
    }
};

// Стратегия при наличии всего одного элемента
template<typename T>
struct onefnd_pol {
    static void OnInitFirstObject() {}
    static int find(ssu name) {
        if (0 == name.compare_iu(T::prev::rusName) || 0 == name.compare_iu(T::prev::engName))
            return 0;
        return -1;
    }
};

// Стратегия поиска при количестве элементов от 2 до 3
template<typename T>
struct simplefnd_pol {
    static void OnInitFirstObject() {}
    static int find(ssu name) {
        return find_<typename T::prev>(name);
    }

    template<typename M>
    static int find_(ssu name) {
        if (0 == name.compare_iu(M::rusName) || 0 == name.compare_iu(M::engName))
            return M::val;
        return find_<typename M::prev>(name);
    }

    template<>
    static int find_<first_def>(ssu name) {
        return -1;
    }
};

// Стратегия при количестве элементов больше 3
template<typename T>
struct hashfnd_pol {
    inline static hashStrMapUIU<int> names;
    static void OnInitFirstObject() {
        init<typename T::prev>();
    }

    template<typename M>
    static void init() {
        names.emplace(M::rusName, M::val);
        names.emplace(M::engName, M::val);
        init<typename M::prev>();
    }

    template<>
    static void init<first_def>() {}

    static int find(ssu name) {
        if (auto fnd = names.find(name); fnd != names.end()) {
            return fnd->second;
        }
        return -1;
    }
};

// Мета-функция, выбирает тип поиска номера по строке,
// в зависимости от количества методов/свойств.
// Использовать: fndselector<T::endmeths>::type, fndselector<T::endprops>::type
template<typename T>
struct fndselector {
    template<int K>
    struct tdef;
    template<>
    struct tdef<0> {
        using type = nofnd_pol<T>;
    };
    template<>
    struct tdef<1> {
        using type = onefnd_pol<T>;
    };
    template<>
    struct tdef<2> {
        using type = simplefnd_pol<T>;
    };
    template<>
    struct tdef<3> {
        using type = hashfnd_pol<T>;
    };

    enum {
        Count = T::val,
        val   = Count == 0 ? 0 : (Count == 1 ? 1 : (Count < 4 ? 2 : 3)),
    };
    using type = typename tdef<val>::type;
};

// Возврат имени из массива имен по индексу.
// Массив имен создается при первом обращении к name.
template<typename T>
struct getname {
    template<int Count>
    struct finder {
        stru names[Count][2];
        finder() {
            fill<typename T::prev>();
        }
        template<typename M>
        void fill() {
            names[M::val][0] = M::engName;
            names[M::val][1] = M::rusName;
            fill<typename M::prev>();
        }
        template<>
        void fill<first_def>() {}

        static const WCHAR_T* name(unsigned methodNumber, unsigned language, AddinBase* a) {
            static const finder<Count> f;
            if (methodNumber < Count && language < 2) {
                return a->copyText(f.names[methodNumber][language]);
            }
            return nullptr;
        }
    };
    template<>
    struct finder<0> {
        static const WCHAR_T* name(unsigned, unsigned, AddinBase*) {
            return nullptr;
        }
    };
    using type = finder<T::val>;
};

// Набор различных метафункций, работающих со списками типов
// описаний методов и свойств.

// Проверка на то, есть END_EXTENSION() и он в правильном месте
template<typename T, int I>
struct test_last {
    testable_static_assert(I < T::val, "No END_EXTENSION in your class");
};

// Проверка на то, что методы с параметрами по умолчанию имеют не меньше 1 параметра
template<int I, bool bDef>
struct check_defval {
    testable_static_assert(!(bDef && I == 0), "Def val with 0 params");
};

// Мета-функция, подсчитывает, сколько среди методов процедур и функций.
// Использовать:
// meth_count<T>::proc - количество процедур
// meth_count<T>::func - количество функций
template<typename T>
struct meth_count {
    template<int I>
    struct counter {
        template<typename M>
        struct check {
            enum {
                next = M::prev::val == -1 ? 0 : 1,
                val  = static_cast<int>(M::IsFunc) + counter<next>::template check<typename M::prev>::val,
            };
        };
    };
    template<>
    struct counter<0> {
        template<typename M>
        struct check {
            enum { val = 0 };
        };
    };

    enum {
        first = T::endmeths::prev::val == -1 ? 0 : 1,
        func  = counter<first>::template check<typename T::endmeths::prev>::val,
        proc  = static_cast<int>(T::endmeths::val) - func,
    };
};

// Мета-функция, определяет, во всех-ли методах одинаковое количество параметров
// Использовать: same_np<T>::val
// Возвращает: -1 - в методах разное количество параметров, N - во всех методах N параметров
template<typename T>
struct same_np {
    template<int I>
    struct _same_np {
        template<typename M>
        struct check {
            enum { val = I };
        };
    };
    template<>
    struct _same_np<-1> {
        template<typename M>
        struct check {
            enum { val = -1 };
        };
    };
    template<>
    struct _same_np<-2> {
        template<typename M>
        struct check {
            enum {
                stop = M::prev::val == -1 ? 1 : 0,
                same = int(M::nParam) == M::prev::nParam ? 1 : 0,
                next = stop ? M::nParam : (same ? -2 : -1),
                val  = _same_np<next>::template check<typename M::prev>::val,
            };
        };
    };
    enum { first = T::endmeths::prev::val == -1 ? 0 : -2, val = _same_np<first>::template check<typename T::endmeths::prev>::val };
};

// Мета-функция, подсчитывает, сколько параметров доступно для чтения/записи.
// Использовать:
// rw_count<T>::read	- доступных для чтения
// rw_count<T>::write	- доступных для записи
// rw_count<T>::noread	- недоступных для чтения
// rw_count<T>::nowrite	- недоступных для записи
template<typename T>
struct rw_count {
    template<int I>
    struct rcounter {
        template<typename M>
        struct check {
            enum {
                next = M::prev::val == -1 ? 0 : 1,
                val  = static_cast<int>(M::IsRead) + static_cast<int>(rcounter<next>::template check<typename M::prev>::val),
            };
        };
    };
    template<>
    struct rcounter<0> {
        template<typename M>
        struct check {
            enum { val = 0 };
        };
    };

    template<int I>
    struct wcounter {
        template<typename M>
        struct check {
            enum {
                next = M::prev::val == -1 ? 0 : 1,
                val  = static_cast<int>(M::IsWrite) + static_cast<int>(wcounter<next>::template check<typename M::prev>::val),
            };
        };
    };
    template<>
    struct wcounter<0> {
        template<typename M>
        struct check {
            enum { val = 0 };
        };
    };

    enum {
        first   = T::endprops::prev::val == -1 ? 0 : 1,
        read    = rcounter<first>::template check<typename T::endprops::prev>::val,
        write   = wcounter<first>::template check<typename T::endprops::prev>::val,
        noread  = static_cast<int>(T::endprops::val) - read,
        nowrite = static_cast<int>(T::endprops::val) - write,
    };
};

template<typename T>
struct meth_pol {
    enum {
        isFuncMin = meth_count<T>::func <= meth_count<T>::proc ? 1 : 0,
        nParamCmn = same_np<T>::val,
    };

    static bool HasRetVal(unsigned method) {
        return policy<isFuncMin != 0>::template hasRetVal<typename T::endmeths::prev>(method);
    }

    static long GetNParams(unsigned method) {
        return nparam<nParamCmn>::GetNParams(method);
    }

    static bool GetDefVal(const T* ptr, unsigned method, unsigned param, tVariant* value) {
        return defVal<typename T::endmeths::prev>(ptr, method, param, value);
    }

    static bool CallAsProc(T* ptr, unsigned method, tVariant* params, unsigned count) {
        return callAsProc<typename T::endmeths::prev>(ptr, method, params, count);
    }
    static bool CallAsFunc(T* ptr, unsigned method, tVariant& ret, tVariant* params, unsigned count) {
        return callAsFunc<typename T::endmeths::prev>(ptr, method, ret, params, count);
    }

    template<bool bFunc /*==TRUE*/>
    struct policy {
        template<typename M>
        static bool hasRetVal(unsigned method) {
            if constexpr (M::IsFunc != 0) {
                if (method == M::val) {
                    return true;
                }
            }
            return hasRetVal<typename M::prev>(method);
        }
        template<>
        static bool hasRetVal<first_def>(unsigned) {
            return false;
        }
    };
    template<>
    struct policy<false> {
        template<typename M>
        static bool hasRetVal(unsigned method) {
            if constexpr (!M::IsFunc) {
                if (method == M::val) {
                    return false;
                }
            }
            return hasRetVal<typename M::prev>(method);
        }
        template<>
        static bool hasRetVal<first_def>(unsigned) {
            return true;
        }
    };

    template<long I>
    struct nparam {
        static long GetNParams(unsigned) {
            return I;
        }
    };

    template<>
    struct nparam<-1> {
        static long GetNParams(unsigned method) {
            return _GetNParams<typename T::endmeths::prev>(method);
        }
        template<typename M>
        static long _GetNParams(unsigned method) {
            if (method == M::val)
                return M::nParam;
            return _GetNParams<typename M::prev>(method);
        }
        template<>
        static long _GetNParams<first_def>(unsigned) {
            return -1;
        }
    };

    template<typename M>
    static bool defVal(const T* ptr, unsigned method, unsigned param, tVariant* value) {
        if (method == M::val) {
            if (M::IsDefVal && param < M::nParam) {
                return M::defVal(ptr, param, value);
            }
            return false;
        }
        return defVal<typename M::prev>(ptr, method, param, value);
    }
    template<>
    static bool defVal<first_def>(const T*, unsigned, unsigned, tVariant*) {
        return false;
    }

    template<typename M>
    static bool callAsProc(T* ptr, unsigned method, tVariant* params, unsigned count) {
        if (method == M::val)
            return !M::IsFunc && (!M::nParam || params != nullptr) && M::callProc(ptr, params, count);
        return callAsProc<typename M::prev>(ptr, method, params, count);
    }
    template<>
    static bool callAsProc<first_def>(T*, unsigned, tVariant*, unsigned) {
        return false;
    }

    template<typename M>
    static bool callAsFunc(T* ptr, unsigned method, tVariant& ret, tVariant* params, unsigned count) {
        if (method == M::val)
            return M::IsFunc && (!M::nParam || params != nullptr) && M::callFunc(ptr, ret, params, count);
        return callAsFunc<typename M::prev>(ptr, method, ret, params, count);
    }
    template<>
    static bool callAsFunc<first_def>(T*, unsigned, tVariant&, tVariant*, unsigned) {
        return false;
    }
};

template<typename T>
struct prop_pol {
    enum {
        bReadMin  = rw_count<T>::read < rw_count<T>::noread ? 1 : 0,
        bWriteMin = rw_count<T>::write < rw_count<T>::nowrite ? 1 : 0,
    };

    static bool IsPropReadable(unsigned prop) {
        return policy<bReadMin>::template isRead<typename T::endprops::prev>(prop);
    }

    static bool IsPropWritable(unsigned prop) {
        return policy<bWriteMin>::template isWrite<typename T::endprops::prev>(prop);
    }

    static bool ReadProp(const T* ptr, unsigned prop, tVariant& val) {
        return readProp<typename T::endprops::prev>(ptr, prop, val);
    }

    static int WriteProp(T* ptr, unsigned prop, const tVariant& val) {
        return writeProp<typename T::endprops::prev>(ptr, prop, val);
    }

    template<bool /*==true*/>
    struct policy {
        template<typename M>
        static bool isRead(unsigned prop) {
            // Читаемых меньше, чем не читаемых
            if constexpr (M::IsRead != 0) {
                if (prop == M::val) {
                    return true;
                }
            }
            return isRead<typename M::prev>(prop);
        }
        template<>
        static bool isRead<first_def>(unsigned) {
            return false;
        }

        template<typename M>
        static bool isWrite(unsigned prop) {
            // Записываемых меньше, чем не записываемых
            if constexpr (M::IsWrite != 0) {
                if (prop == M::val) {
                    return true;
                }
            }
            return isWrite<typename M::prev>(prop);
        }
        template<>
        static bool isWrite<first_def>(unsigned) {
            return false;
        }
    };
    template<>
    struct policy<false> {
        template<typename M>
        static bool isRead(unsigned prop) {
            // Читаемых больше, чем не читаемых
            if constexpr (!M::IsRead) {
                if (prop == M::val) {
                    return false;
                }
            }
            return isRead<typename M::prev>(prop);
        }
        template<>
        static bool isRead<first_def>(unsigned) {
            return true;
        }

        template<typename M>
        static bool isWrite(unsigned prop) {
            // Записываемых больше, чем не записываемых
            if constexpr (!M::IsWrite) {
                if (prop == M::val) {
                    return false;
                }
            }
            return isWrite<typename M::prev>(prop);
        }
        template<>
        static bool isWrite<first_def>(unsigned) {
            return true;
        }
    };

    template<typename M>
    static bool readProp(const T* ptr, unsigned prop, tVariant& val) {
        if (M::IsRead && prop == M::val)
            return M::readProp(ptr, val);
        return readProp<typename M::prev>(ptr, prop, val);
    }
    template<>
    static bool readProp<first_def>(const T*, unsigned, tVariant&) {
        return false;
    }

    template<typename M>
    static bool writeProp(T* ptr, unsigned prop, const tVariant& val) {
        if (M::IsWrite && prop == M::val)
            return M::writeProp(ptr, val);
        return writeProp<typename M::prev>(ptr, prop, val);
    }
    template<>
    static bool writeProp<first_def>(T*, unsigned, const tVariant&) {
        return false;
    }
};

template<typename Impl>
class V8Addin : public AddinBase {
public:
    using ImplType = Impl;
    ImplType& d() {
        return static_cast<ImplType&>(*this);
    }
    const ImplType& d() const {
        return static_cast<const ImplType&>(*this);
    }
    bool init() {
        return true;
    }
    void done() {}

    template<typename P>
    struct end_def {
        using prev = P;
        enum { val = prev::val + 1 };
    };
    // В конечном классе эти типы возможно будут переопределены, указывая на последний метод/свойство
    using endmeths = end_def<first_def>;
    using endprops = end_def<first_def>;

    struct init_all_map {
        init_all_map() {
            fndselector<typename ImplType::endmeths>::type::OnInitFirstObject();
            fndselector<typename ImplType::endprops>::type::OnInitFirstObject();
        }
    };

    V8Addin() {
        static const init_all_map iam;
    }

    bool ADDIN_API Init(void* pConnection) override {
        v8connection_ = reinterpret_cast<IAddInDefBaseEx*>(pConnection);
        return d().init();
    }
    bool error(stru rus, stru eng) {
        return error(selectLocaleStr(rus, eng));
    }
    bool error(auto&& err) {
        lastError_ = std::move(err);
        if (throwErrors_) {
            v8connection_->AddError(ADDIN_E_FAIL, ImplType::ExtensionName.symbols(), lastError_, 0);
        }
        return false;
    }

    void ADDIN_API Done() override {
        d().done();
    }

    bool ADDIN_API RegisterExtensionAs(WCHAR_T** wsExtensionName) override {
        if (wsExtensionName) {
            *wsExtensionName = copyText(ImplType::ExtensionName);
        }
        return true;
    }

    long ADDIN_API GetNProps() override {
        return ImplType::endprops::val;
    }

    long ADDIN_API FindProp(const WCHAR_T* propName) override {
        return propName ? fndselector<typename ImplType::endprops>::type::find(stru{propName}) : -1;
    }

    const WCHAR_T* ADDIN_API GetPropName(long prop, long language) override {
        return getname<typename ImplType::endprops>::type::name(prop, language, this);
    }

    bool ADDIN_API GetPropVal(const long prop, tVariant* value) override {
        return value != nullptr && (unsigned)prop < ImplType::endprops::val && prop_pol<ImplType>::ReadProp(&d(), prop, *value);
    }

    bool ADDIN_API SetPropVal(const long prop, tVariant* value) override {
        return value != nullptr && (unsigned)prop < ImplType::endprops::val && prop_pol<ImplType>::WriteProp(&d(), prop, *value);
    }

    bool ADDIN_API IsPropReadable(const long prop) override {
        return (unsigned)prop < ImplType::endprops::val && prop_pol<ImplType>::IsPropReadable(prop);
    }

    bool ADDIN_API IsPropWritable(const long prop) override {
        return (unsigned)prop < ImplType::endprops::val && prop_pol<ImplType>::IsPropWritable(prop);
    }

    long ADDIN_API GetNMethods() override {
        return ImplType::endmeths::val;
    }

    long ADDIN_API FindMethod(const WCHAR_T* methodName) override {
        return methodName ? fndselector<typename ImplType::endmeths>::type::find(stru{methodName}) : -1;
    }

    const WCHAR_T* ADDIN_API GetMethodName(const long method, const long language) override {
        return getname<typename ImplType::endmeths>::type::name(method, language, this);
    }

    long ADDIN_API GetNParams(const long method) override {
        return (unsigned)method < ImplType::endmeths::val ? meth_pol<ImplType>::GetNParams(method) : -1;
    }

    bool ADDIN_API GetParamDefValue(const long method, const long param, tVariant* value) override {
        if (!value)
            return false;
        value->vt = VTYPE_EMPTY;
        return (unsigned)method >= ImplType::endmeths::val || meth_pol<ImplType>::GetDefVal(&d(), method, param, value);
    }

    bool ADDIN_API HasRetVal(const long method) override {
        return (unsigned)method < ImplType::endmeths::val && meth_pol<ImplType>::HasRetVal(method);
    }

    bool ADDIN_API CallAsProc(const long method, tVariant* params, const long count) override {
        return (unsigned)method < ImplType::endmeths::val && meth_pol<ImplType>::CallAsProc(&d(), method, params, count);
    }

    bool ADDIN_API CallAsFunc(const long method, tVariant* result, tVariant* params, const long count) override {
        return result != nullptr && meth_pol<ImplType>::CallAsFunc(&d(), method, *result, params, count);
    }
};

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)
#define STRINGIZE(x) #x
#define UEFY(x) CONCAT(u, STRINGIZE(x))

#define REGISTER_EXTENSION(eName)                            \
    inline static constexpr ssu ExtensionName{eName};        \
    static IComponentBase* create() {                        \
        return new ImplType;                                 \
    }                                                        \
    inline static AddinInfo registerAddin{eName, create};    \
    template<bool bMeth, int I>                              \
    struct prev_type {                                       \
        using type = typename prev_type<bMeth, I - 1>::type; \
    };                                                       \
    template<>                                               \
    struct prev_type<true, __COUNTER__> {                    \
        using type = first_def;                              \
    };                                                       \
    template<>                                               \
    struct prev_type<false, __COUNTER__> {                   \
        using type = first_def;                              \
    };

#define END_EXTENSION()                                                    \
    using endmeths = end_def<typename prev_type<true, __COUNTER__>::type>; \
    using endprops = end_def<typename prev_type<false, __COUNTER__>::type>

#define BEGIN_DECLARE(parNameE, parNameR, parMeth)              \
    struct parNameE##_def {                                     \
        using prev = prev_type<parMeth, __COUNTER__ - 1>::type; \
        enum { val = prev::val + 1 };                           \
        inline static const stru engName{UEFY(parNameE)};       \
        inline static const stru rusName{parNameR};

#define END_DECLARE(parNameE, parMeth)       \
    }                                        \
    ;                                        \
    friend struct parNameE##_def;            \
    template<>                               \
    struct prev_type<parMeth, __COUNTER__> { \
        using type = parNameE##_def;         \
    };

#define EXT_METHOD_DEF(parNameEng, parNameRus, parNumParam, parCallFunc, parCallProc, parDefProc, parIsFunc, parIsDef) \
    BEGIN_DECLARE(parNameEng, parNameRus, true)                                                                        \
    static int callProc(ImplType* p, tVariant* pp, long s) {                                                           \
        return parCallProc;                                                                                            \
    }                                                                                                                  \
    static int callFunc(ImplType* p, tVariant& r, tVariant* pp, long s) {                                              \
        return parCallFunc;                                                                                            \
    }                                                                                                                  \
    static int defVal(const ImplType* p, long n, tVariant* v) {                                                        \
        return parDefProc;                                                                                             \
    }                                                                                                                  \
    enum { nParam = parNumParam, IsFunc = parIsFunc, IsDefVal = parIsDef };                                            \
    static void test() {                                                                                               \
        check_defval<nParam, IsDefVal> t{};                                                                            \
        (void)t;                                                                                                       \
        test_last<endmeths, val> t2{};                                                                                 \
        (void)t2;                                                                                                      \
    }                                                                                                                  \
    END_DECLARE(parNameEng, true)

#define EXT_DEFVAL_FOR(parNameEng) bool parNameEng##_defVal(long param, tVariant* value) const

#define EXT_PROC_(parNameEng, parNameRus, parNumParam, parDefVal, parIsDef)                                    \
    EXT_METHOD_DEF(parNameEng, parNameRus, parNumParam, false, (p->parNameEng(pp, s)), parDefVal, 0, parIsDef) \
    bool parNameEng(tVariant* params, unsigned count)

#define EXT_FUNC_(parNameEng, parNameRus, parNumParam, parDefVal, parIsDef)                                       \
    EXT_METHOD_DEF(parNameEng, parNameRus, parNumParam, (p->parNameEng(r, pp, s)), false, parDefVal, 1, parIsDef) \
    bool parNameEng(tVariant& retVal, tVariant* params, unsigned count)

// Объявить метод - процедуру контекста, без параметров по умолчанию
#define EXT_PROC(parNameEng, parNameRus, parNumParam) EXT_PROC_(parNameEng, parNameRus, parNumParam, false, 0)

// Объявить метод - процедуру контекста, имеющую параметры по умолчанию
#define EXT_PROC_WITH_DEFVAL(parNameEng, parNameRus, parNumParam) \
    EXT_PROC_(parNameEng, parNameRus, parNumParam, (p->parNameEng##_defVal(n, v)), 1)

// Объявить метод - функцию контекста, без параметров по умолчанию
#define EXT_FUNC(parNameEng, parNameRus, parNumParam) EXT_FUNC_(parNameEng, parNameRus, parNumParam, false, 0)

// Объявить метод - функцию контекста, имеющую параметры по умолчанию
#define EXT_FUNC_WITH_DEFVAL(parNameEng, parNameRus, parNumParam) \
    EXT_FUNC_(parNameEng, parNameRus, parNumParam, (p->parNameEng##_defVal(n, v)), 1)

#define EXT_PROP_(parNameE, parNameR, parGet, parSet, parR, parW) \
    BEGIN_DECLARE(parNameE, parNameR, false)                      \
    static bool readProp(const ImplType* p, tVariant& v) {        \
        return parGet;                                            \
    }                                                             \
    static bool writeProp(ImplType* p, const tVariant& v) {       \
        return parSet;                                            \
    }                                                             \
    enum { IsRead = parR, IsWrite = parW };                       \
    static void test() {                                          \
        test_last<endprops, val> t2{};                            \
        (void)t2;                                                 \
    }                                                             \
    END_DECLARE(parNameE, false)

#define EXT_PROP_READ(parName) bool parName(tVariant& value) const
// Объявить обработчик записи свойства для свойства с чтением/записью
#define EXT_PROP_WRITE(parName) bool set##parName(const tVariant& value)

// Объявить свойство контекста, только для чтения
#define EXT_PROP_RO(parNameEng, parNameRus)                            \
    EXT_PROP_(parNameEng, parNameRus, (p->parNameEng(v)), false, 1, 0) \
    EXT_PROP_READ(parNameEng)

// Объявить свойство контекста, только для записи
#define EXT_PROP_WO(parNameEng, parNameRus)                                 \
    EXT_PROP_(parNameEng, parNameRus, false, (p->set##parNameEng(v)), 0, 1) \
    EXT_PROP_WRITE(parNameEng)

// Объявить свойство контекста, чтение/запись
#define EXT_PROP_RW(parNameEng, parNameRus)                                              \
    EXT_PROP_(parNameEng, parNameRus, (p->parNameEng(v)), (p->set##parNameEng(v)), 1, 1) \
    EXT_PROP_READ(parNameEng)
