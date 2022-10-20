#ifndef NOS3_SAMPLE42DATAPROVIDER_HPP
#define NOS3_SAMPLE42DATAPROVIDER_HPP

#include <boost/property_tree/ptree.hpp>

#include <sim_data_42socket_provider.hpp>

namespace Nos3
{
    // vvv This is pretty standard for a 42 data provider (if one is needed for your sim)
    class Sample42DataProvider : public SimData42SocketProvider
    {
    public:
        // Constructors / destructor
        Sample42DataProvider(const boost::property_tree::ptree& config);

        // Accessors
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        // Disallow these
        ~Sample42DataProvider(void) {};
        Sample42DataProvider& operator=(const Sample42DataProvider&) {};

        int16_t _sc;  // Which spacecraft number to parse out of 42 data
    };
}

#endif
