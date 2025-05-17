#pragma once
#include "simstr/sstring.h"
#include "sqlite3.h"
#include <type_traits>
#include <utility>
using namespace simstr;

class SqliteQuery;

template<typename T>
concept QueryResultReceiver = requires(T& t) {
    {t.setColumnCount(0u)};
    {t.addColumnName(ssu{})};
    {t.addRow()};
    {t.addNull()};
    {t.addInteger(int64_t{})};
    {t.addReal(double{})};
    {t.addText(ssu{})};
    {t.addBlob(ssa{})};
    {t.setResult(int{}, (sqlite3*)nullptr)};
};

class SqliteBase {
public:
    SqliteBase() = default;
    ~SqliteBase() {
        close();
    }
    SqliteBase(const SqliteBase&) = delete;
    SqliteBase(SqliteBase&& other) noexcept : db_(other.db_), opened_(other.opened_) {
        other.db_ = nullptr;
        other.opened_ = false;
    }
    SqliteBase& operator=(SqliteBase other) noexcept {
        std::swap(db_, other.db_);
        std::swap(opened_, other.opened_);
        return *this;
    }

    bool isOpen() const {
        return opened_;
    }
    operator sqlite3*() const {
        return db_;
    }

    bool open(stru name);
    void close();
    int exec(stru query);
    stru lastError() const {
        return stru{ db_ ? (const u16s*)sqlite3_errmsg16(db_) : nullptr};
    }
    int64_t lastId() const {
        return opened_ ? sqlite3_last_insert_rowid(db_) : 0;
    }
    int64_t changes() const {
        return opened_ ? sqlite3_changes64(db_) : 0;
    }
    SqliteQuery prepare(stru query);

protected:
    sqlite3* db_{};
    bool opened_{};
};

template<typename T>
struct db_traits;

// Спецификации для установки параметров разными типами данных
template<typename T>
struct db_int_traits {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const T& value) {
        sqlite3_bind_int(stmt, idx, static_cast<int>(value));
    }
};

template<typename T>
struct db_int64_traits {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const T& value) {
        sqlite3_bind_int64(stmt, idx, static_cast<int64_t>(value));
    }
};

template<typename T>
struct db_double_traits {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const T& value) {
        sqlite3_bind_double(stmt, idx, value);
    }
};

template<>
struct db_traits<int8_t> : db_int_traits<int8_t> {};
template<>
struct db_traits<uint8_t> : db_int_traits<uint8_t> {};
template<>
struct db_traits<short> : db_int_traits<short> {};
template<>
struct db_traits<unsigned short> : db_int_traits<unsigned short> {};
template<>
struct db_traits<int> : db_int_traits<int> {};
template<>
struct db_traits<unsigned int> : db_int_traits<unsigned int> {};
template<>
struct db_traits<long> : std::conditional_t<sizeof(long) == 4, db_int_traits<long>, db_int64_traits<long>> {};
template<>
struct db_traits<unsigned long> : std::conditional_t<sizeof(unsigned long) == 4, db_int_traits<unsigned long>, db_int64_traits<unsigned long>> {};
template<>
struct db_traits<long long> : db_int64_traits<long long> {};
template<>
struct db_traits<unsigned long long> : db_int64_traits<unsigned long long> {};
template<>
struct db_traits<double> : db_double_traits<double> {};
template<>
struct db_traits<float> : db_double_traits<float> {};
template<>
struct db_traits<ssa> {
    static void bind(sqlite3_stmt* stmt, unsigned idx, ssa value) {
        sqlite3_bind_blob(stmt, idx, value.str, (int)value.len, SQLITE_TRANSIENT);
    }
};
template<>
struct db_traits<ssu> {
    static void bind(sqlite3_stmt* stmt, unsigned idx, ssu value) {
        sqlite3_bind_text16(stmt, idx, value.str, (int)value.len * 2, SQLITE_TRANSIENT);
    }
};

template<size_t N>
struct db_traits<u8s[N]> {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const u8s (&value)[N]) {
        sqlite3_bind_blob(stmt, idx, value, N - 1, 0);
    }
};

template<size_t N>
struct db_traits<u16s[N]> {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const u16s (&value)[N]) {
        sqlite3_bind_text16(stmt, idx, value, (N - 1) * 2, 0);
    }
};

struct db_null {};
template<>
struct db_traits<db_null> {
    static void bind(sqlite3_stmt* stmt, unsigned idx, const db_null&) {
        sqlite3_bind_null(stmt, idx);
    }
};


class SqliteQuery {
public:
    SqliteQuery() = default;
    SqliteQuery(sqlite3_stmt* stmt) : stmt_(stmt) {}

    ~SqliteQuery() {
        close();
    }

    SqliteQuery(const SqliteQuery&) = delete;

    SqliteQuery(SqliteQuery&& other) noexcept : stmt_(other.stmt_) {
        other.stmt_ = nullptr;
    }
    SqliteQuery& operator=(SqliteQuery other) noexcept {
        std::swap(stmt_, other.stmt_);
        return *this;
    }
    operator sqlite3_stmt*() const {
        return stmt_;
    }

    bool valid() const {
        return stmt_ != nullptr;
    }

    void close() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
            stmt_ = nullptr;
        }
    }

    template<typename T>
    SqliteQuery& bind(int idx, const T& t) {
        db_traits<T>::bind(stmt_, idx, t);
        return *this;
    }
    template<typename T>
    SqliteQuery& bind(stra name, const T& t) {
        db_traits<T>::bind(stmt_, sqlite3_bind_parameter_index(stmt_, name), t);
        return *this;
    }

    template<QueryResultReceiver Q>
    void exec(Q& receiver) {
        if (!stmt_) {
            receiver.setResult(SQLITE_ERROR, nullptr);
            return;
        }
        unsigned colCount = sqlite3_column_count(stmt_);
        int result;
        if (colCount) {
            receiver.setColumnCount(colCount);
            for (unsigned i = 0; i < colCount; i++) {
                receiver.addColumnName(stru{(const u16s*)sqlite3_column_name16(stmt_, i)});
            }
            for (;;) {
                result = sqlite3_step(stmt_);
                if (SQLITE_ROW == result) {
                    receiver.addRow();
                    for (unsigned col = 0; col < colCount; col++) {
                        switch (sqlite3_column_type(stmt_, col)) {
                        case SQLITE_NULL:
                            receiver.addNull();
                            break;
                        case SQLITE_INTEGER:
                            receiver.addInteger(sqlite3_column_int64(stmt_, col));
                            break;
                        case SQLITE_FLOAT:
                            receiver.addReal(sqlite3_column_double(stmt_, col));
                            break;
                        case SQLITE_TEXT:
                            receiver.addText(ssu{(const u16s*)sqlite3_column_text16(stmt_, col), size_t(sqlite3_column_bytes16(stmt_, col) / 2)});
                            break;
                        case SQLITE_BLOB:
                            receiver.addBlob(ssa{(const u8s*)sqlite3_column_blob(stmt_, col), size_t(sqlite3_column_bytes(stmt_, col))});
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
        } else {
            result = sqlite3_step(stmt_);
        }
        sqlite3_reset(stmt_);
        receiver.setResult(result, sqlite3_db_handle(stmt_));
    }
    template<QueryResultReceiver Q, typename ...Args>
    Q exec(Args&&... args) {
        Q receiver(std::forward<Args>(args)...);
        exec(receiver);
        return receiver;
    }
    
protected:
    sqlite3_stmt* stmt_{};
};

double calcJulianDate(tm& dt);
double calcJulianDate(double winDate);
tm winDateToTm(double winDate);

struct expr_json_str {
    using symb_type = u16s;
    ssu text;
    size_t l;
    size_t length() const noexcept {
        return l;
    }
    u16s* place(u16s* ptr) const noexcept;
    expr_json_str(ssu t);
};

struct expr_str_base64 {
    using symb_type = u16s;
    ssa text;
    size_t l;
    size_t length() const noexcept {
        return l;
    }
    u16s* place(u16s* ptr) const noexcept;
    expr_str_base64(ssa t);
};

struct expr_str_tm {
    using symb_type = u16s;
    const tm& t;
    size_t length() const noexcept {
        return 19;
    }
    u16s* place(u16s* ptr) const noexcept;
    expr_str_tm(const tm& _t) : t(_t) {}
};
