#include <functional>
#include <iterator>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

bool isNegative(int Num) {
    if (Num == 0) {
        return true;
    }
    else { return false; }
};


class Stream {
public:
    double supplyTemperaure;
    double targetTemperature;
    double cP;
    double heatTransferCoefficient;
    int cold;
    double shiftedSupplyTemperature;
    double shiftedTargetTemperature;

    Stream(double supplyTemperature, double targetTemperature, double cP, double heatTransferCoefficient, int cld) {
        this->supplyTemperaure = supplyTemperature;
        this->targetTemperature = targetTemperature;
        this->cP = cP;
        this->heatTransferCoefficient = heatTransferCoefficient;
        this->cold = cld;
    };

    void shiftTemperatures(double dTmin) {
        if (cold == 1) {
            shiftedSupplyTemperature = supplyTemperaure + dTmin;
            shiftedTargetTemperature = targetTemperature + dTmin;
        }
        else {
            shiftedSupplyTemperature = supplyTemperaure - dTmin;
            shiftedTargetTemperature = targetTemperature - dTmin;
        }
    }
};

class temperatureLevel {
public:
    double lowerBound;
    double upperBound;
    double sumofCP;
    double heatFlow;
    double hTerm;
    vector<Stream*> streamsPresent;

    temperatureLevel(double lowerBound, double upperBound, vector<Stream*> streamsP) {
        this->lowerBound = lowerBound;
        this->upperBound = upperBound;
        this->streamsPresent = streamsP;
    };

    void setCP(double cp) {
        this->sumofCP = cp;
        this->heatFlow = (upperBound - lowerBound) * cp;
    };
};

struct Point {
    float temperature;
    float enthaply;
    float cp;
};

class compositeCurve {
public:
    vector<Point> points;
    int cold;
    vector<Stream*> streams;

    compositeCurve(vector<Stream*> streams, int cold){
        vector<float> temps;
        float tempCP;
        for (Stream* stm: streams){
            if(stm->cold == cold){
            this->streams.push_back(stm);
            temps.push_back(stm->supplyTemperaure);
            temps.push_back(stm->targetTemperature);
            }
            else{}
        };
        sort(temps.begin(),temps.end());
        temps.erase(unique(temps.begin(),temps.end()), temps.end());

        for (int i=0; i < temps.size(); i++){
            tempCP = 0;
            for(Stream* stm: this->streams){
                if(temps[i] <= stm->targetTemperature and temps[i] >= stm->supplyTemperaure){
                    tempCP += stm->cP;   
                }

                else if (temps[i] <= stm->supplyTemperaure and temps[i] >= stm->targetTemperature) {
                    // The hot stream has been identified
                    tempCP += stm->cP;   
                }
            }
            this->points.push_back(Point{temps[i],0,tempCP});
        };
    };

    void shiftCompositeCurve(float H){
        for (auto& pt : this->points){
            pt.enthaply += H;
        };
    }

    // Print all points on CC
    void debug(){
        for(Point p:this->points){
            cout << "--------" << endl;
            cout << "Temp:" << endl;
            cout << p.temperature << endl;
            cout << "Enthalpy:" << endl;
            cout << p.enthaply << endl;
            cout << "CP:" << endl;
            cout << p.cp << endl;
            cout << "--------" << endl;
        };
        cout << "Number of Streams: " << this->streams.size();
    };
};

// Create new class to combine both composite curves

class ProblemTable {
public:
    vector<Stream*> streams;
    double sumofCP;
    double hotUtility;
    double coldUtility;
    double dTmin;
    double pinchTemperature;
    vector<double> temperatures;
    vector<temperatureLevel> temperatureLevels;
    vector<float> heatCascade;
    vector<float> heatFlow;

    ProblemTable(vector<Stream*> stms, double dTmin) {
        streams = stms;
        for (Stream* stm : streams) {
            stm->shiftTemperatures(dTmin / 2);
            temperatures.push_back(stm->shiftedTargetTemperature);
            temperatures.push_back(stm->shiftedSupplyTemperature);
        };

    };
    // Step 2: Construct the temperature levels 
    void step2() {
        sort(temperatures.rbegin(), temperatures.rend());
        temperatures.erase(unique(temperatures.begin(), temperatures.end()), temperatures.end());

        for (int i = 0; i < temperatures.size() - 1; i++) {
            double upperB = temperatures[i];
            double lowerB = temperatures[i+1];

            vector<Stream*> strPresent;

            for (int i = 0; i < this->streams.size(); i++) {
                if (streams[i]->cold == 1) {
                    if (streams[i]->shiftedSupplyTemperature <= lowerB
                        and streams[i]->shiftedTargetTemperature >= upperB) {
                        strPresent.push_back(streams[i]);
                    }
                }

                else {
                    if (streams[i]->shiftedSupplyTemperature >= upperB
                        and streams[i]->shiftedTargetTemperature <= lowerB) {
                        strPresent.push_back(streams[i]);
                    }
                }
            };

            this->temperatureLevels.push_back(temperatureLevel(lowerB, upperB, strPresent));
        };
    };

    // Go through the temperature levels and run calculations for CP and H
    void step3() {
        // Sum up the CPs 
        for (int i = 0; i < this->temperatureLevels.size(); i++) {
            double totalCP = 0;
            for (Stream* stm : temperatureLevels[i].streamsPresent) {
                if (stm->cold == 1) {
                    totalCP -= stm->cP;
                }
                else {
                    totalCP += stm->cP;
                }

            };
            temperatureLevels[i].setCP(totalCP);
            heatFlow.push_back(temperatureLevels[i].heatFlow);
        };

        heatCascade.push_back(0);
        for (int i = 0; i < heatFlow.size(); i++) {
            heatCascade.push_back(heatCascade[i] + heatFlow[i]);
        }

        float minElement = *min_element(heatCascade.begin(), heatCascade.end());

        for (auto& value : heatCascade) {

            value += -minElement;
        }

        auto it = find_if(heatCascade.begin(), heatCascade.end(), isNegative);

        if (it != heatCascade.end()) {
            int index = distance(heatCascade.begin(), it);
            this->pinchTemperature = temperatureLevels[index].upperBound;
        }
        else {
            int index = 0;
            this->pinchTemperature = temperatureLevels[index].lowerBound;
        }
        cout << pinchTemperature << endl;

        for (int i = 0; i < heatCascade.size(); i++) {
            cout << heatCascade[i] << endl;
        }

        this->coldUtility = heatCascade[heatCascade.size()-1];
        this->hotUtility = heatCascade[0];

    };
};



int main() {
    Stream stm1(160, 80, 2.3, 0.5, 0);
    Stream stm2(270, 80, 0.2, 0.6, 0);
    Stream stm3(340, 90, 0.54, 0.5, 0);
    Stream stm4(30, 120, 0.93, 0.8, 1);
    Stream stm5(120, 190, 1.96, 0.7, 1);
    Stream stm6(190, 290, 1.8, 0.1, 1);
    vector<Stream*> streams;
    streams.push_back(&stm1);
    streams.push_back(&stm2);
    streams.push_back(&stm3);
    streams.push_back(&stm4);
    streams.push_back(&stm5);
    streams.push_back(&stm6);
    ProblemTable pt(streams, 10);
    pt.step2();
    pt.step3();
    cout << pt.hotUtility << endl;
    cout << pt.coldUtility << endl;
    compositeCurve hotCC(streams,0);
    compositeCurve coldCC(streams,1);
    coldCC.shiftCompositeCurve(pt.coldUtility);
    return 0;
};

