#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace sortix {
	enum class mode {
		numbers,
		strings,
	};

	enum class direction {
		reversed = 0,
		straight = 1
	};

	struct SortPass {
		int field = 0;
		int position = 0;
		direction direction = direction::straight;
	};
	struct Config {
		bool isOk = true;
		mode mode = mode::strings;
		direction direction = direction::straight;
		char separator = ' ';
		fs::path outputFile = "";
		std::vector<fs::path> inputFiles = std::vector<fs::path>();
		std::vector<SortPass> sortPasses = std::vector<SortPass>();
	};

	

	class Sort {
	public:
		Sort(int argc, char* argv[]) {
			setConfig(argc, argv, config);
		}
		bool isCongigOk() const { return config.isOk; }

		void sort() {
			auto& passes = config.sortPasses;
			int i = 0;
			if (config.mode == mode::numbers) {
				std::vector<int> data;
				std::ifstream input;
				if (!config.inputFiles.size()) {
					input.copyfmt(std::cin);
					input.clear(std::cin.rdstate());
					input.basic_ios<char>::rdbuf(std::cin.rdbuf());
					processIfstreamNumbers(data, input);
					input.close();
				}
				else {
					for (auto& path : config.inputFiles) {
						input.open(path);
						processIfstreamNumbers(data, input);
						input.close();
					}
				}
				std::sort(data.begin(), data.end());
				writeNumbersToDest(data);
				return;
			}
			if (config.mode == mode::strings) {
				std::vector<std::vector<std::string>> data;
				std::ifstream input;
				if (!config.inputFiles.size()) {
					input.copyfmt(std::cin);
					input.clear(std::cin.rdstate());
					input.basic_ios<char>::rdbuf(std::cin.rdbuf());
					processIfstreamLines(data, input);
					input.close();
				}
				else {
					for (auto& path : config.inputFiles) {
						input.open(path);
						processIfstreamLines(data, input);
						input.close();
					}
				}
				std::sort(data.begin(), data.end(), [&passes](auto& a, auto& b) {
					if (passes.size())
					for (auto& pass : passes) {
						int field = (pass.field >= a.size() || pass.field >= b.size()) ? 0 : pass.field;
						const auto& wa = a[field], wb = b[field];
						int pos = (pass.position >= wa.size() || pass.position >= wb.size()) ? 0 : pass.position;
						if (wa[pos] == wb[pos]) continue;
						return (bool)((bool)pass.direction ^ (wa[pos] < wb[pos]));
					}
					return true;					
				});
				writeLinesToDest(data);
				return;
			}			
		}

	private:
		Config config;
		void setConfig(int argc, char* argv[], sortix::Config& config) {
			if (argc > 32) {
				config.isOk = false;
				std::cout << "Too many args!\n";
				return;
			}
			int i = 1;
			while (i < argc) {
				char* arg = argv[i];				
				if (arg == "-n") {
					config.mode = mode::numbers;
					i++;
					continue;
				}
				if (arg == "-r") {
					config.direction = direction::reversed;
					i++;
					continue;
				}
				if (arg == "-o") {
					char* out = argv[i];
					config.outputFile = out;
					i++;
					continue;
				}
				if (arg == "-t") {
					i++;
					char separator = argv[i][0];
					config.separator = separator;
					i++;
					continue;
				}

				if (fs::exists(arg)) {
					config.inputFiles.push_back(arg);
					i++;
					continue;
				}
				else if (arg[0] == '+' || arg[0] == '-') {
					SortPass pass;
					if (arg[0] == '-') pass.direction = direction::reversed;
					std::string param = arg;
					param.substr(1, param.size() - 1);					
					if (int point = param.find('.')) {
						int m = std::stoi(param.substr(0, point));
						int n = std::stoi(param.substr(point + 1, param.size() - point));
						pass.field = m;
						pass.position = n;
					}
					else {
						pass.field = std::stoi(param);
					}
					config.sortPasses.push_back(std::move(pass));
				}
				else {
					config.isOk = false;
					std::cout << "Unknown param: " << arg << '\n';
						break;
				}				
			}
			//default configuration
			if (config.isOk && !config.sortPasses.size()) {
				SortPass pass;
				config.sortPasses.push_back(pass);
			}
			
		}	
		void processIfstreamNumbers(std::vector<int>& data, std::ifstream& input) {			
			std::string tmp;
			while (std::getline(std::cin, tmp, config.separator)) data.push_back(std::stoi(tmp));			
		}
		void processIfstreamLines(std::vector<std::vector<std::string>>& data, std::ifstream& input) {
			std::string tmp;
			while (std::getline(std::cin, tmp, '\n')) {
				std::vector<std::string> words;
				std::stringstream ss(tmp);
				std::string word;
				while (std::getline(ss, word, config.separator)) words.push_back(word);
				data.push_back(std::move(words));
			}
		}
		void writeNumbersToDest(std::vector<int>& data) {
			std::ofstream out;
			if (config.outputFile != "") {
				out.open(config.outputFile);
			}
			else {
				out.copyfmt(std::cout);
				out.clear(std::cout.rdstate());
				out.basic_ios<char>::rdbuf(std::cout.rdbuf());
			}

			if (config.direction == direction::straight) {
				auto start = data.begin();
				auto end = data.end();
				while (start != end) {
					out << (*start);
					start++;
				}
			}
			else {
				auto start = data.rbegin();
				auto end = data.rend();
				while (start != end) {
					out << (*start);
					start++;
				}
			}
			out.close();
		}
		void writeLinesToDest(std::vector<std::vector<std::string>>& data) {
			std::ofstream out;
			if (config.outputFile != "") {
				out.open(config.outputFile);
			}
			else {
				out.copyfmt(std::cout);
				out.clear(std::cout.rdstate());
				out.basic_ios<char>::rdbuf(std::cout.rdbuf());
			}

			if (config.direction == direction::straight) {
				auto start = data.begin();
				auto end = data.end();
				while (start != end) {
					for (auto& word : *start)
						out << word << config.separator;
					out << '\n';
					start++;
				}
			}
			else {
				auto start = data.rbegin();
				auto end = data.rend();
				while (start != end) {
					for (auto& word : *start)
						out << word << config.separator;
					out << '\n';
					start++;
				}
			}
			out.close();
		}
	};

}
