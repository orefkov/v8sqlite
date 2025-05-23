﻿#include "stdafx.h"
#include "sqlite.h"

extern "C" int sqlite3_unicode_load();

bool SqliteBase::open(stru name) {
    static struct SqliteUnicodeInit {
        SqliteUnicodeInit() {
            sqlite3_unicode_load();
        }
    } unicodeInit;

    if (db_) {
        close();
    }
    return opened_ = SQLITE_OK == sqlite3_open16(name, &db_);
}

void SqliteBase::close() {
    if (db_) {
        sqlite3_close_v2(db_);
        db_     = nullptr;
        opened_ = false;
    }
}

int SqliteBase::exec(stru query) {
    return opened_ ? sqlite3_exec(db_, lstringa<4096>{query}, nullptr, nullptr, nullptr) : SQLITE_ERROR;
}

SqliteQuery SqliteBase::prepare(stru query) {
    sqlite3_stmt* stmt = nullptr;
    if (opened_) {
        sqlite3_prepare16_v3(db_, (void*)query.str, (int)query.length() * 2, 0, &stmt, nullptr);
    }
    return SqliteQuery(stmt);
}

double calcJulianDate(tm& dt) {
    int Y = dt.tm_year + 1900, M = dt.tm_mon + 1, D = dt.tm_mday, A, B, X1, X2;

    if (M <= 2) {
        Y--;
        M += 12;
    }

    A  = Y / 100;
    B  = 2 - A + (A / 4);
    X1 = 36525 * (Y + 4716) / 100;
    X2 = 306001 * (M + 1) / 10000;

    double result = X1 + X2 + D + B + (dt.tm_hour * 3600000 + dt.tm_min * 60000 + dt.tm_sec * 1000) / 86400000.0 - 1524.5;
    return result;
}

double calcJulianDate(double winDate) {
    return winDate + 2415018.5;
}

static void computeYMD(double jd, tm& p) {
    int64_t iJD = int64_t(jd * 86400000);
    int Z, A, B, C, D, E, X1;
    Z    = (int)((iJD + 43200000) / 86400000);
    A    = (int)((Z - 1867216.25) / 36524.25);
    A    = Z + 1 + A - (A / 4);
    B    = A + 1524;
    C    = (int)((B - 122.1) / 365.25);
    D    = (36525 * (C & 32767)) / 100;
    E    = (int)((B - D) / 30.6001);
    X1   = (int)(30.6001 * E);
    p.tm_mday = B - D - X1;
    p.tm_mon = (E < 14 ? E - 1 : E - 13) - 1;
    p.tm_year = (p.tm_mon > 2 ? C - 4716 : C - 4715) - 1900;
    jd += 0.5;
    int s = int((jd - (int)jd) * 86400.0);
    p.tm_hour = s / 3600;
    s         = s % 3600;
    p.tm_min  = s / 60;
    p.tm_sec  = s % 60;
}

tm winDateToTm(double winDate) {
    tm t;
    computeYMD(calcJulianDate(winDate), t);
    return t;
}

expr_json_str::expr_json_str(ssu t) : text(t) {
    const u16s* ptr = text.symbols();
    size_t add = 0;

    for (size_t i = 0; i < text.length(); i++) {
        switch (*ptr++) {
        case '\b':
        case '\f':
        case '\r':
        case '\n':
        case '\t':
        case '\"':
        case '\\':
            add++;
        }
    }
    l = text.len + add;
}

simstr::u16s* expr_json_str::place(u16s* ptr) const noexcept {
    const u16s *r = text.symbols();
    size_t lenOfText = text.length(), lenOfTail = l;
    while (lenOfTail > lenOfText) {
        u16s s = *r++;
        switch (s) {
        case '\b':
            *ptr++ = '\\';
            *ptr++ = 'b';
            lenOfTail--;
            break;
        case '\f':
            *ptr++ = '\\';
            *ptr++ = 'f';
            lenOfTail--;
            break;
        case '\r':
            *ptr++ = '\\';
            *ptr++ = 'r';
            lenOfTail--;
            break;
        case '\n':
            *ptr++ = '\\';
            *ptr++ = 'n';
            lenOfTail--;
            break;
        case '\t':
            *ptr++ = '\\';
            *ptr++ = 't';
            lenOfTail--;
            break;
        case '\"':
            *ptr++ = '\\';
            *ptr++ = '\"';
            lenOfTail--;
            break;
        case '\\':
            *ptr++ = '\\';
            *ptr++ = '\\';
            lenOfTail--;
            break;
        default:
            *ptr++ = s;
            break;
        }
        lenOfTail--;
        lenOfText--;
    }
    if (lenOfTail) {
        std::char_traits<u16s>::copy(ptr, r, lenOfTail);
        ptr += lenOfTail;
    }
    return ptr;
}

expr_str_base64::expr_str_base64(ssa t) : text(t) {
    l = (text.len + 2) / 3 * 4;
}

u16s* expr_str_base64::place(u16s* ptr) const noexcept {
    static constexpr u8s alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    const unsigned char* t = (const unsigned char*)text.str;

    size_t i = 0;
    if (text.len > 2) {
        for (; i < text.len - 2; i += 3) {
            *ptr++ = alphabet[(t[i] >> 2) & 0x3F];
            *ptr++ = alphabet[((t[i] & 0x3) << 4) | ((int)(t[i + 1] & 0xF0) >> 4)];
            *ptr++ = alphabet[((t[i + 1] & 0xF) << 2) | ((int)(t[i + 2] & 0xC0) >> 6)];
            *ptr++ = alphabet[t[i + 2] & 0x3F];
        }
    }

    if (i < text.len) {
        *ptr++ = alphabet[(t[i] >> 2) & 0x3F];
        if (i == (text.len - 1)) {
            *ptr++ = alphabet[((t[i] & 0x3) << 4)];
            *ptr++ = '=';
        } else {
            *ptr++ = alphabet[((t[i] & 0x3) << 4) | ((int)(t[i + 1] & 0xF0) >> 4)];
            *ptr++ = alphabet[((t[i + 1] & 0xF) << 2)];
        }
        *ptr++ = '=';
    }
    return ptr;
}

simstr::u16s* expr_str_tm::place(u16s* ptr) const noexcept {
    #ifndef __linux__
        std::swprintf(from_w(ptr), 20, L"%04i-%02i-%02i %02i:%02i:%02i", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    #else
        char buf[20];
        std::snprintf(buf, 20, "%04i-%02i-%02i %02i:%02i:%02i", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        for (unsigned i = 0; i < 19; i++) {
            ptr[i] = buf[i];
        }
    #endif
    return ptr + 19;
}
