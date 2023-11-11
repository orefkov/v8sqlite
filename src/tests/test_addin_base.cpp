#include "addin_base.h"
#include <gtest/gtest.h>
#include <vector>

COREAS_API void* core_as_malloc(size_t count) {
    return malloc(count);
}

COREAS_API void* core_as_realloc(void* ptr, size_t count) {
    return realloc(ptr, count);
}

COREAS_API void core_as_free(void* ptr) {
    free(ptr);
}

/*
* Мок для 1Сного менеджера памяти
*/
struct MemoryManager : IMemoryManager {
    bool ADDIN_API AllocMemory(void** pMemory, unsigned long ulCountByte) override {
        if (pMemory) {
            *pMemory = malloc(ulCountByte);
            return *pMemory != nullptr;
        }
        return false;
    }
    void ADDIN_API FreeMemory(void** pMemory) override {
        if (pMemory && *pMemory) {
            ::free(*pMemory);
        }
    }
    template<typename T>
    void free(const T* ptr) {
        FreeMemory((void**)&ptr);
    }
};

/*
* Простой пустой аддин, для него не нужно END_EXTENSION
* Должен отрабатывать все методы аддина с пустым результатом
*/
struct TestEmptyAddin : public V8Addin<TestEmptyAddin> {
    REGISTER_EXTENSION(u"TestEmptyAddin");
};

/*
* Аддин с одной процедурой, для которой установленно наличие параметров
* по умолчанию, но при этом ноль параметров. Для проверки срабатывания
* статического ассерта на такие ситуации. Не в тестовом окружении это
* не скомпилируется.
*/
struct TestAddinProc1 : public V8Addin<TestAddinProc1> {
    REGISTER_EXTENSION(u"TestAddinProc1");

    EXT_PROC_WITH_DEFVAL(Proc1, u"Процедура1", 0) {
        return true;
    }
    EXT_DEFVAL_FOR(Proc1) {
        return true;
    }

    EXT_PROP_RO(Prop1, u"Свойство1") {
        value.vt = VTYPE_BOOL;
        value.bVal = true;
        return true;
    }

    END_EXTENSION();
};

struct TestAddinProc2 : public V8Addin<TestAddinProc2> {
    REGISTER_EXTENSION(u"TestAddinProc2");

    EXT_PROC_WITH_DEFVAL(Proc1, u"Процедура1", 1) {
        return true;
    }
    EXT_DEFVAL_FOR(Proc1) {
        return true;
    }

    EXT_FUNC(Func1, u"Функция1", 0) {
        return true;
    }

    EXT_PROP_WO(Prop1, u"Свойство1") {
        return true;
    }

    END_EXTENSION();
};

struct TestAddinProc3 : public V8Addin<TestAddinProc3> {
    REGISTER_EXTENSION(u"TestAddinProc3");

    EXT_PROC_WITH_DEFVAL(Proc1, u"Процедура1", 1) {
        return true;
    }
    EXT_DEFVAL_FOR(Proc1) {
        return true;
    }

    EXT_FUNC(Func1, u"Функция1", 1) {
        return true;
    }

    EXT_PROC(Proc3, u"Процедура3", 2) {
        return true;
    }

    END_EXTENSION();
};


/*
* Аддин с одной процедурой, но при этом без завершающего макроса END_EXTENSION
* Для проверки срабатывания статического ассерта на такие ситуации.
* Не в тестовом окружении это не скомпилируется.
*/
struct TestNoEndExtesion : public V8Addin<TestNoEndExtesion> {
public:
    REGISTER_EXTENSION(u"TestNoEndExtesion");

    EXT_PROC(Proc1, u"Процедура1", 1) {
        return true;
    }
};

class TestAddin : public V8Addin<TestAddin> {

public:
    int result{0};
    REGISTER_EXTENSION(u"TestAddin");

    EXT_PROC(TestProc, u"ТестПроцедура", 2) {
        EXPECT_EQ(count, 2u);
        EXPECT_EQ(params[0].vt, VTYPE_BOOL);
        EXPECT_TRUE(params[0].bVal);
        EXPECT_EQ(params[1].vt, VTYPE_UI4);
        EXPECT_EQ(params[1].uintVal, 0xDEADBEEF);
        result = params[1].uintVal;
        return true;
    }

    EXT_PROC_WITH_DEFVAL(TestDefValProc, u"ТестЗначПроц", 1) {
        return true;
    }
    EXT_DEFVAL_FOR(TestDefValProc) {
        if (param == 0) {
            if (value) {
                value->vt = VTYPE_BOOL;
                value->bVal = true;
            }
            return true;
        }
        return false;
    }

    EXT_FUNC(TestFunc, u"ТестФункция", 0) {
        return true;
    }

    EXT_FUNC_WITH_DEFVAL(TestDefValFunc, u"ТестЗначФунк", 2) {
        EXPECT_EQ(count, 2u);
        EXPECT_EQ(params[0].vt, VTYPE_BOOL);
        EXPECT_TRUE(params[0].bVal);
        EXPECT_EQ(params[1].vt, VTYPE_UI4);
        EXPECT_EQ(params[1].uintVal, 0xDEADBEEF);
        retVal.vt = VTYPE_BOOL;
        retVal.bVal = true;
        result      = params[1].uintVal;
        return true;
    }
    EXT_DEFVAL_FOR(TestDefValFunc) {
        if (param == 1) {
            if (value) {
                value->vt   = VTYPE_UI4;
                value->uintVal = 0xDEADBEEF;
            }
            return true;
        }
        return false;
    }

    EXT_PROP_RO(TestPropRead, u"ТестСвойствоЧтение") {
        return true;
    }

    EXT_PROP_WO(TestPropWrite, u"ТестСвойствоЗапись") {
        return true;
    }

    EXT_PROP_RW(TestPropRW, u"ТестСвойствоЧЗ") {
        value.vt   = VTYPE_BOOL;
        value.bVal = true;
        return true;
    }
    EXT_PROP_WRITE(TestPropRW) {
        result = 0xDEADBEEF;
        return true;
    }
    END_EXTENSION();
};

/*
* Проверяем, что все типы аддинов зарегистрировались и сформировали строку описания классов.
* Метод GetClassNames должен возвращать строку с названиями всех возможных типов аддинов,
* разделённых символом '|'
*/
TEST(AddinBase, GetClassNames) {
    hashStrMapU<int> addinNames{
        {u"TestEmptyAddin"_h, 0},
        {u"TestAddin"_h, 0},
        {u"TestAddinProc1"_h, 0},
        {u"TestAddinProc2"_h, 0},
        {u"TestAddinProc3"_h, 0},
        {u"TestNoEndExtesion"_h, 0}};

    auto names = stru{GetClassNames()}.split<std::vector<ssu>>(u"|");

    EXPECT_EQ(names.size(), addinNames.size());

    for (const auto& name : names) {
        auto fnd = addinNames.find(name);
        EXPECT_NE(fnd, addinNames.end());
        addinNames.erase(fnd);
    }
}

void testCreate(stru name) {
    IComponentBase* addin = nullptr;
    EXPECT_TRUE(GetClassObject(name, &addin));
    EXPECT_TRUE(addin);

    MemoryManager mm;
    addin->setMemManager(&mm);

    u16s* addinName;
    addin->RegisterExtensionAs(&addinName);
    EXPECT_EQ(stru{addinName}, name);
    mm.free(addinName);

    addin->Done();
    DestroyObject((IComponentBase**)&addin);
}

/*
* Проверим, что через GetClassObject действительно создаются объекты
* и что они нужного типа
*/
TEST(AddinBase, GetClassObject) {
    testCreate(u"TestEmptyAddin");
    testCreate(u"TestAddin");
}

// Проверка, что срабатывает статический ассерт, если забыли END_EXTENSION
TEST(AddinBase, CheckEndExtesion) {
    EXPECT_THROW(TestNoEndExtesion::Proc1_def::test(), std::logic_error);
}

// Проверка, что срабатывает статический ассерт, если забыли в методе с параметрами по умолчанию 0 параметров
TEST(AddinBase, CheckDefvalWithZeroParams) {
    EXPECT_THROW(TestAddinProc1::Proc1_def::test(), std::logic_error);
}

// Проверка количества методов
TEST(AddinBase, GetNMethods) {
    TestEmptyAddin te;
    EXPECT_EQ(te.GetNMethods(), 0);

    TestAddinProc1 t1;
    EXPECT_EQ(t1.GetNMethods(), 1);

    TestAddinProc2 t2;
    EXPECT_EQ(t2.GetNMethods(), 2);

    TestAddinProc3 t3;
    EXPECT_EQ(t3.GetNMethods(), 3);

    TestAddin ta;
    EXPECT_EQ(ta.GetNMethods(), 4);
}

// Проверка поиска методов
TEST(AddinBase, FindMethod) {
    EXPECT_EQ(TestEmptyAddin{}.FindMethod(u"Proc1"), -1);
    EXPECT_EQ(TestAddinProc1{}.FindMethod(u"Noproc"), -1);
    EXPECT_EQ(TestAddinProc1{}.FindMethod(u"proc1"), 0);
    EXPECT_EQ(TestAddinProc1{}.FindMethod(u"процедура1"), 0);
    EXPECT_EQ(TestAddinProc2{}.FindMethod(u"процедура1"), 0);
    EXPECT_EQ(TestAddinProc2{}.FindMethod(u"Func1"), 1);
    EXPECT_EQ(TestAddinProc3{}.FindMethod(u"процЕдура3"), 2);
    EXPECT_EQ(TestAddinProc3{}.FindMethod(u"---"), -1);
    EXPECT_EQ(TestAddin{}.FindMethod(u"---"), -1);
    EXPECT_EQ(TestAddin{}.FindMethod(u"ТестФункция"), 2);
    EXPECT_EQ(TestAddin{}.FindMethod(u"testdefvalfunc"), 3);
    EXPECT_EQ(TestAddin{}.FindMethod(nullptr), -1);
}

template<typename T>
void checkParamsCount(stru name, int metNum, int result) {
    T addin;
    int method = addin.FindMethod(name);
    EXPECT_EQ(method,metNum);
    EXPECT_EQ(addin.GetNParams(method), result);
}

// Проверка количества параметров
TEST(AddinBase, GetNParams) {
    checkParamsCount<TestAddinProc1>(u"proc1", 0, 0);
    checkParamsCount<TestAddinProc2>(u"proc1", 0, 1);
    checkParamsCount<TestAddinProc2>(u"func1", 1, 0);
    checkParamsCount<TestAddinProc3>(u"proc3", 2, 2);
    checkParamsCount<TestAddin>(u"TestDefValFunc", 3, 2);
    checkParamsCount<TestAddin>(u"---", -1, -1);
    EXPECT_EQ(TestAddin{}.GetNParams(100), -1);
}

// Проверка HasRetVal
TEST(AddinBase, HasRetVal) {
    EXPECT_EQ(TestAddinProc1{}.HasRetVal(0), false);
    EXPECT_EQ(TestAddinProc2{}.HasRetVal(0), false);
    EXPECT_EQ(TestAddinProc2{}.HasRetVal(1), true);

    EXPECT_EQ(TestAddinProc3{}.HasRetVal(0), false);
    EXPECT_EQ(TestAddinProc3{}.HasRetVal(1), true);
    EXPECT_EQ(TestAddinProc3{}.HasRetVal(2), false);

    EXPECT_EQ(TestAddin{}.HasRetVal(0), false);
    EXPECT_EQ(TestAddin{}.HasRetVal(2), true);
    EXPECT_EQ(TestAddin{}.HasRetVal(3), true);
}

// Проверка GetMethodName
TEST(AddinBase, GetMethodName) {
    MemoryManager mm;
    TestAddinProc1 a1;
    a1.setMemManager(&mm);
    EXPECT_EQ(stru{a1.GetMethodName(0, 0)}, u"Proc1");
    EXPECT_EQ(stru{a1.GetMethodName(0, 1)}, u"Процедура1");
    EXPECT_EQ(TestAddinProc1{}.GetMethodName(0, 2), nullptr);
    EXPECT_EQ(TestAddinProc1{}.GetMethodName(1, 1), nullptr);
    TestAddin ta;
    ta.setMemManager(&mm);
    EXPECT_EQ(stru{ta.GetMethodName(1, 0)}, u"TestDefValProc");
    EXPECT_EQ(stru{ta.GetMethodName(1, 1)}, u"ТестЗначПроц");
}

// Проверка GetParamDefValue
TEST(AddinBase, GetParamDefValue) {
    TestAddin addin;
    EXPECT_FALSE(addin.GetParamDefValue(1, 0, nullptr));
    EXPECT_FALSE(addin.GetParamDefValue(10, 0, nullptr));
    EXPECT_FALSE(addin.GetParamDefValue(0, 1, nullptr));

    tVariant param;
    EXPECT_TRUE(addin.GetParamDefValue(1, 0, &param));
    EXPECT_EQ(param.vt, VTYPE_BOOL);
    EXPECT_TRUE(param.bVal);

    EXPECT_FALSE(addin.GetParamDefValue(3, 0, &param));
    EXPECT_FALSE(addin.GetParamDefValue(3, 2, &param));
    EXPECT_TRUE(addin.GetParamDefValue(3, 1, &param));
    EXPECT_EQ(param.vt, VTYPE_UI4);
    EXPECT_EQ(param.uintVal, 0xDEADBEEF);
}

// Проверка CallAsProc
TEST(AddinBase, CallAsProc) {
    TestAddin addin;
    // Вызов процедуры с пустыми параметрами
    EXPECT_FALSE(addin.CallAsProc(0, nullptr, 2));
    tVariant params[2];
    // Вызов несуществующей процедуры
    EXPECT_FALSE(addin.CallAsProc(20, params, 2));
    // Вызов функции как процедуры
    EXPECT_FALSE(addin.CallAsProc(2, params, 2));

    params[0].vt = VTYPE_BOOL;
    params[0].bVal = true;
    params[1].vt = VTYPE_UI4;
    params[1].uintVal = 0xDEADBEEF;

    EXPECT_TRUE(addin.CallAsProc(0, params, 2));
    EXPECT_EQ(addin.result, 0xDEADBEEF);
}

// Проверка CallAsFunc
TEST(AddinBase, CallAsFunc) {
    TestAddin addin;
    tVariant params[2], retVal;
    // Вызов функции с пустыми параметрами
    EXPECT_FALSE(addin.CallAsFunc(3, &retVal, nullptr, 2));
    // Вызов функции с пустым возвращаемым значением
    EXPECT_FALSE(addin.CallAsFunc(3, nullptr, params, 2));
    // Вызов несуществующей функции
    EXPECT_FALSE(addin.CallAsFunc(20, &retVal, params, 2));
    // Вызов процедуры как функции
    EXPECT_FALSE(addin.CallAsFunc(0, &retVal, params, 2));

    params[0].vt      = VTYPE_BOOL;
    params[0].bVal    = true;
    params[1].vt      = VTYPE_UI4;
    params[1].uintVal = 0xDEADBEEF;
    retVal.vt         = VTYPE_EMPTY;

    EXPECT_TRUE(addin.CallAsFunc(3, &retVal, params, 2));
    EXPECT_EQ(addin.result, 0xDEADBEEF);

    EXPECT_EQ(retVal.vt, VTYPE_BOOL);
    EXPECT_TRUE(retVal.bVal);
}

// Проверка GetNProps
TEST(AddinBase, GetNProps) {
    EXPECT_EQ(TestEmptyAddin{}.GetNProps(), 0);
    EXPECT_EQ(TestAddinProc1{}.GetNProps(), 1);
    EXPECT_EQ(TestAddinProc2{}.GetNProps(), 1);
    EXPECT_EQ(TestAddinProc3{}.GetNProps(), 0);
    EXPECT_EQ(TestAddin{}.GetNProps(), 3);
}

// Проверка FindProp
TEST(AddinBase, FindProp) {
    EXPECT_EQ(TestEmptyAddin{}.FindProp(u"Prop"), -1);
    EXPECT_EQ(TestAddinProc1{}.FindProp(u"prop1"), 0);
    EXPECT_EQ(TestAddinProc1{}.FindProp(u"свойство1"), 0);
    EXPECT_EQ(TestAddinProc2{}.FindProp(u"prop1"), 0);
    EXPECT_EQ(TestAddinProc2{}.FindProp(u"свойство1"), 0);
    EXPECT_EQ(TestAddin{}.FindProp(u"TestPropRead"), 0);
    EXPECT_EQ(TestAddin{}.FindProp(u"TestPropRW"), 2);
    EXPECT_EQ(TestAddin{}.FindProp(nullptr), -1);
}

// Проверка GetPropName
TEST(AddinBase, GetPropName) {
    MemoryManager mm;
    EXPECT_EQ(TestEmptyAddin{}.GetPropName(0, 0), nullptr);
    TestAddinProc1 t1;
    t1.setMemManager(&mm);
    EXPECT_EQ(stru{t1.GetPropName(0, 0)}, u"Prop1");
    EXPECT_EQ(stru{t1.GetPropName(0, 1)}, u"Свойство1");
    EXPECT_EQ(TestAddinProc1{}.GetPropName(0, 2), nullptr);
    EXPECT_EQ(TestAddinProc1{}.GetPropName(10, 1), nullptr);
    TestAddin ta;
    ta.setMemManager(&mm);
    EXPECT_EQ(stru{ta.GetPropName(1, 0)}, u"TestPropWrite");
    EXPECT_EQ(stru{ta.GetPropName(1, 1)}, u"ТестСвойствоЗапись");
}

// Проверка IsPropReadable
TEST(AddinBase, IsPropReadable) {
    EXPECT_FALSE(TestEmptyAddin{}.IsPropReadable(0));
    EXPECT_TRUE(TestAddinProc1{}.IsPropReadable(0));
    EXPECT_FALSE(TestAddinProc1{}.IsPropReadable(1));
    EXPECT_TRUE(TestAddin{}.IsPropReadable(0));
    EXPECT_FALSE(TestAddin{}.IsPropReadable(1));
    EXPECT_TRUE(TestAddin{}.IsPropReadable(2));
}

// Проверка IsPropWritable
TEST(AddinBase, IsPropWritable) {
    EXPECT_FALSE(TestEmptyAddin{}.IsPropWritable(0));
    EXPECT_FALSE(TestAddinProc1{}.IsPropWritable(0));
    EXPECT_FALSE(TestAddinProc1{}.IsPropWritable(1));
    EXPECT_FALSE(TestAddin{}.IsPropWritable(0));
    EXPECT_TRUE(TestAddin{}.IsPropWritable(1));
    EXPECT_TRUE(TestAddin{}.IsPropWritable(2));
}

// Проверка GetPropVal
TEST(AddinBase, GetPropVal) {
    tVariant prop;
    prop.vt = VTYPE_EMPTY;
    EXPECT_FALSE(TestEmptyAddin{}.GetPropVal(0, &prop));
    EXPECT_FALSE(TestEmptyAddin{}.GetPropVal(0, nullptr));

    EXPECT_FALSE(TestAddinProc1{}.GetPropVal(0, nullptr));
    EXPECT_TRUE(TestAddinProc1{}.GetPropVal(0, &prop));
    EXPECT_EQ(prop.vt, VTYPE_BOOL);
    EXPECT_TRUE(prop.bVal);

    EXPECT_FALSE(TestAddinProc2{}.GetPropVal(0, &prop));
    prop.vt = VTYPE_EMPTY;
    EXPECT_TRUE(TestAddin{}.GetPropVal(2, &prop));
    EXPECT_EQ(prop.vt, VTYPE_BOOL);
    EXPECT_TRUE(prop.bVal);
}

// Проверка SetPropVal
TEST(AddinBase, SetPropVal) {
    tVariant prop;
    prop.vt = VTYPE_EMPTY;
    EXPECT_FALSE(TestEmptyAddin{}.SetPropVal(0, &prop));
    EXPECT_FALSE(TestEmptyAddin{}.SetPropVal(0, nullptr));

    EXPECT_FALSE(TestAddinProc1{}.SetPropVal(0, nullptr));
    EXPECT_FALSE(TestAddinProc1{}.SetPropVal(0, &prop));

    EXPECT_TRUE(TestAddinProc2{}.SetPropVal(0, &prop));

    TestAddin ta;

    EXPECT_TRUE(ta.SetPropVal(2, &prop));
    EXPECT_EQ(ta.result, 0xDEADBEEF);
}
