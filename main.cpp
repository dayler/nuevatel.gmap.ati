/* 
 * File:   main.cpp
 * Author: asalazar
 *
 * Created on July 28, 2015, 6:22 PM
 */
#include "cf/cap/gmapapp.hpp"

#include <cstdlib>
#include <iostream>

using namespace std;

class LoggerHandler : public Handler {
public:

    virtual void publish(LogRecord* logRecord)
    {
        stringstream ss;
        time_t seconds;
        seconds =  logRecord->getSeconds();
        struct tm *timestamp = localtime(&seconds);

        ss << (timestamp->tm_year + 1900) << '-';
        if ((timestamp->tm_mon + 1) < 10)
        {
            ss << '0';
        }
        ss << (timestamp->tm_mon + 1) <<'-';
        if(timestamp->tm_mday < 10)
        {
            ss << '0';
        }
        ss << timestamp->tm_mday << ' ';
        if(timestamp->tm_hour < 10)
        {
            ss << '0';
        }
        ss << timestamp->tm_hour << ':';
        if(timestamp->tm_min < 10)
        {
            ss << '0';
        }
        ss << timestamp->tm_min << ':';
        if(timestamp->tm_sec < 10)
        {
            ss << '0';
        }
        ss << timestamp->tm_sec;
        ss << ' ' << logRecord->getLevel()->getName();
        ss << ' ' << logRecord->getSourceClass();
        ss << '.' << logRecord->getSourceMethod();
        ss << ' ' << logRecord->getMessage();
        cout << ss.str() << std::endl;
    }
    

    virtual void flush()
    {
        // No op
    }
};

/*
 * 
 */
int main(int argc, char** argv)
{
    // logger
    LoggerHandler handler;
    Logger::getLogger()->setHandler(&handler);
    Properties properties;
    
    if (argc > 1)
    {
        // Properties properties;
        properties.load(argv[1]);
        cout<<argv[1]<<endl;
    }
    else
    {
        properties.load("/cf/properties/gmap.ati.10.properties");
        cout<<"using default /cf/properties/gmap.ati.10.properties"<<endl;
    }

    try
    {
        GMAPApp gmapApp(argc, argv, &properties);
        gmapApp.start();
        int s = 0;

        while (appState != ONLINE)
        {
            sleep(1);
            if (++s == 10)
            {
                break;
            }
            
            cout<<"APP OFF LINE..."<<endl;
        }
        
        if (appState == ONLINE)
        {
            while (appState == ONLINE)
            {
                sleep(1);
            }
        }
        else
        {
            Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", "capApp OFFLINE");
        }
    }
    catch (Exception ex)
    {
        Logger::getLogger()->logp(&Level::SEVERE, "<void>", "main", ex.toString());
    }
}
