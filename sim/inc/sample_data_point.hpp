#ifndef NOS3_SAMPLEDATAPOINT_HPP
#define NOS3_SAMPLEDATAPOINT_HPP

#include <boost/shared_ptr.hpp>

#include <sim_42data_point.hpp>

namespace Nos3
{
    // vvv This is pretty standard for a data point (if one is needed to transfer data between a data provider and a hardware model for your sim)
    class SampleDataPoint : public SimIDataPoint
    {
    public:
        // Constructor
        SampleDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        // Accessors
        std::string to_string(void) const;
        // vvv These provide the hardware model a way to get the specific data out of the data point that it will need to send
        // out bytes over its peripheral bus
        double      get_sample_data_x(void) const {return _sample_data[0];}
        double      get_sample_data_y(void) const {return _sample_data[1];}
        double      get_sample_data_z(void) const {return _sample_data[2];}
        bool        is_sample_data_valid(void) const {return _sample_data_is_valid;}
    
    private:
        // Disallow the big 4
        SampleDataPoint(void) {};
        SampleDataPoint(const SampleDataPoint&) {};
        SampleDataPoint& operator=(const SampleDataPoint&) {};
        ~SampleDataPoint(void) {};

        // vvv This would be the specific data you need to get from the data provider to the hardware model so that
        // the hardware model can send out bytes over its peripheral bus... but you only get to this data through the accessors above
        mutable bool   _sample_data_is_valid;
        mutable double _sample_data[3];
    };
}

#endif
