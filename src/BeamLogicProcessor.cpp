/** \file BeamLogicProcessor.cpp
 *
 * Handling of beam logic events
 *
 */

#include <iostream>
#include <string>
#include <vector>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "BeamLogicProcessor.hpp"

using namespace dammIds::logic;

namespace dammIds {
    namespace logic {
        const int D_COUNTER_BEAM = 0;
        const int D_TIME_STOP_LENGTH = 1;
    }
} 

BeamLogicProcessor::BeamLogicProcessor(void) : 
  EventProcessor(OFFSET, RANGE, "beamlogic")
{
    associatedTypes.insert("logic");
}

void BeamLogicProcessor::DeclarePlots(void)
{
    DeclareHistogram1D(D_COUNTER_BEAM, S2, "Beam counter start/stop/trig");
    DeclareHistogram1D(D_TIME_STOP_LENGTH, SA, "Beam stop length (1 s / bin)");
}

bool BeamLogicProcessor::PreProcess(RawEvent &event)
{
    using namespace std;

    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &logicEvents = 
        event.GetSummary("logic", true)->GetList();

    for (vector<ChanEvent*>::const_iterator it = logicEvents.begin();
        it != logicEvents.end(); it++)
    {
        double time   = (*it)->GetTime();	
        string place = (*it)->GetChanID().GetPlaceName();

        // for 2d plot of events 1s / bin
        // Bins in plot
        const unsigned BEAM_START = 0;
        const unsigned BEAM_STOP = 1;
        const unsigned BEAM_TRIG = 2;

        if (place == "logic_start_0") {
            // If beam was stopped, plot the stop length
            if (!TreeCorrelator::get()->place("Beam")->status()) {
                double dt_beam_stop = time - 
                        TreeCorrelator::get()->place(place)->last().time;
                double clockInSeconds = Globals::get()->clockInSeconds();
                const double resolution = 1.0 / clockInSeconds;
                plot(D_TIME_STOP_LENGTH, dt_beam_stop / resolution);
            }
            TreeCorrelator::get()->place("Beam")->activate(time);
            plot(D_COUNTER_BEAM, BEAM_START);
        }
        else if (place == "logic_stop_0") {
            TreeCorrelator::get()->place("Beam")->deactivate(time);
            plot(D_COUNTER_BEAM, BEAM_STOP);
        }
        else if (place == "logic_trigger_0") {
            plot(D_COUNTER_BEAM, BEAM_TRIG);
        }
    }

    return true;
}


bool BeamLogicProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess(); // update processing time
    return true;
}