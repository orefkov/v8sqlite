#include "core_as/str/sstring.h"
#include "stdafx.h"
#include "addin_base.h"

namespace {

AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

} // namespace

long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface) {
    if (!*pInterface) {
        stru name{wsName};
        for (const AddinInfo* ptr = AddinInfo::first; ptr; ptr = ptr->next) {
            if (ptr->name.isEqual_iu(name)) {
                *pInterface = ptr->create();
                break;
            }
        }
        return *pInterface != nullptr;
    }
    return 0;
}

AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities) {
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}

AttachType GetAttachType() {
    return eCanAttachAny;
}

long DestroyObject(IComponentBase** pIntf) {
    if (!*pIntf)
        return -1;

    delete *pIntf;
    *pIntf = 0;
    return 0;
}

const WCHAR_T* GetClassNames() {
    static auto classes = []() {
        lstringu<100> classes;
        for (AddinInfo* ptr = AddinInfo::first; ptr; ptr = ptr->next) {
            classes += ptr->name;
            if (ptr->next) {
                classes += u"|";
            }
        }
        return classes;
    }();
    return classes;
}
