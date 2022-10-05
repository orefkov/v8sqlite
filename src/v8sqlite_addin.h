#pragma once
#include "addin_base.h"
#include "sqlite.h"
#include "version.h"

#define ADDIN_NAME u"v8sqlite"

class V8SqliteAddin : public V8Addin<V8SqliteAddin> {
protected:

    SqliteBase db_;
    hashStrMapUIU<SqliteQuery> prepared_;

    bool reportDbError();

public:
    V8SqliteAddin();
    virtual ~V8SqliteAddin();

    REGISTER_EXTENSION(ADDIN_NAME);
    EXT_PROP_RO(Version, u"Версия") {
        lstringu<40> v{ssa{P_VERSION}};
        value.vt = VTYPE_PWSTR;
        value.pwstrVal = copyText(v);
        value.wstrLen  = v.length();
        return true;
    }

    EXT_PROP_RW(ThrowErrorDescription, u"БросатьОписаниеОшибки") {
        value.vt = VTYPE_BOOL;
        value.bVal = throwErrors_;
        return true;
    }
    EXT_PROP_WRITE(ThrowErrorDescription) {
        if (value.vt == VTYPE_BOOL) {
            throwErrors_ = value.bVal;
            return true;
        }
        return error(u"Ожидается булево значение", u"Boolean value needed");
    }

    EXT_PROP_RO(ErrorDescription, u"ОписаниеОшибки") {
        value.vt       = VTYPE_PWSTR;
        value.pwstrVal = copyText(lastError_);
        value.wstrLen  = (int)lastError_.length();
        return true;
    }

    EXT_PROP_RO(isDataBaseOpen, u"БазаДанныхОткрыта") {
        value.vt   = VTYPE_BOOL;
        value.bVal = db_.isOpen();
        return true;
    }
    EXT_PROP_RO(LastId, u"ПоследнийИд") {
        // 1С почему-то не может принять число VTYPE_I8;
        int64_t lastId = db_.lastId();
        if (lastId > INT_MAX || lastId < INT_MIN ) {
            value.vt = VTYPE_R8;
            value.dblVal = (double)lastId;
        } else {
            value.vt     = VTYPE_I4;
            value.intVal = (int)lastId;
        }
        return true;
    }
    EXT_PROP_RO(Changes, u"СтрокИзменено") {
        // 1С почему-то не может принять число VTYPE_I8;
        int64_t c = db_.changes();
        if (c > INT_MAX || c < INT_MIN) {
            value.vt     = VTYPE_R8;
            value.dblVal = (double)c;
        } else {
            value.vt     = VTYPE_I4;
            value.intVal = (int)c;
        }
        return true;
    }

    EXT_PROC(OpenDataBase, u"ОткрытьБазуДанных", 1);
    EXT_PROC(CloseDataBase, u"ЗакрытьБазуДанных", 0);
    EXT_PROC(Exec, u"Выполнить", 1);
    EXT_PROC(PrepareQuery, u"ПодготовитьЗапрос", 2);
    EXT_PROC_WITH_DEFVAL(BindParam, u"УстановитьПараметр", 3);
    EXT_DEFVAL_FOR(BindParam) {
        if (param >= 2) {
            if (value) {
                value->vt = VTYPE_NULL;
            }
        }
        return true;
    }
    EXT_FUNC_WITH_DEFVAL(ExecQuery, u"ВыполнитьЗапрос", 3);
    EXT_DEFVAL_FOR(ExecQuery) {
        if (param >= 2) {
            if (value) {
                value->vt = VTYPE_NULL;
            }
        }
        return true;
    }
    EXT_PROC(RemoveQuery, u"УдалитьЗапрос", 1);

    END_EXTENSION();
};
