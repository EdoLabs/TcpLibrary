/*
* File:   Device.h
* Author: Ed Alegrid
*
*/

#pragma once

namespace Device {
using namespace std;

class ControlLogic
{
    public:
        ControlLogic() {}
        ~ControlLogic() {}

        // control code operation
        const string processData(string m)
        {
            string data;

            // device1
            if (m == "ON1"){
                return data = "Device1 is ON";
            }
            else if(m == "OFF1"){
                return data = "Device1 is OFF";
            }
            // device2
            else if(m == "ON2"){
                return data = "Device2 is ON";
            }
            else if(m == "OFF2"){
                return data = "Device2 is OFF";
            }
            // device3
            else if (m == "ON3"){
                return data = "Device3 is ON";
            }
            else if(m == "OFF3"){
                return data = "Device3 is OFF";
            }
            // device4
            else if (m == "ON4"){
                return data = "Device4 is ON";
            }
            else if(m == "OFF4"){
                return data = "Device4 is OFF";
            }
            // device5
            else if(m == "ON5"){
                return data = "Device5 is ON";
            }
            else if(m == "OFF5"){
                return data = "Device5 is OFF";
            }
            // device6
            else if(m == "ON6"){
                return data = "Device6 is ON";
            }
            else if(m == "OFF6"){
                return data = "Device6 is OFF";
            }
            else
                return data = "code not recognized";
            }
       };

}
