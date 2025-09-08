#include <sample_shmem_data_provider.hpp>

namespace Nos3
{
    REGISTER_DATA_PROVIDER(SampleShmemDataProvider,"SAMPLE_SHMEM_PROVIDER");

    extern ItcLogger::Logger *sim_logger;

    SampleShmemDataProvider::SampleShmemDataProvider(const boost::property_tree::ptree& config) : SimIDataProvider(config)
    {
        sim_logger->trace("SampleShmemDataProvider::SampleShmemDataProvider:  Constructor executed");
        const std::string shm_name = config.get("simulator.hardware-model.data-provider.shared-memory-name", "Blackboard");
        const size_t shm_size = sizeof(BlackboardData);
        bip::shared_memory_object shm(bip::open_or_create, shm_name.c_str(), bip::read_write);
        shm.truncate(shm_size);
        bip::mapped_region shm_region(shm, bip::read_write);
        _shm_region = std::move(shm_region); // don't let this go out of scope/get destroyed
        _blackboard_data = static_cast<BlackboardData*>(_shm_region.get_address());    
    }

    boost::shared_ptr<SimIDataPoint> SampleShmemDataProvider::get_data_point(void) const
    {
        boost::shared_ptr<SampleDataPoint> dp;
        {
            dp = boost::shared_ptr<SampleDataPoint>(
                new SampleDataPoint(_blackboard_data->svb[0], _blackboard_data->svb[1], _blackboard_data->svb[2]));
        }
        sim_logger->debug("SampleShmemDataProvider::get_data_point: valid=%d, svb=%f/%f/%f", 
            dp->is_sample_data_valid(), dp->get_sample_data_x(), dp->get_sample_data_y(), dp->get_sample_data_z());
        return dp;
    }
}
