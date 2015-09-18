#include "common.h"
#include "scribe_wrapper.cpp"
#include <fstream>

using std::string;
using boost::shared_ptr;

void metrics_writer(std::string filename, unsigned int interval, unsigned int * counter)
{
  while (true) {
      boost::posix_time::seconds workTime(interval);
      boost::this_thread::sleep(workTime);
      std::ofstream metricsFile;
      metricsFile.open (filename.c_str());
      if (metricsFile.fail()) 
	std::cerr << "can't open metrics file: " << filename << std::endl;
      metricsFile << "scribe_stdin.message.count = " << *counter << std::endl;
      metricsFile.close();
    }
}


int main(int argc, char **argv) {
	std::string host;
	unsigned int port;
	unsigned int timeout;
        unsigned int metrics_interval;
	bool debug;
	std::string log;
	std::string category; 
	std::string message;
	std::string metrics;
	unsigned int message_count = 0;

	try {
		boost::program_options::options_description desc("Usage: scribe_stdin [OPTION] message");
		desc.add_options()
			("help", "Print help")
			("host", boost::program_options::value<std::string>(&host)->default_value("127.0.0.1"), "Scribe hostname")
			("port", boost::program_options::value<unsigned int>(&port)->default_value(1463), "Scribe port")
		        ("metrics", boost::program_options::value<std::string>(&metrics)->default_value("/tmp/scribe_stdin_metrics"), "Metrics file")
			("metrics_interval", boost::program_options::value<unsigned int>(&metrics_interval)->default_value(30), "Metrics interval in seconds")
			("timeout", boost::program_options::value<unsigned int>(&timeout)->default_value(30), "timeout")
			("debug", boost::program_options::value<bool>(&debug)->default_value(false), "Enable debug mode")		
			("log", boost::program_options::value<std::string>(&log)->default_value("/tmp/scribewrapper.log"), "Path to scribe log file")		
			("category", boost::program_options::value<std::string>(&category)->default_value("default"), "Scribe category")
		        
		;
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
		
		if(vm.count("help")) {
			std::cout << desc << std::endl;
			return 1;
		}
	} catch(boost::program_options::error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	// start metrics writer thread
	boost::thread workerThread(metrics_writer, metrics, metrics_interval, &message_count);

	scribeWrapper client(host, port, timeout, category, debug, log);
	while(std::cin) {
		getline(std::cin, message);
		boost::algorithm::trim_right(message);
		if(message.empty()) {
			continue;
		}
		if(client.send(message) == -1) {
			std::cerr << "Error! Please check logs" << std::endl;
			return 1;	
		}
		message_count++;
	}
	client.close();
	return 0;
}
