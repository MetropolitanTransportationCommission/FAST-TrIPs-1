/*-------------------------------------------------------
FAST-TrIPs: Flexible Assignment and Simulation Tool for Transit and Intermodal Passengers
Copyright 2014 Alireza Khani and Mark Hickman
Licensed under the Apache License, Version 2.0
-------------------------------------------------------
Code primarily written by Alireza Khani
Under supervision of Mark Hickman

Contact:
    Alireza Khani:  akhani@utexas.edu or akhani@email.arizona.edu
    Mark Hickman:   m.hickman1@uq.edu.au
-------------------------------------------------------
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-------------------------------------------------------*/

using namespace std;

int		initializeSimulatoin(){
	int									tmpInt, noOfPassengers, tmpPassengerStatus;
	string								fileStr, tmpPath;
	list<stop*>::iterator				tmpStopListIter;
	list<trip*>::iterator				tmpTripListIter;
	list<passenger*>::iterator			tmpPassengerListIter;
	passenger*							tmpPassengerPntr;

	passengers2transfer.clear();
	for(tmpStopListIter=stopList.begin();tmpStopListIter!=stopList.end();tmpStopListIter++){
		(*tmpStopListIter)->resetStopForSimulation();
	}
	for(tmpTripListIter=tripList.begin();tmpTripListIter!=tripList.end();tmpTripListIter++){
		(*tmpTripListIter)->resetTripForSimulation();
	}
	noOfPassengers = 0;
	for(tmpPassengerListIter=passengerList.begin();tmpPassengerListIter!=passengerList.end();tmpPassengerListIter++){
		tmpPassengerPntr = (*tmpPassengerListIter);
		tmpPassengerStatus = tmpPassengerPntr->getPassengerStatus();
		tmpPath = tmpPassengerPntr->getAssignedPath();
        if(tmpPath!=""){
            tmpPassengerPntr->resetPathInfo();
			tmpInt = tmpPassengerPntr->initializePath();
			tmpPassengerPntr->setPassengerStatus(0);
			noOfPassengers++;
		}
	}

	//availableCapacity.clear();            //May result in passengers flip-floping if clear the available capacities!
                                            //Will converge otherwise, but some shorter paths may become available in larger iterations although bot being used, this is similar to Hyunsoo's example!
    return noOfPassengers;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		createPassengers(int _t1, int _t2){
	string								buf, tmpPassengerId, tmpVehicleId;
	int									tmpPassengerStatus, tmpStartTime, noOfPassengerCreated;
	list<passenger*>::iterator          tmpPassengerListIter;
	passenger*							passengerPntr;

	noOfPassengerCreated = 0;
    for(tmpPassengerListIter=passengerList.begin();tmpPassengerListIter!=passengerList.end();tmpPassengerListIter++){
		passengerPntr = *tmpPassengerListIter;
        tmpPassengerStatus = passengerPntr->getPassengerStatus();
		tmpStartTime = 60 * passengerPntr->getStartTime();
		if (tmpPassengerStatus==0 && tmpStartTime >= _t1 && tmpStartTime < _t2){
			passengers2transfer.push_back(passengerPntr);
			passengerPntr->setPassengerStatus(1);	//walking
			noOfPassengerCreated++;
		}
	}
	return noOfPassengerCreated;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		transferPassengers(int _t){
	string							tmpIn, tmpPassengerId, tmpBoardingStop, tmpNextTrip, tmpExperiencedPath;
	int								tmpPassCntr, tmpNoOfPassengers, tmpAlightingTime, tmpWalkingTime,
									tmpMissingCase, tmpNumPassengersArrived, tmpNumPassengersRemoved;
	list<passenger*>::iterator		tmpPassenger2transferIter;
	passenger*						passengerPntr;

	tmpNoOfPassengers = passengers2transfer.size();
	tmpPassenger2transferIter = passengers2transfer.begin();
	tmpNumPassengersArrived = 0;
	tmpNumPassengersRemoved = 0;
	tmpMissingCase = 0;

	for(tmpPassCntr=1;tmpPassCntr<=tmpNoOfPassengers;tmpPassCntr++){
		passengerPntr = NULL;
		passengerPntr = *tmpPassenger2transferIter;
		tmpPassengerId = passengerPntr->getPassengerId();

		tmpBoardingStop = passengerPntr->getCurrentBoardingStopId();
		tmpAlightingTime = 60 * passengerPntr->getAlightingTime();
		tmpWalkingTime = 60 * passengerPntr->getCurrentWalkingTime();

		if(_t >= tmpAlightingTime + tmpWalkingTime){
			if(tmpBoardingStop=="-101"){
				passengers2transfer.remove(passengerPntr);
				passengerPntr->setPassengerStatus(5);	//arrived
				passengerPntr->setEndTime((tmpAlightingTime + tmpWalkingTime)/60.0);
				//cout <<"\ttime: "<<_t/60.0<<" Passenger "<<tmpPassengerId<<" arrieved at destination."<<endl;
				tmpNumPassengersRemoved++;
				tmpNumPassengersArrived++;
			}else{
				stopSet[tmpBoardingStop]->addPassenger(tmpPassengerId);
				passengers2transfer.remove(passengerPntr);
				passengerPntr->addArrivalTime((tmpAlightingTime + tmpWalkingTime)/60.0);
				passengerPntr->setPassengerStatus(2);	//waiting
				//cout <<"\ttime: "<<_t/60.0<<" Passenger "<<tmpPassengerId<<" was transfered to stop "<<tmpBoardingStop<<endl;

				tmpNumPassengersRemoved++;
			}
			tmpPassenger2transferIter = passengers2transfer.begin();
			advance(tmpPassenger2transferIter,tmpPassCntr-tmpNumPassengersRemoved);
		}else{
			tmpPassenger2transferIter++;
		}
	}
	return	tmpNumPassengersArrived;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		alightPassengers(int _t, string _tripId, string _stopId){

	string			fileStr, tmpPassengerId, tmpTripId;
	int				tmpPassCntr, tmpNoOfOnBoardPassengers, tmpNoOfAlightings;
	stop*			tmpStopPntr;
	trip*			tmpTripPntr;
	passenger*		tmpPassengerPntr;

	tmpTripPntr = tripSet[_tripId];
	tmpStopPntr = stopSet[_stopId];
	tmpNoOfAlightings = 0;

	tmpNoOfOnBoardPassengers = tmpTripPntr->getNoOfOnBoardPassengers();
	if(tmpNoOfOnBoardPassengers > 0){
		for(tmpPassCntr=1;tmpPassCntr<=tmpNoOfOnBoardPassengers;tmpPassCntr++){
			tmpPassengerId = tmpTripPntr->getPassengerId(tmpPassCntr-1);
			tmpPassengerPntr = passengerSet[tmpPassengerId];
			if(tmpPassengerPntr->getCurrentAlightingStopId()==_stopId){
				//cout<<"\ttime: "<<_t/60.0<<" passenger "<<tmpPassengerId<<" got off at stop "<<_stopId<<"."<<endl;
				tmpPassengerPntr->setPassengerStatus(1);	//walking
				tmpPassengerPntr->increasePathIndex();
				tmpPassengerPntr->addAlightingTime(_t/60.0);
				passengers2transfer.push_back(tmpPassengerPntr);
				tmpNoOfAlightings = tmpNoOfAlightings + 1;
				tmpTripPntr->removePassenger(tmpPassCntr-1);
				tmpNoOfOnBoardPassengers = tmpTripPntr->getNoOfOnBoardPassengers();
				tmpPassCntr = tmpPassCntr - 1;
			}
		}
	}
	tmpTripPntr->setAlightings(tmpNoOfAlightings);
	return tmpNoOfAlightings;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		boardPassengers(int _t, string _tripId, string _stopId){

	string			tmpPassengerId, tmpTripId;
	int				tmpPassCntr, tmpFreeCapacity, tmpNoOfOnBoardPassengers, tmpNoOfWaitingPassengers,
					tmpNoOfBoardings, tmpNoOfMissings, tmpMissingCase;

	//For Available Capacity Definition
	string			fromToAt;
    string          fromToAt2;
    float			tmpArrivaltime, tmpLatestArrivalTime;

	stop*			tmpStopPntr;
	trip*			tmpTripPntr;
	passenger*		tmpPassengerPntr;

	tmpTripPntr = tripSet[_tripId];
	tmpStopPntr = stopSet[_stopId];
	tmpNoOfBoardings = 0;
	tmpNoOfMissings = 0;

	tmpNoOfWaitingPassengers = tmpStopPntr->getNoOfWaitingPassenegrs();
	if(tmpNoOfWaitingPassengers > 0){

		tmpTripId = tmpTripPntr->getTripId();
		tmpFreeCapacity = tmpTripPntr->getFreeCapacity();
		for(tmpPassCntr=1;tmpPassCntr<=tmpNoOfWaitingPassengers;tmpPassCntr++){
			tmpPassengerId = tmpStopPntr->getPassengerId(tmpPassCntr-1);
			tmpPassengerPntr = passengerSet[tmpPassengerId];
			if(tmpPassengerPntr->getCurrentTripId() == _tripId){
				if(tmpFreeCapacity==0){
					tmpMissingCase = 4;		//No space on the bus for the passenger to board!
					tmpPassengerPntr->setPassengerStatus(4);	//missed
					//cout <<"\tPassenger "<<tmpPassengerId<<" missed bus #"<<_tripId<<" at stop "<<_stopId<<" at time "<<_t/60.0<<"\tMC: "<<tmpMissingCase<<endl;
					tmpNoOfMissings = tmpNoOfMissings + 1;

					fromToAt = tmpTripId + "," + _stopId;
                    tmpArrivaltime = tmpPassengerPntr->getArrivalTime();
					if(availableCapacity.find(fromToAt)==availableCapacity.end()){
						availableCapacity[fromToAt] = tmpArrivaltime;
					}else{
						tmpLatestArrivalTime = availableCapacity[fromToAt];
						if(tmpArrivaltime < tmpLatestArrivalTime){
							availableCapacity[fromToAt] = tmpArrivaltime;
						}
					}
                    
                    fromToAt2 = tmpPassengerPntr->getLastTripId();
                    if(fromToAt2=="Access"){
                        fromToAt2 = fromToAt2 + "," + tmpTripId + "," + _stopId;
                    }else{
                        fromToAt2 = fromToAt2 + "," + tmpPassengerPntr->getLastAlightingStop();
                        fromToAt2 = fromToAt2 + "," + tmpTripId + "," + _stopId;
                    }
                    if(availableCapacity2.find(fromToAt2)!=availableCapacity2.end()){
                        availableCapacity2[fromToAt2]=0;
                    }
					
				}else{
					tmpTripPntr->addPassenger(tmpPassengerId);
					tmpPassengerPntr->addBoardingTime(_t/60.0);
					tmpPassengerPntr->setPassengerStatus(3);	//on board
					//cout <<"\ttime: "<<_t/60.0<<"  Passenger "<<tmpPassengerId<<" got on the vehicle #"<<_tripId<<"."<<endl;
					tmpNoOfBoardings = tmpNoOfBoardings + 1;
					tmpFreeCapacity = tmpFreeCapacity - 1;
				}
				tmpStopPntr->removePassenger(tmpPassCntr-1);
				tmpNoOfWaitingPassengers = tmpNoOfWaitingPassengers - 1;
				tmpPassCntr = tmpPassCntr - 1;
			}
		}
	}
	tmpTripPntr->setResidualCapacity(-1);
	tmpTripPntr->setBoardings(tmpNoOfBoardings);
	tmpNoOfOnBoardPassengers = tmpTripPntr->getNoOfOnBoardPassengers();
	tmpTripPntr->setOnBoards(tmpNoOfOnBoardPassengers);
	return	tmpNoOfMissings;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		calculateDwellTime(int _t, string _tripId, string _stopId){

	string			tmpTripId;
	int				tmpNoOfBoardings, tmpNoOfAlightings, tmpDwellTime, tmpRouteType;
	stop*			tmpStopPntr;
	trip*			tmpTripPntr;

	tmpTripPntr = tripSet[_tripId];
	tmpStopPntr = stopSet[_stopId];
	tmpNoOfBoardings = tmpTripPntr->getNoOfBoardings();
	tmpNoOfAlightings = tmpTripPntr->getNoOfAlightings();

	tmpRouteType = tmpTripPntr->getRouteType();
	if(tmpRouteType==0){
		tmpDwellTime = 30;
	}else{
		if(tmpNoOfBoardings>0 || tmpNoOfAlightings>0){
			tmpDwellTime = 4 + max(4*tmpNoOfBoardings,2*tmpNoOfAlightings);
		}else{
			tmpDwellTime = 0;
		}
	}
	if(tmpDwellTime<0){
		cout <<"Error - Negative Dwell Time!"<<endl;
	}
	tmpTripPntr->setDwellTime(tmpDwellTime);
	tmpTripPntr->increaseStopIndex();
	return	tmpDwellTime;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		simulation(){
	int					counter, maxEvent, tmpOut, tmpEventTime, numAssignedPassengers, numArrivedPassengers, numMissedPassengers;
	string				buf, tmpEventStr, tmpEventTrip, tmpEventStop, tmpEventType;
    double              startTime, endTime, cpuTime;
    char                chr[99];
	vector<string>		tokens;
	trip*				tripPntr;

	numAssignedPassengers = initializeSimulatoin();
	numArrivedPassengers = 0;
	numMissedPassengers = 0;

    startTime = clock()*1.0/CLOCKS_PER_SEC;
    tmpOut = createPassengers(0, 24*3600);
    maxEvent = eventList.size();
    for(counter=0;counter<maxEvent;counter++){
        if (counter>0 && counter%10000==0){
            endTime = clock()*1.0/CLOCKS_PER_SEC;
            cpuTime = round(100 * (endTime - startTime))/100.0;
            cout <<counter<<" / "<<maxEvent<<" events simulated ; "<<"time elapsed: "<<cpuTime<<"\tseconds"<<endl;
        }
        tmpEventStr = eventList.front();
        stringstream ss(tmpEventStr);
        buf.clear();
        tokens.clear();
        while (getline(ss, buf, ',')){
            tokens.push_back(buf);
        }
        tmpEventTime = atoi(tokens[0].c_str());
        tmpEventType = tokens[1];
        tmpEventTrip = tokens[2];
        tmpEventStop = tokens[3];
        tripPntr = tripSet[tmpEventTrip];
        if(tmpEventType=="a"){
            alightPassengers(tmpEventTime, tmpEventTrip, tmpEventStop);
        }else if(tmpEventType=="d"){
            if(passengers2transfer.size()>0){
                tmpOut = transferPassengers(tmpEventTime);
                numArrivedPassengers = numArrivedPassengers + tmpOut;
            }
            tmpOut = boardPassengers(tmpEventTime, tmpEventTrip, tmpEventStop);
            numMissedPassengers = numMissedPassengers + tmpOut;
            calculateDwellTime(tmpEventTime, tmpEventTrip, tmpEventStop);
        }
        eventList.pop_front();
        eventList.push_back(tmpEventStr);
    }

    // print out available capacity
    map<string,double>::iterator availCapIter;
    cout << "availableCapacity ------------------" << endl;
    for (availCapIter=availableCapacity.begin(); availCapIter!=availableCapacity.end(); availCapIter++) {
    	cout << availCapIter->first << " -> " << availCapIter->second << endl;
    }

    // print out available capacity 2
    map<string,int>::iterator availCap2Iter;
    cout << "availableCapacity2 ------------------" << endl;
    for (availCap2Iter=availableCapacity2.begin(); availCap2Iter!=availableCapacity2.end(); availCap2Iter++) {
        cout << availCap2Iter->first << " -> " << availCap2Iter->second << endl;
    }

    endTime = clock()*1.0/CLOCKS_PER_SEC;
    cpuTime = round(100 * (endTime - startTime))/100.0;
    cout <<counter<<" / "<<maxEvent<<" events simulated ; "<<"time elapsed: "<<cpuTime<<"\tseconds"<<endl;
    return	numArrivedPassengers;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		printLoadProfile(){
	int									i, tmpMaxSeq, tmpNumStopTimes, tmpDwellTime, tmpVehLoad, tmpBoardings, tmpAlightings;
	double								tmpDepartureTime, tmpHeadway;
	string								tmpOut, tmpRouteId, tmpShapeId, tmpTripId, tmpDirection, tmpStopId;
	list<trip*>::iterator				tmpTripListIter;
	trip*								tmpTripPntr;

	ofstream outFile1;
	outFile1.open("ft_output_loadProfile.dat");
	outFile1 <<"routeId\tshapeId\ttripId\tdirection\tstopId\ttraveledDist\tdepartureTime\theadway\tdwellTime\tboardings\talightings\tload"<<endl;

	tmpNumStopTimes = 0;
	for(tmpTripListIter=tripList.begin();tmpTripListIter!=tripList.end();tmpTripListIter++){
		tmpTripPntr = (*tmpTripListIter);
		tmpMaxSeq = tmpTripPntr->getMaxSequence();
		tmpRouteId = tmpTripPntr->getRouteId();
		tmpShapeId = tmpTripPntr->getShapeId();
		tmpTripId = tmpTripPntr->getTripId();
		tmpDirection = tmpTripPntr->getDirection();
		for(i=1;i<=tmpMaxSeq;i++){
			tmpStopId = tmpTripPntr->getStop(i);
			tmpDepartureTime = tmpTripPntr->getSchDeparture(i);
			tmpHeadway = tmpTripPntr->getSchHeadway(i);
			tmpDwellTime = tmpTripPntr->getDwellTime(i);
			tmpBoardings = tmpTripPntr->getBoardings(i);
			tmpAlightings = tmpTripPntr->getAlightings(i);
			tmpVehLoad = tmpTripPntr->getVehLoad(i);
            outFile1 <<tmpRouteId.substr(1,99)<<"\t"<<tmpShapeId<<"\t"<<tmpTripId.substr(1,99)<<"\t"<<tmpDirection<<"\t"
                    <<tmpStopId.substr(1,99)<<"\t-1\t"<<tmpDepartureTime<<"\t"<<tmpHeadway<<"\t"<<tmpDwellTime<<"\t"<<tmpBoardings<<"\t"<<tmpAlightings<<"\t"<<tmpVehLoad<<endl;
            tmpNumStopTimes++;
            }
	}
	outFile1.close();
	return tmpNumStopTimes;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		printPassengerPaths(){
	//cout <<"***************************** PRINTING PATHS *****************************"<<endl;
	int									noOfPassengers, tmpMode, tmpStatus;
	double								tmpPAT;
	string								fileStr, tmpPassengerId, tmpOriginTaz, tmpDestinationTaz, tmpPath;
	list<passenger*>::iterator        	tmpPassengerListIter;
	passenger*							passengerPntr;

	noOfPassengers = 0;
	ofstream	outFile;
	outFile.open("ft_output_passengerPaths.dat");
	outFile <<"passengerId\tmode\toriginTaz\tdestinationTaz\tstartTime\tboardingStops\tboardingTrips\talightingStops\twalkingTimes"<<endl;
	for(tmpPassengerListIter=passengerList.begin();tmpPassengerListIter!=passengerList.end();tmpPassengerListIter++){
		passengerPntr = NULL;
		passengerPntr = *tmpPassengerListIter;
		tmpPassengerId = passengerPntr->getPassengerId();
		tmpMode = passengerPntr->getMode();
		tmpOriginTaz = passengerPntr->getOriginTAZ();
		tmpDestinationTaz = passengerPntr->getDestinationTAZ();
		tmpPAT = passengerPntr->getPAT();
		tmpPath = passengerPntr->getAssignedPath();
		tmpStatus = passengerPntr->getPassengerStatus();
		if(tmpPath!=""){
			outFile <<tmpPassengerId.substr(1,99)<<"\t"<<tmpMode<<"\t"<<tmpOriginTaz.substr(1,99)<<"\t"<<tmpDestinationTaz.substr(1,99)<<"\t"<<tmpPath<<endl;
			noOfPassengers++;
		}
	}
	outFile.close();
	return noOfPassengers;
}
//////////////////////////////////////////////////////////////////////////////////////////////
int		printPassengerTimes(){
	int									noOfPassengers, tmpStatus;
	double								tmpTravelCost;
	string								tmpExperiencedPath;
	list<passenger*>::iterator			tmpPassengerListIter;
	passenger*							tmpPassengerPntr;

	ofstream outFile2;
	outFile2.open("ft_output_passengerTimes.dat");
	outFile2 <<"passengerId\tmode\toriginTaz\tdestinationTaz\tstartTime\tendTime\tarrivalTimes\tboardingTimes\talightingTimes\ttravelCost"<<endl;

	noOfPassengers = 0;
	for(tmpPassengerListIter=passengerList.begin();tmpPassengerListIter!=passengerList.end();tmpPassengerListIter++){
		tmpPassengerPntr = NULL;
		tmpPassengerPntr = *tmpPassengerListIter;
		tmpStatus = tmpPassengerPntr->getPassengerStatus();
		if(tmpStatus==5){
			tmpExperiencedPath = tmpPassengerPntr->getExperiencedPath();
            tmpPassengerPntr->calculateExperiencedCost();
			tmpTravelCost = tmpPassengerPntr->getExperiencedCost();
			outFile2 <<tmpExperiencedPath<<"\t"<<floor(100*tmpTravelCost)/100.0<<endl;
			noOfPassengers++;
		}
	}
	outFile2.close();
	return noOfPassengers;
}
//////////////////////////////////////////////////////////////////////////////////////////////
