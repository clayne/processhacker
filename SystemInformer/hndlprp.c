/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     wj32    2010-2013
 *     dmex    2018-2024
 *
 */

#include <phapp.h>
#include <hndlprv.h>
#include <phplug.h>
#include <phsettings.h>
#include <kphuser.h>

#include <emenu.h>
#include <settings.h>
#include <hndlinfo.h>
#include <procprv.h>
#include <secedit.h>
#include <phafd.h>

typedef enum _PHP_HANDLE_GENERAL_CATEGORY
{
    // common
    PH_HANDLE_GENERAL_CATEGORY_BASICINFO,
    PH_HANDLE_GENERAL_CATEGORY_REFERENCES,
    PH_HANDLE_GENERAL_CATEGORY_QUOTA,
    // extra
    PH_HANDLE_GENERAL_CATEGORY_ALPC,
    PH_HANDLE_GENERAL_CATEGORY_FILE,
    PH_HANDLE_GENERAL_CATEGORY_SECTION,
    PH_HANDLE_GENERAL_CATEGORY_MUTANT,
    PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD,
    PH_HANDLE_GENERAL_CATEGORY_ETW,
    PH_HANDLE_GENERAL_CATEGORY_SYMBOLICLINK,

    PH_HANDLE_GENERAL_CATEGORY_MAXIMUM
} PHP_HANDLE_GENERAL_CATEGORY;

typedef enum _PHP_HANDLE_GENERAL_INDEX
{
    PH_HANDLE_GENERAL_INDEX_NAME,
    PH_HANDLE_GENERAL_INDEX_TYPE,
    PH_HANDLE_GENERAL_INDEX_OBJECT,
    PH_HANDLE_GENERAL_INDEX_ACCESSMASK,

    PH_HANDLE_GENERAL_INDEX_REFERENCES,
    PH_HANDLE_GENERAL_INDEX_HANDLES,

    PH_HANDLE_GENERAL_INDEX_PAGED,
    PH_HANDLE_GENERAL_INDEX_NONPAGED,

    PH_HANDLE_GENERAL_INDEX_FLAGS,
    PH_HANDLE_GENERAL_INDEX_SEQUENCENUMBER,
    PH_HANDLE_GENERAL_INDEX_PORTCONTEXT,

    PH_HANDLE_GENERAL_INDEX_FILETYPE,
    PH_HANDLE_GENERAL_INDEX_FILEMODE,
    PH_HANDLE_GENERAL_INDEX_FILEPOSITION,
    PH_HANDLE_GENERAL_INDEX_FILESIZE,
    PH_HANDLE_GENERAL_INDEX_FILEPRIORITY,
    PH_HANDLE_GENERAL_INDEX_FILEDRIVER,
    PH_HANDLE_GENERAL_INDEX_FILEDRIVERIMAGE,

    PH_HANDLE_GENERAL_INDEX_SECTIONTYPE,
    PH_HANDLE_GENERAL_INDEX_SECTIONFILE,
    PH_HANDLE_GENERAL_INDEX_SECTIONSIZE,

    PH_HANDLE_GENERAL_INDEX_MUTANTCOUNT,
    PH_HANDLE_GENERAL_INDEX_MUTANTABANDONED,
    PH_HANDLE_GENERAL_INDEX_MUTANTOWNER,

    PH_HANDLE_GENERAL_INDEX_ALPCCONNECTION,
    PH_HANDLE_GENERAL_INDEX_ALPCSERVER,
    PH_HANDLE_GENERAL_INDEX_ALPCCLIENT,
    PH_HANDLE_GENERAL_INDEX_ALPCOWNER,

    PH_HANDLE_GENERAL_INDEX_PROCESSTHREADNAME,
    PH_HANDLE_GENERAL_INDEX_PROCESSTHREADCREATETIME,
    PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITTIME,
    PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITCODE,

    PH_HANDLE_GENERAL_INDEX_ETWORIGINALNAME,
    PH_HANDLE_GENERAL_INDEX_ETWGROUPNAME,

    PH_HANDLE_GENERAL_INDEX_SYMBOLICLINKLINK,

    PH_HANDLE_GENERAL_INDEX_MAXIMUM
} PHP_HANDLE_GENERAL_INDEX;

typedef struct _HANDLE_PROPERTIES_CONTEXT
{
    HWND ListViewHandle;
    HWND ParentWindow;
    HANDLE ProcessId;
    IListView* ListViewClass;
    PPH_HANDLE_ITEM HandleItem;
    PH_LAYOUT_MANAGER LayoutManager;
    INT ListViewRowCache[PH_HANDLE_GENERAL_INDEX_MAXIMUM];
    PPH_PLUGIN OwnerPlugin;
} HANDLE_PROPERTIES_CONTEXT, *PHANDLE_PROPERTIES_CONTEXT;

#define PH_FILEMODE_ASYNC 0x01000000
#define PhFileModeUpdAsyncFlag(mode) \
    ((mode) & (FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT) ? (mode) &~ PH_FILEMODE_ASYNC: (mode) | PH_FILEMODE_ASYNC)

CONST PH_ACCESS_ENTRY FileModeAccessEntries[] =
{
    { L"FILE_FLAG_OVERLAPPED", PH_FILEMODE_ASYNC, FALSE, FALSE, L"Asynchronous" },
    { L"FILE_FLAG_WRITE_THROUGH", FILE_WRITE_THROUGH, FALSE, FALSE, L"Write through" },
    { L"FILE_FLAG_SEQUENTIAL_SCAN", FILE_SEQUENTIAL_ONLY, FALSE, FALSE, L"Sequential" },
    { L"FILE_FLAG_NO_BUFFERING", FILE_NO_INTERMEDIATE_BUFFERING, FALSE, FALSE, L"No buffering" },
    { L"FILE_SYNCHRONOUS_IO_ALERT", FILE_SYNCHRONOUS_IO_ALERT, FALSE, FALSE, L"Synchronous alert" },
    { L"FILE_SYNCHRONOUS_IO_NONALERT", FILE_SYNCHRONOUS_IO_NONALERT, FALSE, FALSE, L"Synchronous non-alert" },
};

CONST PH_ACCESS_ENTRY AlpcFlags[] =
{
    { L"ALPC_PORFLG_LPC_MODE", ALPC_PORFLG_LPC_MODE, FALSE, FALSE, L"LPC mode"},
    { L"ALPC_PORFLG_ALLOW_IMPERSONATION", ALPC_PORFLG_ALLOW_IMPERSONATION, FALSE, FALSE, L"Allow impersonation"},
    { L"ALPC_PORFLG_ALLOW_LPC_REQUESTS", ALPC_PORFLG_ALLOW_LPC_REQUESTS, FALSE, FALSE, L"Allow LPC requests"},
    { L"ALPC_PORFLG_WAITABLE_PORT", ALPC_PORFLG_WAITABLE_PORT, FALSE, FALSE, L"Waitable"},
    { L"ALPC_PORFLG_ALLOW_DUP_OBJECT", ALPC_PORFLG_ALLOW_DUP_OBJECT, FALSE, FALSE, L"Allow object duplication"},
    { L"ALPC_PORFLG_SYSTEM_PROCESS", ALPC_PORFLG_SYSTEM_PROCESS, FALSE, FALSE, L"System process only"},
    { L"ALPC_PORFLG_WAKE_POLICY1", ALPC_PORFLG_WAKE_POLICY1, FALSE, FALSE, L"Wake policy (1)"},
    { L"ALPC_PORFLG_WAKE_POLICY2", ALPC_PORFLG_WAKE_POLICY2, FALSE, FALSE, L"Wake policy (2)"},
    { L"ALPC_PORFLG_WAKE_POLICY3", ALPC_PORFLG_WAKE_POLICY3, FALSE, FALSE, L"Wake policy (3)"},
    { L"ALPC_PORFLG_DIRECT_MESSAGE", ALPC_PORFLG_DIRECT_MESSAGE, FALSE, FALSE, L"No shared section (direct)"},
    { L"ALPC_PORFLG_ALLOW_MULTIHANDLE_ATTRIBUTE", ALPC_PORFLG_ALLOW_MULTIHANDLE_ATTRIBUTE, FALSE, FALSE, L"Allow multi-handle attributes"},
};

INT_PTR CALLBACK PhpHandleGeneralDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    );

static NTSTATUS PhpDuplicateHandleFromProcess(
    _Out_ PHANDLE Handle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ PVOID Context
    )
{
    PHANDLE_PROPERTIES_CONTEXT context = Context;
    NTSTATUS status;
    HANDLE processHandle;

    *Handle = NULL;

    if (NT_SUCCESS(status = PhOpenProcess(
        &processHandle,
        PROCESS_DUP_HANDLE,
        context->ProcessId
        )))
    {
        status = NtDuplicateObject(
            processHandle,
            context->HandleItem->Handle,
            NtCurrentProcess(),
            Handle,
            DesiredAccess,
            0,
            0
            );
        NtClose(processHandle);
    }

    if (!NT_SUCCESS(status) && KsiLevel() >= KphLevelMax)
    {
        if (NT_SUCCESS(status = PhOpenProcess(
            &processHandle,
            PROCESS_QUERY_LIMITED_INFORMATION,
            context->ProcessId
            )))
        {
            status = KphDuplicateObject(
                processHandle,
                context->HandleItem->Handle,
                DesiredAccess,
                Handle
                );
            NtClose(processHandle);
        }
    }

    return status;
}

static NTSTATUS PhpDuplicateHandleCloseProcess(
    _In_opt_ HANDLE Handle,
    _In_opt_ BOOLEAN Release,
    _In_opt_ PVOID Context
    )
{
    if (Handle)
        NtClose(Handle);
    return STATUS_SUCCESS;
}

typedef struct _HANDLE_PROPERTIES_THREAD_CONTEXT
{
    HWND ParentWindowHandle;
    HANDLE ProcessId;
    PPH_HANDLE_ITEM HandleItem;
    PPH_PLUGIN OwnerPlugin;
    PWSTR Caption;
} HANDLE_PROPERTIES_THREAD_CONTEXT, *PHANDLE_PROPERTIES_THREAD_CONTEXT;

BOOLEAN PhpIsVerboseBestObjectName(
    _In_ PPH_HANDLE_ITEM HandleItem
    )
{
    // Some handles use a verbose BestObjectName for the sake of searching. And will display
    // extended information in the property window consisting of the information contained in the
    // BestObjectName. This routine is used to fall back to the normal ObjectName in the property
    // window for these cases. (jxy-s)

    if (PhIsNullOrEmptyString(HandleItem->TypeName))
        return FALSE;

    if (PhEqualString2(HandleItem->TypeName, L"ALPC Port", TRUE))
        return TRUE;

    if (PhEqualString2(HandleItem->TypeName, L"File", TRUE) &&
        PhAfdIsSocketObjectName(HandleItem->ObjectName))
        return TRUE;

    return FALSE;
}

_Function_class_(USER_THREAD_START_ROUTINE)
NTSTATUS PhpShowHandlePropertiesThread(
    _In_ PVOID Parameter
    )
{
    PHANDLE_PROPERTIES_THREAD_CONTEXT handleContext = Parameter;
    PROPSHEETHEADER propSheetHeader = { sizeof(PROPSHEETHEADER) };
    PROPSHEETPAGE propSheetPage;
    HPROPSHEETPAGE pages[16];
    HANDLE_PROPERTIES_CONTEXT context;
    PH_AUTO_POOL autoPool;

    memset(&context, 0, sizeof(HANDLE_PROPERTIES_CONTEXT));
    context.ProcessId = handleContext->ProcessId;
    context.HandleItem = handleContext->HandleItem;
    context.OwnerPlugin = handleContext->OwnerPlugin;
    context.ParentWindow = handleContext->ParentWindowHandle;

    PhInitializeAutoPool(&autoPool);

    propSheetHeader.dwFlags =
        PSH_MODELESS |
        PSH_NOAPPLYNOW |
        PSH_NOCONTEXTHELP |
        PSH_PROPTITLE;
    propSheetHeader.hInstance = PhInstanceHandle;
    propSheetHeader.hwndParent = PhCsForceNoParent ? NULL : handleContext->ParentWindowHandle;
    propSheetHeader.pszCaption = handleContext->Caption ? handleContext->Caption : L"Handle";
    propSheetHeader.nPages = 0;
    propSheetHeader.nStartPage = 0;
    propSheetHeader.phpage = pages;

    // General page
    memset(&propSheetPage, 0, sizeof(PROPSHEETPAGE));
    propSheetPage.dwSize = sizeof(PROPSHEETPAGE);
    propSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_HNDLGENERAL);
    propSheetPage.hInstance = PhInstanceHandle;
    propSheetPage.pfnDlgProc = PhpHandleGeneralDlgProc;
    propSheetPage.lParam = (LPARAM)&context;
    pages[propSheetHeader.nPages++] = CreatePropertySheetPage(&propSheetPage);

    // Object-specific page
    if (PhIsNullOrEmptyString(handleContext->HandleItem->TypeName))
    {
        NOTHING;
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Event", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateEventPage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"EventPair", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateEventPairPage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Job", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateJobPage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context,
            NULL
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Semaphore", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateSemaphorePage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Timer", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateTimerPage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Token", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateTokenPage(
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            context.ProcessId,
            &context,
            NULL
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"Section", TRUE))
    {
        pages[propSheetHeader.nPages++] = PhCreateMappingsPage(
            handleContext->ProcessId,
            handleContext->HandleItem->Handle
            );
    }
    else if (PhEqualString2(handleContext->HandleItem->TypeName, L"File", TRUE) &&
        PhAfdIsSocketObjectName(handleContext->HandleItem->ObjectName))
    {
        pages[propSheetHeader.nPages++] = PhCreateAfdSocketPage(
            context.ProcessId,
            context.HandleItem->Handle
            );
    }

    // Security page
    {
        PCWSTR objectName;

        if (PhpIsVerboseBestObjectName(handleContext->HandleItem))
            objectName = PhGetStringOrEmpty(handleContext->HandleItem->ObjectName);
        else
            objectName = PhGetStringOrEmpty(handleContext->HandleItem->BestObjectName);

        pages[propSheetHeader.nPages++] = PhCreateSecurityPage(
            objectName,
            PhGetStringOrEmpty(handleContext->HandleItem->TypeName),
            PhpDuplicateHandleFromProcess,
            PhpDuplicateHandleCloseProcess,
            &context
            );
    }

    if (PhPluginsEnabled)
    {
        PH_PLUGIN_OBJECT_PROPERTIES objectProperties;
        PH_PLUGIN_HANDLE_PROPERTIES_CONTEXT propertiesContext;

        memset(&propertiesContext, 0, sizeof(PH_PLUGIN_HANDLE_PROPERTIES_CONTEXT));
        propertiesContext.ParentWindowHandle = handleContext->ParentWindowHandle;
        propertiesContext.ProcessId = handleContext->ProcessId;
        propertiesContext.HandleItem = handleContext->HandleItem;
        propertiesContext.OwnerPlugin = handleContext->OwnerPlugin;

        memset(&objectProperties, 0, sizeof(PH_PLUGIN_OBJECT_PROPERTIES));
        objectProperties.Parameter = &propertiesContext;
        objectProperties.NumberOfPages = propSheetHeader.nPages;
        objectProperties.MaximumNumberOfPages = RTL_NUMBER_OF(pages);
        objectProperties.Pages = pages;

        PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackHandlePropertiesInitializing), &objectProperties);

        propSheetHeader.nPages = objectProperties.NumberOfPages;
    }

    PhModalPropertySheet(&propSheetHeader);

    PhDeleteAutoPool(&autoPool);

    PhDereferenceObject(handleContext->HandleItem);
    PhFree(handleContext);

    return STATUS_SUCCESS;
}

VOID PhShowHandleProperties(
    _In_ HWND ParentWindowHandle,
    _In_ HANDLE ProcessId,
    _In_ PPH_HANDLE_ITEM HandleItem
    )
{
    PhShowHandlePropertiesEx(ParentWindowHandle, ProcessId, HandleItem, NULL, NULL);
}

VOID PhShowHandlePropertiesEx(
    _In_ HWND ParentWindowHandle,
    _In_ HANDLE ProcessId,
    _In_ PPH_HANDLE_ITEM HandleItem,
    _In_opt_ PPH_PLUGIN OwnerPlugin,
    _In_opt_ PWSTR Caption
    )
{
    PHANDLE_PROPERTIES_THREAD_CONTEXT context;

    context = PhAllocateZero(sizeof(HANDLE_PROPERTIES_THREAD_CONTEXT));
    context->ParentWindowHandle = ParentWindowHandle;
    context->ProcessId = ProcessId;
    context->HandleItem = HandleItem;
    context->OwnerPlugin = OwnerPlugin;
    context->Caption = Caption;
    PhReferenceObject(HandleItem);

    PhCreateThread2(PhpShowHandlePropertiesThread, context);
}

VOID PhAddHandleListViewItem(
    _In_ IListView* ListViewClass,
    _In_ LONG GroupId,
    _In_ LONG Index,
    _In_ PCWSTR Text
    )
{
    PhAddIListViewGroupItem(ListViewClass, GroupId, Index, Text, UlongToPtr(Index));
}

VOID PhSetHandleListViewItem(
    _In_ PHANDLE_PROPERTIES_CONTEXT Context,
    _In_ LONG Index,
    _In_ LONG SubItemIndex,
    _In_ PCWSTR Text
    )
{
    LONG index = PhFindListViewItemByParam(Context->ListViewHandle, INT_ERROR, UlongToPtr(Index));

    if (index != INT_ERROR)
    {
        PhSetIListViewSubItem(Context->ListViewClass, index, SubItemIndex, Text);
    }
}

VOID PhpUpdateHandleGeneralListViewGroups(
    _In_ PHANDLE_PROPERTIES_CONTEXT Context
    )
{
    IListView_EnableGroupView(Context->ListViewClass, TRUE);
    PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_BASICINFO, L"Basic information");
    PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_REFERENCES, L"References");
    PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_QUOTA, L"Quota charges");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_BASICINFO, PH_HANDLE_GENERAL_INDEX_NAME, L"Name");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_BASICINFO, PH_HANDLE_GENERAL_INDEX_TYPE, L"Type");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_BASICINFO, PH_HANDLE_GENERAL_INDEX_OBJECT, L"Object address");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_BASICINFO, PH_HANDLE_GENERAL_INDEX_ACCESSMASK, L"Granted access");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_REFERENCES, PH_HANDLE_GENERAL_INDEX_REFERENCES, L"References");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_REFERENCES, PH_HANDLE_GENERAL_INDEX_HANDLES, L"Handles");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_QUOTA, PH_HANDLE_GENERAL_INDEX_PAGED, L"Paged");
    PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_QUOTA, PH_HANDLE_GENERAL_INDEX_NONPAGED, L"Virtual size");

    if (PhIsNullOrEmptyString(Context->HandleItem->TypeName))
    {
        NOTHING;
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"ALPC Port", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, L"ALPC Port");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_FLAGS, L"Flags");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_SEQUENCENUMBER, L"Sequence Number");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_PORTCONTEXT, L"Port Context");

        if (KsiLevel() >= KphLevelMed)
        {
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_ALPCCONNECTION, L"Connection");
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_ALPCSERVER, L"Server");
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_ALPCCLIENT, L"Client");
        }

        if (WindowsVersion >= WINDOWS_10_19H2)
        {
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ALPC, PH_HANDLE_GENERAL_INDEX_ALPCOWNER, L"Owner");
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"EtwRegistration", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ETW, L"Event trace information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ETW, PH_HANDLE_GENERAL_INDEX_ETWORIGINALNAME, L"GUID");
        //PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_ETW, PH_HANDLE_GENERAL_INDEX_ETWGROUPNAME, L"Group GUID");
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"File", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, L"File information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILETYPE, L"Type");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILEMODE, L"Mode");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILEPOSITION, L"Position");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILESIZE, L"Size");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, L"Priority");

        if (KsiLevel() >= KphLevelMed)
        {
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILEDRIVER, L"Driver");
            PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_FILE, PH_HANDLE_GENERAL_INDEX_FILEDRIVERIMAGE, L"Driver Image");
        }
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"Section", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SECTION, L"Section information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SECTION, PH_HANDLE_GENERAL_INDEX_SECTIONTYPE, L"Type");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SECTION, PH_HANDLE_GENERAL_INDEX_SECTIONFILE, L"File");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SECTION, PH_HANDLE_GENERAL_INDEX_SECTIONSIZE, L"Size");
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"Mutant", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_MUTANT, L"Mutant information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_MUTANT, PH_HANDLE_GENERAL_INDEX_MUTANTCOUNT, L"Count");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_MUTANT, PH_HANDLE_GENERAL_INDEX_MUTANTABANDONED, L"Abandoned");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_MUTANT, PH_HANDLE_GENERAL_INDEX_MUTANTOWNER, L"Owner");
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"Process", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, L"Process information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADNAME, L"Name");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADCREATETIME, L"Created");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITTIME, L"Exited");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITCODE, L"Exit status");
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"Thread", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, L"Thread information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADNAME, L"Name");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADCREATETIME, L"Created");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITTIME, L"Exited");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_PROCESSTHREAD, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITCODE, L"Exit status");
    }
    else if (PhEqualStringRef2(&Context->HandleItem->TypeName->sr, L"SymbolicLink", TRUE))
    {
        PhAddIListViewGroup(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SYMBOLICLINK, L"Symbolic Link information");
        PhAddHandleListViewItem(Context->ListViewClass, PH_HANDLE_GENERAL_CATEGORY_SYMBOLICLINK, PH_HANDLE_GENERAL_INDEX_SYMBOLICLINKLINK, L"Link target");
    }
}

VOID PhpUpdateHandleGeneral(
    _In_ PHANDLE_PROPERTIES_CONTEXT Context
    )
{
    HANDLE processHandle;
    PPH_ACCESS_ENTRY accessEntries;
    ULONG numberOfAccessEntries;
    WCHAR string[PH_INT64_STR_LEN_1];

    if (PhpIsVerboseBestObjectName(Context->HandleItem))
        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_NAME, 1, PhGetStringOrEmpty(Context->HandleItem->ObjectName));
    else
        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_NAME, 1, PhGetStringOrEmpty(Context->HandleItem->BestObjectName));

    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_TYPE, 1, PhGetStringOrEmpty(Context->HandleItem->TypeName));

    if (Context->HandleItem->Object)
    {
        PhPrintPointer(string, Context->HandleItem->Object);
        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_OBJECT, 1, string);
    }

    if (PhGetAccessEntries(
        PhGetStringOrEmpty(Context->HandleItem->TypeName),
        &accessEntries,
        &numberOfAccessEntries
        ))
    {
        PPH_STRING accessString;
        PPH_STRING grantedAccessString;

        accessString = PH_AUTO(PhGetAccessString(
            Context->HandleItem->GrantedAccess,
            accessEntries,
            numberOfAccessEntries
            ));

        if (accessString->Length != 0)
        {
            grantedAccessString = PH_AUTO(PhFormatString(
                L"0x%x (%s)",
                Context->HandleItem->GrantedAccess,
                accessString->Buffer
                ));

            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ACCESSMASK, 1, grantedAccessString->Buffer);
        }
        else
        {
            PhPrintPointer(string, UlongToPtr(Context->HandleItem->GrantedAccess));
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ACCESSMASK, 1, string);
        }

        PhFree(accessEntries);
    }
    else
    {
        PhPrintPointer(string, UlongToPtr(Context->HandleItem->GrantedAccess));
        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ACCESSMASK, 1, string);
    }

    if (NT_SUCCESS(PhOpenProcess(
        &processHandle,
        (KsiLevel() >= KphLevelMed ? PROCESS_QUERY_LIMITED_INFORMATION : PROCESS_DUP_HANDLE),
        Context->ProcessId
        )))
    {
        OBJECT_BASIC_INFORMATION basicInfo;

        if (NT_SUCCESS(PhGetHandleInformation(
            processHandle,
            Context->HandleItem->Handle,
            ULONG_MAX,
            &basicInfo,
            NULL,
            NULL,
            NULL
            )))
        {
            PhPrintUInt32(string, basicInfo.PointerCount);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_REFERENCES, 1, string);

            PhPrintUInt32(string, basicInfo.HandleCount);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_HANDLES, 1, string);

            PhPrintUInt32(string, basicInfo.PagedPoolCharge);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PAGED, 1, string);

            PhPrintUInt32(string, basicInfo.NonPagedPoolCharge);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_NONPAGED, 1, string);
        }

        NtClose(processHandle);
    }

    if (PhIsNullOrEmptyString(Context->HandleItem->TypeName))
    {
        NOTHING;
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"ALPC Port", TRUE))
    {
        NTSTATUS status;

        if (KsiLevel() >= KphLevelMed && NT_SUCCESS(PhOpenProcess(
                &processHandle,
                PROCESS_QUERY_LIMITED_INFORMATION,
                Context->ProcessId
                )))
        {
            //
            // TODO this path doesn't use all the ALPC info returned yet
            // see: KPH_ALPC_BASIC_INFORMATION.State
            //
            KPH_ALPC_BASIC_INFORMATION basicInfo;
            PKPH_ALPC_COMMUNICATION_NAMES_INFORMATION connectionNames;
            KPH_ALPC_COMMUNICATION_INFORMATION connectionInfo;

            if (NT_SUCCESS(KphAlpcQueryInformation(
                processHandle,
                Context->HandleItem->Handle,
                KphAlpcBasicInformation,
                &basicInfo,
                sizeof(basicInfo),
                NULL
                )))
            {
                PH_FORMAT format[5];
                PPH_STRING alpcFlagsString;

                alpcFlagsString = PhGetAccessString(
                    basicInfo.Flags,
                    (PPH_ACCESS_ENTRY)AlpcFlags,
                    RTL_NUMBER_OF(AlpcFlags)
                    );

                PhInitFormatS(&format[0], L"0x");
                PhInitFormatX(&format[1], basicInfo.Flags);
                PhInitFormatS(&format[2], L" (");
                PhInitFormatSR(&format[3], alpcFlagsString->sr);
                PhInitFormatS(&format[4], L")");

                PhMoveReference(&alpcFlagsString, PhFormat(format, RTL_NUMBER_OF(format), 40));
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FLAGS, 1, PhGetString(alpcFlagsString));
                PhDereferenceObject(alpcFlagsString);

                PhPrintUInt32(string, basicInfo.SequenceNo);
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SEQUENCENUMBER, 1, string);

                PhPrintPointer(string, basicInfo.PortContext);
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PORTCONTEXT, 1, string);
            }

            if (!NT_SUCCESS(KphAlpcQueryComminicationsNamesInfo(
                processHandle,
                Context->HandleItem->Handle,
                &connectionNames)))
            {
                connectionNames = NULL;
            }

            if (NT_SUCCESS(KphAlpcQueryInformation(
                processHandle,
                Context->HandleItem->Handle,
                KphAlpcCommunicationInformation,
                &connectionInfo,
                sizeof(connectionInfo),
                NULL
                )))
            {
                CLIENT_ID clientId;
                PPH_STRING name;

                if (connectionInfo.ConnectionPort.OwnerProcessId)
                {
                    clientId.UniqueProcess = connectionInfo.ConnectionPort.OwnerProcessId;
                    clientId.UniqueThread = 0;

                    name = PhStdGetClientIdName(&clientId);

                    if (connectionNames && connectionNames->ConnectionPort.Length > 0)
                    {
                        PPH_STRING newName;
                        PH_FORMAT format[3];

                        PhInitFormatSR(&format[0], name->sr);
                        PhInitFormatS(&format[1], L" - ");
                        PhInitFormatUCS(&format[2], &connectionNames->ConnectionPort);

                        newName = PhFormat(format, 3, MAX_PATH);
                        PhDereferenceObject(name);
                        name = newName;
                    }

                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ALPCCONNECTION, 1, name->Buffer);
                    PhDereferenceObject(name);
                }

                if (connectionInfo.ServerCommunicationPort.OwnerProcessId)
                {
                    clientId.UniqueProcess = connectionInfo.ServerCommunicationPort.OwnerProcessId;
                    clientId.UniqueThread = 0;

                    name = PhStdGetClientIdName(&clientId);

                    if (connectionNames && connectionNames->ServerCommunicationPort.Length > 0)
                    {
                        PPH_STRING newName;
                        PH_FORMAT format[3];

                        PhInitFormatSR(&format[0], name->sr);
                        PhInitFormatS(&format[1], L" - ");
                        PhInitFormatUCS(&format[2], &connectionNames->ServerCommunicationPort);

                        newName = PhFormat(format, 3, MAX_PATH);
                        PhDereferenceObject(name);
                        name = newName;
                    }

                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ALPCSERVER, 1, name->Buffer);
                    PhDereferenceObject(name);
                }

                if (connectionInfo.ClientCommunicationPort.OwnerProcessId)
                {
                    clientId.UniqueProcess = connectionInfo.ClientCommunicationPort.OwnerProcessId;
                    clientId.UniqueThread = 0;

                    name = PhStdGetClientIdName(&clientId);

                    if (connectionNames && connectionNames->ClientCommunicationPort.Length > 0)
                    {
                        PPH_STRING newName;
                        PH_FORMAT format[3];

                        PhInitFormatSR(&format[0], name->sr);
                        PhInitFormatS(&format[1], L" - ");
                        PhInitFormatUCS(&format[2], &connectionNames->ClientCommunicationPort);

                        newName = PhFormat(format, 3, MAX_PATH);
                        PhDereferenceObject(name);
                        name = newName;
                    }

                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ALPCCLIENT, 1, name->Buffer);
                    PhDereferenceObject(name);
                }

                if (connectionNames)
                    PhFree(connectionNames);

                NtClose(processHandle);
            }
            else
            {
                HANDLE alpcPortHandle = NULL;

                if (NT_SUCCESS(status = PhOpenProcess(
                    &processHandle,
                    PROCESS_DUP_HANDLE,
                    Context->ProcessId
                    )))
                {
                    status = NtDuplicateObject(
                        processHandle,
                        Context->HandleItem->Handle,
                        NtCurrentProcess(),
                        &alpcPortHandle,
                        READ_CONTROL,
                        0,
                        0
                        );
                    NtClose(processHandle);
                }

                if (NT_SUCCESS(status) && alpcPortHandle)
                {
                    ALPC_BASIC_INFORMATION alpcBasicInfo;

                    if (NT_SUCCESS(NtAlpcQueryInformation(
                        alpcPortHandle,
                        AlpcBasicInformation,
                        &alpcBasicInfo,
                        sizeof(ALPC_BASIC_INFORMATION),
                        NULL
                        )))
                    {
                        PH_FORMAT format[5];
                        PPH_STRING alpcFlagsString;

                        alpcFlagsString = PhGetAccessString(
                            alpcBasicInfo.Flags,
                            (PPH_ACCESS_ENTRY)AlpcFlags,
                            RTL_NUMBER_OF(AlpcFlags)
                            );

                        PhInitFormatS(&format[0], L"0x");
                        PhInitFormatX(&format[1], alpcBasicInfo.Flags);
                        PhInitFormatS(&format[2], L" (");
                        PhInitFormatSR(&format[3], alpcFlagsString->sr);
                        PhInitFormatS(&format[4], L")");

                        PhMoveReference(&alpcFlagsString, PhFormat(format, RTL_NUMBER_OF(format), 40));

                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FLAGS, 1, PhGetString(alpcFlagsString));

                        PhDereferenceObject(alpcFlagsString);

                        PhPrintUInt32(string, basicInfo.SequenceNo);
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SEQUENCENUMBER, 1, string);

                        PhPrintPointer(string, basicInfo.PortContext);
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PORTCONTEXT, 1, string);
                    }

                    if (WindowsVersion >= WINDOWS_10_19H2)
                    {
                        ALPC_SERVER_SESSION_INFORMATION serverInfo;

                        if (NT_SUCCESS(NtAlpcQueryInformation(
                            alpcPortHandle,
                            AlpcServerSessionInformation,
                            &serverInfo,
                            sizeof(ALPC_SERVER_SESSION_INFORMATION),
                            NULL
                            )))
                        {
                            CLIENT_ID clientId;
                            PPH_STRING name;

                            clientId.UniqueProcess = UlongToHandle(serverInfo.ProcessId);
                            clientId.UniqueThread = NULL;

                            name = PhGetClientIdName(&clientId);
                            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ALPCOWNER, 1, name->Buffer);
                            PhDereferenceObject(name);
                        }
                    }

                    NtClose(alpcPortHandle);
                }
            }
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"EtwRegistration", TRUE))
    {
        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_ETWORIGINALNAME, 1, PhGetString(Context->HandleItem->ObjectName));
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"File", TRUE))
    {
        NTSTATUS status;

        if (KsiLevel() >= KphLevelMed && NT_SUCCESS(PhOpenProcess(
                &processHandle,
                PROCESS_QUERY_LIMITED_INFORMATION,
                Context->ProcessId
                )))
        {
            BOOLEAN isFileOrDirectory = FALSE;
            BOOLEAN isConsoleHandle = FALSE;
            FILE_FS_DEVICE_INFORMATION fileDeviceInfo;
            FILE_MODE_INFORMATION fileModeInfo;
            FILE_STANDARD_INFORMATION fileStandardInfo;
            FILE_POSITION_INFORMATION filePositionInfo;
            FILE_IO_PRIORITY_HINT_INFORMATION_EX priorityInfo;
            IO_STATUS_BLOCK isb;
            KPH_FILE_OBJECT_DRIVER fileObjectDriver;

            if (NT_SUCCESS(KphQueryVolumeInformationFile(
                processHandle,
                Context->HandleItem->Handle,
                FileFsDeviceInformation,
                &fileDeviceInfo,
                sizeof(FILE_FS_DEVICE_INFORMATION),
                &isb
                )))
            {
                switch (fileDeviceInfo.DeviceType)
                {
                case FILE_DEVICE_NAMED_PIPE:
                    //isPipeHandle = TRUE;
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Pipe");
                    break;
                case FILE_DEVICE_NETWORK:
                    //isNetworkHandle = TRUE;
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Network");
                    break;
                case FILE_DEVICE_CD_ROM:
                case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                case FILE_DEVICE_CONTROLLER:
                case FILE_DEVICE_DATALINK:
                case FILE_DEVICE_DFS:
                case FILE_DEVICE_DISK:
                case FILE_DEVICE_DISK_FILE_SYSTEM:
                case FILE_DEVICE_VIRTUAL_DISK:
                    isFileOrDirectory = TRUE;
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"File or directory");
                    break;
                case FILE_DEVICE_CONSOLE:
                    isConsoleHandle = TRUE;
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Console");
                    break;
                default:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Other");
                    break;
                }
            }

            // Note: These devices deadlock without a timeout (dmex)
            // 1) Named pipes
            // 2) \Device\ConDrv\CurrentIn
            // 3) \Device\VolMgrControl

            if (NT_SUCCESS(status = PhCallKphQueryFileInformationWithTimeout(
                processHandle,
                Context->HandleItem->Handle,
                FileModeInformation,
                &fileModeInfo,
                sizeof(FILE_MODE_INFORMATION)
                )))
            {
                PH_FORMAT format[5];
                PPH_STRING fileModeAccessStr;
                WCHAR fileModeString[MAX_PATH];

                // Since FILE_MODE_INFORMATION has no flag for asynchronous I/O we should use our own flag and set
                // it only if none of synchronous flags are present. That's why we need PhFileModeUpdAsyncFlag.
                fileModeAccessStr = PhGetAccessString(
                    PhFileModeUpdAsyncFlag(fileModeInfo.Mode),
                    (PPH_ACCESS_ENTRY)FileModeAccessEntries,
                    RTL_NUMBER_OF(FileModeAccessEntries)
                    );

                PhInitFormatS(&format[0], L"0x");
                PhInitFormatX(&format[1], fileModeInfo.Mode);
                PhInitFormatS(&format[2], L" (");
                PhInitFormatSR(&format[3], fileModeAccessStr->sr);
                PhInitFormatS(&format[4], L")");

                if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), fileModeString, sizeof(fileModeString), NULL))
                {
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEMODE, 1, fileModeString);
                }

                PhDereferenceObject(fileModeAccessStr);
            }

            if (!isConsoleHandle)
            {
                if (NT_SUCCESS(status = PhCallKphQueryFileInformationWithTimeout(
                    processHandle,
                    Context->HandleItem->Handle,
                    FileStandardInformation,
                    &fileStandardInfo,
                    sizeof(FILE_STANDARD_INFORMATION)
                    )))
                {
                    PH_FORMAT format[1];
                    WCHAR fileSizeString[PH_INT64_STR_LEN];

                    PhInitFormatSize(&format[0], fileStandardInfo.EndOfFile.QuadPart);

                    if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), fileSizeString, sizeof(fileSizeString), NULL))
                    {
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILESIZE, 1, fileSizeString);
                    }

                    if (isFileOrDirectory)
                    {
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, fileStandardInfo.Directory ? L"Directory" : L"File");
                    }

                    //disableFlushButton |= fileStandardInfo.Directory;
                }

                if (NT_SUCCESS(status = PhCallKphQueryFileInformationWithTimeout(
                    processHandle,
                    Context->HandleItem->Handle,
                    FilePositionInformation,
                    &filePositionInfo,
                    sizeof(FILE_POSITION_INFORMATION)
                    )))
                {
                    if (filePositionInfo.CurrentByteOffset.QuadPart != 0 && fileStandardInfo.EndOfFile.QuadPart != 0)
                    {
                        PH_FORMAT format[4];
                        WCHAR filePositionString[PH_INT64_STR_LEN];

                        PhInitFormatI64UGroupDigits(&format[0], filePositionInfo.CurrentByteOffset.QuadPart);
                        PhInitFormatS(&format[1], L" (");
                        PhInitFormatF(&format[2], (FLOAT)filePositionInfo.CurrentByteOffset.QuadPart / fileStandardInfo.EndOfFile.QuadPart * 100.f, 1);
                        PhInitFormatS(&format[3], L"%)");

                        if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), filePositionString, sizeof(filePositionString), NULL))
                        {
                            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPOSITION, 1, filePositionString);
                        }
                    }
                    else if (filePositionInfo.CurrentByteOffset.QuadPart != 0)
                    {
                        PH_FORMAT format[1];
                        WCHAR filePositionString[PH_INT64_STR_LEN];

                        PhInitFormatI64UGroupDigits(&format[0], filePositionInfo.CurrentByteOffset.QuadPart);

                        if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), filePositionString, sizeof(filePositionString), NULL))
                        {
                            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPOSITION, 1, filePositionString);
                        }
                    }
                }
            }

            if (NT_SUCCESS(status = PhCallKphQueryFileInformationWithTimeout(
                processHandle,
                Context->HandleItem->Handle,
                FileIoPriorityHintInformation,
                &priorityInfo,
                sizeof(priorityInfo)
                )))
            {
                switch (priorityInfo.PriorityHint)
                {
                case IoPriorityVeryLow:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Very Low");
                    break;
                case IoPriorityLow:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Low");
                    break;
                case IoPriorityNormal:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Normal");
                    break;
                case IoPriorityHigh:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"High");
                    break;
                case IoPriorityCritical:
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Critical");
                    break;
                }
            }

            if (NT_SUCCESS(KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectFileObjectDriver,
                &fileObjectDriver,
                sizeof(fileObjectDriver),
                NULL
                )))
            {
                PPH_STRING driverName;

                if (NT_SUCCESS(PhGetDriverName(fileObjectDriver.DriverHandle, &driverName)))
                {
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEDRIVER, 1, PhGetString(driverName));
                    PhDereferenceObject(driverName);
                }

                if (NT_SUCCESS(PhGetDriverImageFileName(fileObjectDriver.DriverHandle, &driverName)))
                {
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEDRIVERIMAGE, 1, PhGetString(driverName));
                    PhDereferenceObject(driverName);
                }

                NtClose(fileObjectDriver.DriverHandle);
            }

            NtClose(processHandle);
        }
        else
        {
            HANDLE fileHandle = NULL;

            if (NT_SUCCESS(status = PhOpenProcess(
                &processHandle,
                PROCESS_DUP_HANDLE,
                Context->ProcessId
                )))
            {
                status = NtDuplicateObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    NtCurrentProcess(),
                    &fileHandle,
                    FILE_READ_ACCESS | SYNCHRONIZE,
                    0,
                    0
                    );

                if (!NT_SUCCESS(status))
                {
                    status = NtDuplicateObject(
                        processHandle,
                        Context->HandleItem->Handle,
                        NtCurrentProcess(),
                        &fileHandle,
                        FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        0,
                        0
                        );
                }

                //if (!NT_SUCCESS(status))
                //{
                //    status = NtDuplicateObject(
                //        processHandle,
                //        Context->HandleItem->Handle,
                //        NtCurrentProcess(),
                //        &fileHandle,
                //        MAXIMUM_ALLOWED | SYNCHRONIZE,
                //        0,
                //        0
                //        );
                //}

                if (!NT_SUCCESS(status))
                {
                    status = NtDuplicateObject(
                        processHandle,
                        Context->HandleItem->Handle,
                        NtCurrentProcess(),
                        &fileHandle,
                        0,
                        0,
                        DUPLICATE_SAME_ACCESS
                        );

                    if (NT_SUCCESS(status))
                    {
                        HANDLE newhandle;

                        if (NT_SUCCESS(PhReOpenFile(
                            &newhandle,
                            fileHandle,
                            FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                            FILE_SHARE_READ,
                            0
                            )))
                        {
                            NtClose(fileHandle);
                            fileHandle = newhandle;
                        }
                    }
                }

                NtClose(processHandle);
            }

            if (NT_SUCCESS(status) && fileHandle)
            {
                //BOOLEAN disableFlushButton = FALSE;
                BOOLEAN isFileOrDirectory = FALSE;
                BOOLEAN isConsoleHandle = FALSE;
                //BOOLEAN isPipeHandle = FALSE;
                //BOOLEAN isNetworkHandle = FALSE;
                FILE_FS_DEVICE_INFORMATION fileDeviceInfo;
                FILE_MODE_INFORMATION fileModeInfo;
                FILE_STANDARD_INFORMATION fileStandardInfo;
                FILE_POSITION_INFORMATION filePositionInfo;
                FILE_IO_PRIORITY_HINT_INFORMATION_EX priorityInfo;
                IO_STATUS_BLOCK isb;

                if (NT_SUCCESS(NtQueryVolumeInformationFile(
                    fileHandle,
                    &isb,
                    &fileDeviceInfo,
                    sizeof(FILE_FS_DEVICE_INFORMATION),
                    FileFsDeviceInformation
                    )))
                {
                    switch (fileDeviceInfo.DeviceType)
                    {
                    case FILE_DEVICE_NAMED_PIPE:
                        //isPipeHandle = TRUE;
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Pipe");
                        break;
                    case FILE_DEVICE_NETWORK:
                        //isNetworkHandle = TRUE;
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Network");
                        break;
                    case FILE_DEVICE_CD_ROM:
                    case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                    case FILE_DEVICE_CONTROLLER:
                    case FILE_DEVICE_DATALINK:
                    case FILE_DEVICE_DFS:
                    case FILE_DEVICE_DISK:
                    case FILE_DEVICE_DISK_FILE_SYSTEM:
                    case FILE_DEVICE_VIRTUAL_DISK:
                        isFileOrDirectory = TRUE;
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"File or directory");
                        break;
                    case FILE_DEVICE_CONSOLE:
                        isConsoleHandle = TRUE;
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Console");
                        break;
                    default:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, L"Other");
                        break;
                    }
                }

                // Note: These devices deadlock without a timeout (dmex)
                // 1) Named pipes
                // 2) \Device\ConDrv\CurrentIn
                // 3) \Device\VolMgrControl

                if (NT_SUCCESS(status = PhCallNtQueryFileInformationWithTimeout(
                    fileHandle,
                    FileModeInformation,
                    &fileModeInfo,
                    sizeof(FILE_MODE_INFORMATION)
                    )))
                {
                    PH_FORMAT format[5];
                    PPH_STRING fileModeAccessStr;
                    WCHAR fileModeString[MAX_PATH];

                    // Since FILE_MODE_INFORMATION has no flag for asynchronous I/O we should use our own flag and set
                    // it only if none of synchronous flags are present. That's why we need PhFileModeUpdAsyncFlag.
                    fileModeAccessStr = PhGetAccessString(
                        PhFileModeUpdAsyncFlag(fileModeInfo.Mode),
                        (PPH_ACCESS_ENTRY)FileModeAccessEntries,
                        RTL_NUMBER_OF(FileModeAccessEntries)
                        );

                    PhInitFormatS(&format[0], L"0x");
                    PhInitFormatX(&format[1], fileModeInfo.Mode);
                    PhInitFormatS(&format[2], L" (");
                    PhInitFormatSR(&format[3], fileModeAccessStr->sr);
                    PhInitFormatS(&format[4], L")");

                    if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), fileModeString, sizeof(fileModeString), NULL))
                    {
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEMODE, 1, fileModeString);
                    }

                    PhDereferenceObject(fileModeAccessStr);
                }

                if (!isConsoleHandle)
                {
                    if (NT_SUCCESS(status = PhCallNtQueryFileInformationWithTimeout(
                        fileHandle,
                        FileStandardInformation,
                        &fileStandardInfo,
                        sizeof(FILE_STANDARD_INFORMATION)
                        )))
                    {
                        PH_FORMAT format[1];
                        WCHAR fileSizeString[PH_INT64_STR_LEN];

                        PhInitFormatSize(&format[0], fileStandardInfo.EndOfFile.QuadPart);

                        if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), fileSizeString, sizeof(fileSizeString), NULL))
                        {
                            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILESIZE, 1, fileSizeString);
                        }

                        if (isFileOrDirectory)
                        {
                            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILETYPE, 1, fileStandardInfo.Directory ? L"Directory" : L"File");
                        }

                        //disableFlushButton |= fileStandardInfo.Directory;
                    }

                    if (NT_SUCCESS(status = PhCallNtQueryFileInformationWithTimeout(
                        fileHandle,
                        FilePositionInformation,
                        &filePositionInfo,
                        sizeof(FILE_POSITION_INFORMATION)
                        )))
                    {
                        if (fileStandardInfo.EndOfFile.QuadPart != 0)
                        {
                            PH_FORMAT format[4];
                            WCHAR filePositionString[PH_INT64_STR_LEN];

                            PhInitFormatI64UGroupDigits(&format[0], filePositionInfo.CurrentByteOffset.QuadPart);
                            PhInitFormatS(&format[1], L" (");
                            PhInitFormatF(&format[2], (FLOAT)filePositionInfo.CurrentByteOffset.QuadPart / fileStandardInfo.EndOfFile.QuadPart * 100.f, 1);
                            PhInitFormatS(&format[3], L"%)");

                            if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), filePositionString, sizeof(filePositionString), NULL))
                            {
                                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPOSITION, 1, filePositionString);
                            }
                        }
                        else
                        {
                            PH_FORMAT format[1];
                            WCHAR filePositionString[PH_INT64_STR_LEN];

                            PhInitFormatI64UGroupDigits(&format[0], filePositionInfo.CurrentByteOffset.QuadPart);

                            if (PhFormatToBuffer(format, RTL_NUMBER_OF(format), filePositionString, sizeof(filePositionString), NULL))
                            {
                                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPOSITION, 1, filePositionString);
                            }
                        }
                    }
                }


                if (NT_SUCCESS(status = PhCallNtQueryFileInformationWithTimeout(
                    fileHandle,
                    FileIoPriorityHintInformation,
                    &priorityInfo,
                    sizeof(priorityInfo)
                    )))
                {
                    switch (priorityInfo.PriorityHint)
                    {
                    case IoPriorityVeryLow:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Very Low");
                        break;
                    case IoPriorityLow:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Low");
                        break;
                    case IoPriorityNormal:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Normal");
                        break;
                    case IoPriorityHigh:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"High");
                        break;
                    case IoPriorityCritical:
                        PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_FILEPRIORITY, 1, L"Critical");
                        break;
                    }
                }

                NtClose(fileHandle);
            }
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"Section", TRUE))
    {
        NTSTATUS status;
        SECTION_BASIC_INFORMATION basicInfo;
        PPH_STRING fileName = NULL;

        if (KsiLevel() >= KphLevelMed && NT_SUCCESS(PhOpenProcess(
                &processHandle,
                PROCESS_QUERY_LIMITED_INFORMATION,
                Context->ProcessId
                )))
        {
            ULONG bufferSize;
            ULONG returnLength;
            PUNICODE_STRING buffer;
            NTSTATUS status2;

            returnLength = 0;
            bufferSize = 0x100;
            buffer = PhAllocate(bufferSize);

            status = KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectSectionBasicInformation,
                &basicInfo,
                sizeof(basicInfo),
                NULL
                );

            status2 = KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectSectionFileName,
                buffer,
                bufferSize,
                &returnLength
                );
            if (status2 == STATUS_BUFFER_OVERFLOW && returnLength > 0)
            {
                PhFree(buffer);
                bufferSize = returnLength;
                buffer = PhAllocate(returnLength);

                status2 = KphQueryInformationObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    KphObjectSectionFileName,
                    buffer,
                    bufferSize,
                    &returnLength
                    );
            }

            if (NT_SUCCESS(status2))
            {
                fileName = PhCreateStringFromUnicodeString(buffer);
            }

            PhFree(buffer);
            NtClose(processHandle);
        }
        else
        {
            HANDLE sectionHandle = NULL;

            if (NT_SUCCESS(status = PhOpenProcess(
                &processHandle,
                PROCESS_DUP_HANDLE,
                Context->ProcessId
                )))
            {
                status = NtDuplicateObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    NtCurrentProcess(),
                    &sectionHandle,
                    SECTION_QUERY | SECTION_MAP_READ,
                    0,
                    0
                    );

                if (!NT_SUCCESS(status))
                {
                    status = NtDuplicateObject(
                        processHandle,
                        Context->HandleItem->Handle,
                        NtCurrentProcess(),
                        &sectionHandle,
                        SECTION_QUERY,
                        0,
                        0
                        );
                }

                NtClose(processHandle);
            }

            if (NT_SUCCESS(status) && sectionHandle)
            {
                status = PhGetSectionBasicInformation(sectionHandle, &basicInfo);

                if (!NT_SUCCESS(PhGetSectionFileName(sectionHandle, &fileName)))
                {
                    fileName = NULL;
                }

                NtClose(sectionHandle);
            }
        }

        if (NT_SUCCESS(status))
        {
            PCWSTR sectionType = L"Unknown";
            PPH_STRING sectionSize = NULL;

            if (FlagOn(basicInfo.AllocationAttributes, SEC_COMMIT))
                sectionType = L"Commit";
            else if (FlagOn(basicInfo.AllocationAttributes, SEC_FILE))
                sectionType = L"File";
            else if (FlagOn(basicInfo.AllocationAttributes, SEC_IMAGE))
                sectionType = L"Image";
            else if (FlagOn(basicInfo.AllocationAttributes, SEC_RESERVE))
                sectionType = L"Reserve";

            sectionSize = PhaFormatSize(basicInfo.MaximumSize.QuadPart, ULONG_MAX);

            if (fileName)
            {
                PhMoveReference(&fileName, PhGetFileName(fileName));
            }

            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SECTIONFILE, 1, PhGetStringOrDefault(fileName, L"N/A"));
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SECTIONTYPE, 1, sectionType);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SECTIONSIZE, 1, PhGetStringOrDefault(sectionSize, L"Unknown"));
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"Mutant", TRUE))
    {
        NTSTATUS status;
        HANDLE mutantHandle = NULL;

        if (NT_SUCCESS(status = PhOpenProcess(
            &processHandle,
            PROCESS_DUP_HANDLE,
            Context->ProcessId
            )))
        {
            status = NtDuplicateObject(
                processHandle,
                Context->HandleItem->Handle,
                NtCurrentProcess(),
                &mutantHandle,
                SEMAPHORE_QUERY_STATE,
                0,
                0
                );
            NtClose(processHandle);
        }

        if (NT_SUCCESS(status) && mutantHandle)
        {
            MUTANT_BASIC_INFORMATION basicInfo;
            MUTANT_OWNER_INFORMATION ownerInfo;

            if (NT_SUCCESS(PhGetMutantBasicInformation(mutantHandle, &basicInfo)))
            {
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_MUTANTCOUNT, 1, PhaFormatUInt64(basicInfo.CurrentCount, TRUE)->Buffer);
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_MUTANTABANDONED, 1, basicInfo.AbandonedState ? L"True" : L"False");
            }

            if (NT_SUCCESS(PhGetMutantOwnerInformation(mutantHandle, &ownerInfo)))
            {
                PPH_STRING name;

                if (ownerInfo.ClientId.UniqueProcess)
                {
                    name = PhGetClientIdName(&ownerInfo.ClientId);
                    PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_MUTANTOWNER, 1, name->Buffer);
                    PhDereferenceObject(name);
                }
            }

            NtClose(mutantHandle);
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"Process", TRUE))
    {
        NTSTATUS status;
        NTSTATUS exitStatus = STATUS_PENDING;
        PPH_STRING fileName = NULL;
        PROCESS_BASIC_INFORMATION basicInfo;
        KERNEL_USER_TIMES times;
        BOOLEAN haveTimes = FALSE;

        if (KsiLevel() >= KphLevelMed && NT_SUCCESS(PhOpenProcess(
                &processHandle,
                PROCESS_QUERY_LIMITED_INFORMATION,
                Context->ProcessId
                )))
        {
            ULONG bufferSize;
            ULONG returnLength;
            PUNICODE_STRING buffer;
            NTSTATUS status2;

            returnLength = 0;
            bufferSize = 0x100;
            buffer = PhAllocate(bufferSize);

            if (NT_SUCCESS(KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectProcessBasicInformation,
                &basicInfo,
                sizeof(basicInfo),
                NULL
                )))
            {
                exitStatus = basicInfo.ExitStatus;
            }

            haveTimes = NT_SUCCESS(KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectProcessTimes,
                &times,
                sizeof(times),
                NULL
                ));

            status2 = KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectProcessImageFileName,
                buffer,
                bufferSize,
                &returnLength
                );
            if (status2 == STATUS_BUFFER_TOO_SMALL && returnLength > 0)
            {
                PhFree(buffer);
                bufferSize = returnLength;
                buffer = PhAllocate(returnLength);

                status2 = KphQueryInformationObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    KphObjectProcessImageFileName,
                    buffer,
                    bufferSize,
                    &returnLength
                    );
            }

            if (NT_SUCCESS(status2))
            {
                fileName = PhCreateStringFromUnicodeString(buffer);
                PhMoveReference(&fileName, PhGetFileName(fileName));
            }

            NtClose(processHandle);
        }
        else
        {
            HANDLE dupHandle = NULL;

            if (NT_SUCCESS(status = PhOpenProcess(
                &processHandle,
                PROCESS_DUP_HANDLE,
                Context->ProcessId
                )))
            {
                status = NtDuplicateObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    NtCurrentProcess(),
                    &dupHandle,
                    PROCESS_QUERY_LIMITED_INFORMATION,
                    0,
                    0
                    );

                NtClose(processHandle);
            }

            if (NT_SUCCESS(status) && dupHandle)
            {
                if (NT_SUCCESS(PhGetProcessImageFileName(dupHandle, &fileName)))
                {
                    PhMoveReference(&fileName, PhGetFileName(fileName));
                }

                if (NT_SUCCESS(PhGetProcessBasicInformation(dupHandle, &basicInfo)))
                {
                    exitStatus = basicInfo.ExitStatus;
                }

                haveTimes = NT_SUCCESS(PhGetProcessTimes(dupHandle, &times));

                NtClose(dupHandle);
            }
        }

        if (fileName)
        {
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADNAME, 1, PhGetStringOrEmpty(fileName));
            PhDereferenceObject(fileName);
        }

        if (haveTimes)
        {
            SYSTEMTIME time;

            PhLargeIntegerToLocalSystemTime(&time, &times.CreateTime);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADCREATETIME, 1, PhaFormatDateTime(&time)->Buffer);

            if (exitStatus != STATUS_PENDING)
            {
                PhLargeIntegerToLocalSystemTime(&time, &times.ExitTime);
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITTIME, 1, PhaFormatDateTime(&time)->Buffer);
            }
        }

        if (exitStatus != STATUS_PENDING)
        {
            PPH_STRING message;
            PPH_STRING exitcode;

            message = PhGetStatusMessage(exitStatus, 0);
            exitcode = PhFormatString(L"0x%x (%s)", exitStatus, PhGetStringOrDefault(message, L"Unknown"));
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITCODE, 1, PhGetStringOrEmpty(exitcode));
            PhClearReference(&exitcode);
            PhClearReference(&message);
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"Thread", TRUE))
    {
        NTSTATUS status;
        BOOLEAN isTerminated = FALSE;
        PPH_STRING name = NULL;
        KERNEL_USER_TIMES times;
        NTSTATUS exitStatus = STATUS_PENDING;

        if (KsiLevel() >= KphLevelMed && NT_SUCCESS(PhOpenProcess(
                &processHandle,
                PROCESS_QUERY_LIMITED_INFORMATION,
                Context->ProcessId
                )))
        {
            PPH_STRING threadName;
            ULONG threadIsTerminated;
            THREAD_BASIC_INFORMATION basicInfo;

            if (NT_SUCCESS(KphQueryObjectThreadName(
                processHandle,
                Context->HandleItem->Handle,
                &threadName
                )))
            {
                name = threadName;
            }

            if (NT_SUCCESS(KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectThreadIsTerminated,
                &threadIsTerminated,
                sizeof(threadIsTerminated),
                NULL
                )))
            {
                isTerminated = !!threadIsTerminated;
            }

            if (isTerminated && NT_SUCCESS(KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectThreadBasicInformation,
                &basicInfo,
                sizeof(basicInfo),
                NULL
                )))
            {
                exitStatus = basicInfo.ExitStatus;
            }

            status = KphQueryInformationObject(
                processHandle,
                Context->HandleItem->Handle,
                KphObjectThreadTimes,
                &times,
                sizeof(times),
                NULL
                );

            NtClose(processHandle);
        }
        else
        {
            HANDLE dupHandle = NULL;
            PPH_STRING threadName;
            THREAD_BASIC_INFORMATION basicInfo;

            if (NT_SUCCESS(status = PhOpenProcess(
                &processHandle,
                PROCESS_DUP_HANDLE,
                Context->ProcessId
                )))
            {
                status = NtDuplicateObject(
                    processHandle,
                    Context->HandleItem->Handle,
                    NtCurrentProcess(),
                    &dupHandle,
                    THREAD_QUERY_LIMITED_INFORMATION,
                    0,
                    0
                    );

                NtClose(processHandle);
            }

            if (NT_SUCCESS(status) && dupHandle)
            {
                if (NT_SUCCESS(PhGetThreadName(dupHandle, &threadName)))
                {
                    name = threadName;
                }

                PhGetThreadIsTerminated(dupHandle, &isTerminated);

                if (isTerminated && NT_SUCCESS(PhGetThreadBasicInformation(dupHandle, &basicInfo)))
                {
                    exitStatus = basicInfo.ExitStatus;
                }

                status = PhGetThreadTimes(dupHandle, &times);

                NtClose(dupHandle);
            }
        }

        if (name)
        {
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADNAME, 1, PhGetStringOrEmpty(name));
            PhDereferenceObject(name);
        }

        if (isTerminated)
        {
            PPH_STRING message;
            PPH_STRING exitcode;

            message = PhGetStatusMessage(exitStatus, 0);
            exitcode = PhFormatString(L"0x%x (%s)", exitStatus, PhGetStringOrDefault(message, L"Unknown"));
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITCODE, 1, PhGetStringOrEmpty(exitcode));
            PhClearReference(&exitcode);
            PhClearReference(&message);
        }

        if (NT_SUCCESS(status))
        {
            SYSTEMTIME time;

            PhLargeIntegerToLocalSystemTime(&time, &times.CreateTime);
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADCREATETIME, 1, PhaFormatDateTime(&time)->Buffer);

            if (isTerminated)
            {
                PhLargeIntegerToLocalSystemTime(&time, &times.ExitTime);
                PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_PROCESSTHREADEXITTIME, 1, PhaFormatDateTime(&time)->Buffer);
            }
        }
    }
    else if (PhEqualString2(Context->HandleItem->TypeName, L"SymbolicLink", TRUE))
    {
        PPH_STRING linkTarget;

        if (!PhIsNullOrEmptyString(Context->HandleItem->ObjectName) &&
            NT_SUCCESS(PhQuerySymbolicLinkObject(&linkTarget, NULL, &Context->HandleItem->ObjectName->sr)))
        {
            PhSetHandleListViewItem(Context, PH_HANDLE_GENERAL_INDEX_SYMBOLICLINKLINK, 1, PhGetStringOrEmpty(linkTarget));
            PhDereferenceObject(linkTarget);
        }
    }
}

INT_PTR CALLBACK PhpHandleGeneralDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    PHANDLE_PROPERTIES_CONTEXT context;

    if (uMsg == WM_INITDIALOG)
    {
        LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
        context = (PHANDLE_PROPERTIES_CONTEXT)propSheetPage->lParam;
        PhSetDialogContext(hwndDlg, context);
    }
    else
    {
        context = PhGetDialogContext(hwndDlg);
    }

    if (!context)
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PhSetApplicationWindowIcon(context->ParentWindow);

            context->ListViewHandle = GetDlgItem(hwndDlg, IDC_LIST);
            context->ParentWindow = GetParent(hwndDlg);
            context->ListViewClass = PhGetListViewInterface(context->ListViewHandle);

            PhSetListViewStyle(context->ListViewHandle, FALSE, TRUE);
            PhSetControlTheme(context->ListViewHandle, L"explorer");
            PhAddListViewColumn(context->ListViewHandle, 0, 0, 0, LVCFMT_LEFT, 120, L"Name");
            PhAddListViewColumn(context->ListViewHandle, 1, 1, 1, LVCFMT_LEFT, 250, L"Value");
            PhSetExtendedListView(context->ListViewHandle);

            // Plugins can load window position in GeneralCallbackHandlePropertiesWindowInitialized, ex. Object Manager (Dart Vanya)
            if (!PhPluginsEnabled || !context->OwnerPlugin)
            {
                // HACK
                if (PhValidWindowPlacementFromSetting(L"HandlePropertiesWindowPosition"))
                    PhLoadWindowPlacementFromSetting(L"HandlePropertiesWindowPosition", NULL, context->ParentWindow);
                else
                    PhCenterWindow(context->ParentWindow, GetParent(context->ParentWindow)); // HACK
            }

            PhInitializeLayoutManager(&context->LayoutManager, hwndDlg);
            PhAddLayoutItem(&context->LayoutManager, context->ListViewHandle, NULL, PH_ANCHOR_ALL);

            PhpUpdateHandleGeneralListViewGroups(context);
            PhpUpdateHandleGeneral(context);

            PhRegisterWindowCallback(context->ParentWindow, PH_PLUGIN_WINDOW_EVENT_TYPE_TOPMOST, NULL);

            if (PhPluginsEnabled)
            {
                PPH_PLUGIN_HANDLE_PROPERTIES_WINDOW_CONTEXT Context;
                Context = (PPH_PLUGIN_HANDLE_PROPERTIES_WINDOW_CONTEXT)context;

                PhInvokeCallback(PhGetGeneralCallback(GeneralCallbackHandlePropertiesWindowInitialized), Context);
            }

            if (PhEnableThemeSupport) // TODO: Required for compat (dmex)
                PhInitializeWindowTheme(context->ParentWindow, PhEnableThemeSupport);
            else
                PhInitializeWindowTheme(hwndDlg, FALSE);
        }
        break;
    case WM_DESTROY:
        {
            PhRemoveDialogContext(hwndDlg);

            PhUnregisterWindowCallback(context->ParentWindow);

            PhSaveWindowPlacementToSetting(L"HandlePropertiesWindowPosition", NULL, context->ParentWindow);

            PhDeleteLayoutManager(&context->LayoutManager);

            PhDestroyListViewInterface(context->ListViewClass);
        }
        break;
    case WM_SIZE:
        {
            PhLayoutManagerLayout(&context->LayoutManager);
            ExtendedListView_SetColumnWidth(context->ListViewHandle, 0, ELVSCW_AUTOSIZE_REMAININGSPACE);
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR header = (LPNMHDR)lParam;

            PhHandleListViewNotifyBehaviors(lParam, context->ListViewHandle, PH_LIST_VIEW_DEFAULT_1_BEHAVIORS);

            switch (header->code)
            {
            case PSN_QUERYINITIALFOCUS:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)context->ListViewHandle);
                return TRUE;
            }

            REFLECT_MESSAGE_DLG(hwndDlg, context->ListViewHandle, uMsg, wParam, lParam);
        }
        break;
    case WM_CONTEXTMENU:
        {
            if ((HWND)wParam == context->ListViewHandle)
            {
                POINT point;
                PPH_EMENU menu;
                PPH_EMENU item;
                PVOID *listviewItems;
                ULONG numberOfItems;

                point.x = GET_X_LPARAM(lParam);
                point.y = GET_Y_LPARAM(lParam);

                if (point.x == -1 && point.y == -1)
                    PhGetIListViewContextMenuPoint(context->ListViewClass, &point);

                PhGetSelectedIListViewItemParams(context->ListViewClass, &listviewItems, &numberOfItems);

                if (numberOfItems != 0)
                {
                    menu = PhCreateEMenu();
                    PhInsertEMenuItem(menu, PhCreateEMenuItem(0, IDC_COPY, L"&Copy", NULL, NULL), ULONG_MAX);
                    PhInsertCopyIListViewEMenuItem(menu, IDC_COPY, context->ListViewHandle, context->ListViewClass);

                    item = PhShowEMenu(
                        menu,
                        hwndDlg,
                        PH_EMENU_SHOW_SEND_COMMAND | PH_EMENU_SHOW_LEFTRIGHT,
                        PH_ALIGN_LEFT | PH_ALIGN_TOP,
                        point.x,
                        point.y
                        );

                    if (item)
                    {
                        BOOLEAN handled = FALSE;

                        handled = PhHandleCopyListViewEMenuItem(item);

                        //if (!handled && PhPluginsEnabled)
                        //    handled = PhPluginTriggerEMenuItem(&menuInfo, item);

                        if (!handled)
                        {
                            switch (item->Id)
                            {
                            case IDC_COPY:
                                {
                                    PhCopyIListView(context->ListViewHandle, context->ListViewClass);
                                }
                                break;
                            }
                        }
                    }

                    PhDestroyEMenu(menu);
                }

                PhFree(listviewItems);
            }
        }
        break;
    case WM_CTLCOLORBTN:
        return HANDLE_WM_CTLCOLORBTN(hwndDlg, wParam, lParam, PhWindowThemeControlColor);
    case WM_CTLCOLORDLG:
        return HANDLE_WM_CTLCOLORDLG(hwndDlg, wParam, lParam, PhWindowThemeControlColor);
    case WM_CTLCOLORSTATIC:
        return HANDLE_WM_CTLCOLORSTATIC(hwndDlg, wParam, lParam, PhWindowThemeControlColor);
    }

    return FALSE;
}
