//
/// Timer class
//
#ifndef TIMER_INC
#define TIMER_INC

class CTimer {
private:
	int lowshift;
	double pfreq;

	unsigned int oldtime;
	double miniTime;
public:
	CTimer(){}

	void Start();
	void Update();
	double DeltaTime;
	double Seconds;

	void StartMiniTimer();
	double StopMiniTimer();
};

#endif



