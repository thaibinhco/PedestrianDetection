
// PersonDetectionDlg.h : header file
//

#pragma once

#include "afxwin.h"
#include "CvMFCImage.h"
#include <string>
#include <cstddef>	// std::size_t
#include "HOGClassifier.h"
#include "PersonDetector.h"

// CPersonDetectionDlg dialog
class CPersonDetectionDlg : public CDialogEx
{
// Construction
public:
	CPersonDetectionDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
//#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PersonDetection_DIALOG };
//#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


private:
	HOGClassifier hogClassifier;
	PersonDetector detector;


public:
	int m_fileChooser;	// value of m_fileChooser: image = 0, video = 1, other = -1
	CString m_strPath;
	CString m_strName;
	CString m_strTrainPath;
	CString m_strTestPath;
	CString m_strSVMTrainPath;
	char m_strSVMTrain[400];
	CString m_strResPath;
	CStatic m_cPicView;
	CStatic m_cPicRes;
	cv::VideoCapture m_video;
	IplImage* m_pImage;
	IplImage* m_resImage;
	int m_pLoad;
	int m_resLoad;
	int m_bCalc;
	afx_msg void OnBnClickedBrowse();
	CString m_type;
	CString m_save;
	int m_bins;
	int m_wcell;
	int m_hcell;
	int m_wblock;
	int m_hblock;
	UINT m_wwindow;
	UINT m_hwindow;
	int m_distance;
	int m_maxSafe;

	// Cac ham tien ich
	bool getFolder(std::string& folderpath, const char* szCaption = NULL, HWND hOwner = NULL);
	//void stringExplode(string str, string separator, vector<string>* results);
	int getValueChosser(std::string strExt);
	void detectImage();
	void detectVideo();

	// Cac ham xu ly su kien tu giao dien
	afx_msg void OnBnClickedDoopen();
	afx_msg void OnBnClickedDocalc();
	float m_overlap;
	afx_msg void OnDestroy();
	afx_msg void OnStnClickedPicreview();
	afx_msg void OnBnClickedBrowseTrain();
	afx_msg void OnBnClickedStartTraining();
	afx_msg void OnBnClickedStartDetecting();
	afx_msg void OnBnClickedBrowseTest();
	afx_msg void OnBnClickedBrowseRes();
	afx_msg void OnBnClickedBrowseSvmTrain();
	afx_msg void OnBnClickedDodetect();
	afx_msg void OnStnClickedPicResult();
	afx_msg void OnBnClickedDosaveresult();
};
