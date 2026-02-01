#ifndef TIME_H
#define TIME_H

#ifndef STD_STDIO_H
    #define STD_STDIO_H
    #include <stdio.h>
#endif // !STD_STDIO_H

#ifdef __linux__
#ifndef STD_TIME_H
    #define STD_TIME_H
    #include <time.h>
#endif // !STD_TIME_H
#endif

#ifdef _WIN64
#ifndef STD_WINDOWS_H
    #define STD_WINDOWS_H
    #include <windows.h>
#endif // !STD_WINDOWS_H
#endif

#define TIME_NANOTOSEC  1000000000ULL
#define TIME_FNANOTOSEC 1.0E9

typedef struct Time_t {
    unsigned short Nano;
    unsigned short Micro;
    unsigned short Mill;
    unsigned short Sec;
    unsigned short Min;
    unsigned short Hour;
    unsigned short Day;
    unsigned short Month;
    unsigned short Year;
} Time_t;

typedef unsigned long long Timepoint;
typedef unsigned long long Duration;
typedef double FDuration;

double Time_DNowSec(){
    #ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC,&time);
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (double)seconds + ((double)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (double)time.QuadPart / (double)freq.QuadPart;
    #endif
}
float Time_FNowSec(){
	#ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC,&time);
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (float)seconds + ((float)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (float)time.QuadPart / (float)freq.QuadPart;
    #endif
}

double Time_DSec(){
    #ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_REALTIME,&time);//CLOCK_MONOTONIC
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (double)seconds + ((double)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (double)time.QuadPart / (double)freq.QuadPart;
    #endif
}
float Time_FSec(){
	#ifdef __linux__
    struct timespec time;
    clock_gettime(CLOCK_REALTIME,&time);//CLOCK_MONOTONIC
    long seconds = time.tv_sec;
    long nanoseconds = time.tv_nsec;
    return (float)seconds + ((float)nanoseconds) / TIME_FNANOTOSEC;
    #endif
    #ifdef _WIN64
    LARGE_INTEGER freq,time;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);
    return (float)time.QuadPart / (float)freq.QuadPart;
    #endif
}

Timepoint Time_Nano(){
	return (unsigned long long)(Time_DNowSec() * 1.0E9);
}
Timepoint Time_Micro(){
	return (unsigned long long)(Time_DNowSec() * 1.0E6);
}
Timepoint Time_Mill(){
	return (unsigned long long)(Time_DNowSec() * 1.0E3);
}
Timepoint Time_Sec(){
	return (unsigned long long)(Time_DNowSec());
}
Timepoint Time_Min(){
	return (unsigned long long)(Time_DNowSec() / 60.0);
}
Timepoint Time_Hour(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0));
}
Timepoint Time_Day(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0));
}
Timepoint Time_Month(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0 * 30.0));
}
Timepoint Time_Year(){
	return (unsigned long long)(Time_DNowSec() / (60.0 * 60.0 * 24.0 * 365.25));
}
Duration Time_ElapsedD(Timepoint Start,Timepoint End){
    return (Duration)(End - Start);
}
FDuration Time_Elapsed(Timepoint Start,Timepoint End){
    return (FDuration)(End - Start) / TIME_FNANOTOSEC;
}
Timepoint Time_SecToNano(double Secs){
	return (Timepoint)(Secs * (double)1.0E9);
}
double Time_NanoToSec(Timepoint Nanos){
	return (double)Nanos / (double)1.0E9;
}


Timepoint Time_Real_Nano(){
	return (unsigned long long)(Time_DSec() * 1.0E9);
}
Timepoint Time_Real_Micro(){
	return (unsigned long long)(Time_DSec() * 1.0E6);
}
Timepoint Time_Real_Mill(){
	return (unsigned long long)(Time_DSec() * 1.0E3);
}
Timepoint Time_Real_Sec(){
	return (unsigned long long)(Time_DSec());
}
Timepoint Time_Real_Min(){
	return (unsigned long long)(Time_DSec() / 60.0);
}
Timepoint Time_Real_Hour(){
	return (unsigned long long)(Time_DSec() / (60.0 * 60.0));
}
Timepoint Time_Real_Day(){
	return (unsigned long long)(Time_DSec() / (60.0 * 60.0 * 24.0));
}
Timepoint Time_Real_Month(){
	return (unsigned long long)(Time_DSec() / (60.0 * 60.0 * 24.0 * 30.0));
}
Timepoint Time_Real_Year(){
	return (unsigned long long)(Time_DSec() / (60.0 * 60.0 * 24.0 * 365.25));
}
FDuration Time_Real_Elapsed(Timepoint Start,Timepoint End){
    return (FDuration)(End-Start) / TIME_FNANOTOSEC;
}
Timepoint Time_Real_SecToNano(double Secs){
	return (Timepoint)(Secs * (double)1.0E9);
}
double Time_Real_NanoToSec(Timepoint Nanos){
	return (double)Nanos / (double)1.0E9;
}

unsigned char MONTH_DAYS[] = {
    31U,    // J
    28U,    // F
    31U,    // M
    30U,    // A
    31U,    // M
    30U,    // J
    31U,    // J
    31U,    // A
    30U,    // S
    31U,    // O
    30U,    // N
    31U,    // D
};

char Time_isleap(unsigned int year) {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}
Time_t Time_Get(Timepoint Nano){
    Time_t t;
    t.Nano = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Micro = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Mill = Nano - (Nano / 1000) * 1000;
    Nano /= 1000;
    t.Sec = Nano - (Nano / 60) * 60;
    Nano /= 60;
    t.Min = Nano - (Nano / 60) * 60;
    Nano /= 60;
    t.Hour = Nano - (Nano / 24) * 24;
    Nano /= 24;
    t.Day = Nano;
    t.Year = 1970;// Startpoint / Unix-epoche (1970-01-01 00:00:00 UTC)
    
    char leap = Time_isleap(t.Year);
    while((t.Day >= 365U + leap)){
        t.Day -= 365U + leap;
        t.Year++;
        leap = Time_isleap(t.Year);
    }
    for(int i = 0;i<12;i++){
        unsigned char days = MONTH_DAYS[i];
        if(i==1 && leap) days++;

        if(t.Day < days) break;

        t.Day -= days;
        t.Month++;
    }

    t.Hour++;
    t.Month++;
    t.Day++;
    
    t.Hour++;// MEZ(DE) = UTC + 1
    
    if(t.Hour >= 24){
        t.Hour -= 24;
        t.Day++;
    }
    return t;
}
void Time_Str(char* Buffer,unsigned long long Nano){
    Time_t t = Time_Get(Nano);
    sprintf(Buffer,"%2d.%2d.%2d [%2d:%2d:%2d]",t.Day,t.Month,t.Year,t.Hour,t.Min,t.Sec);
}
void Time_Sleep(double Duration){
    Timepoint Start = Time_Nano();
    while(Time_Elapsed(Start,Time_Nano())<Duration){}
}

#endif