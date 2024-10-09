#include "v8sqlite_addin.h"
#include "stdafx.h"
#include <unordered_set>

V8SqliteAddin::V8SqliteAddin() = default;

V8SqliteAddin::~V8SqliteAddin() = default;

bool V8SqliteAddin::OpenDataBase(tVariant* params, unsigned count) {
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Ожидается строковый параметр", u"String parameter needed");
    }
    db_.open(varToTextU(params[0]));
    if (!db_.isOpen()) {
        return reportDbError();
    }
    lastError_.make_empty();
    return true;
}

bool V8SqliteAddin::CloseDataBase(tVariant* params, unsigned count) {
    prepared_.clear();
    db_.close();
    lastError_.make_empty();
    return true;
}

bool V8SqliteAddin::Exec(tVariant* params, unsigned count) {
    if (!db_.isOpen()) {
        return error(u"База данных не открыта", u"Database not opened");
    }
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Ожидается строковый параметр", u"String parameter needed");
    }
    lastError_.make_empty();
    return db_.exec(varToTextU(params[0])) == SQLITE_OK || reportDbError();
}

bool V8SqliteAddin::PrepareQuery(tVariant* params, unsigned count) {
    if (!db_.isOpen()) {
        return error(u"База данных не открыта", u"Database not opened");
    }
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Первый параметр должен быть строкой", u"The first parameter must be a string");
    }
    if (params[1].vt != VTYPE_PWSTR) {
        return error(u"Второй параметр должен быть строкой", u"The second parameter must be a string");
    }
    auto query = db_.prepare(varToTextU(params[1]));
    if (!query.valid()) {
        return reportDbError();
    }
    prepared_.emplace_or_assign(varToTextU(params[0]), std::move(query));
    lastError_.make_empty();
    return true;
}

static bool makeBind(SqliteQuery& query, tVariant& param, unsigned paramNum) {
    switch (param.vt) {
    case VTYPE_EMPTY:
    case VTYPE_NULL:
        query.bind(paramNum, db_null{});
        break;
    case VTYPE_BOOL:
        query.bind(paramNum, param.bVal ? 1 : 0);
        break;
    case VTYPE_I1:
        query.bind(paramNum, param.i8Val);
        break;
    case VTYPE_I2:
        query.bind(paramNum, param.shortVal);
        break;
    case VTYPE_I4:
        query.bind(paramNum, param.intVal);
        break;
    case VTYPE_ERROR:
        query.bind(paramNum, param.errCode);
        break;
    case VTYPE_I8:
        query.bind(paramNum, param.llVal);
        break;
    case VTYPE_UI1:
        query.bind(paramNum, param.ui8Val);
        break;
    case VTYPE_UI2:
        query.bind(paramNum, param.ushortVal);
        break;
    case VTYPE_UI4:
        query.bind(paramNum, param.uintVal);
        break;
    case VTYPE_UI8:
        query.bind(paramNum, param.ullVal);
        break;
    case VTYPE_R4:
        query.bind(paramNum, param.fltVal);
        break;
    case VTYPE_R8:
        query.bind(paramNum, param.dblVal);
        break;
    case VTYPE_PWSTR:
        query.bind(paramNum, (ssu)varToTextU(param));
        break;
    case VTYPE_BLOB:
    case VTYPE_PSTR:
        query.bind(paramNum, (ssa)varToTextA(param));
        break;
    case VTYPE_DATE:
        query.bind(paramNum, lstringu<30>{expr_str_tm{winDateToTm(param.date)}}.toStr());
        break;
    case VTYPE_TM:
        query.bind(paramNum, lstringu<30>{expr_str_tm{param.tmVal}}.toStr());
        break;
    default:
        return false;
    }
    return true;
}

bool V8SqliteAddin::BindParam(tVariant* params, unsigned count) {
    if (!db_.isOpen()) {
        return error(u"База данных не открыта", u"Database not opened");
    }
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Первый параметр должен быть строкой", u"The first parameter must be a string");
    }
    if (params[1].vt != VTYPE_PWSTR && !isInteger(params[1])) {
        return error(u"Второй параметр должен быть строкой или целым числом", u"The second parameter must be a string or an integer");
    }
    auto find = prepared_.find(varToTextU(params[0]));
    if (find == prepared_.end()) {
        return error(u"Запрос с таким именем не найден", u"Query with this name not found");
    }

    SqliteQuery& query = find->second;

    int paramNum = params[1].vt == VTYPE_PWSTR
        ? sqlite3_bind_parameter_index(query, lstringa<100>{varToTextU(params[1])})
        : getInteger(params[1]);

    if (!paramNum || paramNum > sqlite3_bind_parameter_count(query)) {
        return error(u"Неверный параметр запроса", u"Bad query param");
    }

    if (!makeBind(query, params[2], paramNum)) {
        return error(u"Неизвестный тип параметра", u"Bad param type");
    }
    lastError_.make_empty();
    return true;
}

struct ToTextReceiver {
    chunked_string_builder<u16s>& vtText;
    hashStrMapUIU<int>& datesColumns;
    std::vector<char> dates;
    unsigned currentCol{0};
    unsigned colCount{0};
    int error{0};

    void doSetColCount(unsigned cc) {
        colCount = cc;
        dates.resize(colCount, 0);
        for (const auto& [d, _1]: datesColumns) {
            auto [colIdx, err, _2] = d.to_str().toInt<unsigned, true, 10, false>();
            if (err == IntConvertResult::Success && colIdx < colCount) {
                dates[colIdx] = 1;
            }
        }
    }

    void checkColumnForDates(ssu colName) {
        if (datesColumns.find(colName) != datesColumns.end()) {
            dates[currentCol] = 1;
        }
    }

    ToTextReceiver(chunked_string_builder<u16s>& t, hashStrMapUIU<int>& d) : vtText(t), datesColumns(d) {}
};

struct ValueTableReceiver : ToTextReceiver {
    using ToTextReceiver::ToTextReceiver;

    unsigned rowCount{0};
    unsigned startOfRowCount = 0;

    void setColumnCount(unsigned cc) {
        doSetColCount(cc);
        vtText << u"{\"#\",acf6192e-81ca-46ef-93a6-5a6968b78663,{9,{"_ss + cc + u",";
    }

    void addColumnName(ssu name) {
        checkColumnForDates(name);
        if (name.find('\"') != npos) {
            lstringu<200> idName{e_repl(name, u"\"", u"\"\"")};
            vtText << u"{"_ss + currentCol + u",\"" + idName + u"\",{\"Pattern\"},\"" + idName + u"\",0},";
        } else {
            vtText << u"{"_ss + currentCol + u",\"" + name + u"\",{\"Pattern\"},\"" + name + u"\",0},";
        }
        currentCol++;
    }
    enum {SpaceForRowCount = 20};
    void addRow() {
        if (rowCount == 0) {
            vtText << u"},{2,"_ss + colCount + u",";
            for (unsigned i = 0; i < colCount; i++) {
                vtText << eeu + i + u"," + i + u",";
            }
            vtText << u"{1,";
            startOfRowCount = (int)vtText.length();
            vtText << expr_spaces<u16s, SpaceForRowCount>{} + u",";
        } else {
            vtText << u"0},";
        }
        vtText << u"{2,"_ss + rowCount + u"," + currentCol + u",";
        currentCol = 0;
        rowCount++;
    }
    void addNull() {
        vtText << u"{\"L\"},";
        currentCol++;
    }
    void addInteger(int64_t v) {
        vtText << u"{\"N\","_ss + v + u"},";
        currentCol++;
    }
    void addReal(double v) {
        if constexpr (wchar_is_u16) {
            vtText << u"{\"N\","_ss + v + u"},";
        } else {
            vtText << u"{\"N\"," + lstringu<40>{lstringa<40>{eea + v}} + u"},";
        }
        currentCol++;
    }
    void addText(ssu v) {
        if (dates[currentCol] && v.len == 19) {
            vtText << u"{\"D\"," + v(0, 4) + v(5, 2) + v(8, 2) + v(11, 2) + v(14, 2) + v(17, 2) + u"},";
        } else {
            vtText << u"{\"S\",\"" + e_repl(v, u"\"", u"\"\"") + u"\"},";
        }
        currentCol++;
    }
    void addBlob(ssa v) {
        vtText << u"{\"#\",87126200-3e98-44e0-b931-ccb1d7edc497,{1,{#base64:" + expr_str_base64(v) + u"}}},";
        currentCol++;
    }
    void setResult(int e, sqlite3* db) {
        error = e;
        if (SQLITE_DONE == e && colCount) {
            if (!rowCount) {
                vtText << u"},{2,"_ss + colCount + u",";
                for (unsigned i = 0; i < colCount; i++) {
                    vtText << eeu + i + u"," + i + u",";
                }
                vtText << u"{1,0},2,-1},{0,0}}}";
            } else
                vtText << u"0}},1,2},{0,0}}}";
        }
    }

    void fixAnswer(WCHAR_T* answer) const {
        if (rowCount) {
            fromInt(answer + startOfRowCount + SpaceForRowCount, rowCount);
        }
    }
};

struct JsonReceiver : ToTextReceiver {
    using ToTextReceiver::ToTextReceiver;

    void fixAnswer(WCHAR_T* answer) {}

    void setColumnCount(unsigned cc) {
        doSetColCount(cc);
        vtText << uR"({"#type":"jv8:Array","#value":[{"#type":"jv8:Array","#value":[)";
    }

    void addDelim() {
        currentCol++;
        if (currentCol == colCount) {
            vtText << u"]}";
        } else
            vtText << u",";
    }

    void addColumnName(ssu name) {
        checkColumnForDates(name);
        vtText << uR"({"#type":"jxs:string","#value":")" + expr_json_str(name) + u"\"}";
        addDelim();
    }
    void addRow() {
        vtText << uR"(,{"#type":"jv8:Array","#value":[)";
        currentCol = 0;
    }
    void addNull() {
        vtText << uR"({"#type":"jv8:Null","#value":""})";
        addDelim();
    }
    void addInteger(int64_t v) {
        vtText << uR"({"#type":"jxs:decimal","#value":)"_ss + v + u"}";
        addDelim();
    }
    void addReal(double v) {
        if constexpr (wchar_is_u16) {
            vtText << uR"({"#type":"jxs:decimal","#value":)"_ss + v + u"}";
        } else {
            vtText << uR"({"#type":"jxs:decimal","#value":)" + lstringu<40>{lstringa<40>{eea + v}} + u"}";
        }
        addDelim();
    }
    void addText(ssu v) {
        if (dates[currentCol] && v.len == 19) {
            vtText << uR"({"#type":"jxs:dateTime","#value":")" + v(0, 10) + u'T' + v(11) + u"\"}";
        } else {
            vtText << uR"({"#type":"jxs:string","#value":")" + expr_json_str(v) + u"\"}";
        }
        addDelim();
    }
    void addBlob(ssa v) {
        vtText << uR"({"#type":"jxs:base64Binary","#value":")" + expr_str_base64(v) + u"\"}";
        addDelim();
    }
    void setResult(int e, sqlite3* db) {
        error = e;
        if (SQLITE_DONE == e && colCount) {
            vtText << u"]}";
        }
    }
};

template<typename T>
static bool execQuery(SqliteQuery& query, tVariant& retVal, hashStrMapUIU<int>& dates, IMemoryManager* mm) {
    chunked_string_builder<u16s> text;
    T receiver(text, dates);
    query.exec(receiver);
    if (SQLITE_DONE != receiver.error) {
        return false;
    }
    if (receiver.colCount) {
        retVal.vt      = VTYPE_PWSTR;
        retVal.wstrLen = (int)text.length();
        mm->AllocMemory((void**)&retVal.pwstrVal, (int)(text.length() + 1) * 2);

        *text.place(retVal.pwstrVal) = 0;

        receiver.fixAnswer(retVal.pwstrVal);
    }
    return true;
}

bool V8SqliteAddin::ExecQuery(tVariant& retVal, tVariant* params, unsigned count) {
    if (!db_.isOpen()) {
        return error(u"База данных не открыта", u"Database not opened");
    }
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Первый параметр должен быть строкой", u"First param must be string");
    }
    if (params[1].vt != VTYPE_PWSTR) {
        return error(u"Второй параметр должен быть строкой", u"Second parameter must be a string");
    }

    stru resultFormat = varToTextU(params[1]);

    hashStrMapUIU<int> dates;

    if (params[2].vt != VTYPE_NULL) {
        if (params[2].vt == VTYPE_PWSTR) {
            auto vals = varToTextU(params[2]).splitter(u",");
            while (!vals.isDone()) {
                dates.emplace(vals.next().trimmed(), 0);
            }
        } else {
            return error(u"Третий параметр должен быть строкой или отсутствовать", u"Third param must be string or empty");
        }
    }

    SqliteQuery tmpQuery, *query = nullptr;
    auto find = prepared_.find(varToTextU(params[0]));
    if (find == prepared_.end()) {
        tmpQuery = db_.prepare(varToTextU(params[0]));
        if (!tmpQuery.valid())
            return reportDbError();
        query = &tmpQuery;
    } else {
        query = &find->second;
    }

    bool result = false;
    lastError_.make_empty();

    if (resultFormat.compare_ia(u"ValueTable") == 0 || resultFormat.compare_iu(u"ТаблицаЗначений") == 0) {
        result = execQuery<ValueTableReceiver>(*query, retVal, dates, memoryManager_);
    } else if (resultFormat.compare_ia(u"JSON") == 0) {
        result = execQuery<JsonReceiver>(*query, retVal, dates, memoryManager_);
    } else {
        return error(u"Неизвестный формат для результата", u"Unknown result format");
    }
    return result || reportDbError();
}

bool V8SqliteAddin::RemoveQuery(tVariant* params, unsigned count) {
    if (params[0].vt != VTYPE_PWSTR) {
        return error(u"Первый параметр должен быть строкой", u"First param must be string");
    }
    auto find = prepared_.find(varToTextU(params[0]));
    if (find == prepared_.end()) {
        return error(u"Запрос с таким именем не найден", u"No query found for that name");
    }
    prepared_.erase(find);
    lastError_.make_empty();
    return true;
}

bool V8SqliteAddin::reportDbError() {
    return error(selectLocaleStr(u"Произошла ошибка базы данных", u"Database error") + u": " + db_.lastError());
}
