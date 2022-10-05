#include "../sqlite.h"
#include<gtest/gtest.h>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <sysinfoapi.h>
#include <OleAuto.h>
#endif


TEST(Sqlite, CreateBase) {
    SqliteBase base;
    EXPECT_FALSE(base.isOpen());
    EXPECT_EQ(base.lastError(), u"");
    EXPECT_EQ(base.changes(), 0ll);
    EXPECT_EQ(base.lastId(), 0ll);

    EXPECT_TRUE(base.open(u":memory:"));
    EXPECT_TRUE(base.isOpen());
    base.close();
    EXPECT_FALSE(base.isOpen());
}

TEST(Sqlite, ErrorCreateBase) {
    SqliteBase base;
#ifdef _WIN32
    EXPECT_FALSE(base.open(u"c:\\nul\\com"));
#else
    EXPECT_FALSE(base.open(u"/sys"));
#endif
    EXPECT_EQ(base.lastError(), u"unable to open database file");
    EXPECT_FALSE(base.isOpen());
}

TEST(Sqlite, Exec) {
    SqliteBase base;
    base.open(u":memory:");
    EXPECT_NE(base.exec(u"create table test(a, b, c"), SQLITE_OK);
    EXPECT_EQ(base.lastError(), u"incomplete input");
    EXPECT_EQ(base.exec(u"create table test(a, b, c)"), SQLITE_OK);
    EXPECT_EQ(base.lastError(), u"not an error");
    EXPECT_EQ(base.changes(), 0ll);
    EXPECT_EQ(base.lastId(), 0ll);
    EXPECT_EQ(base.exec(u"insert into test values(1, 2, 3)"), SQLITE_OK);
    EXPECT_EQ(base.lastError(), u"not an error");
    EXPECT_EQ(base.changes(), 1ll);
    EXPECT_EQ(base.lastId(), 1ll);
}

TEST(Sqlite, Prepare) {
    SqliteBase base;
    base.open(u":memory:");
    EXPECT_EQ(base.exec(u"create table test(a, b, c)"), SQLITE_OK);
    auto query = base.prepare(u"select * from test");
    EXPECT_TRUE(query.valid());
}

struct SimpleResultReceiver {
    std::vector<stringu> columns;
    int64_t lastId_{0};
    int64_t changes_{0};
    int error_{0};
    stringu errMessage_;

    void setColumnCount(unsigned cc) {
        columns.reserve(cc);
    }
    void addColumnName(ssu name) {
        columns.emplace_back(name);
    }
    void setResult(int e, sqlite3* db) {
        error_ = e;
        if (db) {
            lastId_     = sqlite3_last_insert_rowid(db);
            changes_    = sqlite3_changes64(db);
            errMessage_ = stru{(const u16symbol*)sqlite3_errmsg16(db)};
        }
    }
    struct value {
        enum class Type {
            Null, Int, Real, Text, Blob
        } type;
        value() : type(Type::Null) {}
        value(int64_t i) : type(Type::Int), iVal(i) {}
        value(double i) : type(Type::Real), dVal(i) {}
        value(ssa i) : type(Type::Blob) {
            new (bVal) stringa(i);
        }
        value(ssu i) : type(Type::Text) {
            new (tVal) stringu(i);
        }
        ~value() {
            if (type == Type::Text) {
                text().~stringu();
            } else if (type == Type::Blob) {
                blob().~stringa();
            }
        }
        value(const value&) = delete;
        value(value&& other) {
            memcpy(this, &other, sizeof(*this));
            other.type = Type::Null;
        }
        value& operator=(value other) {
            this->~value();
            new (this) value(std::move(other));
            return *this;
        }

        stringa& blob() {
            return *(stringa*)bVal;
        }
        stringu& text() {
            return *(stringu*)tVal;
        }
        union {
            int64_t iVal;
            double dVal;
            char tVal[sizeof(stringu)];
            char bVal[sizeof(stringa)];
        };
    };
    std::vector<std::vector<value>> rows;

    void addRow() {
        rows.emplace_back();
    }
    void addNull() {
        rows.back().emplace_back();
    }
    void addInteger(int64_t v) {
        rows.back().emplace_back(v);
    }
    void addReal(double v) {
        rows.back().emplace_back(v);
    }
    void addText(ssu v) {
        rows.back().emplace_back(v);
    }
    void addBlob(ssa v){
        rows.back().emplace_back(v);
    }
};

TEST(Sqlite, ExecPrepared) {
    SqliteBase base;
    base.open(u":memory:");
    EXPECT_EQ(base.exec(u"create table test(a, b, c)"), SQLITE_OK);
    EXPECT_EQ(base.exec(u"insert into test values(1, 2.0, 'text')"), SQLITE_OK);
    EXPECT_EQ(base.exec(u"insert into test values(null, x'4142430041', '3')"), SQLITE_OK);
    auto query = base.prepare(u"select * from test");
    EXPECT_TRUE(query.valid());

    SimpleResultReceiver sr;
    query.exec(sr);

    EXPECT_EQ(sr.columns.size(), 3u);
    EXPECT_EQ(sr.rows.size(), 2u);
    EXPECT_EQ(sr.rows[0][0].type, SimpleResultReceiver::value::Type::Int);
    EXPECT_EQ(sr.rows[0][0].iVal, 1);
    EXPECT_EQ(sr.rows[0][1].type, SimpleResultReceiver::value::Type::Real);
    EXPECT_EQ(sr.rows[0][1].dVal, 2.0);
    EXPECT_EQ(sr.rows[0][2].type, SimpleResultReceiver::value::Type::Text);
    EXPECT_EQ(sr.rows[0][2].text(), u"text");
    EXPECT_EQ(sr.rows[1][0].type, SimpleResultReceiver::value::Type::Null);
    EXPECT_EQ(sr.rows[1][1].type, SimpleResultReceiver::value::Type::Blob);
    EXPECT_EQ(sr.rows[1][1].blob(), "ABC\0A");
    EXPECT_EQ(sr.rows[1][2].type, SimpleResultReceiver::value::Type::Text);
    EXPECT_EQ(sr.rows[1][2].text(), u"3");
}

TEST(Sqlite, ExecPreparedBind) {
    SqliteBase base;
    base.open(u":memory:");
    EXPECT_EQ(base.exec(u"create table test(a, b, c)"), SQLITE_OK);
    auto query = base.prepare(u"insert into test values(?,?,?)");
    EXPECT_TRUE(query.valid());
    SimpleResultReceiver srInsert;
    query.bind(1, 1).bind(2, 2.0).bind(3, u"text").exec(srInsert);
    EXPECT_EQ(srInsert.error_, SQLITE_DONE);
    EXPECT_EQ(srInsert.changes_, 1ll);
    EXPECT_EQ(srInsert.lastId_, 1ll);
    query.bind(1, db_null{}).bind(2, "ABC\0A").bind(3, u"3").exec(srInsert);
    EXPECT_EQ(srInsert.error_, SQLITE_DONE);
    EXPECT_EQ(srInsert.changes_, 1ll);
    EXPECT_EQ(srInsert.lastId_, 2ll);

    query = base.prepare(u"select * from test");
    EXPECT_TRUE(query.valid());

    SimpleResultReceiver sr;
    query.exec(sr);

    EXPECT_EQ(sr.columns.size(), 3u);
    EXPECT_EQ(sr.rows.size(), 2u);
    EXPECT_EQ(sr.rows[0][0].type, SimpleResultReceiver::value::Type::Int);
    EXPECT_EQ(sr.rows[0][0].iVal, 1);
    EXPECT_EQ(sr.rows[0][1].type, SimpleResultReceiver::value::Type::Real);
    EXPECT_EQ(sr.rows[0][1].dVal, 2.0);
    EXPECT_EQ(sr.rows[0][2].type, SimpleResultReceiver::value::Type::Text);
    EXPECT_EQ(sr.rows[0][2].text(), u"text");
    EXPECT_EQ(sr.rows[1][0].type, SimpleResultReceiver::value::Type::Null);
    EXPECT_EQ(sr.rows[1][1].type, SimpleResultReceiver::value::Type::Blob);
    EXPECT_EQ(sr.rows[1][1].blob(), "ABC\0A");
    EXPECT_EQ(sr.rows[1][2].type, SimpleResultReceiver::value::Type::Text);
    EXPECT_EQ(sr.rows[1][2].text(), u"3");
}

TEST(Sqlite, JulianDay) {
    SqliteBase base;
    base.open(u":memory:");
    auto query = base.prepare(u"select 0.0 + strftime('%J', 'now')");
    SimpleResultReceiver sr;
    query.exec(sr);

    auto _t   = time(0);
    tm t      = *gmtime(&_t);
    double jd = calcJulianDate(t);

#ifdef _WIN32
    SYSTEMTIME systm;
    GetSystemTime(&systm);
    double winTime;
    SystemTimeToVariantTime(&systm, &winTime);
    tm newT = winDateToTm(winTime);
    EXPECT_EQ(systm.wYear - 1900, newT.tm_year);
    EXPECT_EQ(systm.wMonth - 1, newT.tm_mon);
    EXPECT_EQ(systm.wDay, newT.tm_mday);
    EXPECT_EQ(systm.wHour, newT.tm_hour);
    EXPECT_EQ(systm.wMinute, newT.tm_min);
    //EXPECT_EQ(systm.wSecond, newT.tm_sec);

    winTime = calcJulianDate(winTime);
    EXPECT_LT(fabs(winTime - jd), 0.001);

#endif

    EXPECT_EQ(sr.rows.size(), 1u);
    EXPECT_EQ(sr.rows[0][0].type, SimpleResultReceiver::value::Type::Real);
    EXPECT_LT(fabs(sr.rows[0][0].dVal - jd), 0.001);


}

TEST(Sqlite, JsonBase64) {
    EXPECT_EQ(lstringu<20>{eeu & expr_str_base64("a")}, u"YQ==");
    EXPECT_EQ(lstringu<20>{eeu & expr_str_base64("ab")}, u"YWI=");
    EXPECT_EQ(lstringu<20>{eeu & expr_str_base64("abc")}, u"YWJj");
    lstringu<20> t{eeu & expr_str_base64("ABC")};
    EXPECT_EQ(t, u"QUJD");
    
    EXPECT_EQ(lstringu<20>{eeu & expr_json_str(u"abc\n\"\r")}, u"abc\\n\\\"\\r");
}

TEST(Sqlite, Unicode) {
    SqliteBase base;
    base.open(u":memory:");
    {
        SimpleResultReceiver srr;
        base.prepare(u"select upper('РусскийТекст')").exec(srr);
        EXPECT_EQ(srr.rows[0][0].type, SimpleResultReceiver::value::Type::Text);
        EXPECT_EQ(srr.rows[0][0].text(), u"РУССКИЙТЕКСТ");
    }
    {
        SimpleResultReceiver srr;
        base.prepare(u"select lower('РусскийТекст')").exec(srr);
        EXPECT_EQ(srr.rows[0][0].type, SimpleResultReceiver::value::Type::Text);
        EXPECT_EQ(srr.rows[0][0].text(), u"русскийтекст");
    }
    {
        SimpleResultReceiver srr;
        base.prepare(u"select 'РусскийТекст' collate nocase ='рУсскИЙтекст'").exec(srr);
        EXPECT_EQ(srr.rows[0][0].type, SimpleResultReceiver::value::Type::Int);
        EXPECT_EQ(srr.rows[0][0].iVal, 1);
    }
    {
        SimpleResultReceiver srr;
        base.prepare(u"select unaccent('āăąēîïĩíĝġńñšŝśûůŷё')").exec(srr);
        EXPECT_EQ(srr.rows[0][0].type, SimpleResultReceiver::value::Type::Text);
        stringu tt = srr.rows[0][0].text();
        EXPECT_EQ(srr.rows[0][0].text(), u"aaaeiiiiggnnsssuuyе");
    }
}
