// gyazowin.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "gyazowin.h"

// �O���[�o���ϐ�:
HINSTANCE hInst;							// ���݂̃C���^�[�t�F�C�X
TCHAR *szTitle			= _T("gyazowin");	// �^�C�g�� �o�[�̃e�L�X�g
TCHAR *szWindowClass	= _T("GYAZOWIN");	// ���C�� �E�B���h�E �N���X��

int ofX, ofY;	// ��ʃI�t�Z�b�g
std::map<std::wstring, std::wstring> g_Settings;

// �v���g�^�C�v�錾
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

// �G���g���[�|�C���g
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

	// ���g�̃f�B���N�g�����擾����
	sLen = GetModuleFileName(NULL, szThisPath, MAX_PATH);
	for(unsigned int i = sLen; i >= 0; i--) {
		if(szThisPath[i] == _T('\\')) {
			szThisPath[i] = _T('\0');
			break;
		}
	}

	// �J�����g�f�B���N�g���� exe �Ɠ����ꏊ�ɐݒ�
	SetCurrentDirectory(szThisPath);

	g_Settings = loadSettings(_T("gyazowin+.ini"), _T("gyazowin+"));

	// �����Ƀt�@�C�����w�肳��Ă�����
	if ( 2 == __argc )
	{
		// �t�@�C�����A�b�v���[�h���ďI��
		if (isPng(__targv[1])) {
			// PNG �͂��̂܂�upload
			uploadFile(NULL, __targv[1]);
		}else {
			// PNG �`���ɕϊ�
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (convertPNG(tmpFile, __targv[1])) {
				//�A�b�v���[�h
				uploadFile(NULL, tmpFile);
			} else {
				// PNG�ɕϊ��ł��Ȃ�����...
				MessageBox(NULL, _T("cannot convert this image"), _T("ERROR"), 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// �E�B���h�E�N���X��o�^
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	// ���C�� ���b�Z�[�W ���[�v:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// �ݒ�����[�h����
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

// �w�b�_������ PNG �摜���ǂ���(�ꉞ)�`�F�b�N
BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];
	
	FILE *fp = NULL;
	
	if (0 != _tfopen_s(&fp, fileName, _T("rb")) ||
		8 != fread(readHead, 1, 8, fp)) {
		// �t�@�C�����ǂ߂Ȃ�	
		return FALSE;
	}
	fclose(fp);
	
	// compare
	for(unsigned int i=0;i<8;i++)
		if(pngHead[i] != readHead[i]) return FALSE;

	return TRUE;

}

// �E�B���h�E�N���X��o�^
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style         = 0;							// WM_PAINT �𑗂�Ȃ�
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GYAZOWIN));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + �̃J�[�\��
	wc.hbrBackground = 0;							// �w�i���ݒ肵�Ȃ�
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}


// �C���X�^���X�̏������i�S��ʂ��E�B���h�E�ŕ����j
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

	int x, y, w, h;

	// ���z�X�N���[���S�̂��J�o�[
	x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// x, y �̃I�t�Z�b�g�l���o���Ă���
	ofX = x; ofY = y;

	// ���S�ɓ��߂����E�B���h�E�����
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if(_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);

	// ���Ȃ�����...?
	if (!hWnd) return FALSE;
	
	// �S��ʂ𕢂�
	MoveWindow(hWnd, x, y, w, h, FALSE);
	
	// nCmdShow �𖳎� (SW_MAXIMIZE �Ƃ������ƍ���)
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	return TRUE;
}

// �w�肳�ꂽ�t�H�[�}�b�g�ɑΉ����� Encoder �� CLSID ���擾����
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

// ���o�[�o���h��`��.
VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	
	static BOOL firstDraw = TRUE;	// 1 ��ڂ͑O�̃o���h�̏������s��Ȃ�
	static RECT lastRect  = {0};	// �Ō�ɕ`�悵���o���h
	
	// XOR �ŕ`��
	int hPreRop = SetROP2(hdc, R2_XORPEN);

	// �_��
	HPEN hPen = CreatePen(PS_DOT , 1, 0);
	SelectObject(hdc, hPen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if(!firstDraw) {
		// �O�̂�����
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);
	} else {
		firstDraw = FALSE;
	}
	
	// �V�������W���L��
	lastRect = *newRect;
	
	if (!erase) {

		// �g��`��
		Rectangle(hdc, lastRect.left, lastRect.top, 
			lastRect.right + 1, lastRect.bottom + 1);

	}

	// �㏈��
	SetROP2(hdc, hPreRop);
	DeleteObject(hPen);

}

// PNG �`���ɕϊ�
BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ �̏�����
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (0 == b->GetLastStatus()) {
		if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
			// save!
			if (0 == b->Save(destFile, &clsidEncoder, 0) ) {
					// �ۑ��ł���
					res = TRUE;
			}
		}
	}

	// ��n��
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// PNG �`���ŕۑ� (GDI+ �g�p)
BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				res = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// GDI+ �̏�����
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	// HBITMAP ���� Bitmap ���쐬
	Bitmap *b = new Bitmap(newBMP, NULL);
	
	if (GetEncoderClsid(L"image/png", &clsidEncoder)) {
		// save!
		if (0 ==
			b->Save(fileName, &clsidEncoder, 0) ) {
				// �ۑ��ł���
				res = TRUE;
		}
	}
	
	// ��n��
	delete b;
	GdiplusShutdown(gdiplusToken);

	return res;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	
	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};
	
	switch (message)
	{
	
	case WM_RBUTTONDOWN:
		// �L�����Z��
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		if (onClip) {
			
			// �V�������W���Z�b�g
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
			// �N���b�v�J�n
			onClip = TRUE;
			
			// �����ʒu���Z�b�g
			clipRect.left = LOWORD(lParam) + ofX;
			clipRect.top  = HIWORD(lParam) + ofY;
			
			// �}�E�X���L���v�`��
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{
			// �N���b�v�I��
			onClip = FALSE;
			
			// �}�E�X�̃L���v�`��������
			ReleaseCapture();
		
			// �V�������W���Z�b�g
			clipRect.right  = LOWORD(lParam) + ofX;
			clipRect.bottom = HIWORD(lParam) + ofY;

			// ��ʂɒ��ڕ`��C���Č`
			HDC hdc = GetDC(NULL);

			// ��������
			drawRubberband(hdc, &clipRect, TRUE);

			// ���W�`�F�b�N
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
			
			// �摜�̃L���v�`��
			int iWidth, iHeight;
			iWidth  = clipRect.right  - clipRect.left + 1;
			iHeight = clipRect.bottom - clipRect.top  + 1;

			if(iWidth == 0 || iHeight == 0) {
				// �摜�ɂȂ��ĂȂ�, �Ȃɂ����Ȃ�
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// �r�b�g�}�b�v�o�b�t�@���쐬
			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);
			
			// �֘A�Â�
			SelectObject(newDC, newBMP);

			// �摜���擾
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);
			
			// �E�B���h�E���B��!
			ShowWindow(hWnd, SW_HIDE);
			
			/*
			// �摜���N���b�v�{�[�h�ɃR�s�[
			if ( OpenClipboard(hWnd) ) {
				// ����
				EmptyClipboard();
				// �Z�b�g
				SetClipboardData(CF_BITMAP, newBMP);
				// ����
				CloseClipboard();
			}
			*/
			
			// �e���|�����t�@�C����������
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);
			
			if (savePNG(tmpFile, newBMP)) {

				// ����
				if (!uploadFile(hWnd, tmpFile)) {
					// �A�b�v���[�h�Ɏ��s...
					// �G���[���b�Z�[�W�͊��ɕ\������Ă���

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
				// PNG�ۑ����s...
				MessageBox(hWnd, _T("cannot save png image"), _T("ERROR"), 
					MB_OK | MB_ICONERROR);
			}

			// ��n��
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

// �N���b�v�{�[�h�ɕ�������R�s�[
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
	
	// �N���b�v�{�[�h���J��
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hText);
	CloseClipboard();

	// ���
	GlobalFree(hText);
}

// �w�肳�ꂽ URL (char*) ���u���E�U�ŊJ��
VOID execUrl(const char* str)
{
	size_t  slen;
	size_t  dcount;
	slen  = strlen(str) + 1; // NULL

	TCHAR *wcUrl = (TCHAR *)malloc(slen * sizeof(TCHAR));
	
	// ���C�h�����ɕϊ�
	mbstowcs_s(&dcount, wcUrl, slen, str, slen);
	
	// open �R�}���h�����s
	SHELLEXECUTEINFO lsw = {0};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = wcUrl;

	ShellExecuteEx(&lsw);

	free(wcUrl);
}

// ID �𐶐��E���[�h����
std::string getId()
{
	const char*	 idFile			= "id.txt";
	std::string idStr;

	// �܂��̓t�@�C������ ID �����[�h
	std::ifstream ifs;

	ifs.open(idFile);
	if (! ifs.fail()) {
		// ID ��ǂݍ���
		ifs >> idStr;
		ifs.close();
	} else{
		// default��ݒ�: ���t(strftime)
		char		timebuf[64];
		struct tm	dt;
		time_t		now	= time(NULL);

		localtime_s(&dt, &now);
		strftime(timebuf, 64, "%Y%m%d%H%M%S", &dt);
		
		// ID �m��
		idStr = timebuf;

		// ID ��ۑ�����
		std::ofstream ofs;
		ofs.open(idFile);
		ofs << idStr;
		ofs.close();
	}

	return idStr;
}

// PNG �t�@�C�����A�b�v���[�h����.
BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					// ���s(CR+LF)
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;	// ���M���b�Z�[�W
	std::string			idStr;	// ID

	LPCWSTR lpwcUploadServer;	// �A�b�v���[�h��T�[�o
	LPCWSTR lpwcUploadPath;		// �A�b�v���[�h��p�X

	LPCWSTR lpwcId;			// �F�ؗpID
	LPCWSTR lpwcPassword;	// �F�ؗp�p�X���[�h

	DWORD dwFlags;	// �t���O
	INTERNET_PORT port;		// �|�[�g

	// �A�b�v���[�h�m�F
	if (g_Settings.count(L"up_dialog") && g_Settings[L"up_dialog"] == L"yes") {
		if (MessageBox(hwnd,_T("�A�b�v���[�h���܂����H"),_T("Question"),MB_OK|MB_ICONQUESTION|MB_YESNO) != IDYES) {
			return TRUE;
		}
	}

	// ID ���擾
	idStr = getId();

	// ���b�Z�[�W�̍\��
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
	buf << "Content-type: image/png";	// �ꉞ
	buf << sCrLf;
	buf << sCrLf;

	// �{��: PNG �t�@�C����ǂݍ���
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail()) {
		MessageBox(hwnd, _T("png open failed"), _T("ERROR"), MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// �Ō�
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// ���b�Z�[�W����
	std::string oMsg(buf.str());

	// �A�b�v���[�h��
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

	// �F�؃f�[�^����
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

	// WinInet ������ (proxy �� �K��̐ݒ�𗘗p)
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

	// �ڑ���
	HINTERNET hConnection = InternetConnect(hSession, 
		lpwcUploadServer, port,
		lpwcId, lpwcPassword, INTERNET_SERVICE_HTTP, 0, NULL);
	if(NULL == hConnection) {
		MessageBox(hwnd, _T("cannot initiate connection"),
			_T("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// �v����̐ݒ�
	HINTERNET hRequest    = HttpOpenRequest(hConnection,
		_T("POST"), lpwcUploadPath, NULL,
		NULL, NULL, dwFlags, NULL);
	if(NULL == hRequest) {
		MessageBox(hwnd, _T("cannot compose post request"),
			_T("Error"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// ������CA�t�@�C����������
	// HttpSendRequest GetLastError 12045(ERROR_INTERNET_INVALID_CA) �΍�
	// http://support.microsoft.com/kb/182888/ja
	if (g_Settings.count(L"use_ssl") && g_Settings[L"use_ssl"] == L"yes"){
		DWORD dwFlags;
		DWORD dwBuffLen = sizeof(dwFlags);

		InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags) );
	}

	// �v���𑗐M
	if (HttpSendRequest(hRequest,
                    szHeader,
					lstrlen(szHeader),
                    (LPVOID)oMsg.c_str(),
					(DWORD) oMsg.length()))
	{
		// �v���͐���
		
		DWORD resLen = 8;
		TCHAR resCode[8];

		// status code ���擾
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if( _ttoi(resCode) != 200 ) {
			// upload ���s (status error)
			MessageBox(hwnd, _T("failed to upload (unexpected result code, under maintainance?)"),
				_T("Error"), MB_ICONERROR | MB_OK);
		} else {
			// upload �����C���� (URL) ��ǎ��
			DWORD len;
			char  resbuf[1024];
			std::string result;
			
			// ����Ȃɒ������Ƃ͂Ȃ����ǂ܂��ꉞ
			while(InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			// �擾���ʂ� NULL terminate ����Ă��Ȃ��̂�
			result += '\0';

			// �N���b�v�{�[�h�� URL ���R�s�[
			if (g_Settings.count(L"copy_url") && g_Settings[L"copy_url"] == L"yes") {
				setClipBoardText(result.c_str());
				if (g_Settings.count(L"copy_dialog") && g_Settings[L"copy_dialog"] == L"yes") {
					MessageBox(hwnd,_T("�N���b�v�{�[�h�ɃA�h���X���R�s�[���܂���"),_T("Info"),MB_OK|MB_ICONINFORMATION);
				}
			}
			
			// URL ���N��
			if (g_Settings.count(L"open_browser") && g_Settings[L"open_browser"] == L"yes") {
				execUrl(result.c_str());
			}

			return TRUE;
		}
	} else {
		// �A�b�v���[�h���s...
		MessageBox(hwnd, _T("failed to upload"), _T("Error"), MB_ICONERROR | MB_OK);
	}

	return FALSE;

}