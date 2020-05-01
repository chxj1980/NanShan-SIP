#ifndef NVSTypeDefine_h__
#define NVSTypeDefine_h__

#pragma once
//V1.5 版本    (与V1.4版本一样头文件)
typedef char XY_CHAR;
typedef unsigned char XY_BYTE;
typedef short XY_SHORT;
typedef unsigned short XY_USHORT;
typedef int XY_INT;
typedef unsigned int XY_UINT;
typedef long XY_LONG;
typedef float XY_FLOAT;
typedef double XY_DOUBLE;
typedef unsigned long XY_DWORD;
typedef unsigned short XY_WORD;
typedef long long XY_INT64;
typedef unsigned long long XY_UINT64;
typedef int XY_BOOL;
typedef void XY_VOID;

typedef XY_CHAR* XY_CHAR_PTR;
typedef XY_BYTE* XY_BYTE_PTR;
typedef XY_INT* XY_INT_PTR;
typedef XY_UINT* XY_UINT_PTR;
typedef XY_LONG* XY_LONG_PTR;
typedef XY_FLOAT* XY_FLOAT_PTR;
typedef XY_DOUBLE* XY_DOUBLE_PTR;
typedef XY_DWORD* XY_DWORD_PTR;
typedef XY_WORD* XY_WORD_PTR;
typedef XY_INT64* XY_INT64_PTR;
typedef XY_BOOL* XY_BOOL_PTR;
typedef XY_VOID* XY_VOID_PTR;

#define XY_TRUE  0
#define XY_FALSE -1
#define XY_NULL 0

#define XY_ISFAIL(bvalue) (bvalue)!=XY_TRUE ?true:false
#define XY_ISSUCCESS(bvalue) (bvalue)==XY_TRUE ?true:false


#define XY_ISFAIL_RETURN_FALSE(bvalue) if(XY_ISFAIL(bvalue)) return XY_FALSE;
#define XY_ISSUCCESS_RETURN_TRUE(bvalue) if(XY_ISSUCCESS(bvalue)) return XY_TRUE;


#if defined(WIN32) || defined(WIN64) || defined(_WINDOWS)
typedef  HWND XY_HWND;
#define  XY_CALLBACK __stdcall
#define  XY_WINAPI  __stdcall
#define  XY_HINSTANCE HINSTANCE
#define  XY_HANDLE	HANDLE
#else
typedef void* XY_HWND;
#define  XY_CALLBACK 
#define  CALLBACK 
#define  XY_WINAPI  
#define  XY_HINSTANCE XY_VOID_PTR
#define  XY_HANDLE XY_VOID_PTR
#define __stdcall

#ifndef LPVOID
typedef void*              LPVOID;
#endif

#ifndef HANDLE
typedef void*              HANDLE;
#endif

#ifndef LPDWORD
typedef unsigned int*      LPDWORD;
#endif
#ifndef UINT64
typedef unsigned long long UINT64;
#endif

#ifndef CHAR
typedef char CHAR;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef INT
typedef int INT;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef LONG
typedef int LONG;
#endif

#ifndef FLOAT
typedef float FLOAT;
#endif

#ifndef DOUBLE
typedef double DOUBLE;
#endif

#ifndef DWORD
typedef unsigned int DWORD;
#endif

#ifndef WORD
typedef unsigned short WORD;
#endif

#ifndef INT64
typedef long long INT64;
#endif

#ifndef UINT64
typedef unsigned long long UINT64;
#endif
#ifndef HWND
typedef unsigned int HWND;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef CHAR_PTR
typedef CHAR* CHAR_PTR;
#endif

#ifndef BYTE_PTR
typedef BYTE* BYTE_PTR;
#endif

#ifndef INT_PTR
typedef INT* INT_PTR;
#endif

#ifndef UINT_PTR
typedef UINT* UINT_PTR;
#endif

#ifndef LONG_PTR
typedef LONG* LONG_PTR;
#endif

#ifndef FLOAT_PTR
typedef FLOAT* FLOAT_PTR;
#endif

#ifndef DOUBLE_PTR
typedef DOUBLE* DOUBLE_PTR;
#endif

#ifndef DWORD_PTR
typedef DWORD* DWORD_PTR;
#endif

#ifndef WORD_PTR
typedef WORD* WORD_PTR;
#endif

#ifndef INT64_PTR
typedef INT64* INT64_PTR;
#endif

#ifndef BOOL_PTR
typedef BOOL* BOOL_PTR;
#endif

#ifndef CALLBACK
#define  CALLBACK
#endif

#ifndef	WINAPI
#define  WINAPI
#endif

#ifndef VOID
typedef void VOID;
#endif

#ifndef VOID_PTR
typedef VOID* VOID_PTR;
#endif

#ifndef HINSTANCE
#define HINSTANCE VOID_PTR
#endif

#ifndef HANDLE
#define HANDLE VOID_PTR
#endif

#endif

#endif // NVSTypeDefine_h__
