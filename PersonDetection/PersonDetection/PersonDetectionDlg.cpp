
// PersonDetectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PersonDetectionApp.h"
#include "PersonDetectionDlg.h"
#include "afxdialogex.h"
#include <algorithm>
#include <string>
#include <cmath>
#include <ctime>
#include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPersonDetectionDlg dialog
///"./Dataset/Train/TrainSVM.xml"

CPersonDetectionDlg::CPersonDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPersonDetectionDlg::IDD, pParent)
	, m_fileChooser(-1)
	, m_strPath(_T(""))
	, m_strName(_T(""))
	, m_strResPath(_T(""))
	, m_strTrainPath(_T(""))
	, m_strTestPath(_T(""))
	, m_strSVMTrainPath(_T("Dataset/Train/TrainSVM.xml"))
	, m_pImage(NULL)
	, m_resImage(NULL)
	, m_pLoad(0)
	, m_resLoad(0)
	, m_bCalc(0)
	, m_type(_T("R-HOG"))
	, m_save(_T(""))
	, m_bins(9)
	, m_wcell(8)
	, m_hcell(8)
	, m_overlap(0.5)
	, m_wblock(2)
	, m_hblock(2)
	, m_wwindow(64)
	, m_hwindow(128)
	, m_distance(10)
	, m_maxSafe(6)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_strSVMTrain, 0, sizeof(m_strSVMTrain));
}

void CPersonDetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PATH, m_strPath);
	DDX_Text(pDX, IDC_PATH_RES, m_strResPath);
	DDX_Text(pDX, IDC_PATH_TRAIN, m_strTrainPath);
	DDX_Text(pDX, IDC_PATH_TEST, m_strTestPath);
	DDX_Text(pDX, IDC_PATH_SVM_TRAIN, m_strSVMTrainPath);
	DDX_Control(pDX, IDC_PICREVIEW, m_cPicView);
	DDX_Control(pDX, IDC_PIC_RESULT, m_cPicRes);
	DDX_Text(pDX, IDC_TYPE, m_type);
	DDX_Text(pDX, IDC_BINS, m_bins);
	DDX_Text(pDX, IDC_WCELL, m_wcell);
	DDX_Text(pDX, IDC_HCELL, m_hcell);
	DDX_Text(pDX, IDC_WBLOCK, m_wblock);
	DDX_Text(pDX, IDC_HBLOCK, m_hblock);
	DDX_Text(pDX, IDC_WWINDOW, m_wwindow);
	DDX_Text(pDX, IDC_HWINDOW, m_hwindow);
	DDX_Text(pDX, IDC_OVERLAP, m_overlap);
	DDX_Text(pDX, IDC_DISTANCE, m_distance);
	DDX_Text(pDX, IDC_MAX_SAFE, m_maxSafe);
}

BEGIN_MESSAGE_MAP(CPersonDetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSE, &CPersonDetectionDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_DoOPEN, &CPersonDetectionDlg::OnBnClickedDoopen)
	ON_BN_CLICKED(IDC_DoCALC, &CPersonDetectionDlg::OnBnClickedDocalc)
	ON_WM_DESTROY()
	ON_STN_CLICKED(IDC_PICREVIEW, &CPersonDetectionDlg::OnStnClickedPicreview)
	ON_BN_CLICKED(IDC_BROWSE_TRAIN, &CPersonDetectionDlg::OnBnClickedBrowseTrain)
	ON_BN_CLICKED(IDC_START_TRAINING, &CPersonDetectionDlg::OnBnClickedStartTraining)
	ON_BN_CLICKED(IDC_START_DETECTING, &CPersonDetectionDlg::OnBnClickedStartDetecting)
	ON_BN_CLICKED(IDC_BROWSE_TEST, &CPersonDetectionDlg::OnBnClickedBrowseTest)
	ON_BN_CLICKED(IDC_BROWSE_RES, &CPersonDetectionDlg::OnBnClickedBrowseRes)
	ON_BN_CLICKED(IDC_BROWSE_SVM_TRAIN, &CPersonDetectionDlg::OnBnClickedBrowseSvmTrain)
	ON_BN_CLICKED(IDC_DoDETECT, &CPersonDetectionDlg::OnBnClickedDodetect)
	ON_STN_CLICKED(IDC_PIC_RESULT, &CPersonDetectionDlg::OnStnClickedPicResult)
	ON_BN_CLICKED(IDC_DoSaveResult, &CPersonDetectionDlg::OnBnClickedDosaveresult)
END_MESSAGE_MAP()


// CPersonDetectionDlg message handlers

BOOL CPersonDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPersonDetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPersonDetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}

	if (m_pLoad == true)
	{
		CRect rct;
		// Pic 1
		m_cPicView.GetClientRect(&rct);

		CvMFCImage cvMFCImg;
		cvMFCImg.CopyOf(m_pImage, 1);
		cvMFCImg.DrawToHDC(m_cPicView.GetDC()->m_hDC, rct);
	}

	if (m_resLoad == true)
	{
		CRect rct;
		// Pic 1
		m_cPicRes.GetClientRect(&rct);

		CvMFCImage cvMFCImg;
		cvMFCImg.CopyOf(m_resImage, 1);
		cvMFCImg.DrawToHDC(m_cPicRes.GetDC()->m_hDC, rct);
	}
	else
	{
		CRect rct;
		// Pic 1
		m_cPicRes.GetClientRect(&rct);

		CvMFCImage cvMFCImg;
		cvMFCImg.CopyOf(m_pImage, 1);
		cvMFCImg.DrawToHDC(m_cPicRes.GetDC()->m_hDC, rct);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPersonDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::string imageExt[] = { "jpg", "jpeg", "png", "bmp" };
std::string videoExt[] = { "m4v", "mp4", "mpg", "avi" };

int CPersonDetectionDlg::getValueChosser(std::string strExt)
{
	std::transform(strExt.begin(), strExt.end(), strExt.begin(), (int(*)(int))tolower);

	// Image
	for each (std::string str in imageExt)
	{
		if (str == strExt)
		{
			return 0;
		}
	}

	// Video
	for each (std::string str in videoExt)
	{
		if (str == strExt)
		{
			return 1;
		}
	}

	// other
	return -1;
}

void CPersonDetectionDlg::OnBnClickedBrowse()
{
	char* openStr = "Image (*.jpg, *.jpeg, *.png, *.bmp)|*.jpg;*.jpeg;*.png;*.bmp|Video (*.m4v, *.mp4, *.mpg, *.avi)|*.m4v;*.mp4;*.mpg;*.avi|All Files (*.*)|*.*|";
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, NULL, OFN_EXTENSIONDIFFERENT, _T(openStr), 0);

	if (dlg.DoModal() == IDOK)
	{
		m_strPath = dlg.GetPathName();
		m_strName = dlg.GetFileName();
		UpdateData(FALSE);

		// Giai phong du lieu
		m_video.release();
		if (m_pImage != NULL)
		{
			cvReleaseImage(&m_pImage);
		}
		if (m_bCalc == true)
		{
			m_bCalc = false;
		}
		if (!m_save.IsEmpty())
		{
			m_save.Empty();
		}

		// Update du lieu
		char* str = (char*)(LPCTSTR)m_strPath;

		// Xac dinh kieu file
		CString fileExt = dlg.GetFileExt();
		std::string strExt(fileExt.operator LPCSTR());
		m_fileChooser = getValueChosser(strExt);

		switch (m_fileChooser)
		{
		case 0:
			m_pImage = cvLoadImage(str);
			break;
		case 1:
		{
			m_video.open(str);
			if (!m_video.isOpened())
			{
				MessageBox("Error opening video file!!", _T("ERROR"), MB_OK | MB_ICONINFORMATION);
				return;
			}
			cv::Mat frame;
			m_video >> frame;
			m_pImage = cvCloneImage(&(IplImage)frame);
			break;
		}
		default:
			MessageBox("Invalid file extension!!", _T("ERROR"), MB_OK | MB_ICONINFORMATION);
			return;
		}

		m_pLoad = true;
		this->Invalidate(FALSE);
	}
}

void CPersonDetectionDlg::OnBnClickedDosaveresult()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	if (m_resImage == NULL)
	{
		MessageBox("Please do detect a source image before saving!!", _T("Failed"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	CFileDialog dlg(FALSE, NULL, NULL, OFN_EXTENSIONDIFFERENT, _T("Image (*.jpg, *.jpeg, *.png, *.bmp)|*.jpg;*.jpeg;*.png;*.bmp|All Files (*.*)|*.*|"), 0);
	if (dlg.DoModal() == IDOK)
	{
		cvSaveImage(dlg.GetPathName(), m_resImage);
		MessageBox("Save Completed", _T("Notification"), MB_OK | MB_ICONINFORMATION);
	}
}

void CPersonDetectionDlg::OnStnClickedPicResult() {}

void CPersonDetectionDlg::OnBnClickedDoopen()
{
	// TODO: Add your control notification handler code here
	if (m_pLoad == true && m_bCalc == true)
	{
		ShellExecute(NULL, _T("open"), _T("c:\\windows\\notepad.exe"), m_save, _T(""), SW_SHOWNORMAL);
	}
	else {
		MessageBox(_T("Chua tinh features cho anh!"), _T("Thong bao"), MB_OK | MB_ICONEXCLAMATION);
	}
}

void CPersonDetectionDlg::OnBnClickedDocalc()
{
	// TODO: Add your control notification handler code here
	if (m_pLoad == true)
	{
		UpdateData(TRUE);

		if (m_wwindow < m_wcell * m_wblock || m_hwindow < m_hcell * m_hblock)
		{
			MessageBox(_T("Size cua window >= size cua block"), _T("Thong bao"), MB_OK | MB_ICONEXCLAMATION);
			m_wwindow = 64;
			m_hwindow = 128;
			UpdateData(FALSE);
		}

		char* imgname = (char*)(LPCTSTR)m_strPath;
		char fileFeaturesName[400];
		strcpy(fileFeaturesName, imgname);
		strcat(fileFeaturesName, "_features.txt");
		std::ofstream fileFeatures(fileFeaturesName);
		HOGProcessor hog;
		hog.setParams(cvSize(m_wcell, m_hcell), cvSize(m_wblock, m_hblock), m_overlap);
		CvMat* features;
		features = hog.calcHOGFromImage(imgname, cvSize(m_wwindow, m_hwindow), 1);
		hog.writeFeatureVector(features, "test", fileFeatures);
		m_save = fileFeaturesName;
		m_bCalc = true;
		MessageBox(_T("Da tinh xong features cho anh!"), _T("Thong bao"), MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(_T("Chua load anh!"), _T("Thong bao"), MB_OK | MB_ICONEXCLAMATION);
	}
}

void CPersonDetectionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	if (m_pImage != NULL)
	{
		cvReleaseImage(&m_pImage);
	}
	if (m_resImage != NULL)
	{
		cvReleaseImage(&m_resImage);
	}
}

void CPersonDetectionDlg::OnStnClickedPicreview()
{
	// TODO: Add your control notification handler code here
}

/*void CPersonDetectionDlg::stringExplode(string str, string separator, vector<string>* results)	// Extra function to spit string by separators
{
	int found;
	found = str.find_first_of(separator);
	while (found != string::npos)
	{
		if (found > 0)
		{
			results->push_back(str.substr(0, found));
		}
		str = str.substr(found + 1);
		found = str.find_first_of(separator);
	}
	if (str.length() > 0)
	{
		results->push_back(str);
	}
}*/

bool CPersonDetectionDlg::getFolder(std::string& folderpath, const char* szCaption, HWND hOwner)
{
	bool retVal = false;

	// The BROWSEINFO struct tells the shell 
	// How it should display the dialog.
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));

	bi.ulFlags = BIF_USENEWUI;
	bi.hwndOwner = hOwner;
	bi.lpszTitle = szCaption;

	// Must call this if using BIF_USENEWUI
	::OleInitialize(NULL);

	// Show the dialog and get the itemIDList for the 
	// Selected folder.
	LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

	if (pIDL != NULL)
	{
		// Create a buffer to store the path, then 
		// Get the path.
		char buffer[_MAX_PATH] = { '\0' };
		if (::SHGetPathFromIDList(pIDL, buffer) != 0)
		{
			// Set the string value.
			folderpath = buffer;
			retVal = true;
		}

		// Free the item id list
		CoTaskMemFree(pIDL);
	}

	::OleUninitialize();

	return retVal;
}

void CPersonDetectionDlg::OnBnClickedBrowseTrain()
{
	std::string szPath("");

	if (getFolder(szPath, "Select a folder.") == true)
	{
		UpdateData(TRUE);
		m_strTrainPath = szPath.c_str();
		UpdateData(FALSE);
		this->Invalidate(FALSE);
	}
	else
	{
		printf("No folder selected!!\n");
	}
}

void CPersonDetectionDlg::OnBnClickedBrowseTest()
{
	std::string szPath("");

	if (getFolder(szPath, "Select a folder.") == true)
	{
		UpdateData(TRUE);
		m_strTestPath = szPath.c_str();
		UpdateData(FALSE);
		this->Invalidate(FALSE);
	}
	else
	{
		printf("No folder selected!!\n");
	}
}

void CPersonDetectionDlg::OnBnClickedBrowseSvmTrain()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_EXTENSIONDIFFERENT, _T("XML (*.xml)|*.xml|All Files (*.*)|*.*|"), 0);

	if (dlg.DoModal() == IDOK)
	{
		m_strSVMTrainPath = dlg.GetPathName();
		UpdateData(FALSE);
		//this->Invalidate(FALSE);
		strcpy(m_strSVMTrain, (char*)(LPCTSTR)m_strSVMTrainPath);
		MessageBox(_T("The SVM Train Data is now loading. \nPlease wait for a few second! OK?"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
		detector.setSVMTrainPath(m_strSVMTrain);
		MessageBox(_T("SVM Train Data Loading Complete!!"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
	}
}

void CPersonDetectionDlg::OnBnClickedBrowseRes()
{
	std::string szPath("");

	if (getFolder(szPath, "Select a folder.") == true)
	{
		UpdateData(TRUE);
		m_strResPath = szPath.c_str();
		UpdateData(FALSE);
		this->Invalidate(FALSE);
	}
	else
	{
		printf("No folder selected!!\n");
	}
}

void CPersonDetectionDlg::OnBnClickedStartTraining()
{
	// Dataset/small train/
	UpdateData(FALSE);
	std::string posTrainPath(m_strTrainPath.operator LPCSTR());
	std::string negTrainPath(m_strTrainPath.operator LPCSTR());
	posTrainPath.append("/pos/*");
	negTrainPath.append("/neg/*");
	CvSize cell = cvSize(8, 8);
	CvSize window = cvSize(64, 128);
	std::string savePosTrain(m_strTrainPath.operator LPCSTR());
	std::string saveNegTrain(m_strTrainPath.operator LPCSTR());
	std::string saveTrainPath(m_strTrainPath.operator LPCSTR());
	savePosTrain.append("/train64x128_Pos.xml");
	saveNegTrain.append("/train64x128_Neg.xml");

	hogClassifier.trainDataset_64x128(posTrainPath, cell, window, (char*)savePosTrain.c_str());
	hogClassifier.trainDataset_64x128(negTrainPath, cell, window, (char*)saveNegTrain.c_str());
	/*hogClassifier.trainLargeDataset(negTrainPath, cell, window, 5, 2, (char*)saveNegTrain.c_str());*/
	//hogClassifier.trainSVM(NULL, NULL, (char*)saveTrainPath.append("/TrainSVM.xml").c_str(), (char*)savePosTrain.c_str(), (char*)saveNegTrain.c_str());
	MessageBox(_T("Hoan tat qua trinh Train!!"), _T("Congratulation"), MB_OK | MB_ICONINFORMATION);
}

void CPersonDetectionDlg::OnBnClickedStartDetecting()
{
	char path[400];
	strcpy(path, (char*)(LPCTSTR)m_strPath);

	if (strlen(m_strSVMTrain) == 0)
	{
		char* temp = (char*)(LPCTSTR)m_strSVMTrainPath;
		strcpy(m_strSVMTrain, temp);
		detector.setSVMTrainPath(m_strSVMTrain);
	}

	std::string posTestsPath(m_strTestPath.operator LPCSTR());
	std::string negTestsPath(m_strTestPath.operator LPCSTR());
	posTestsPath.append("/pos/");
	negTestsPath.append("/neg/");

	char* testDef = "Dataset/Test/testDef.txt";
	char testFile[1000];
	std::string fileName;
	std::fstream file;
	file.open(testDef, std::ios::in);

	IplImage* testImg;
	while (!file.eof())
	{
		file.getline(testFile, 1000);
		testImg = cvLoadImage(testFile, 1);
		detector.DetectMultiScale(testImg);
		if (testImg != NULL)
		{
			cvReleaseImage(&testImg);
		}
	}
	file.close();

	MessageBox(_T("Hoan tat qua trinh Detect!!"), _T("Thong bao"), MB_OK | MB_ICONINFORMATION);
}

void CPersonDetectionDlg::detectImage()
{
	IplImage* detectImage = cvCreateImage(cvSize(726, 406), m_pImage->depth, m_pImage->nChannels);
	cvResize(m_pImage, detectImage);

	clock_t startTime, endTime;
	startTime = clock();
	std::pair<IplImage*, std::vector<window>> resDetect = detector.DetectMultiScale(detectImage);
	endTime = clock();

	if (detectImage != NULL)
	{
		cvReleaseImage(&detectImage);
	}

	char totalTime[20];
	itoa((int)(endTime - startTime) / CLOCKS_PER_SEC, totalTime, 10);

	char strResult[100];
	if (resDetect.second.size() != 0)
	{
		strcpy(strResult, "Da detect duoc Nguoi.\nThoi gian Detect: ");
		cvReleaseImage(&m_resImage);
		//cvShowImage("Detect Result: ", resultImage);
		m_resImage = resDetect.first;
		m_resLoad = true;
	}
	else
	{
		m_resLoad = false;
		strcpy(strResult, "Khong co doi tuong nguoi trong anh.\nThoi gian Detect: ");
	}
	this->Invalidate(FALSE);

	int len1 = strlen(strResult);
	int len2 = strlen(totalTime);
	for (int i = 0; i < len2; i++)
	{
		strResult[i + len1] = totalTime[i];
	}
	strResult[len1 + len2] = '\0';
	strcat(strResult, " (s)");

	MessageBox(strResult, _T("Ket qua Detect!!"), MB_OK | MB_ICONINFORMATION);
}

void CPersonDetectionDlg::detectVideo()
{
	std::string strResPath = (m_strResPath == "") ? "../RES" : (char*)(LPCTSTR)m_strResPath;
	strResPath.append("/" + std::string(m_strName.operator LPCSTR()));
	mkdir(strResPath.c_str());

	// All
	std::string strResAll = strResPath;
	strResAll.append("/All");
	mkdir(strResAll.c_str());

	std::string strResAll_log = strResAll;
	strResAll_log.append("/log.txt");
	std::ofstream fo_all(strResAll_log);

	std::string strResAll_img = strResAll;
	strResAll_img.append("/Image");
	mkdir(strResAll_img.c_str());

	std::string strResAll_save;

	// Threats
	std::string strResThreats = strResPath;
	strResThreats.append("/Threats");
	mkdir(strResThreats.c_str());

	std::string strResThreats_log = strResThreats;
	strResThreats_log.append("/log.txt");
	std::ofstream fo_Threats(strResThreats_log);

	std::string strResThreats_img = strResThreats;
	strResThreats_img.append("/Image");
	mkdir(strResThreats_img.c_str());

	std::string strResThreats_save;

	// Detect
	std::pair<IplImage*, std::vector<window> > pResDetect;
	std::vector<window> vPreResWindow, vPtrWindow;
	std::vector<int> vPreAppear, vAppear;
	int preLen, resLen, appear;

	double duration;
	std::string strTime;

	cv::Mat frame;
	IplImage* detectImage;
	//int percent = %;
	cv::namedWindow("show", 1);
	while (m_video.grab())
	{
		duration = m_video.get(CV_CAP_PROP_POS_MSEC) / 1000;
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1) << duration;
		strTime = ss.str();

		/*detectImage = cvCreateImage(cvSize((int)((m_pImage->width * percent) / 100), (int)((m_pImage->height * percent) / 100)),
		m_pImage->depth, m_pImage->nChannels);*/
		
		detectImage = cvCreateImage(cvSize(726, 396), m_pImage->depth, m_pImage->nChannels);
		cvResize(m_pImage, detectImage);

		if (detectImage != NULL)
		{
			pResDetect = detector.DetectMultiScale(detectImage);
			cvReleaseImage(&m_resImage);
			m_resImage = pResDetect.first;
			m_resLoad = true;

			if (!pResDetect.second.empty())	// Co nguoi trong anh
			{
				// Ghi ra file log
				fo_all << duration << std::endl;

				// Luu lai tat ca ket qua co doi tuong
				strResAll_save = strResAll_img;
				strResAll_save.append("/" + strTime + ".jpg");
				cvSaveImage(strResAll_save.c_str(), m_resImage);
				strResAll_save.clear();

				// Luu lai cac moi hiem hoa
				preLen = vPreResWindow.size();
				resLen = pResDetect.second.size();
				vAppear.resize(resLen, 1);
				for (int pre = 0; pre < preLen; pre++)
				{
					appear = 0;
					while (appear < resLen)
					{
						if (abs(vPreResWindow[pre].x - pResDetect.second[appear].x) <= m_distance &&
							abs(vPreResWindow[pre].y - pResDetect.second[appear].y) <= m_distance)
						{
							break;
						}
						appear++;
					}
					if (appear != resLen)
					{
						pResDetect.second[appear].x = (vPreResWindow[pre].x + pResDetect.second[appear].x) / 2;
						pResDetect.second[appear].y = (vPreResWindow[pre].y + pResDetect.second[appear].y) / 2;
						vAppear[appear] = vPreAppear[pre] + 1;
					}
					else
					{
						if (vPreAppear[pre] > m_maxSafe)
						{
							cvRectangle(detectImage, cvPoint(vPreResWindow[pre].x, vPreResWindow[pre].y),
								cvPoint(vPreResWindow[pre].x + (int)vPreResWindow[pre].width, vPreResWindow[pre].y + (int)vPreResWindow[pre].height),
								cvScalar(0, 0, 0, 255), 2, 8, 0);

							// Ghi file log
							fo_Threats << duration << std::endl;

							// Luu hiem hoa
							strResThreats_save = strResThreats_img;
							strResThreats_save.append("/" + strTime + ".jpg");
							cvSaveImage(strResThreats_save.c_str(), detectImage);
							strResThreats_save.clear();
						}
					}
				}
				vPreResWindow.clear();
				vPreResWindow = move(pResDetect.second);
				vPreAppear.clear();
				vPreAppear = move(vAppear);
			}
			else
			{
				vPreResWindow.clear();
				vPreAppear.clear();
			}
			cvReleaseImage(&detectImage);
		}
		else
		{
			vPreResWindow.clear();
			vPreAppear.clear();
			pResDetect.second.clear();
			vAppear.clear();
		}

		strTime.clear();
		ss.clear();

		m_video.read(frame);
		cvReleaseImage(&m_pImage);
		m_pImage = cvCloneImage(&(IplImage)frame);
		frame.release();

		this->Invalidate(FALSE);
		if (cv::waitKey(25) >= 0) break;
	}

	cv::destroyAllWindows();
	m_video.release();

	strResPath.clear();
	strResAll.clear();
	strResAll_log.clear();
	strResAll_img.clear();
	strResAll_save.clear();
	strResThreats.clear();
	strResThreats_log.clear();
	strResThreats_img.clear();
	strResThreats_save.clear();
}

void CPersonDetectionDlg::OnBnClickedDodetect()
{
	if (m_fileChooser == -1)
	{
		MessageBox(_T("Please choose an image or video first!!"), _T("ERROR"), MB_OK | MB_ICONWARNING);
		return;
	}
	if (strlen(m_strSVMTrain) == 0)
	{
		char* temp = (char*)(LPCTSTR)m_strSVMTrainPath;
		std::ifstream ifile(temp);
		if (!ifile)
		{
			MessageBox(_T("SVM Train file is not exist!!"), _T("ERROR"), MB_OK | MB_ICONWARNING);
			return;
		}
		strcpy(m_strSVMTrain, temp);
		MessageBox(_T("The SVM Train Data is now loading.\nPlease wait for a few second! OK?"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
		detector.setSVMTrainPath(m_strSVMTrain);
		MessageBox(_T("SVM Train Data Loading Complete!!"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
	}

	UpdateData(FALSE);
	if (m_fileChooser == 0)// Image
	{
		detectImage();
	}
	else
	{
		detectVideo();
	}
}
