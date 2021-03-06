// gyazowin.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "gyazowin.h"

// グローバル変数:
HINSTANCE hInst;							// 現在のインターフェイス
TCHAR *szTitle			= _T("gyazowin");	// タイトル バーのテキスト
TCHAR *szWindowClass	= _T("GYAZOWIN");	// メイン ウィンドウ クラス名

int ofX, ofY;	// 画面オフセット
std::map<std::wstring, std::wstring> g_Settings;

// プロトタイプ宣言
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL				isPng(LPCTSTR fileName);
VOID				drawRubberband(HDC hdc, LPRECT newRect, BOOL erase);
VOID				execUrl(const char* str);
VOID				setClipBoardText(const char* str);
BOOL				convertPNG(LPCTSTR destFile, LPCTSTR srcFile);
BOOL				savePNG(LPCTSTR fileName, HBITMAP newBMP);
BOOL				uploadFile(HWND hwnd, LPCTSTR fileName);
std::string			getId();
std::map<std::wstring, std::wstring> loadSettings(LPCWSTR fileName, LPCWSTR sectionName);

// エントリーポイント
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	TCHAR	szThisPath[MAX_PATH];
	DWORD   sLen;

	// 自身のディレクトリを取得する
	sLen = GetModuleFileName(NULL, szThisPath, MAX_PATH);
	for(unsigned int i = sLen; i >= 0; i--) {
		if(szThisPath[i] == _T('\\')) {
			szThisPath[i] = _T('\0');
			break;
		}
	}

	// カレントディレクトリを exe と同じ場所に設定
	SetCurrentDirectory(szThisPath);

	g_Settings = loadSettings(_T("gyazowin+.ini"), _T("gyazowin+"));

	// 引数にファイルが指定されていたら
	if ( 2 == __argc )
	{
		// ファイルをアップロードして終了
		if (isPng(__targv[1])) {
			// PNG はそのままupload
			uploadFile(NULL, __targv[1]);
		}else {
			// PNG 形式に変換
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (convertPNG(tmpFile, __targv[1])) {
				//アップロード
				uploadFile(NULL, tmpFile);
			} else {
				// PNGに変換できなかった...
				MessageBox(NULL, _T("cannot convert this image"), _T("ERROR"), 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// ウィンドウクラスを登録
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// 設定をロードする
std::map<std::wstring, std::wstring> loadSettings(LPCWSTR fileName, LPCWSTR sectionName)
{
	std::map<std::wstring, std::wstring> settings;
	TCHAR wcFilePath[MAX_PATH];
	TCHAR wcSettings[32767];
	TCHAR *lpwcCurrent;
	TCHAR *lpwcString;
	TCHAR *lpwcContext;
	size_t len;
	std::wstring key;
	std::wstring value;

	GetFullPathName(fileName, MAX_PATH, wcFilePath, NULL);
	GetPrivateProfileSection(sectionName, wcSettings, 32767, wcFilePath);

	lpwcCurrent = wcSettings;
	while (*lpwcCurrent) {
		len = wcslen(lpwcCurrent);
		lpwcString = wcstok_s(lpwcCurrent, _T("="), &lpwcContext);
		key = std::wstring(lpwcString);
		value = std::wstring(lpwcString + wcslen(lpwcString) + 1);
		settings[key] = value;
		lpwcCurrent += len + 1;
	}

	return settings;
}

// ヘッダを見て PNG 画像かどうか(一応)チェック
BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];
	
	FILE *fp = NULL;
	
	if (0 != _tfopen_s(&fp, fileName, _T("rb")) ||
		8 != fread(readHead, 1, 8, fp)) {
		// ファイルが読めない	
		return FALSE;
	}
	fclose(fp);
	
	// compare
	for(unsigned int i=0;i<8;i++)
		if(pngHead[i] != readHead[i]) return FALSE;

	return TRUE;

}

// ウィンドウクラスを登録
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style         = 0;							// WM_PAINT を送らない
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + のカーソル
	wc.hbrBackground = 0;							// 背景も設定しない
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}


// インスタンスの初期化（全画面をウィンドウで覆う）
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

	int x, y, w, h;

	// 仮想スクリーン全体をカバー
	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// x, y のオフセット値を覚えておく
	ofX = x; ofY = y;

	// 完全に透過したウィンドウを作る
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);

	// 作れなかった...?
	if (!hWnd) return FALSE;
	
	// 全画面を覆う
	MoveWindow(hWnd, x, y, w, h, FALSE);
	
	// nCmdShow を無視 (SW_MAXIMIZE とかされると困る)
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	return TRUE;
}

// 指定されたフォーマットに対応する Encoder の CLSID を取得する
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

// ラバーバンドを描画.
VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	
	static BOOL firstDraw = TRUE;	// 1 回目は前のバンドの消去を行わない
	static RECT lastRect  = {0};	// 最後に描画したバンド
	
	// XOR で描画
	int hPreRop = SetROP2(hdc, R2_XORPEN);

	// 点線
	HPEN hPen = CreatePen(PS_DOT , 1, 0);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if(!firstDraw) {
		// 前のを消す
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);
	} else {
		firstDraw = FALSE;
	}
	
	// 新しい座標を記憶
	lastRect = *newRect;
	
	if (!erase) {

		// 枠を描画
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);

	}

	// 後処理
	SetROP2(hdc, hPreRop);
	DeleteObject(hPen);

}

// PNG 形式に変換
BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ の初期化
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (0 == b->GetLastStatus()) {
		if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
			// save!
			if (0 == b->Save(destFile, &clsidEncoder, 0) ) {
					// 保存できた
					res = TRUE;
			}
		}
	}

	// 後始末
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// PNG 形式で保存 (GDI+ 使用)
BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ の初期化
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	// HBITMAP から Bitmap を作成
	Bitmap *b = new Bitmap(newBMP, NULL);
	
	if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
		// save!
		if (0 ==
			b->Save(fileName, &clsidEncoder, 0) ) {
				// 保存できた
				res = TRUE;
		}
	}
	
	// 後始末
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};
	
	switch (message)
	{
	
	case WM_RBUTTONDOWN:
		// キャンセル
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		if (onClip) {
			
			// 新しい座標をセット
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;
			
			hdc = GetDC(NULL);
			drawRubberband(hdc, &clipRect, FALSE);
			ReleaseDC(NULL, hdc);
			
			// EndPaint(hWnd, &ps);
		}
		break;
	

	case WM_LBUTTONDOWN:
		{
			// クリップ開始
			onClip = TRUE;
			
			// 初期位置をセット
			clipRect.left = LOWORD(lParam) + ofX;
			clipRect.top  = HIWORD(lParam) + ofY;
			
			// マウスをキャプチャ
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{
			// クリップ終了
			onClip = FALSE;
			
			// マウスのキャプチャを解除
			ReleaseCapture();
		
			// 新しい座標をセット
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;

			// 画面に直接描画，って形
			HDC hdc = GetDC(NULL);

			// 線を消す
			drawRubberband(hdc, &clipRect, TRUE);

			// 座標チェック
			if ( clipRect.right  < clipRect.left ) {
				int tmp = clipRect.left;
				clipRect.left   = clipRect.right;
				clipRect.right  = tmp;
			}
			if ( clipRect.bottom < clipRect.top  ) {
				int tmp = clipRect.top;
				clipRect.top    = clipRect.bottom;
				clipRect.bottom = tmp;
			}
			
			// 画像のキャプチャ
			int iWidth, iHeight;
			iWidth  = clipRect.right  - clipRect.left + 1;
			iHeight = clipRect.bottom - clipRect.top  + 1;

			if(iWidth == 0 || iHeight == 0) {
				// 画像になってない, なにもしない
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// ビットマップバッファを作成
			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);
			
			// 関連づけ
			SelectObject(newDC, newBMP);

			// 画像を取得
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);
			
			// ウィンドウを隠す!
			ShowWindow(hWnd, SW_HIDE);
			
			/*
			// 画像をクリップボードにコピー
			if ( OpenClipboard(hWnd) ) {
				// 消去
				EmptyClipboard();
				// セット
				SetClipboardData(CF_BITMAP, newBMP);
				// 閉じる
				CloseClipboard();
			}
			*/
			
			// テンポラリファイル名を決定
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (savePNG(tmpFile, newBMP)) {

				// うｐ
				if (!uploadFile(hWnd, tmpFile)) {
					// アップロードに失敗...
					// エラーメッセージは既に表示されている

					/*
					TCHAR sysDir[MAX_PATH];
					if (SUCCEEDED(StringCchCopy(sysDir, MAX_PATH, tmpFile)) &&
						SUCCEEDED(StringCchCat(sysDir, MAX_PATH, _T(".png")))) {
						
						MoveFile(tmpFile, sysDir);
						SHELLEXECUTEINFO lsw = {0};
						
						lsw.hwnd	= hWnd;
						lsw.cbSize	= sizeof(SHELLEXECUTEINFO);
						lsw.lpVerb	= _T("open");
						lsw.lpFile	= sysDir;

						ShellExecuteEx(&lsw);
					}
					*/
				}
			} else {
				// PNG保存失敗...
				MessageBox(hWnd, _T("cannot save png image"), _T("ERROR"), 
					MB_OK | MB_ICONERROR);
			}

			// 後始末
			DeleteFile(tmpFile);
			
			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// クリップボードに文字列をコピー
VOID setClipBoardText(const char* str)
{

	HGLOBAL hText;
	char    *pText;
	size_t  slen;

	slen  = strlen(str) + 1; // NULL

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (char *)GlobalLock(hText);
	strncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);
	
	// クリップボードを開く
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hText);
	CloseClipboard();

	// 解放
	GlobalFree(hText);
}

// 指定された URL (char*) をブラウザで開く
VOID execUrl(const char* str)
{
	size_t  slen;
	size_t  dcount;
	slen  = strlen(str) + 1; // NULL

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));
	
	// ワイド文字に変換
	mbstowcs_s(&dcount, wcUrl, slen, str, slen);
	
	// open コマンドを実行
	SHELLEXECUTEINFO lsw = {0};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}

// ID を生成・ロードする
std::string getId()
{
	const char*	 idFile			= "id.txt";
	std::string idStr;

	// まずはファイルから ID をロード
	std::ifstream ifs;

	ifs.open(idFile);
	if (! ifs.fail()) {
		// ID を読み込む
		ifs >> idStr;
		ifs.close();
	} else{
		// defaultを設定: 日付(strftime)
		char		timebuf[64];
		struct tm	dt;
		time_t		now	= time(NULL);

		localtime_s(&dt, &now);
		strftime(timebuf, 64, "%Y%m%d%H%M%S", &dt);
		
		// ID 確定
		idStr = timebuf;

		// ID を保存する
		std::ofstream ofs;
		ofs.open(idFile);
		ofs << idStr;
		ofs.close();
	}

	return idStr;
}

// PNG ファイルをアップロードする.
BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					// 改行(CR+LF)
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;	// 送信メッセージ
	std::string			idStr;	// ID

	LPCWSTR lpwcUploadServer;	// アップロード先サーバ
	LPCWSTR lpwcUploadPath;		// アップロード先パス

	LPCWSTR lpwcId;			// 認証用ID
	LPCWSTR lpwcPassword;	// 認証用パスワード

	DWORD dwFlags;	// フラグ
	INTERNET_PORT port;		// ポート

	// アップロード確認
	if (g_Settings.count(L"up_dialog") && g_Settings[L"up_dialog"] == L"yes") {
		if (MessageBox(hwnd,_T("アップロードしますか？"),_T("Question"),MB_OK|MB_ICONQUESTION|MB_YESNO) != IDYES) {
			return TRUE;
		}
	}

	// ID を取得
	idStr = getId();

	// メッセージの構成
	// -- "id" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;

	// -- "imagedata" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"data.png\"";
	buf << sCrLf;
	buf << "Content-type: image/png";	// 一応
	buf << sCrLf;
	buf << sCrLf;

	// 本文: PNG ファイルを読み込む
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail()) {
		MessageBox(hwnd, _T("png open failed"), _T("ERROR"), MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// 最後
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// メッセージ完成
	std::string oMsg(buf.str());

	// アップロード先
	if (g_Settings.count(L"upload_server")) {
		lpwcUploadServer = g_Settings[L"upload_server"].c_str();
	}else{
		lpwcUploadServer = L"gyazo.com";
	}
	if (g_Settings.count(L"upload_path")) {
		lpwcUploadPath = g_Settings[L"upload_path"].c_str();
	}else{
		lpwcUploadPath = L"/upload.cgi";
	}

	// 認証データ準備
	if (g_Settings.count(L"use_auth") && g_Settings[L"use_auth"] == L"yes") {
		if (g_Settings.count(L"auth_id")) {
			lpwcId = g_Settings[L"auth_id"].c_str();
		}else{
			lpwcId = L"";
		}
		if (g_Settings.count(L"auth_pw")) {
			lpwcPassword = g_Settings[L"auth_pw"].c_str();
		}else{
			lpwcPassword = L"";
		}
	}else{
		lpwcId = NULL;
		lpwcPassword = NULL;
	}

	// WinInet を準備 (proxy は 規定の設定を利用)
	HINTERNET hSession    = InternetOpen(szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(NULL == hSession) {
		MessageBox(hwnd, _T("cannot configure wininet"),
			_T("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// SSL
	dwFlags = INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD;
	if (g_Settings.count(L"use_ssl") && g_Settings[L"use_ssl"] == L"yes") {
		dwFlags |= INTERNET_FLAG_SECURE;
		if (g_Settings.count(L"ssl_check_cert") && g_Settings[L"ssl_check_cert"] == L"no") {
			dwFlags |= INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
		}
	}

	// port
	port = INTERNET_DEFAULT_HTTP_PORT;
	if (g_Settings.count(L"port")) {
		port = _ttoi(g_Settings[L"port"].c_str());
	}

	// 接続先
	HINTERNET hConnection = InternetConnect(hSession, 
		lpwcUploadServer, port,
		lpwcId, lpwcPassword, INTERNET_SERVICE_HTTP, 0, NULL);
	if(NULL == hConnection) {
		MessageBox(hwnd, _T("cannot initiate connection"),
			_T("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// 要求先の設定
	HINTERNET hRequest    = HttpOpenRequest(hConnection,
		_T("POST"), lpwcUploadPath, NULL,
		NULL, NULL, dwFlags, NULL);
	if(NULL == hRequest) {
		MessageBox(hwnd, _T("cannot compose post request"),
			_T("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// 無効なCAファイルを許可する
	// HttpSendRequest GetLastError 12045(ERROR_INTERNET_INVALID_CA) 対策
	// http://support.microsoft.com/kb/182888/ja
	if (g_Settings.count(L"use_ssl") && g_Settings[L"use_ssl"] == L"yes"){
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);

		InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags) );
	}

	// 要求を送信
	if (HttpSendRequest(hRequest,
                    szHeader,
					lstrlen(szHeader),
                    (LPVOID)oMsg.c_str(),
					(DWORD) oMsg.length()))
	{
		// 要求は成功
		
		DWORD resLen = 8;
		TCHAR resCode[8];

		// status code を取得
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if( _ttoi(resCode) != 200 ) {
			// upload 失敗 (status error)
			MessageBox(hwnd, _T("failed to upload (unexpected result code, under maintainance?)"),
				_T("Error"), MB_ICONERROR | MB_OK);
		} else {
			// upload 成功，結果 (URL) を読取る
			DWORD len;
			char  resbuf[1024];
			std::string result;
			
			// そんなに長いことはないけどまあ一応
			while(InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			// 取得結果は NULL terminate されていないので
			result += '\0';

			// クリップボードに URL をコピー
			if (g_Settings.count(L"copy_url") && g_Settings[L"copy_url"] == L"yes") {
				setClipBoardText(result.c_str());
				if (g_Settings.count(L"copy_dialog") && g_Settings[L"copy_dialog"] == L"yes") {
					MessageBox(hwnd,_T("クリップボードにアドレスをコピーしました"),_T("Info"),MB_OK|MB_ICONINFORMATION);
				}
			}
			
			// URL を起動
			if (g_Settings.count(L"open_browser") && g_Settings[L"open_browser"] == L"yes") {
				execUrl(result.c_str());
			}

			return TRUE;
		}
	} else {
		// アップロード失敗...
		MessageBox(hwnd, _T("failed to upload"), _T("Error"), MB_ICONERROR | MB_OK);
	}

	return FALSE;

}