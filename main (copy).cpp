#include <functional>
#include <iterator>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

bool isNegative(int Num){
    return Num > 0;
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

        Stream(double supplyTemperature, double targetTemperature, double cP, double heatTransferCoefficient, int cld){
            this ->supplyTemperaure = supplyTemperature;
            this ->targetTemperature = targetTemperature;
            this ->cP = cP;
            this ->heatTransferCoefficient = heatTransferCoefficient;
            this ->cold = cld;
        };

        void shiftTemperatures(double dTmin){
            if(cold == 1){
                shiftedSupplyTemperature = supplyTemperaure + dTmin;
                shiftedTargetTemperature = targetTemperature + dTmin;
            }
            else {
                shiftedSupplyTemperature = supplyTemperaure - dTmin;
                shiftedTargetTemperature = targetTemperature - dTmin;
            }
        }
};

class temperatureLevel{
public:
    double lowerBound;
    double upperBound;
    double sumofCP;
    double heatFlow;
    double hTerm;
    vector<Stream*> streamsPresent;

    temperatureLevel(double lowerBound, double upperBound, vector<Stream*> streamsP){
        this->lowerBound = lowerBound;
        this->upperBound = upperBound;
        this->streamsPresent = streamsP;
    };

    void setCP(double cp){
        this->sumofCP = cp;
        this->heatFlow = (upperBound - lowerBound) * cp;
    };
};

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
        vector<double> heatCascade;

    ProblemTable(vector<Stream*> stms, double dTmin){
        streams = stms;
        for(Stream* stm : streams){
            stm -> shiftTemperatures(dTmin / 2);
            cout << "shifted supply to" << endl;
            cout << stm->shiftedSupplyTemperature << endl;
            temperatures.push_back(stm ->shiftedTargetTemperature);
            temperatures.push_back(stm ->shiftedSupplyTemperature);
        };

    };
    // Step 2: Construct the temperature levels 
    void step2() {
       sort(temperatures.begin(),temperatures.end(), greater_equal<int>());
       temperatures.erase(unique(temperatures.begin(),temperatures.end()), temperatures.end());
    
       for (int i=1;i<temperatures.size()-1;i++){
            double upperB = temperatures[i];
            double lowerB = temperatures[i+1];
            vector<Stream*> strPresent;
            for(Stream* stm : streams){

                if(stm ->cold == 1){
                    if(stm ->shiftedSupplyTemperature <= lowerB 
                            and stm ->shiftedTargetTemperature >= upperB){
                        strPresent.push_back(stm);
                    }
                }

                else{
                    if(stm ->shiftedSupplyTemperature >= upperB 
                            and stm ->shiftedTargetTemperature <=  lowerB){
                        strPresent.push_back(stm);
                    }
                }
            };

            this->temperatureLevels.push_back(temperatureLevel(lowerB,upperB,strPresent));
        };
    };

    // Go through the temperature levels and run calculations for CP and H
    void step3(){
        // Sum up the CPs 
        for (int i=0; i<this->temperatureLevels.size();i++){
            double totalCP = 0; 
            for(Stream* stm : temperatureLevels[i].streamsPresent){
                if(stm->cold == 1){
                    cout << stm->cP;
                    totalCP -= stm->cP;
                }
                else{
                    totalCP += stm->cP;
                }
            };
        temperatureLevels[i].setCP(totalCP);
        heatCascade.push_back(temperatureLevels[i].heatFlow);
        };

        auto it = find_if(heatCascade.begin(),heatCascade.end(), isNegative);

        if (it != heatCascade.end()){
            int index = distance(heatCascade.begin(), it);
            this->pinchTemperature = temperatureLevels[index].lowerBound;
        }
        else{ 
            int index = 0;
            this->pinchTemperature = temperatureLevels[index].lowerBound;
        }
       cout << pinchTemperature << endl; 
       for (int i = 0; i < heatCascade.size();i++) {
            cout << heatCascade[i] << endl;
       }
    };
};

int main(){
    Stream stm1(160,80,2.3,0.5,0);
    Stream stm2(270,80,0.2,0.6,0);
    Stream stm3(340,90,0.54,0.5,0);
    Stream stm4(30,120,0.93,0.8,1);
    Stream stm5(120,190,1096,0.7,1);
    Stream stm6(190,290,1.8,0.1,1);
    vector<Stream*> streams;
    streams.push_back(& stm1);
    streams.push_back(& stm2);
    streams.push_back(& stm3);
    streams.push_back(& stm4);
    streams.push_back(& stm5);
    streams.push_back(& stm6);
    ProblemTable pt(streams,10);
    pt.step2();
    pt.step3();
    return 0;
};

