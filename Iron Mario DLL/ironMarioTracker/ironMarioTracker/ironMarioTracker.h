// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the IRONMARIOTRACKER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// IRONMARIOTRACKER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IRONMARIOTRACKER_EXPORTS
#define IRONMARIOTRACKER_API __declspec(dllexport)
#else
#define IRONMARIOTRACKER_API __declspec(dllimport)
#endif

// This class is exported from the dll
class IRONMARIOTRACKER_API CironMarioTracker {
public:
	CironMarioTracker(void);
	// TODO: add your methods here.
};

extern IRONMARIOTRACKER_API int nironMarioTracker;

IRONMARIOTRACKER_API int fnironMarioTracker(void);
