#include <sample_hardware_model.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(SampleHardwareModel,"SAMPLE");

    extern ItcLogger::Logger *sim_logger;

    SampleHardwareModel::SampleHardwareModel(const boost::property_tree::ptree& config) : SimIHardwareModel(config), 
    _enabled(SAMPLE_SIM_SUCCESS), _count(0), _config(0), _status(0)
    {
        /* Get the NOS engine connection string */
        std::string connection_string = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001"); 
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  NOS Engine connection string: %s.", connection_string.c_str());

        /* Get a data provider */
        std::string dp_name = config.get("simulator.hardware-model.data-provider.type", "SAMPLE_PROVIDER");
        _sample_dp = SimDataProviderFactory::Instance().Create(dp_name, config);
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  Data provider %s created.", dp_name.c_str());

        /* Get on a protocol bus */
        /* Note: Initialized defaults in case value not found in config file */
        std::string bus_name = "usart_29";
        int node_port = 29;
        if (config.get_child_optional("simulator.hardware-model.connections")) 
        {
            /* Loop through the connections for hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections"))
            {
                /* v.second is the child tree (v.first is the name of the child) */
                if (v.second.get("type", "").compare("usart") == 0)
                {
                    /* Configuration found */
                    bus_name = v.second.get("bus-name", bus_name);
                    node_port = v.second.get("node-port", node_port);
                    break;
                }
            }
        }
        _uart_connection.reset(new NosEngine::Uart::Uart(_hub, config.get("simulator.name", "sample_sim"), connection_string, bus_name));
        _uart_connection->open(node_port);
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  Now on UART bus name %s, port %d.", bus_name.c_str(), node_port);

        /* Sub-Sim (Sim to Sim) connection*/
        std::string subsim_bus_name = "usart_28";
        int subsim_node_port = 28;
        _uart_subsim.reset(new NosEngine::Uart::Uart(_hub, config.get("simulator.name", "sample_sim"), connection_string, subsim_bus_name));
        _uart_subsim->open(subsim_node_port);
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  Now on UART bus name %s, port %d.", subsim_bus_name.c_str(), subsim_node_port);
    
        /* Configure protocol callback */
        _uart_connection->set_read_callback(std::bind(&SampleHardwareModel::uart_read_callback, this, std::placeholders::_1, std::placeholders::_2));

        /* Get on the command bus*/
        std::string time_bus_name = "command";
        if (config.get_child_optional("hardware-model.connections")) 
        {
            /* Loop through the connections for the hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("hardware-model.connections"))
            {
                /* v.first is the name of the child */
                /* v.second is the child tree */
                if (v.second.get("type", "").compare("time") == 0) // 
                {
                    time_bus_name = v.second.get("bus-name", "command");
                    /* Found it... don't need to go through any more items*/
                    break; 
                }
            }
        }
        _time_bus.reset(new NosEngine::Client::Bus(_hub, connection_string, time_bus_name));
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  Now on time bus named %s.", time_bus_name.c_str());

        /* Construction complete */
        sim_logger->info("SampleHardwareModel::SampleHardwareModel:  Construction complete.");
    }


    SampleHardwareModel::~SampleHardwareModel(void)
    {        
        /* Close the protocol bus */
        _uart_connection->close();
        _uart_subsim->close();

        /* Clean up the data provider */
        delete _sample_dp;
        _sample_dp = nullptr;

        /* The bus will clean up the time node */
    }


    /* Automagically set up by the base class to be called */
    void SampleHardwareModel::command_callback(NosEngine::Common::Message msg)
    {
        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        sim_logger->info("SampleHardwareModel::command_callback:  Received command: %s.", dbf.data);

        /* Do something with the data */
        std::string command = dbf.data;
        std::string response = "SampleHardwareModel::command_callback:  INVALID COMMAND! (Try HELP)";
        boost::to_upper(command);
        if (command.compare("HELP") == 0) 
        {
            response = "SampleHardwareModel::command_callback: Valid commands are HELP, ENABLE, DISABLE, STATUS=X, or STOP";
        }
        else if (command.compare(0,6,"ENABLE") == 0) 
        {
            _enabled = SAMPLE_SIM_SUCCESS;
            response = "SampleHardwareModel::command_callback:  Enabled\n";
        }
        else if (command.compare(0,7,"DISABLE") == 0) 
        {
            _enabled = SAMPLE_SIM_ERROR;
            _count = 0;
            _config = 0;
            _status = 0;
            response = "SampleHardwareModel::command_callback:  Disabled";
        }
        else if (command.substr(0,7).compare("STATUS=") == 0)
        {
            try
            {
                _status = std::stod(command.substr(7));
                response = "SampleHardwareModel::command_callback:  Status set";
            }
            catch (...)
            {
                response = "SampleHardwareModel::command_callback:  Status invalid";
            }            
        }
        else if (command.compare(0,4,"STOP") == 0) 
        {
            _keep_running = false;
            response = "SampleHardwareModel::command_callback:  Stopping";
        }
        /* TODO: Add anything additional commands here */

        /* Send a reply */
        sim_logger->info("SampleHardwareModel::command_callback:  Sending reply: %s", response.c_str());
        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }


    /* Custom function to prepare the Sample HK telemetry */
    void SampleHardwareModel::create_sample_hk(std::vector<uint8_t>& out_data)
    {
        /* Prepare data size */
        out_data.resize(16, 0x00);

        /* Streaming data header - 0xDEAD */
        out_data[0] = 0xDE;
        out_data[1] = 0xAD;
        
        /* Sequence count */
        out_data[2] = (_count >> 24) & 0x000000FF; 
        out_data[3] = (_count >> 16) & 0x000000FF; 
        out_data[4] = (_count >>  8) & 0x000000FF; 
        out_data[5] =  _count & 0x000000FF;
        
        /* Configuration */
        out_data[6] = (_config >> 24) & 0x000000FF; 
        out_data[7] = (_config >> 16) & 0x000000FF; 
        out_data[8] = (_config >>  8) & 0x000000FF; 
        out_data[9] =  _config & 0x000000FF;

        /* Device Status */
        out_data[10] = (_status >> 24) & 0x000000FF; 
        out_data[11] = (_status >> 16) & 0x000000FF; 
        out_data[12] = (_status >>  8) & 0x000000FF; 
        out_data[13] =  _status & 0x000000FF;

        /* Streaming data trailer - 0xBEEF */
        out_data[14] = 0xBE;
        out_data[15] = 0xEF;
    }


    /* Custom function to prepare the Sample Data */
    void SampleHardwareModel::create_sample_data(std::vector<uint8_t>& out_data)
    {
        boost::shared_ptr<SampleDataPoint> data_point = boost::dynamic_pointer_cast<SampleDataPoint>(_sample_dp->get_data_point());

        /* Prepare data size */
        out_data.resize(14, 0x00);

        /* Streaming data header - 0xDEAD */
        out_data[0] = 0xDE;
        out_data[1] = 0xAD;
        
        /* Sequence count */
        out_data[2] = (_count >> 24) & 0x000000FF; 
        out_data[3] = (_count >> 16) & 0x000000FF; 
        out_data[4] = (_count >>  8) & 0x000000FF; 
        out_data[5] =  _count & 0x000000FF;
        
        /* 
        ** Payload 
        ** 
        ** Device is big engian (most significant byte first)
        ** Assuming data is valid regardless of dynamic / environmental data
        ** Floating poing numbers are extremely problematic 
        **   (https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html)
        ** Most hardware transmits some type of unsigned integer (e.g. from an ADC), so that's what we've done
        ** Scale each of the x, y, z (which are in the range [-1.0, 1.0]) by 32767, 
        **   and add 32768 so that the result fits in a uint16
        */
        double dx = data_point->get_sample_data_x();
        double dy = data_point->get_sample_data_y();
        double dz = data_point->get_sample_data_z();
        uint16_t x   = (uint16_t)(dx*32767.0 + 32768.0);
        out_data[6]  = (x >> 8) & 0x00FF;
        out_data[7]  =  x       & 0x00FF;
        uint16_t y   = (uint16_t)(dy*32767.0 + 32768.0);
        out_data[8]  = (y >> 8) & 0x00FF;
        out_data[9]  =  y       & 0x00FF;
        uint16_t z   = (uint16_t)(dz*32767.0 + 32768.0);
        out_data[10] = (z >> 8) & 0x00FF;
        out_data[11] =  z       & 0x00FF;

        sim_logger->debug("SampleHardwareModel::create_sample_data: data_point=%f, %f, %f, converted values=%u, %u, %u.", dx, dy, dz, x, y, z);

        /* Streaming data trailer - 0xBEEF */
        out_data[12] = 0xBE;
        out_data[13] = 0xEF;
    }


    /* Protocol callback */
    void SampleHardwareModel::uart_read_callback(const uint8_t *buf, size_t len)
    {
        std::vector<uint8_t> out_data; 
        std::uint8_t valid = SAMPLE_SIM_SUCCESS;
        
        /* Retrieve data and log in man readable format */
        std::vector<uint8_t> in_data(buf, buf + len);
        
        /* Subsim variables */
        std::vector<uint8_t> subsim_data(buf, buf + len);
        size_t subsim_data_len = 0;
        std::uint8_t subsim_success = SAMPLE_SIM_SUCCESS;

        sim_logger->debug("SampleHardwareModel::uart_read_callback:  REQUEST %s",
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str());

        /* Check simulator is enabled */
        if (_enabled != SAMPLE_SIM_SUCCESS)
        {
            sim_logger->debug("SampleHardwareModel::uart_read_callback:  Sample sim disabled!");
            valid = SAMPLE_SIM_ERROR;
        }
        else
        {
            /* Check if message is incorrect size */
            if (in_data.size() != 9)
            {
                sim_logger->debug("SampleHardwareModel::uart_read_callback:  Invalid command size of %d received!", in_data.size());
                valid = SAMPLE_SIM_ERROR;
            }
            else
            {
                /* Check header - 0xDEAD */
                if ((in_data[0] != 0xDE) || (in_data[1] !=0xAD))
                {
                    sim_logger->debug("SampleHardwareModel::uart_read_callback:  Header incorrect!");
                    valid = SAMPLE_SIM_ERROR;
                }
                else
                {
                    /* Check trailer - 0xBEEF */
                    if ((in_data[7] != 0xBE) || (in_data[8] !=0xEF))
                    {
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  Trailer incorrect!");
                        valid = SAMPLE_SIM_ERROR;
                    }
                    else
                    {
                        /* Increment count as valid command format received */
                        _count++;
                    }
                }
            }

            if (valid == SAMPLE_SIM_SUCCESS)
            {   
                /* Process command */
                switch (in_data[2])
                {
                    case 0:
                        /* NOOP */
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  NOOP command received!");

                        /* Forward command to sub-sim (sim-to-sim) and confirm valid response returned */
                        if (valid == SAMPLE_SIM_SUCCESS)
                        {
                            /* Forward command */
                            subsim_data_len = in_data.size();
                            _uart_subsim->write(&in_data[0], subsim_data_len);

                            /* Read response */
                            _uart_subsim->read(&subsim_data[0], subsim_data_len);
                            
                            /* Confirm command echo */
                            for (size_t i = 0; i < subsim_data_len; i++)
                            {
                                if (in_data[i] != subsim_data[i])
                                {
                                    subsim_success = SAMPLE_SIM_ERROR;
                                }
                            }
                            
                            /* Print status */
                            if (subsim_success == SAMPLE_SIM_SUCCESS)
                            {
                                sim_logger->debug("SampleHardwareModel::uart_read_callback:NOOP  SubSim communication returned success!");
                            }
                            else
                            {
                                sim_logger->debug("SampleHardwareModel::uart_read_callback:NOOP  SubSim communication returned error! What is wrong?");
                            }
                        }
                        break;

                case 1:
                        /* Request HK */
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  Send HK command received!");
                        create_sample_hk(out_data);
                        break;

                    case 2:
                        /* Request data */
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  Send data command received!");
                        create_sample_data(out_data);
                        break;

                    case 3:
                        /* Configuration */
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  Configuration command received!");
                        _config  = in_data[3] << 24;
                        _config |= in_data[4] << 16;
                        _config |= in_data[5] << 8;
                        _config |= in_data[6];
                        break;
                    
                    default:
                        /* Unused command code */
                        valid = SAMPLE_SIM_ERROR;
                        sim_logger->debug("SampleHardwareModel::uart_read_callback:  Unused command %d received!", in_data[2]);
                        break;
                }
            }
        }

        /* Increment count and echo command since format valid */
        if (valid == SAMPLE_SIM_SUCCESS)
        {
            _count++;
            _uart_connection->write(&in_data[0], in_data.size());

            /* Send response if existing */
            if (out_data.size() > 0)
            {
                sim_logger->debug("SampleHardwareModel::uart_read_callback:  REPLY %s",
                    SimIHardwareModel::uint8_vector_to_hex_string(out_data).c_str());
                _uart_connection->write(&out_data[0], out_data.size());
            }
        }
    }
}
