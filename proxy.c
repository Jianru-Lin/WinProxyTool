// 链接声明
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Shell32.lib")

// 头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>

// 函数声明
BOOL setProxy();
BOOL unsetProxy();

void logParseArgs();
void logParseArgsSuccess();
void logParseArgsFailure();
void logArgs();

void logSetProxy();
void logSetProxySuccess();
void logSetProxyFailure();

void logUnsetProxy();
void logUnsetProxySuccess();
void logUnsetProxyFailure();

// 定义几个常用的宏
#define ActionSetProxy 1
#define ActionUnsetProxy 2

// 全局变量
LPWSTR*	argv;
int		argc;
int		action;
LPWSTR	proxyAddress;
LPWSTR	parseArgsError;

int WINAPI wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PWSTR pCmdLine,
	int nCmdShow) {

	// 附加到上级控制台
	AttachConsole(ATTACH_PARENT_PROCESS);

	// 将命令行参数分解开
	argv = CommandLineToArgvW(pCmdLine, &argc);

	// 解析输入的参数
	logParseArgs();
	if (!parseArgs()) {
		logParseArgsFailure();
		return 1;
	}
	logParseArgsSuccess();
	logArgs();

	// 根据指令不同执行不同的操作
	if (action == ActionSetProxy) {
		// 设置代理
		logSetProxy();
		if (!setProxy()) {
			logSetProxyFailure();
			return 1;
		}
		logSetProxySuccess();
	} else if (action == ActionUnsetProxy) {
		// 取消代理
		logUnsetProxy();
		if (!unsetProxy()) {
			logUnsetProxyFailure();
			return 1;
		}
		logUnsetProxySuccess();
	}

	return 0;
}

BOOL parseArgs() {
	int i = 0;
	LPWSTR p = NULL;
	LPWSTR pAction = NULL;
	LPWSTR pProxy = NULL;

	// 清空错误信息
	parseArgsError = NULL;

	for (i = 0; i < argc; ++i) {
		// 找 action=...
		p = wcsstr(argv[i], L"action=");
		if (p == argv[i]) {
			pAction = p;
			continue;
		}

		// 找 proxy=...
		p = wcsstr(argv[i], L"proxy=");
		if (p == argv[i]) {
			pProxy = p;
			continue;
		}
	}

	// 如果没找到 action=... 那么解析就失败
	if (pAction == NULL) {
		parseArgsError = L"'action' not found";
		return FALSE;
	}

	// 根据 action=... 的不同而设置不同的动作
	if (wcscmp(pAction, L"action=set") == 0) {
		action = ActionSetProxy;
	} else if (wcscmp(pAction, L"action=unset") == 0) {
		action = ActionUnsetProxy;
	} else {
		parseArgsError = L"unknown action";
		return FALSE;
	}

	// 如果 action=set 那么 pProxy 必须给出
	if (action == ActionSetProxy) {
		if (pProxy != NULL) {
			// 注意 pProxy 的值是类似 proxy=... 这样子的
			// 但是我们的 proxyAddress 不需要 proxy= 开头的部分
			// 所以指针指向的位置需要偏移一下
			proxyAddress = pProxy + wcslen(L"proxy=");
			//proxyAddress = L"http=127.0.0.1:8888;https=127.0.0.1:8888";
		} else {
			parseArgsError = L"'proxy' not found";
			return FALSE;
		}
	}

	return TRUE;
}

BOOL setProxy() {
	INTERNET_PER_CONN_OPTION_LIST	list;
	INTERNET_PER_CONN_OPTION		option[3];
	unsigned long					nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	option[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	option[0].Value.pszValue = proxyAddress;

	option[1].dwOption = INTERNET_PER_CONN_FLAGS;
	option[1].Value.dwValue = PROXY_TYPE_PROXY | PROXY_TYPE_DIRECT;

	option[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
	option[2].Value.pszValue = L"<local>";

	list.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	list.pszConnection = NULL;
	list.dwOptionCount = 3;
	list.dwOptionError = 0;
	list.pOptions = option;

	if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, nSize)) {
		return FALSE;
	}

	if (!InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0)) {
		return FALSE;
	}

	return TRUE;
}

BOOL unsetProxy() {
	INTERNET_PER_CONN_OPTION_LIST	list;
	INTERNET_PER_CONN_OPTION		option[1];
	BOOL						bReturn;
	DWORD					nSize = sizeof(list);

	option[0].dwOption = INTERNET_PER_CONN_FLAGS;
	option[0].Value.dwValue = PROXY_TYPE_DIRECT;

	list.dwSize = sizeof(list);
	list.pszConnection = NULL;
	list.dwOptionCount = 1;
	list.dwOptionError = 0;
	list.pOptions = option;

	if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, nSize)) {
		return FALSE;
	}

	if (!InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0)) {
		return FALSE;
	}

	if (!InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0)) {
		return FALSE;
	}

	return TRUE;
}

void logParseArgs() {
	wprintf(L"parse args ");
}

void logParseArgsSuccess() {
	wprintf(L"success\n");
}

void logParseArgsFailure() {
	wprintf(L"failure, %s\n", parseArgsError);
}

void logSetProxy() {
	wprintf(L"set proxy ");
}

void logSetProxySuccess() {
	wprintf(L"success\n");
}

void logSetProxyFailure() {
	wprintf(L"failure (%d)\n", GetLastError());
}

void logUnsetProxy() {
	wprintf(L"unset proxy ");
}

void logUnsetProxySuccess() {
	wprintf(L"success\n");
}

void logUnsetProxyFailure() {
	wprintf(L"failure (%d)\n", GetLastError());
}

void logArgs() {
	if (action == ActionUnsetProxy) {
		wprintf(L"unset proxy\n");
	} else if (action == ActionSetProxy) {
		wprintf(L"set proxy: %s\n", proxyAddress);
	} else {
		wprintf(L"stupid programmer, logArgs() meets unknown 'action' value\n");
	}
}